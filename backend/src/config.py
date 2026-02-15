"""
LearningBuddy Backend - Configuration
"""

import os
from pathlib import Path
from dotenv import load_dotenv

env_path = Path(__file__).parent.parent / ".env"
load_dotenv(dotenv_path=env_path)


class Config:
    SECRET_KEY = os.getenv("FLASK_SECRET_KEY", "dev-secret")
    JWT_SECRET_KEY = os.getenv("JWT_SECRET_KEY", "dev-jwt-secret")
    JWT_ACCESS_TOKEN_EXPIRES = 86400  # 24 hours in seconds

    GEMINI_API_KEY = os.getenv("GEMINI_API_KEY", "")

    MONGODB_URI = os.getenv("MONGODB_URI", "")
    MONGODB_DB_NAME = os.getenv("MONGODB_DB_NAME", "learningbuddy")

    # Path to the built frontend (SvelteKit outputs to backend/build/)
    FRONTEND_BUILD_DIR = os.getenv(
        "FRONTEND_BUILD_DIR",
        str(Path(__file__).parent.parent / "build"),
    )

    UPLOAD_FOLDER = str(Path(__file__).parent.parent / "uploads")
    MAX_CONTENT_LENGTH = 500 * 1024 * 1024  # 500 MB max upload

    # Gemini model config
    GEMINI_CHAT_MODEL = "gemini-3-pro-preview"
    GEMINI_EMBEDDING_MODEL = "gemini-embedding-001"

    # Vector search config
    VECTOR_INDEX_NAME = "lecture_vector_index"
    EMBEDDING_DIMENSIONS = 3072
    CHUNK_SIZE = 1000
    CHUNK_OVERLAP = 200
