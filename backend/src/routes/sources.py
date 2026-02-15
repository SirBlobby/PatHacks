"""
Sources Routes - Upload, manage, and process learning sources.
Supports text input, file uploads (PDFs, docs, code, images, audio, video).
"""

import os
import uuid
from flask import Blueprint, request, jsonify
from flask_jwt_extended import jwt_required, get_jwt_identity
from bson import ObjectId
from datetime import datetime
from werkzeug.utils import secure_filename

from src.db import get_db
from src.config import Config
from src.services.document_loader import (
    load_document,
    get_file_type,
    get_supported_extensions,
    FILE_TYPE_LABELS,
)

sources_bp = Blueprint("sources", __name__, url_prefix="/api/sources")


@sources_bp.route("", methods=["GET"])
@jwt_required()
def list_sources():
    """List all sources for the current user."""
    user_id = get_jwt_identity()
    db = get_db()

    filter_query = {"user_id": user_id}
    q = request.args.get("q", "").strip()
    if q:
        filter_query["title"] = {"$regex": q, "$options": "i"}

    sources = list(
        db.sources.find(filter_query).sort("created_at", -1)
    )

    return jsonify([
        {
            "id": str(s["_id"]),
            "title": s["title"],
            "source_type": s.get("source_type", "text"),
            "file_type": s.get("file_type", "text"),
            "status": s.get("status", "pending"),
            "duration_seconds": s.get("duration_seconds", 0),
            "char_count": s.get("char_count", 0),
            "created_at": s["created_at"].isoformat() if s.get("created_at") else None,
        }
        for s in sources
    ]), 200


@sources_bp.route("", methods=["POST"])
@jwt_required()
def create_source():
    """
    Create a new source. Supports two modes:
    1. JSON body with { title, content } for pasting text
    2. Multipart form upload with a file
    """
    user_id = get_jwt_identity()
    db = get_db()

    # ── Mode 1: File upload ──
    if "file" in request.files:
        file = request.files["file"]
        title = request.form.get("title", "").strip() or file.filename or "Untitled"

        if not file.filename:
            return jsonify({"error": "No file selected"}), 400

        # Save the file
        ext = os.path.splitext(file.filename)[1].lower()
        file_id = str(uuid.uuid4())
        safe_name = secure_filename(f"{file_id}{ext}")
        file_path = os.path.join(Config.UPLOAD_FOLDER, safe_name)
        os.makedirs(Config.UPLOAD_FOLDER, exist_ok=True)
        file.save(file_path)

        file_type = get_file_type(file_path)
        if file_type is None:
            os.remove(file_path)
            return jsonify({
                "error": f"Unsupported file type: {ext}",
                "supported": get_supported_extensions(),
            }), 400

        # Process the document
        result = load_document(file_path)

        source_doc = {
            "user_id": user_id,
            "title": title,
            "source_type": "upload",
            "file_type": file_type,
            "file_path": file_path,
            "original_filename": file.filename,
            "status": "processing",
            "created_at": datetime.utcnow(),
        }

        # Handle audio/video that needs transcription
        if result.get("requires_transcription"):
            source_doc["status"] = "transcribing"
            inserted = db.sources.insert_one(source_doc)
            source_id = str(inserted.inserted_id)

            # Transcribe in-line (blocking for now)
            try:
                from src.services.transcription import transcribe_file
                transcript = transcribe_file(file_path, model_size="base")

                content = transcript["text"]
                source_doc_update = {
                    "content": content,
                    "transcript_segments": transcript.get("segments", []),
                    "language": transcript.get("language", ""),
                    "duration_seconds": transcript.get("duration", 0),
                    "char_count": len(content),
                    "status": "processing",
                }
                db.sources.update_one(
                    {"_id": ObjectId(source_id)},
                    {"$set": source_doc_update},
                )
            except Exception as e:
                db.sources.update_one(
                    {"_id": ObjectId(source_id)},
                    {"$set": {"status": "error", "error": str(e)}},
                )
                return jsonify({
                    "id": source_id,
                    "message": f"File uploaded but transcription failed: {e}",
                    "status": "error",
                }), 201

        elif result.get("success"):
            content = result["text"]
            source_doc["content"] = content
            source_doc["char_count"] = len(content)
            source_doc["status"] = "processing"
            inserted = db.sources.insert_one(source_doc)
            source_id = str(inserted.inserted_id)
        else:
            os.remove(file_path)
            return jsonify({"error": result.get("error", "Failed to process file")}), 400

        # Generate summary + embeddings
        _process_source(db, source_id)

        source = db.sources.find_one({"_id": ObjectId(source_id)})
        return jsonify({
            "id": source_id,
            "title": source["title"],
            "source_type": source.get("source_type"),
            "file_type": source.get("file_type"),
            "status": source.get("status", "processed"),
            "message": "Source uploaded and processed",
        }), 201

    # ── Mode 2: Text/paste input ──
    data = request.get_json()
    if not data:
        return jsonify({"error": "No data provided. Send JSON or upload a file."}), 400

    title = data.get("title", "").strip()
    content = data.get("content", "").strip()

    if not title:
        return jsonify({"error": "Title is required"}), 400
    if not content:
        return jsonify({"error": "Content is required"}), 400

    source_doc = {
        "user_id": user_id,
        "title": title,
        "source_type": "text",
        "file_type": "text",
        "content": content,
        "char_count": len(content),
        "status": "processing",
        "created_at": datetime.utcnow(),
    }

    result = db.sources.insert_one(source_doc)
    source_id = str(result.inserted_id)

    # Generate summary + embeddings
    _process_source(db, source_id)

    source = db.sources.find_one({"_id": ObjectId(source_id)})
    return jsonify({
        "id": source_id,
        "title": source["title"],
        "source_type": "text",
        "file_type": "text",
        "status": source.get("status", "processed"),
        "message": "Source created and processed",
    }), 201


