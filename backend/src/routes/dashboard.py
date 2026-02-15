"""
Dashboard Routes - Aggregate stats for the user's dashboard.
"""

from flask import Blueprint, jsonify
from flask_jwt_extended import jwt_required, get_jwt_identity
from src.db import get_db

dashboard_bp = Blueprint("dashboard", __name__, url_prefix="/api/dashboard")


@dashboard_bp.route("", methods=["GET"])
@jwt_required()
def get_dashboard():
    """Return dashboard summary stats for the authenticated user."""
    user_id = get_jwt_identity()
    db = get_db()

    # Count sources
    total_sources = db.sources.count_documents({"user_id": user_id})

    # Sum total duration (hours) — applicable to audio/video sources
    duration_pipeline = [
        {"$match": {"user_id": user_id}},
        {"$group": {"_id": None, "total_seconds": {"$sum": "$duration_seconds"}}},
    ]
    duration_result = list(db.sources.aggregate(duration_pipeline))
    total_seconds = duration_result[0]["total_seconds"] if duration_result else 0
    hours_processed = round(total_seconds / 3600, 1)

    # Count devices
    active_devices = db.devices.count_documents({"user_id": user_id})
    online_devices = db.devices.count_documents({"user_id": user_id, "status": "online"})

    # Recent activity (last 5 sources)
    recent_sources = list(
        db.sources.find({"user_id": user_id})
        .sort("created_at", -1)
        .limit(5)
    )

    recent_activity = []
    for src in recent_sources:
        recent_activity.append({
            "id": str(src["_id"]),
            "title": src.get("title", "Untitled"),
            "source_type": src.get("source_type", "text"),
            "file_type": src.get("file_type", "text"),
            "created_at": src["created_at"].isoformat() if src.get("created_at") else None,
            "status": src.get("status", "processing"),
            "duration_seconds": src.get("duration_seconds", 0),
        })

    return jsonify({
        "total_sources": total_sources,
        "hours_processed": hours_processed,
        "active_devices": active_devices,
        "online_devices": online_devices,
        "recent_activity": recent_activity,
    }), 200
