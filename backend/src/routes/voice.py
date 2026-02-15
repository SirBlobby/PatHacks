"""
Voice Routes - ElevenLabs Conversational AI integration.
Provides signed URLs for real-time voice conversations with source context.
"""

import requests
from flask import Blueprint, jsonify
from flask_jwt_extended import jwt_required, get_jwt_identity
from bson import ObjectId
from src.db import get_db
from src.config import Config
from src.services import vector_search

voice_bp = Blueprint("voice", __name__, url_prefix="/api/sources")


def _build_voice_system_prompt(context: str, source_title: str) -> str:
    """
    Build a system prompt for the ElevenLabs voice agent.
    Tailored for spoken conversation (shorter, more conversational).
    """
    return (
        "You are LearningBuddy AI, a friendly voice tutor helping a student study. "
        "You answer questions about their learning source based on the context provided below. "
        "Keep your answers concise and conversational since this is a spoken dialogue. "
        "Use simple language and short sentences that sound natural when spoken aloud. "
        "If the context doesn't contain enough information, say so honestly. "
        "Reference specific timestamps when available in the context. "
        "Never reference chunk numbers or indexes.\n\n"
        f"=== SOURCE: {source_title} ===\n"
        f"=== CONTEXT ===\n{context}\n=== END CONTEXT ==="
    )


@voice_bp.route("/<source_id>/voice/signed-url", methods=["GET"])
@jwt_required()
def get_voice_signed_url(source_id):
    """
    Generate a signed URL for an ElevenLabs voice conversation.
    Fetches RAG context for the source and returns it alongside the signed URL
    so the frontend can pass it as a system prompt override.
    """
    user_id = get_jwt_identity()
    db = get_db()

    # Validate config
    if not Config.ELEVENLABS_API_KEY or not Config.ELEVENLABS_AGENT_ID:
        return jsonify({
            "error": "ElevenLabs is not configured. Set ELEVENLABS_API_KEY and ELEVENLABS_AGENT_ID in .env"
        }), 503

    # Verify source exists and belongs to user
    try:
        source = db.sources.find_one({"_id": ObjectId(source_id), "user_id": user_id})
    except Exception:
        return jsonify({"error": "Invalid source ID"}), 400

    if not source:
        return jsonify({"error": "Source not found"}), 404

    # Get RAG context for this source (broad retrieval for voice conversation)
    # Use a generic query to get a representative sample of the source content
    context = vector_search.get_context_for_query(
        source_id, source.get("title", "overview summary"), n_results=30
    )

    # If no vector chunks exist, fall back to raw content
    if not context.strip():
        content = source.get("content", "")
        context = content[:6000] if content else "No content available."

    # Build the system prompt with source context
    system_prompt = _build_voice_system_prompt(context, source.get("title", ""))

    # Request a signed URL from ElevenLabs
    try:
        resp = requests.get(
            "https://api.elevenlabs.io/v1/convai/conversation/get-signed-url",
            params={"agent_id": Config.ELEVENLABS_AGENT_ID},
            headers={"xi-api-key": Config.ELEVENLABS_API_KEY},
            timeout=10,
        )
        resp.raise_for_status()
        signed_url = resp.json().get("signed_url")
    except requests.exceptions.RequestException as e:
        return jsonify({"error": f"Failed to get signed URL from ElevenLabs: {str(e)}"}), 502

    if not signed_url:
        return jsonify({"error": "ElevenLabs returned an empty signed URL"}), 502

    return jsonify({
        "signed_url": signed_url,
        "system_prompt": system_prompt,
        "source_title": source.get("title", ""),
        "agent_id": Config.ELEVENLABS_AGENT_ID,
    }), 200
