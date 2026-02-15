import os
from pydantic_settings import BaseSettings, SettingsConfigDict

class Settings(BaseSettings):
    """
    Application settings configuration using Pydantic BaseSettings.
    Loads values from environment variables or .env file.
    """
    
    # Core Application Settings
    PROJECT_NAME: str = "Learning Buddy Backend"
    API_V1_STR: str = "/api/v1"
    SECRET_KEY: str = "CHANGE_THIS_TO_A_SECURE_SECRET_KEY"  # Replace in production!
    ALGORITHM: str = "HS256"
    ACCESS_TOKEN_EXPIRE_MINUTES: int = 60 * 24 * 7  # 1 week

    # Database Settings (MongoDB)
    # Default to local mongo if not provided
    MONGODB_URL: str = "mongodb://localhost:27017"
    DATABASE_NAME: str = "learning_buddy_db"

    # AI & Service API Keys
    GOOGLE_API_KEY: str = ""  # For Gemini API

    # File Storage
    UPLOAD_DIR: str = "uploads"
    TRANSCRIPT_DIR: str = "transcripts"
    
    # Faster Whisper Model Size
    WHISPER_MODEL_SIZE: str = "base" # larger models require more RAM/VRAM

    # Validates that variables are loaded from .env if present
    model_config = SettingsConfigDict(env_file=".env", case_sensitive=True)

settings = Settings()

# Ensure directories exist
os.makedirs(settings.UPLOAD_DIR, exist_ok=True)
os.makedirs(settings.TRANSCRIPT_DIR, exist_ok=True)
