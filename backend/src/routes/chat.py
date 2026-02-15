"""
Chat Routes - RAG-powered chat with sources via Gemini.
"""

import json
from flask import Blueprint, request, jsonify, Response, stream_with_context
from flask_jwt_extended import jwt_required, get_jwt_identity
from bson import ObjectId
from datetime import datetime
from src.db import get_db
from src.services import vector_search, gemini_service

chat_bp = Blueprint("chat", __name__, url_prefix="/api/sources")


@chat_bp.route("/<source_id>/chat", methods=["POST"])
@jwt_required()
def chat(source_id):
    """
    Send a message and receive an AI response based on the source's content.
    Uses MongoDB Atlas Vector Search to find relevant chunks and Gemini for generation.
    """
    user_id = get_jwt_identity()
    data = request.get_json()
    db = get_db()

    message = data.get("message", "").strip()
    if not message:
        return jsonify({"error": "Message is required"}), 400

    # Verify source exists and belongs to user
    try:
        source = db.sources.find_one({"_id": ObjectId(source_id), "user_id": user_id})
    except Exception:
        return jsonify({"error": "Invalid source ID"}), 400

    if not source:
        return jsonify({"error": "Source not found"}), 404

    # Get chat history for this source
    history_docs = list(
        db.chat_history.find({"source_id": source_id, "user_id": user_id})
        .sort("created_at", 1)
        .limit(20)
    )
    chat_history = [
        {"role": h["role"], "content": h["content"]}
        for h in history_docs
    ]

    # Get relevant context via vector search
    context = vector_search.get_context_for_query(source_id, message, n_results=50)

    # If no vector chunks exist, fall back to the raw content (truncated)
    if not context.strip():
        content = source.get("content", "")
        context = content[:8000] if content else "No content available."

    # Generate AI response
    try:
        response_text = gemini_service.chat_with_context(
            user_message=message,
            context=context,
            chat_history=chat_history,
            source_title=source.get("title", ""),
        )
    except Exception as e:
        return jsonify({"error": f"AI generation failed: {str(e)}"}), 500

    # Save both messages to chat history
    now = datetime.utcnow()
    db.chat_history.insert_many([
        {
            "source_id": source_id,
            "user_id": user_id,
            "role": "user",
            "content": message,
            "created_at": now,
        },
        {
            "source_id": source_id,
            "user_id": user_id,
            "role": "assistant",
            "content": response_text,
            "created_at": now,
        },
    ])

    return jsonify({
        "role": "assistant",
        "content": response_text,
    }), 200


@chat_bp.route("/<source_id>/chat/stream", methods=["POST"])
@jwt_required()
def chat_stream(source_id):
    """
    Stream a chat response using Server-Sent Events (SSE).
    """
    user_id = get_jwt_identity()
    data = request.get_json()
    db = get_db()

    message = data.get("message", "").strip()
    if not message:
        return jsonify({"error": "Message is required"}), 400

    try:
        source = db.sources.find_one({"_id": ObjectId(source_id), "user_id": user_id})
    except Exception:
        return jsonify({"error": "Invalid source ID"}), 400

    if not source:
        return jsonify({"error": "Source not found"}), 404

    # Get chat history
    history_docs = list(
        db.chat_history.find({"source_id": source_id, "user_id": user_id})
        .sort("created_at", 1)
        .limit(20)
    )
    chat_history = [{"role": h["role"], "content": h["content"]} for h in history_docs]

    # Get relevant context
    context = vector_search.get_context_for_query(source_id, message, n_results=50)
    if not context.strip():
        content = source.get("content", "")
        context = content[:8000] if content else "No content available."

    # Save user message immediately
    now = datetime.utcnow()
    db.chat_history.insert_one({
        "source_id": source_id,
        "user_id": user_id,
        "role": "user",
        "content": message,
        "created_at": now,
    })

    def generate():
        full_response = ""
        try:
            for chunk in gemini_service.chat_with_context_stream(
                user_message=message,
                context=context,
                chat_history=chat_history,
                source_title=source.get("title", ""),
            ):
                full_response += chunk
                yield f"data: {json.dumps({'chunk': chunk})}\n\n"

            # Save assistant response to history
            db.chat_history.insert_one({
                "source_id": source_id,
                "user_id": user_id,
                "role": "assistant",
                "content": full_response,
                "created_at": datetime.utcnow(),
            })

            yield f"data: {json.dumps({'done': True})}\n\n"
        except Exception as e:
            yield f"data: {json.dumps({'error': str(e)})}\n\n"

    return Response(
        stream_with_context(generate()),
        mimetype="text/event-stream",
        headers={
            "Cache-Control": "no-cache",
            "X-Accel-Buffering": "no",
        },
    )


@chat_bp.route("/<source_id>/chat/history", methods=["GET"])
@jwt_required()
def get_chat_history(source_id):
    """Get the chat history for a source."""
    user_id = get_jwt_identity()
    db = get_db()

    history_docs = list(
        db.chat_history.find({"source_id": source_id, "user_id": user_id})
        .sort("created_at", 1)
    )

    messages = []
    for h in history_docs:
        messages.append({
            "id": str(h["_id"]),
            "role": h["role"],
            "content": h["content"],
            "created_at": h["created_at"].isoformat() if h.get("created_at") else None,
        })

    return jsonify(messages), 200


@chat_bp.route("/<source_id>/chat/history", methods=["DELETE"])
@jwt_required()
def clear_chat_history(source_id):
    """Clear all chat history for a source."""
    user_id = get_jwt_identity()
    db = get_db()

    result = db.chat_history.delete_many({"source_id": source_id, "user_id": user_id})

    return jsonify({"message": f"Deleted {result.deleted_count} messages"}), 200
