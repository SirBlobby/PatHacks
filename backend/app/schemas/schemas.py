from pydantic import BaseModel, Field
from datetime import datetime
from typing import Optional, List

# User Schemas
class UserBase(BaseModel):
    email: str
    full_name: Optional[str] = None

class UserCreate(UserBase):
    password: str

class UserInDB(UserBase):
    id: str  # MongoDB ObjectId as string
    hashed_password: str
    created_at: datetime = datetime.now()

class User(UserBase):
    id: str
    is_active: bool = True
    
    class Config:
        from_attributes = True

# Device Schemas
class DeviceBase(BaseModel):
    name: str = "My Device"
    description: Optional[str] = None

class DeviceCreate(DeviceBase):
    pass

class Device(DeviceBase):
    id: str
    owner_id: str
    status: str = "offline"
    last_sync: Optional[datetime] = None
    created_at: datetime = datetime.now()

    class Config:
        from_attributes = True

# Recording Schemas
class RecordingBase(BaseModel):
    title: str = "Untitled Lecture"
    course_name: Optional[str] = None
    duration_seconds: Optional[int] = 0

class RecordingCreate(RecordingBase):
    device_id: Optional[str] = None # uploaded manually or from device

class Recording(RecordingBase):
    id: str
    user_id: str
    device_id: Optional[str] = None
    file_path: str
    status: str = "queued" # queued, processing, completed, failed
    created_at: datetime = datetime.now()

    class Config:
        from_attributes = True

# Transcript & Summary Schemas
class TranscriptChunk(BaseModel):
    start: float
    end: float
    text: str

class Transcript(BaseModel):
    recording_id: str
    full_text: str
    chunks: List[TranscriptChunk] = []
    language: str = "en"

class Summary(BaseModel):
    recording_id: str
    short_summary: str
    detailed_summary: str
    key_points: List[str] = []

# Auth Token Schema
class Token(BaseModel):
    access_token: str
    token_type: str

class TokenData(BaseModel):
    username: Optional[str] = None
