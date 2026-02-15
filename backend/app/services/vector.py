import google.generativeai as genai
from app.core.config import settings
from typing import List

if settings.GOOGLE_API_KEY:
    genai.configure(api_key=settings.GOOGLE_API_KEY)

async def generate_embedding(text: str) -> List[float]:
    """
    Generate embedding for a given text using Google's embedding model.
    """
    if not settings.GOOGLE_API_KEY:
        return []
    
    try:
        result = await genai.embed_content_async(
            model="models/embedding-001",
            content=text,
            task_type="retrieval_document",
            title="Embedding of transcript chunk" # Optional
        )
        return result['embedding']
    except Exception as e:
        print(f"Error generating embedding: {e}")
        return []

async def chunk_text(text: str, chunk_size: int = 1000, overlap: int = 100) -> List[str]:
    """
    Split text into overlapping chunks.
    """
    chunks = []
    start = 0
    while start < len(text):
        end = start + chunk_size
        chunk = text[start:end]
        chunks.append(chunk)
        start += (chunk_size - overlap)
    return chunks
