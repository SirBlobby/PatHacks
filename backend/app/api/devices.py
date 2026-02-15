from uuid import uuid4
from datetime import datetime
from fastapi import APIRouter, Depends, HTTPException, status
from app.api.deps import get_current_active_user, get_database
from app.schemas.schemas import DeviceCreate, Device, User

router = APIRouter()

@router.post("/", response_model=Device)
async def create_device(
    device_in: DeviceCreate,
    current_user: User = Depends(get_current_active_user),
    db=Depends(get_database)
):
    """
    Register a new device for the current user.
    """
    new_device = {
        "name": device_in.name,
        "description": device_in.description,
        "owner_id": current_user.id,
        "status": "offline",
        "last_sync": None,
        "created_at": datetime.now(),
        # Can generate a pairing code or api key here later
        "api_key": str(uuid4())
    }
    
    result = await db.devices.insert_one(new_device)
    
    created_device = {
        "id": str(result.inserted_id),
        **new_device
    }
    return created_device

@router.get("/", response_model=list[Device])
async def read_devices(
    skip: int = 0,
    limit: int = 100,
    current_user: User = Depends(get_current_active_user),
    db=Depends(get_database)
):
    """
    Retrieve devices owned by the current user.
    """
    cursor = db.devices.find({"owner_id": current_user.id}).skip(skip).limit(limit)
    devices = []
    async for doc in cursor:
        doc["id"] = str(doc["_id"])
        devices.append(doc)
    return devices
