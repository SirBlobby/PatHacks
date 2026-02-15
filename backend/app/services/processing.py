from app.core import database
from app.api.deps import get_database
from app.schemas.schemas import Recording
from app.services import transcription, summary, vector
from datetime import datetime
from bson import ObjectId
import asyncio

async def run_processing_pipeline(recording_id: str, file_path: str):
    """
    Background task to process audio: transcribe -> embed -> summarize.
    """
    db = database.db_instance.db
    if db is None:
        print("Database not connected in background task!")
        return

    try:
        obj_id = ObjectId(recording_id)
        
        # Update status to processing
        await db.recordings.update_one(
            {"_id": obj_id},
            {"$set": {"status": "transcribing"}}
        )

        # 1. Transcribe
        print(f"Starting transcription for {recording_id}...")
        transcript_result = await transcription.transcribe_audio(file_path)
        full_text = transcript_result["text"]
        segments = transcript_result["segments"]
        duration = transcript_result["duration"]
        
        # Save transcript
        await db.transcripts.insert_one({
            "recording_id": recording_id,
            "full_text": full_text,
            "segments": segments,
            "created_at": datetime.now()
        })
        
        # Update duration and status
        await db.recordings.update_one(
            {"_id": obj_id},
            {"$set": {"duration_seconds": duration, "status": "indexing"}}
        )

        # 2. Vector Indexing
        print(f"Starting vector indexing for {recording_id}...")
        text_chunks = await vector.chunk_text(full_text)
        chunk_docs = []
        
        for i, chunk in enumerate(text_chunks):
            embedding = await vector.generate_embedding(chunk)
            chunk_docs.append({
                "recording_id": recording_id,
                "text": chunk,
                "vector": embedding,
                "index": i,
                "created_at": datetime.now()
            })
            
        if chunk_docs:
            await db.transcript_chunks.insert_many(chunk_docs)

        # 3. Summarization
        print(f"Starting summarization for {recording_id}...")
        await db.recordings.update_one(
            {"_id": obj_id},
            {"$set": {"status": "summarizing"}}
        )
        
        summary_result = await summary.generate_summary(full_text)
        
        await db.summaries.insert_one({
            "recording_id": recording_id,
            "short_summary": summary_result.get("short_summary", ""),
            "detailed_summary": summary_result.get("detailed_summary", ""),
            "key_points": summary_result.get("key_points", []),
            "created_at": datetime.now()
        })
        
        # Finalize
        await db.recordings.update_one(
            {"_id": obj_id},
            {"$set": {"status": "ready"}}
        )
        print(f"Processing complete for {recording_id}")

    except Exception as e:
        print(f"Error processing recording {recording_id}: {e}")
        # Mark as failed
        try:
            obj_id = ObjectId(recording_id)
            await db.recordings.update_one(
                {"_id": obj_id},
                {"$set": {"status": "failed", "error": str(e)}}
            )
        except:
            pass
