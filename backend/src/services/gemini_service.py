"""
LearningBuddy Backend - Gemini AI Service.
Handles embeddings generation and chat completions via Google Gemini.
"""

from google import genai
from google.genai import types
from src.config import Config
from typing import Generator


_client: genai.Client | None = None


def _get_client() -> genai.Client:
    """Lazy-init the Gemini client."""
    global _client
    if _client is None:
        _client = genai.Client(api_key=Config.GEMINI_API_KEY)
    return _client


def generate_embedding(text: str) -> list[float]:
    """Generate an embedding vector for the given text."""
    client = _get_client()
    result = client.models.embed_content(
        model=Config.GEMINI_EMBEDDING_MODEL,
        contents=text,
    )
    return result.embeddings[0].values


def generate_embeddings_batch(texts: list[str]) -> list[list[float]]:
    """Generate embeddings for a batch of texts."""
    client = _get_client()
    result = client.models.embed_content(
        model=Config.GEMINI_EMBEDDING_MODEL,
        contents=texts,
    )
    return [e.values for e in result.embeddings]


def chat_with_context(
    user_message: str,
    context: str,
    chat_history: list[dict] | None = None,
    source_title: str = "",
) -> str:
    """Send a message to Gemini with RAG context and return the full response."""
    client = _get_client()

    system_instruction = _build_system_prompt(context, source_title)

    contents = _build_contents(chat_history, user_message)

    response = client.models.generate_content(
        model=Config.GEMINI_CHAT_MODEL,
        contents=contents,
        config=types.GenerateContentConfig(
            system_instruction=system_instruction,
            temperature=0.7,
            max_output_tokens=2048,
        ),
    )

    return response.text


def chat_with_context_stream(
    user_message: str,
    context: str,
    chat_history: list[dict] | None = None,
    source_title: str = "",
) -> Generator[str, None, None]:
    """Stream a chat response from Gemini with RAG context."""
    client = _get_client()

    system_instruction = _build_system_prompt(context, source_title)
    contents = _build_contents(chat_history, user_message)

    response_stream = client.models.generate_content_stream(
        model=Config.GEMINI_CHAT_MODEL,
        contents=contents,
        config=types.GenerateContentConfig(
            system_instruction=system_instruction,
            temperature=0.7,
            max_output_tokens=2048,
        ),
    )

    for chunk in response_stream:
        if chunk.text:
            yield chunk.text


def summarize_transcript(transcript: str, title: str = "") -> str:
    """Generate a structured summary of a lecture transcript."""
    client = _get_client()

    prompt = (
        f"Please provide a comprehensive summary of the following source content.\n\n"
        f"Structure your response as:\n"
        f"## Key Takeaways\n- bullet points of main ideas\n\n"
        f"## Detailed Summary\nA thorough narrative summary.\n\n"
        f"## Topics Covered\n- list of topics\n\n"
        f"{'Title: ' + title if title else ''}\n\n"
        f"CONTENT:\n{transcript}"
    )

    response = client.models.generate_content(
        model=Config.GEMINI_CHAT_MODEL,
        contents=prompt,
        config=types.GenerateContentConfig(
            temperature=0.3,
            max_output_tokens=4096,
        ),
    )

    return response.text


# ── Private helpers ──

def _build_system_prompt(context: str, source_title: str) -> str:
    return (
        "You are LearningBuddy AI, a helpful assistant that answers questions about learning sources. "
        "You have access to the source content provided as context below. "
        "Always base your answers on the provided context. If the context doesn't contain "
        "enough information, say so honestly. Reference specific timestamps (e.g. 02:30) when available in the context, "
        "but NEVER reference chunk numbers (e.g. 'Chunk 1'). "
        "Be concise but thorough.\n\n"
        f"=== SOURCE: {source_title} ===\n"
        f"=== CONTEXT ===\n{context}\n=== END CONTEXT ==="
    )


def _build_contents(
    chat_history: list[dict] | None,
    user_message: str,
) -> list[types.Content]:
    contents = []
    if chat_history:
        for msg in chat_history[-10:]:
            role = "user" if msg["role"] == "user" else "model"
            contents.append(
                types.Content(role=role, parts=[types.Part.from_text(text=msg["content"])])
            )
    contents.append(
        types.Content(role="user", parts=[types.Part.from_text(text=user_message)])
    )
    return contents
