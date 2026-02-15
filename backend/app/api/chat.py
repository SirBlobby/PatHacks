from fastapi import APIRouter, Depends, HTTPException
from app.api.deps import get_current_active_user, get_database
from app.services import vector, summary
from app.schemas.schemas import User
from bson import ObjectId
import numpy as np

router = APIRouter()

def cosine_similarity(v1, v2):
    return np.dot(v1, v2) / (np.linalg.norm(v1) * np.linalg.norm(v2))

@router.post("/")
async def chat_with_recording(
    recording_id: str,
    question: str,
    current_user: User = Depends(get_current_active_user),
    db=Depends(get_database)
):
    """
    Simulate RAG chat with a specific recording.
    """
    try:
        obj_id = ObjectId(recording_id)
    except:
        raise HTTPException(status_code=400, detail="Invalid ID format")

    recording = await db.recordings.find_one({"_id": obj_id, "user_id": current_user.id})
    if not recording:
        raise HTTPException(status_code=404, detail="Recording not found")
        
    # Get transcript chunks from DB (assuming they are stored in 'transcript_chunks' collection)
    chunks_cursor = db.transcript_chunks.find({"recording_id": recording_id})
    chunks = await chunks_cursor.to_list(length=1000)
    
    if not chunks:
        # Fallback if no chunks: use full transcript or summary
        # For now, return generic response
        return {"answer": "Transcript not processed yet.", "sources": []}
        
    # Generate embedding for question
    q_embedding = await vector.generate_embedding(question)
    
    # Find most relevant chunks (simple linear search for demo)
    scored_chunks = []
    for chunk in chunks:
        if "vector" in chunk and chunk["vector"]:
            score = cosine_similarity(q_embedding, chunk["vector"])
            scored_chunks.append((score, chunk))
            
    scored_chunks.sort(key=lambda x: x[0], reverse=True)
    top_chunks = scored_chunks[:3]
    
    context_text = "\n\n".join([c[1]["text"] for c in top_chunks])
    
    # Generate answer using Gemini
    import google.generativeai as genai
    model = genai.GenerativeModel('gemini-pro')
    
    prompt = f"""
    Answer the following question based on the provided context.
    
    Context:
    {context_text}
    
    Question: {question}
    
    Answer:
    """
    
    response = await model.generate_content_async(prompt)
    
    return {
        "answer": response.text,
        "sources": [c[1]["start_time"] for c in top_chunks if "start_time" in c[1]]
    }
