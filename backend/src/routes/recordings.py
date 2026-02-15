"""
Recordings Routes - WebSocket audio streaming from devices + REST management.

Protocol (device <-> backend via Socket.IO):
  1. Device emits 'auth'        { key: "ABC123" }
     -> server replies 'auth_ok' or 'auth_error'
  2. Device emits 'rec_start'   { title?: "Lecture" }
     -> server replies 'rec_started' { recording_id }
  3. Device emits 'audio_data'  <raw bytes>   (binary frames of 16-bit PCM, 16 kHz mono)
     -> server appends to WAV file on disk
  4. Device emits 'rec_stop'    {}
     -> server finalises WAV header, runs transcription pipeline, creates source
     -> server replies 'rec_finished' { recording_id, source_id }

REST endpoints (JWT-protected, for the frontend):
  GET  /api/recordings                  - list recordings for the user
  GET  /api/recordings/<id>             - single recording details
  DELETE /api/recordings/<id>           - delete a recording and its file
"""

import os
import struct
import uuid
import threading
from datetime import datetime

from flask import Blueprint, request, jsonify
from flask_jwt_extended import jwt_required, get_jwt_identity
from bson import ObjectId

from src.db import get_db
from src.config import Config

recordings_bp = Blueprint("recordings", __name__, url_prefix="/api/recordings")

# ──────────────────────────────────────────────
# In-memory state for active WebSocket sessions
# ──────────────────────────────────────────────
# sid -> { device_id, user_id, device_key }
_authenticated_devices: dict[str, dict] = {}
# sid -> { recording_id, file_path, file_handle, byte_count }
_active_recordings: dict[str, dict] = {}

# Audio format constants (must match ESP32 I2S config)
SAMPLE_RATE = 16000
BITS_PER_SAMPLE = 16
NUM_CHANNELS = 1


