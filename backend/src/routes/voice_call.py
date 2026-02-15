"""
Voice Call Routes - ESP32 push-to-talk voice conversation via Socket.IO.

Protocol (device <-> backend via Socket.IO):
  1. Device emits 'call_start'           {}
     -> server replies 'call_started'    {}
  2. Device emits 'call_audio'           <raw bytes>  (16-bit PCM, 16 kHz mono)
     -> server accumulates in memory buffer
  3. Device emits 'call_stop_listening'  {}
     -> server runs pipeline: STT -> LLM (Gemini + RAG) -> TTS (ElevenLabs) -> PCM
     -> server emits 'call_response'     <raw bytes>  (16-bit PCM, 16 kHz mono)
  4. Device emits 'call_end'             {}
     -> server replies 'call_ended'      {}
     -> cleanup session

The device must be authenticated (via 'auth' event in recordings.py) before
using voice call events. We share the _authenticated_devices dict.

RAG context: uses the user's most recently created source. Falls back to
general knowledge if no sources exist.
"""

import io
import os
import struct
import tempfile
import threading
import subprocess

import requests

from flask import request as flask_request

from src.config import Config
from src.db import get_db
from src.services import vector_search
from src.services.gemini_service import chat_with_context

# Audio format constants (must match ESP32 I2S config)
SAMPLE_RATE = 16000
BITS_PER_SAMPLE = 16
NUM_CHANNELS = 1

# ElevenLabs TTS config
ELEVENLABS_VOICE_ID = "JBFqnCBsd6RMkjVDRZzb"  # "George" - clear male voice
ELEVENLABS_MODEL_ID = "eleven_flash_v2_5"  # Low-latency model

# ──────────────────────────────────────────────
# In-memory state for active voice call sessions
# ──────────────────────────────────────────────
# sid -> { audio_buffer: bytearray, source_id: str|None, source_title: str, chat_history: list }
_active_calls: dict[str, dict] = {}


