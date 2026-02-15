from fastapi import APIRouter, Depends, HTTPException, UploadFile, File, BackgroundTasks, Header
from app.api.deps import get_current_device, get_database
from app.schemas.schemas import Device
from app.core.config import settings
from app.services.processing import run_processing_pipeline
from datetime import datetime
from uuid import uuid4
from bson import ObjectId
import os
import shutil

router = APIRouter()

@router.post("/upload")
async def device_upload_recording(
    background_tasks: BackgroundTasks,
    file: UploadFile = File(...),
    title: str = "Device Recording",
    course_name: str = None,
    device: Device = Depends(get_current_device),
    db=Depends(get_database)
):
    """
    Upload a recording from a device.
    """
    os.makedirs(settings.UPLOAD_DIR, exist_ok=True)
    
    file_ext = os.path.splitext(file.filename)[1]
    unique_filename = f"{uuid4()}{file_ext}"
    file_path = os.path.join(settings.UPLOAD_DIR, unique_filename)
    
    with open(file_path, "wb") as buffer:
        shutil.copyfileobj(file.file, buffer)
        
    new_recording_doc = {
        "title": title,
        "course_name": course_name,
        "user_id": device.owner_id,
        "device_id": device.id,
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
    
    return {
        "id": recording_id,
        "status": "queued"
    }

@router.post("/heartbeat")
async def device_heartbeat(
    device: Device = Depends(get_current_device),
    db=Depends(get_database)
):
    """
    Update device last sync time.
    """
    obj_id = ObjectId(device.id)
    await db.devices.update_one(
        {"_id": obj_id},
        {"$set": {"last_sync": datetime.now(), "status": "online"}}
    )
    return {"status": "ok"}
