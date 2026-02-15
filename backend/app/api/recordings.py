from fastapi import APIRouter, Depends, HTTPException, UploadFile, File, BackgroundTasks
from app.api.deps import get_current_active_user, get_database
from app.schemas.schemas import Recording, User
from app.core.config import settings
from app.services.processing import run_processing_pipeline
from datetime import datetime
from uuid import uuid4
from bson import ObjectId
import os
import shutil

router = APIRouter()

@router.post("/upload", response_model=Recording)
async def upload_recording(
    background_tasks: BackgroundTasks,
    file: UploadFile = File(...),
    title: str = "Untitled Lecture",
    current_user: User = Depends(get_current_active_user),
    db=Depends(get_database)
):
    """
    Upload a new audio recording.
    """
    os.makedirs(settings.UPLOAD_DIR, exist_ok=True)
    
    file_ext = os.path.splitext(file.filename)[1]
    unique_filename = f"{uuid4()}{file_ext}"
    file_path = os.path.join(settings.UPLOAD_DIR, unique_filename)
    
    with open(file_path, "wb") as buffer:
        shutil.copyfileobj(file.file, buffer)
        
    new_recording_doc = {
        "title": title,
        "user_id": current_user.id,
        "filename": unique_filename,
        "original_filename": file.filename,
        "file_path": file_path,
        "status": "queued",
        "created_at": datetime.now(),
        "duration_seconds": 0,
    }
    
    result = await db.recordings.insert_one(new_recording_doc)
    recording_id = str(result.inserted_id)
    
    # Trigger background processing
    background_tasks.add_task(run_processing_pipeline, recording_id, file_path)
    
    # Return Pydantic-compatible dict
    response_recording = {
        "id": recording_id,
        "title": title,
        "user_id": current_user.id,
        "device_id": None,
        "file_path": file_path,
        "status": "queued",
        "created_at": new_recording_doc["created_at"],
        "duration_seconds": 0,
        "course_name": None
    }
    
    return response_recording

@router.get("/", response_model=list[Recording])
async def list_recordings(
    skip: int = 0,
    limit: int = 100,
    current_user: User = Depends(get_current_active_user),
    db=Depends(get_database)
):
    cursor = db.recordings.find({"user_id": current_user.id}).sort("created_at", -1).skip(skip).limit(limit)
    recordings = []
    async for doc in cursor:
        doc["id"] = str(doc["_id"])
        recordings.append(doc)
    return recordings

@router.get("/{recording_id}", response_model=Recording)
async def get_recording(
    recording_id: str,
    current_user: User = Depends(get_current_active_user),
    db=Depends(get_database)
):
    try:
        obj_id = ObjectId(recording_id)
    except:
        raise HTTPException(status_code=400, detail="Invalid ID format")

    recording = await db.recordings.find_one({"_id": obj_id, "user_id": current_user.id})
    if not recording:
        raise HTTPException(status_code=404, detail="Recording not found")
        
    recording["id"] = str(recording["_id"])
    return recording

@router.delete("/{recording_id}")
async def delete_recording(
    recording_id: str,
    current_user: User = Depends(get_current_active_user),
    db=Depends(get_database)
):
    try:
        obj_id = ObjectId(recording_id)
    except:
        raise HTTPException(status_code=400, detail="Invalid ID format")

    recording = await db.recordings.find_one({"_id": obj_id, "user_id": current_user.id})
    if not recording:
        raise HTTPException(status_code=404, detail="Recording not found")
    
    if os.path.exists(recording["file_path"]):
        os.remove(recording["file_path"])
        
    await db.recordings.delete_one({"_id": obj_id})
    
    return {"status": "success", "message": "Recording deleted"}
