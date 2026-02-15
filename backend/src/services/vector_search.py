"""
LearningBuddy Backend - MongoDB Atlas Vector Search Service.
Handles chunking documents, storing embeddings, and querying via Atlas Vector Search.
"""

from src.db import get_db
from src.config import Config
from src.services.gemini_service import generate_embedding, generate_embeddings_batch


def chunk_text(text: str, chunk_size: int | None = None, overlap: int | None = None) -> list[str]:
    """
    Split text into overlapping chunks.
    Tries to split on sentence boundaries for cleaner chunks.
    """
    chunk_size = chunk_size or Config.CHUNK_SIZE
    overlap = overlap or Config.CHUNK_OVERLAP

    if len(text) <= chunk_size:
        return [text]

    chunks = []
    start = 0
    while start < len(text):
        end = start + chunk_size

        # Try to break at a sentence boundary
        if end < len(text):
            for sep in [". ", ".\n", "? ", "!\n", "! ", "?\n", "\n\n"]:
                last_sep = text[start:end].rfind(sep)
                if last_sep > chunk_size // 2:
                    end = start + last_sep + len(sep)
                    break
        else:
            # We reached the end of the text
            chunks.append(text[start:].strip())
            break

        chunks.append(text[start:end].strip())
        start = end - overlap

    return [c for c in chunks if c]


def process_and_store_embeddings(source_id: str, text: str, title: str = "") -> int:
    """
    Chunk text, generate embeddings, and store in MongoDB.
    Returns the number of chunks created.
    """
    db = get_db()
    chunks = chunk_text(text)

    if not chunks:
        return 0

    # Generate embeddings in batches of 20
    batch_size = 20
    all_docs = []

    for i in range(0, len(chunks), batch_size):
        batch = chunks[i : i + batch_size]
        embeddings = generate_embeddings_batch(batch)

        for j, (chunk, embedding) in enumerate(zip(batch, embeddings)):
            all_docs.append({
                "source_id": source_id,
                "chunk_index": i + j,
                "content": chunk,
                "embedding": embedding,
                "title": title,
            })

    if all_docs:
        db.lecture_chunks.insert_many(all_docs)

    return len(all_docs)


def search_similar_chunks(
    source_id: str,
    query: str,
    n_results: int = 5,
) -> list[dict]:
    """
    Perform a vector search for chunks similar to the query.
    Uses MongoDB Atlas Vector Search ($vectorSearch aggregation stage).
    """
    db = get_db()
    query_embedding = generate_embedding(query)

    pipeline = [
        {
            "$vectorSearch": {
                "index": Config.VECTOR_INDEX_NAME,
                "path": "embedding",
                "queryVector": query_embedding,
                "numCandidates": n_results * 10,
                "limit": n_results,
                "filter": {"source_id": source_id},
            }
        },
        {
            "$project": {
                "_id": 0,
                "content": 1,
                "chunk_index": 1,
                "title": 1,
                "score": {"$meta": "vectorSearchScore"},
            }
        },
    ]

    results = list(db.lecture_chunks.aggregate(pipeline))
    return results


def get_context_for_query(
    source_id: str,
    query: str,
    n_results: int = 5,
) -> str:
    """
    Retrieve relevant context for a user query.
    Returns a formatted string of the most relevant chunks.
    """
    results = search_similar_chunks(source_id, query, n_results)

    if not results:
        # Fallback 1: simple keyword search using regex
        db = get_db()
        search_terms = [t for t in query.split() if len(t) > 3]
        
        if search_terms:
            keyword_filter = {
                "source_id": source_id,
                "$or": [
                    {"content": {"$regex": term, "$options": "i"}} 
                    for term in search_terms
                ]
            }
            results = list(
                db.lecture_chunks.find(
                    keyword_filter,
                    {"_id": 0, "content": 1, "chunk_index": 1}
                ).limit(n_results)
            )

        # Fallback 2: sequential chunks if keyword search fails
        if not results:
            results = list(
                db.lecture_chunks.find(
                    {"source_id": source_id},
                    {"_id": 0, "content": 1, "chunk_index": 1},
                )
                .sort("chunk_index", 1)
                .limit(n_results)
            )

    context_parts = []
    for i, r in enumerate(results, 1):
        context_parts.append(
            f"--- Chunk {i} (index: {r.get('chunk_index', '?')}) ---\n"
            f"{r['content']}\n"
        )

    return "\n".join(context_parts)


def delete_source_chunks(source_id: str) -> int:
    """Delete all vector chunks for a source. Returns count deleted."""
    db = get_db()
    result = db.lecture_chunks.delete_many({"source_id": source_id})
    return result.deleted_count