@sources_bp.route("/<source_id>", methods=["GET"])
@jwt_required()
def get_source(source_id):
    """Get a specific source with full content."""
    user_id = get_jwt_identity()
    db = get_db()

    source = db.sources.find_one({"_id": ObjectId(source_id), "user_id": user_id})
    if not source:
        return jsonify({"error": "Source not found"}), 404

    return jsonify({
        "id": str(source["_id"]),
        "title": source["title"],
        "source_type": source.get("source_type", "text"),
        "file_type": source.get("file_type", "text"),
        "content": source.get("content", ""),
        "transcript_segments": source.get("transcript_segments", []),
        "summary": source.get("summary", ""),
        "status": source.get("status", "pending"),
        "duration_seconds": source.get("duration_seconds", 0),
        "char_count": source.get("char_count", 0),
        "original_filename": source.get("original_filename", ""),
        "created_at": source["created_at"].isoformat() if source.get("created_at") else None,
    }), 200


@sources_bp.route("/<source_id>", methods=["DELETE"])
@jwt_required()
def delete_source(source_id):
    """Delete a source and its associated data."""
    user_id = get_jwt_identity()
    db = get_db()

    source = db.sources.find_one({"_id": ObjectId(source_id), "user_id": user_id})
    if not source:
        return jsonify({"error": "Source not found"}), 404

    # Delete associated vector chunks
    db.lecture_chunks.delete_many({"source_id": source_id})

    # Delete associated chat history
    db.chat_history.delete_many({"source_id": source_id})

    # Delete uploaded file if it exists
    file_path = source.get("file_path")
    if file_path and os.path.exists(file_path):
        os.remove(file_path)

    db.sources.delete_one({"_id": ObjectId(source_id)})

    return jsonify({"message": "Source deleted"}), 200


@sources_bp.route("/<source_id>/regenerate-summary", methods=["POST"])
@jwt_required()
def regenerate_summary(source_id):
    """Regenerate the AI summary for a source."""
    user_id = get_jwt_identity()
    db = get_db()

    source = db.sources.find_one({"_id": ObjectId(source_id), "user_id": user_id})
    if not source:
        return jsonify({"error": "Source not found"}), 404

    content = source.get("content", "")
    if not content:
        return jsonify({"error": "No content to summarize"}), 400

    try:
        from src.services.gemini_service import summarize_transcript
        summary = summarize_transcript(content)
        db.sources.update_one(
            {"_id": ObjectId(source_id)},
            {"$set": {"summary": summary}},
        )
        return jsonify({"summary": summary}), 200
    except Exception as e:
        return jsonify({"error": f"Summary generation failed: {e}"}), 500


@sources_bp.route("/supported-types", methods=["GET"])
def supported_types():
    """Return list of supported file types."""
    return jsonify({
        "extensions": get_supported_extensions(),
        "labels": FILE_TYPE_LABELS,
    }), 200


def _process_source(db, source_id: str):
    """
    Process a source: generate summary and vector embeddings.
    Called after content is available.
    """
    source = db.sources.find_one({"_id": ObjectId(source_id)})
    if not source:
        return

    content = source.get("content", "")
    if not content:
        db.sources.update_one(
            {"_id": ObjectId(source_id)},
            {"$set": {"status": "error", "error": "No content to process"}},
        )
        return

    try:
        # Generate summary
        from src.services.gemini_service import summarize_transcript
        summary = summarize_transcript(content)

        # Generate vector embeddings
        from src.services.vector_search import process_and_store_embeddings
        process_and_store_embeddings(
            source_id=source_id,
            text=content,
        )

        db.sources.update_one(
            {"_id": ObjectId(source_id)},
            {"$set": {
                "summary": summary,
                "status": "processed",
            }},
        )
    except Exception as e:
        print(f"[Source Processing Error] {e}")
        db.sources.update_one(
            {"_id": ObjectId(source_id)},
            {"$set": {"status": "error", "error": str(e)}},
        )