# ───────────────────────────
# WAV helpers
# ───────────────────────────
def _write_wav_header(f, data_size: int = 0):
    """Write a WAV header for 16-bit 16 kHz mono PCM. data_size=0 for placeholder."""
    byte_rate = SAMPLE_RATE * NUM_CHANNELS * (BITS_PER_SAMPLE // 8)
    block_align = NUM_CHANNELS * (BITS_PER_SAMPLE // 8)

    f.write(b"RIFF")
    f.write(struct.pack("<I", 36 + data_size))  # file size - 8
    f.write(b"WAVE")
    f.write(b"fmt ")
    f.write(struct.pack("<I", 16))               # fmt chunk size
    f.write(struct.pack("<H", 1))                # PCM format
    f.write(struct.pack("<H", NUM_CHANNELS))
    f.write(struct.pack("<I", SAMPLE_RATE))
    f.write(struct.pack("<I", byte_rate))
    f.write(struct.pack("<H", block_align))
    f.write(struct.pack("<H", BITS_PER_SAMPLE))
    f.write(b"data")
    f.write(struct.pack("<I", data_size))


def _finalize_wav(file_path: str, data_size: int):
    """Re-write the WAV header with the correct data size."""
    with open(file_path, "r+b") as f:
        _write_wav_header(f, data_size)


# ───────────────────────────────────────────
# Post-recording pipeline (runs in a thread)
# ───────────────────────────────────────────
def _process_recording(recording_id: str, file_path: str, user_id: str, device_id: str, title: str):
    """Transcribe the WAV, create a source, update the recording doc."""
    db = get_db()

    try:
        db.recordings.update_one(
            {"_id": ObjectId(recording_id)},
            {"$set": {"status": "transcribing"}},
        )

        from src.services.transcription import transcribe_file
        transcript = transcribe_file(file_path, model_size="base")

        content = transcript["text"]
        if not content or not content.strip():
            db.recordings.update_one(
                {"_id": ObjectId(recording_id)},
                {"$set": {"status": "error", "error": "Transcription produced no text"}},
            )
            return

        # Create a source from the recording
        source_doc = {
            "user_id": user_id,
            "title": title,
            "source_type": "recording",
            "file_type": "audio",
            "file_path": file_path,
            "original_filename": os.path.basename(file_path),
            "content": content,
            "transcript_segments": transcript.get("segments", []),
            "language": transcript.get("language", ""),
            "duration_seconds": transcript.get("duration", 0),
            "char_count": len(content),
            "device_id": device_id,
            "recording_id": recording_id,
            "status": "processing",
            "created_at": datetime.utcnow(),
        }
        inserted = db.sources.insert_one(source_doc)
        source_id = str(inserted.inserted_id)

        # Generate summary + vector embeddings (reuse sources pipeline)
        from src.routes.sources import _process_source
        _process_source(db, source_id)

        # Update recording with result
        db.recordings.update_one(
            {"_id": ObjectId(recording_id)},
            {"$set": {
                "status": "completed",
                "source_id": source_id,
                "duration_seconds": transcript.get("duration", 0),
                "language": transcript.get("language", ""),
                "completed_at": datetime.utcnow(),
            }},
        )
        print(f"[Recording] {recording_id} -> source {source_id} created successfully")

    except Exception as e:
        print(f"[Recording] Processing error for {recording_id}: {e}")
        db.recordings.update_one(
            {"_id": ObjectId(recording_id)},
            {"$set": {"status": "error", "error": str(e)}},
        )


# ───────────────────────────────────
# Socket.IO event handlers
# ───────────────────────────────────
def register_socketio_events(socketio):
    """Register all Socket.IO events for device audio streaming."""

    @socketio.on("connect")
    def handle_connect():
        print(f"[WS] Client connected: {request.sid}")

    @socketio.on("disconnect")
    def handle_disconnect():
        sid = request.sid
        # Clean up any in-progress recording
        if sid in _active_recordings:
            rec = _active_recordings.pop(sid)
            try:
                rec["file_handle"].close()
                _finalize_wav(rec["file_path"], rec["byte_count"])
            except Exception:
                pass
            # Mark as error since it wasn't properly stopped
            try:
                db = get_db()
                db.recordings.update_one(
                    {"_id": ObjectId(rec["recording_id"])},
                    {"$set": {"status": "error", "error": "Device disconnected during recording"}},
                )
            except Exception:
                pass

        _authenticated_devices.pop(sid, None)
        print(f"[WS] Client disconnected: {sid}")

    @socketio.on("auth")
    def handle_auth(data):
        """Authenticate device by its 6-char key."""
        sid = request.sid

        if not isinstance(data, dict):
            socketio.emit("auth_error", {"error": "Invalid payload"}, to=sid)
            return

        key = (data.get("key") or "").strip().upper()
        if not key:
            socketio.emit("auth_error", {"error": "Missing key"}, to=sid)
            return

        db = get_db()
        device = db.devices.find_one({"key": key})

        if not device:
            socketio.emit("auth_error", {"error": "Invalid device key"}, to=sid)
            return

        if not device.get("user_id"):
            socketio.emit("auth_error", {"error": "Device not claimed by a user"}, to=sid)
            return

        # Mark authenticated
        _authenticated_devices[sid] = {
            "device_id": str(device["_id"]),
            "user_id": device["user_id"],
            "device_key": key,
        }

        # Update device status
        db.devices.update_one(
            {"_id": device["_id"]},
            {"$set": {"last_seen": datetime.utcnow(), "status": "online"}},
        )

        socketio.emit("auth_ok", {
            "device_id": str(device["_id"]),
            "message": "Authenticated",
        }, to=sid)
        print(f"[WS] Device authenticated: key={key} sid={sid}")

    @socketio.on("rec_start")
    def handle_rec_start(data):
        """Start a new recording session."""
        sid = request.sid

        if sid not in _authenticated_devices:
            socketio.emit("auth_error", {"error": "Not authenticated"}, to=sid)
            return

        if sid in _active_recordings:
            socketio.emit("rec_error", {"error": "Recording already in progress"}, to=sid)
            return

        dev = _authenticated_devices[sid]
        title = "Lecture Recording"
        if isinstance(data, dict) and data.get("title"):
            title = data["title"].strip()

        # Create recording doc
        db = get_db()
        now = datetime.utcnow()
        file_id = str(uuid.uuid4())
        file_path = os.path.join(Config.UPLOAD_FOLDER, f"{file_id}.wav")
        os.makedirs(Config.UPLOAD_FOLDER, exist_ok=True)

        recording_doc = {
            "device_id": dev["device_id"],
            "user_id": dev["user_id"],
            "title": title,
            "file_path": file_path,
            "status": "recording",
            "source_id": None,
            "duration_seconds": 0,
            "byte_count": 0,
            "language": None,
            "error": None,
            "started_at": now,
            "completed_at": None,
            "created_at": now,
        }
        inserted = db.recordings.insert_one(recording_doc)
        recording_id = str(inserted.inserted_id)

        # Open file and write placeholder WAV header
        fh = open(file_path, "wb")
        _write_wav_header(fh, data_size=0)

        _active_recordings[sid] = {
            "recording_id": recording_id,
            "file_path": file_path,
            "file_handle": fh,
            "byte_count": 0,
            "title": title,
        }

        socketio.emit("rec_started", {"recording_id": recording_id}, to=sid)
        print(f"[WS] Recording started: {recording_id} from device {dev['device_id']}")

    @socketio.on("audio_data")
    def handle_audio_data(data):
        """Receive a chunk of raw PCM audio bytes."""
        sid = request.sid

        if sid not in _active_recordings:
            return  # silently drop if no active recording

        if not isinstance(data, (bytes, bytearray)):
            return  # must be binary

        rec = _active_recordings[sid]
        rec["file_handle"].write(data)
        rec["byte_count"] += len(data)

    @socketio.on("rec_stop")
    def handle_rec_stop(_data=None):
        """Stop the recording, finalize WAV, kick off transcription pipeline."""
        sid = request.sid

        if sid not in _active_recordings:
            socketio.emit("rec_error", {"error": "No active recording"}, to=sid)
            return

        rec = _active_recordings.pop(sid)
        dev = _authenticated_devices.get(sid, {})

        # Close and finalize WAV
        rec["file_handle"].close()
        _finalize_wav(rec["file_path"], rec["byte_count"])

        # Calculate duration
        bytes_per_second = SAMPLE_RATE * NUM_CHANNELS * (BITS_PER_SAMPLE // 8)
        duration = rec["byte_count"] / bytes_per_second if bytes_per_second else 0

        # Update recording status
        db = get_db()
        db.recordings.update_one(
            {"_id": ObjectId(rec["recording_id"])},
            {"$set": {
                "status": "processing",
                "byte_count": rec["byte_count"],
                "duration_seconds": round(duration, 2),
            }},
        )

        socketio.emit("rec_stopped", {
            "recording_id": rec["recording_id"],
            "duration_seconds": round(duration, 2),
            "status": "processing",
        }, to=sid)

        print(f"[WS] Recording stopped: {rec['recording_id']} "
              f"({rec['byte_count']} bytes, {duration:.1f}s)")

        # Run transcription + source creation in background thread
        thread = threading.Thread(
            target=_process_recording,
            args=(
                rec["recording_id"],
                rec["file_path"],
                dev.get("user_id", ""),
                dev.get("device_id", ""),
                rec["title"],
            ),
            daemon=True,
        )
        thread.start()


# ──────────────────────────────
# REST endpoints (for frontend)
# ──────────────────────────────
@recordings_bp.route("", methods=["GET"])
@jwt_required()
def list_recordings():
    """List all recordings for the authenticated user."""
    user_id = get_jwt_identity()
    db = get_db()

    recordings = list(
        db.recordings.find({"user_id": user_id}).sort("created_at", -1)
    )

    return jsonify([
        {
            "id": str(r["_id"]),
            "device_id": r.get("device_id"),
            "title": r.get("title", "Untitled"),
            "status": r.get("status", "unknown"),
            "source_id": r.get("source_id"),
            "duration_seconds": r.get("duration_seconds", 0),
            "language": r.get("language"),
            "error": r.get("error"),
            "started_at": r["started_at"].isoformat() if r.get("started_at") else None,
            "completed_at": r["completed_at"].isoformat() if r.get("completed_at") else None,
            "created_at": r["created_at"].isoformat() if r.get("created_at") else None,
        }
        for r in recordings
    ]), 200


@recordings_bp.route("/<recording_id>", methods=["GET"])
@jwt_required()
def get_recording(recording_id):
    """Get a single recording's details."""
    user_id = get_jwt_identity()
    db = get_db()

    try:
        recording = db.recordings.find_one({
            "_id": ObjectId(recording_id),
            "user_id": user_id,
        })
    except Exception:
        return jsonify({"error": "Invalid recording ID"}), 400

    if not recording:
        return jsonify({"error": "Recording not found"}), 404

    return jsonify({
        "id": str(recording["_id"]),
        "device_id": recording.get("device_id"),
        "title": recording.get("title", "Untitled"),
        "status": recording.get("status", "unknown"),
        "source_id": recording.get("source_id"),
        "duration_seconds": recording.get("duration_seconds", 0),
        "byte_count": recording.get("byte_count", 0),
        "language": recording.get("language"),
        "error": recording.get("error"),
        "started_at": recording["started_at"].isoformat() if recording.get("started_at") else None,
        "completed_at": recording["completed_at"].isoformat() if recording.get("completed_at") else None,
        "created_at": recording["created_at"].isoformat() if recording.get("created_at") else None,
    }), 200


@recordings_bp.route("/<recording_id>", methods=["DELETE"])
@jwt_required()
def delete_recording(recording_id):
    """Delete a recording and its WAV file."""
    user_id = get_jwt_identity()
    db = get_db()

    try:
        recording = db.recordings.find_one({
            "_id": ObjectId(recording_id),
            "user_id": user_id,
        })
    except Exception:
        return jsonify({"error": "Invalid recording ID"}), 400

    if not recording:
        return jsonify({"error": "Recording not found"}), 404

    # Delete the WAV file if it exists
    file_path = recording.get("file_path")
    if file_path and os.path.exists(file_path):
        os.remove(file_path)

    db.recordings.delete_one({"_id": ObjectId(recording_id)})

    return jsonify({"message": "Recording deleted"}), 200


@recordings_bp.route("/device/<device_id>", methods=["GET"])
@jwt_required()
def list_device_recordings(device_id):
    """List recordings for a specific device (must belong to user)."""
    user_id = get_jwt_identity()
    db = get_db()

    # Verify device ownership
    try:
        device = db.devices.find_one({
            "_id": ObjectId(device_id),
            "user_id": user_id,
        })
    except Exception:
        return jsonify({"error": "Invalid device ID"}), 400

    if not device:
        return jsonify({"error": "Device not found"}), 404

    recordings = list(
        db.recordings.find({"device_id": device_id}).sort("created_at", -1).limit(50)
    )

    return jsonify([
        {
            "id": str(r["_id"]),
            "title": r.get("title", "Untitled"),
            "status": r.get("status", "unknown"),
            "source_id": r.get("source_id"),
            "duration_seconds": r.get("duration_seconds", 0),
            "language": r.get("language"),
            "error": r.get("error"),
            "started_at": r["started_at"].isoformat() if r.get("started_at") else None,
            "completed_at": r["completed_at"].isoformat() if r.get("completed_at") else None,
            "created_at": r["created_at"].isoformat() if r.get("created_at") else None,
        }
        for r in recordings
    ]), 200
