"""
General Chat Routes - RAG-powered chat across ALL user sources via Gemini.

Unlike the source-specific chat (chat.py), this pulls context from every
source the user has uploaded, so the student can ask about anything.
"""

import json
from flask import Blueprint, request, jsonify, Response, stream_with_context
from flask_jwt_extended import jwt_required, get_jwt_identity
from datetime import datetime
from src.db import get_db
from src.services import vector_search, gemini_service

general_chat_bp = Blueprint("general_chat", __name__, url_prefix="/api/chat")


def _build_general_system_prompt(context: str, source_titles: list[str]) -> str:
    """Build the system prompt for general multi-source chat."""
    titles_str = ", ".join(source_titles) if source_titles else "No sources"
    return (
        "You are LearningBuddy AI, a helpful study assistant. "
        "The student has uploaded multiple learning sources and can ask you about ANY of them. "
        "You have access to relevant excerpts from their sources provided as context below. "
        "Always base your answers on the provided context. If the context doesn't contain "
        "enough information to fully answer, say so honestly and share what you can. "
        "When referencing information, mention which source it comes from. "
        "Reference specific timestamps (e.g. 02:30) when available in the context, "
        "but NEVER reference chunk numbers or indexes. "
        "Be concise but thorough.\n\n"
        f"=== STUDENT'S SOURCES: {titles_str} ===\n"
        f"=== CONTEXT ===\n{context}\n=== END CONTEXT ==="
    )


@general_chat_bp.route("", methods=["POST"])
@jwt_required()
def general_chat():
    """
    Send a message and receive an AI response based on ALL user sources.
    Uses vector search across every source to find relevant chunks.
    """
    user_id = get_jwt_identity()
    data = request.get_json()
    db = get_db()

    message = data.get("message", "").strip()
    if not message:
        return jsonify({"error": "Message is required"}), 400

    # Get chat history for general chat
    history_docs = list(
        db.chat_history.find({"user_id": user_id, "source_id": "__general__"})
        .sort("created_at", 1)
        .limit(20)
    )
    chat_history = [
        {"role": h["role"], "content": h["content"]}
        for h in history_docs
    ]

    # Get relevant context across ALL sources
    context = vector_search.get_context_for_all_sources(user_id, message, n_results=50)

    # Get source titles for the system prompt
    user_sources = list(db.sources.find({"user_id": user_id, "status": "processed"}, {"title": 1}))
    source_titles = [s.get("title", "Untitled") for s in user_sources]

    if not context.strip():
        # Fall back to raw content snippets from each source
        sources = list(db.sources.find({"user_id": user_id, "status": "processed"}, {"title": 1, "content": 1}).limit(10))
        parts = []
        for s in sources:
            content = s.get("content", "")
            if content:
                parts.append(f"--- {s.get('title', 'Untitled')} ---\n{content[:2000]}\n")
        context = "\n".join(parts) if parts else "No sources available."

    # Generate AI response
    system_prompt = _build_general_system_prompt(context, source_titles)
    try:
        response_text = gemini_service.chat_with_context_custom(
            user_message=message,
            system_prompt=system_prompt,
            chat_history=chat_history,
        )
    except Exception as e:
        return jsonify({"error": f"AI generation failed: {str(e)}"}), 500

    # Save both messages to chat history
    now = datetime.utcnow()
    db.chat_history.insert_many([
        {
            "source_id": "__general__",
            "user_id": user_id,
            "role": "user",
            "content": message,
            "created_at": now,
        },
        {
            "source_id": "__general__",
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


@general_chat_bp.route("/stream", methods=["POST"])
@jwt_required()
def general_chat_stream():
    """
    Stream a general chat response using Server-Sent Events (SSE).
    Pulls context from all user sources.
    """
    user_id = get_jwt_identity()
    data = request.get_json()
    db = get_db()

    message = data.get("message", "").strip()
    if not message:
        return jsonify({"error": "Message is required"}), 400

    # Get chat history
    history_docs = list(
        db.chat_history.find({"user_id": user_id, "source_id": "__general__"})
        .sort("created_at", 1)
        .limit(20)
    )
    chat_history = [{"role": h["role"], "content": h["content"]} for h in history_docs]

    # Get relevant context across ALL sources
    context = vector_search.get_context_for_all_sources(user_id, message, n_results=50)

    # Get source titles
    user_sources = list(db.sources.find({"user_id": user_id, "status": "processed"}, {"title": 1}))
    source_titles = [s.get("title", "Untitled") for s in user_sources]

    if not context.strip():
        sources = list(db.sources.find({"user_id": user_id, "status": "processed"}, {"title": 1, "content": 1}).limit(10))
        parts = []
        for s in sources:
            content = s.get("content", "")
            if content:
                parts.append(f"--- {s.get('title', 'Untitled')} ---\n{content[:2000]}\n")
        context = "\n".join(parts) if parts else "No sources available."

    system_prompt = _build_general_system_prompt(context, source_titles)

    # Save user message immediately
    now = datetime.utcnow()
    db.chat_history.insert_one({
        "source_id": "__general__",
        "user_id": user_id,
        "role": "user",
        "content": message,
        "created_at": now,
    })

    def generate():
        full_response = ""
        try:
            for chunk in gemini_service.chat_with_context_stream_custom(
                user_message=message,
                system_prompt=system_prompt,
                chat_history=chat_history,
            ):
                full_response += chunk
                yield f"data: {json.dumps({'chunk': chunk})}\n\n"

            # Save assistant response to history
            db.chat_history.insert_one({
                "source_id": "__general__",
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


@general_chat_bp.route("/history", methods=["GET"])
@jwt_required()
def get_general_chat_history():
    """Get the general chat history for the user."""
    user_id = get_jwt_identity()
    db = get_db()

    history_docs = list(
        db.chat_history.find({"source_id": "__general__", "user_id": user_id})
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


@general_chat_bp.route("/history", methods=["DELETE"])
@jwt_required()
def clear_general_chat_history():
    """Clear all general chat history for the user."""
    user_id = get_jwt_identity()
    db = get_db()

    result = db.chat_history.delete_many({"source_id": "__general__", "user_id": user_id})

    return jsonify({"message": f"Deleted {result.deleted_count} messages"}), 200
