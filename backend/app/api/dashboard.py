from fastapi import APIRouter, Depends
from app.api.deps import get_current_active_user, get_database
from app.schemas.schemas import User
from datetime import datetime, timedelta

router = APIRouter()

@router.get("/")
async def get_dashboard_stats(current_user: User = Depends(get_current_active_user), db=Depends(get_database)):
    """
    Retrieve dashboard statistics for the current user.
    """
    # Count recordings
    recordings_count = await db.recordings.count_documents({"user_id": current_user.id})
    
    # Count devices
    devices_count = await db.devices.count_documents({"owner_id": current_user.id})
    
    # Get recent recordings (last 5)
    cursor = db.recordings.find({"user_id": current_user.id}).sort("created_at", -1).limit(5)
    recent_recordings = []
    async for doc in cursor:
        doc["id"] = str(doc["_id"])
        recent_recordings.append(doc)
    
    # Calculate transcribed hours (mock for now, or sum duration)
    pipeline = [
        {"$match": {"user_id": current_user.id}},
        {"$group": {"_id": None, "total_duration": {"$sum": "$duration_seconds"}}}
    ]
    result = await db.recordings.aggregate(pipeline).to_list(length=1)
    total_seconds = result[0]["total_duration"] if result else 0
    transcribed_hours = round(total_seconds / 3600, 1)

    return {
        "total_recordings": recordings_count,
        "active_devices": devices_count,
        "transcribed_hours": transcribed_hours,
        "recent_activity": recent_recordings
    }