def _write_wav_to_buffer(pcm_data: bytes) -> bytes:
    """Wrap raw PCM data in a WAV header and return the full WAV bytes."""
    data_size = len(pcm_data)
    byte_rate = SAMPLE_RATE * NUM_CHANNELS * (BITS_PER_SAMPLE // 8)
    block_align = NUM_CHANNELS * (BITS_PER_SAMPLE // 8)

    buf = io.BytesIO()
    buf.write(b"RIFF")
    buf.write(struct.pack("<I", 36 + data_size))
    buf.write(b"WAVE")
    buf.write(b"fmt ")
    buf.write(struct.pack("<I", 16))
    buf.write(struct.pack("<H", 1))  # PCM
    buf.write(struct.pack("<H", NUM_CHANNELS))
    buf.write(struct.pack("<I", SAMPLE_RATE))
    buf.write(struct.pack("<I", byte_rate))
    buf.write(struct.pack("<H", block_align))
    buf.write(struct.pack("<H", BITS_PER_SAMPLE))
    buf.write(b"data")
    buf.write(struct.pack("<I", data_size))
    buf.write(pcm_data)
    return buf.getvalue()


def _get_user_latest_source(user_id: str) -> dict | None:
    """Get the user's most recently created source (for RAG context)."""
    db = get_db()
    source = db.sources.find_one(
        {"user_id": user_id, "status": "ready"},
        sort=[("created_at", -1)],
    )
    return source


def _get_rag_context(source_id: str, query: str, source_title: str) -> str:
    """Retrieve RAG context from a source for the voice query."""
    context = vector_search.get_context_for_query(source_id, query, n_results=5)

    if not context.strip():
        # Fallback: get raw content
        db = get_db()
        from bson import ObjectId
        source = db.sources.find_one({"_id": ObjectId(source_id)})
        if source and source.get("content"):
            context = source["content"][:4000]

    return context


def _transcribe_pcm(pcm_data: bytes) -> str:
    """Transcribe raw PCM audio using faster-whisper via a temp WAV file."""
    wav_data = _write_wav_to_buffer(pcm_data)

    # Write to temp file for faster-whisper
    tmp_fd, tmp_path = tempfile.mkstemp(suffix=".wav")
    try:
        with os.fdopen(tmp_fd, "wb") as f:
            f.write(wav_data)

        from src.services.transcription import transcribe_audio
        result = transcribe_audio(tmp_path, model_size="base")
        return result.get("text", "").strip()
    finally:
        if os.path.exists(tmp_path):
            os.remove(tmp_path)


def _generate_tts(text: str) -> bytes | None:
    """
    Generate speech audio from text using ElevenLabs TTS API.
    Returns raw 16-bit 16 kHz mono PCM bytes, or None on failure.
    """
    api_key = Config.ELEVENLABS_API_KEY
    if not api_key:
        print("[VoiceCall] ElevenLabs API key not configured")
        return None

    url = f"https://api.elevenlabs.io/v1/text-to-speech/{ELEVENLABS_VOICE_ID}"

    headers = {
        "xi-api-key": api_key,
        "Content-Type": "application/json",
        "Accept": "audio/mpeg",  # Get MP3, then convert to PCM
    }

    payload = {
        "text": text,
        "model_id": ELEVENLABS_MODEL_ID,
        "voice_settings": {
            "stability": 0.5,
            "similarity_boost": 0.75,
            "style": 0.0,
            "use_speaker_boost": True,
        },
    }

    try:
        resp = requests.post(url, json=payload, headers=headers, timeout=30)
        resp.raise_for_status()
        mp3_data = resp.content

        if not mp3_data:
            print("[VoiceCall] ElevenLabs returned empty audio")
            return None

        # Convert MP3 to 16-bit 16 kHz mono PCM using ffmpeg
        return _convert_to_pcm(mp3_data, input_format="mp3")

    except requests.exceptions.RequestException as e:
        print(f"[VoiceCall] ElevenLabs TTS error: {e}")
        return None


def _convert_to_pcm(audio_data: bytes, input_format: str = "mp3") -> bytes | None:
    """Convert audio data to 16-bit 16 kHz mono PCM using ffmpeg."""
    try:
        result = subprocess.run(
            [
                "ffmpeg", "-y",
                "-f", input_format, "-i", "pipe:0",
                "-f", "s16le",  # raw 16-bit signed little-endian
                "-acodec", "pcm_s16le",
                "-ar", str(SAMPLE_RATE),
                "-ac", str(NUM_CHANNELS),
                "pipe:1",
            ],
            input=audio_data,
            capture_output=True,
            timeout=15,
        )

        if result.returncode != 0:
            print(f"[VoiceCall] ffmpeg conversion failed: {result.stderr.decode()[:200]}")
            return None

        return result.stdout

    except FileNotFoundError:
        print("[VoiceCall] ffmpeg not installed")
        return None
    except subprocess.TimeoutExpired:
        print("[VoiceCall] ffmpeg conversion timed out")
        return None


def _build_voice_system_prompt(context: str, source_title: str) -> str:
    """Build system prompt for voice conversation with RAG context."""
    base = (
        "You are LearningBuddy AI, a friendly voice tutor helping a student study. "
        "Keep your answers SHORT and conversational since this is spoken dialogue "
        "on a small ESP32 device with limited speaker playback time. "
        "Aim for 1-3 sentences. Use simple words. "
        "If you don't know the answer, say so briefly."
    )

    if context and source_title:
        return (
            f"{base}\n\n"
            f"The student is studying: {source_title}\n"
            f"Answer based on this context when relevant:\n"
            f"=== CONTEXT ===\n{context}\n=== END CONTEXT ==="
        )

    return f"{base}\n\nNo specific study material loaded. Answer general knowledge questions."


def _process_voice_call(sid: str, socketio, pcm_data: bytes,
                        source_id: str | None, source_title: str,
                        chat_history: list):
    """
    Background pipeline: STT -> LLM -> TTS -> emit response.
    Runs in a daemon thread to avoid blocking the Socket.IO event loop.
    """
    try:
        # ── Step 1: Speech-to-Text ──
        print(f"[VoiceCall] STT starting for {sid} ({len(pcm_data)} bytes)")
        user_text = _transcribe_pcm(pcm_data)

        if not user_text:
            print(f"[VoiceCall] STT produced no text for {sid}")
            socketio.emit("call_response", b"", to=sid)
            return

        print(f"[VoiceCall] STT result: \"{user_text}\"")

        # ── Step 2: Get RAG context ──
        context = ""
        if source_id:
            try:
                context = _get_rag_context(source_id, user_text, source_title)
            except Exception as e:
                print(f"[VoiceCall] RAG context error: {e}")

        # ── Step 3: LLM response ──
        system_prompt = _build_voice_system_prompt(context, source_title)
        print(f"[VoiceCall] Generating LLM response...")

        # Use Gemini chat with context
        llm_response = chat_with_context(
            user_message=user_text,
            context=context,
            chat_history=chat_history[-6:],  # Last 3 exchanges
            source_title=source_title,
        )

        if not llm_response:
            llm_response = "Sorry, I couldn't generate a response."

        print(f"[VoiceCall] LLM response: \"{llm_response[:100]}...\"")

        # Update chat history in the session
        if sid in _active_calls:
            _active_calls[sid]["chat_history"].append({"role": "user", "content": user_text})
            _active_calls[sid]["chat_history"].append({"role": "assistant", "content": llm_response})

        # ── Step 4: Text-to-Speech ──
        print(f"[VoiceCall] Generating TTS...")
        pcm_response = _generate_tts(llm_response)

        if not pcm_response:
            print(f"[VoiceCall] TTS failed, sending empty response")
            socketio.emit("call_response", b"", to=sid)
            return

        print(f"[VoiceCall] Sending {len(pcm_response)} bytes of TTS audio to {sid}")

        # ── Step 5: Emit binary PCM to device ──
        socketio.emit("call_response", pcm_response, to=sid)

    except Exception as e:
        print(f"[VoiceCall] Pipeline error for {sid}: {e}")
        import traceback
        traceback.print_exc()
        try:
            socketio.emit("call_response", b"", to=sid)
        except Exception:
            pass


# ───────────────────────────────────
# Socket.IO event handlers
# ───────────────────────────────────
def register_voice_call_events(socketio):
    """Register Socket.IO events for ESP32 voice call sessions."""

    # Import the shared auth dict from recordings
    from src.routes.recordings import _authenticated_devices

    @socketio.on("call_start")
    def handle_call_start(_data=None):
        """Start a voice call session."""
        sid = flask_request.sid

        if sid not in _authenticated_devices:
            socketio.emit("auth_error", {"error": "Not authenticated"}, to=sid)
            return

        if sid in _active_calls:
            socketio.emit("call_error", {"error": "Call already in progress"}, to=sid)
            return

        dev = _authenticated_devices[sid]
        user_id = dev["user_id"]

        # Find the user's most recent source for RAG context
        source = _get_user_latest_source(user_id)
        source_id = str(source["_id"]) if source else None
        source_title = source.get("title", "") if source else ""

        _active_calls[sid] = {
            "audio_buffer": bytearray(),
            "source_id": source_id,
            "source_title": source_title,
            "chat_history": [],
        }

        socketio.emit("call_started", {}, to=sid)
        context_msg = f" (source: {source_title})" if source_title else " (no source, general knowledge)"
        print(f"[VoiceCall] Call started: sid={sid}{context_msg}")

    @socketio.on("call_audio")
    def handle_call_audio(data):
        """Receive a chunk of raw PCM audio from the device's mic."""
        sid = flask_request.sid

        if sid not in _active_calls:
            return  # Silently drop if no active call

        if not isinstance(data, (bytes, bytearray)):
            return

        _active_calls[sid]["audio_buffer"].extend(data)

    @socketio.on("call_stop_listening")
    def handle_call_stop_listening(_data=None):
        """
        Device released the talk button. Process the accumulated audio:
        STT -> LLM -> TTS -> emit call_response.
        """
        sid = flask_request.sid

        if sid not in _active_calls:
            socketio.emit("call_error", {"error": "No active call"}, to=sid)
            return

        call = _active_calls[sid]
        pcm_data = bytes(call["audio_buffer"])
        call["audio_buffer"] = bytearray()  # Reset buffer for next utterance

        if len(pcm_data) < 3200:  # Less than 0.1 seconds of audio
            print(f"[VoiceCall] Audio too short ({len(pcm_data)} bytes), ignoring")
            socketio.emit("call_response", b"", to=sid)
            return

        duration = len(pcm_data) / (SAMPLE_RATE * NUM_CHANNELS * (BITS_PER_SAMPLE // 8))
        print(f"[VoiceCall] Processing {len(pcm_data)} bytes ({duration:.1f}s) from {sid}")

        # Run the full pipeline in a background thread
        thread = threading.Thread(
            target=_process_voice_call,
            args=(
                sid,
                socketio,
                pcm_data,
                call["source_id"],
                call["source_title"],
                list(call["chat_history"]),  # Copy to avoid race
            ),
            daemon=True,
        )
        thread.start()

    @socketio.on("call_end")
    def handle_call_end(_data=None):
        """End the voice call session and clean up."""
        sid = flask_request.sid

        if sid in _active_calls:
            _active_calls.pop(sid)

        socketio.emit("call_ended", {}, to=sid)
        print(f"[VoiceCall] Call ended: sid={sid}")
