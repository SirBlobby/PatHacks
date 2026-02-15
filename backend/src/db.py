"""
LearningBuddy Backend - MongoDB connection manager.
"""

from pymongo import MongoClient
from pymongo.database import Database
from src.config import Config


_client: MongoClient | None = None
_db: Database | None = None


def get_db() -> Database:
    """Get the MongoDB database instance. Creates connection on first call."""
    global _client, _db
    if _db is None:
        _client = MongoClient(Config.MONGODB_URI)
        _db = _client[Config.MONGODB_DB_NAME]
    return _db


def close_db():
    """Close the MongoDB connection."""
    global _client, _db
    if _client:
        _client.close()
        _client = None
        _db = None


def init_db():
    """
    Initialize database collections and indexes.
    Call once at application startup.
    """
    db = get_db()

    # ── Users ──
    if "users" not in db.list_collection_names():
        db.create_collection("users")
    db.users.create_index("email", unique=True)

    # ── Devices ──
    if "devices" not in db.list_collection_names():
        db.create_collection("devices")
    db.devices.create_index("user_id")
    db.devices.create_index("serial_number", unique=True)

    # ── Sources ──
    if "sources" not in db.list_collection_names():
        db.create_collection("sources")
    db.sources.create_index("user_id")
    db.sources.create_index("device_id")

    # ── Chat History ──
    if "chat_history" not in db.list_collection_names():
        db.create_collection("chat_history")
    db.chat_history.create_index("source_id")
    db.chat_history.create_index("user_id")

    # ── Lecture Chunks (for vector search) ──
    if "lecture_chunks" not in db.list_collection_names():
        db.create_collection("lecture_chunks")
    db.lecture_chunks.create_index("source_id")

    print("[DB] MongoDB initialized with collections and indexes.")
