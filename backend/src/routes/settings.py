"""
Settings Routes - User preferences.
"""

from flask import Blueprint, request, jsonify
from flask_jwt_extended import jwt_required, get_jwt_identity
from bson import ObjectId
from datetime import datetime
from src.db import get_db
import bcrypt

settings_bp = Blueprint("settings", __name__, url_prefix="/api/settings")


@settings_bp.route("", methods=["GET"])
@jwt_required()
def get_settings():
    """Get the authenticated user's settings."""
    user_id = get_jwt_identity()
    db = get_db()

    user = db.users.find_one({"_id": ObjectId(user_id)})
    if not user:
        return jsonify({"error": "User not found"}), 404

    return jsonify(user.get("settings", {
        "theme": "dark",
    })), 200


@settings_bp.route("", methods=["PUT"])
@jwt_required()
def update_settings():
    """Update user settings (theme, notifications, etc.)."""
    user_id = get_jwt_identity()
    data = request.get_json()
    db = get_db()

    settings_update = {}
    if "theme" in data:
        settings_update["settings.theme"] = data["theme"]

    if not settings_update:
        return jsonify({"error": "No valid settings to update"}), 400

    settings_update["updated_at"] = datetime.utcnow()
    db.users.update_one({"_id": ObjectId(user_id)}, {"$set": settings_update})

    return jsonify({"message": "Settings updated"}), 200


@settings_bp.route("/change-password", methods=["POST"])
@jwt_required()
def change_password():
    """Change the user's password."""
    user_id = get_jwt_identity()
    data = request.get_json()
    db = get_db()

    current_password = data.get("current_password", "")
    new_password = data.get("new_password", "")

    if not current_password or not new_password:
        return jsonify({"error": "Current and new passwords are required"}), 400

    if len(new_password) < 6:
        return jsonify({"error": "New password must be at least 6 characters"}), 400

    user = db.users.find_one({"_id": ObjectId(user_id)})
    if not user:
        return jsonify({"error": "User not found"}), 404

    if not bcrypt.checkpw(current_password.encode("utf-8"), user["password"].encode("utf-8")):
        return jsonify({"error": "Current password is incorrect"}), 401

    hashed = bcrypt.hashpw(new_password.encode("utf-8"), bcrypt.gensalt())
    db.users.update_one(
        {"_id": ObjectId(user_id)},
        {"$set": {"password": hashed.decode("utf-8"), "updated_at": datetime.utcnow()}},
    )

    return jsonify({"message": "Password changed successfully"}), 200


@settings_bp.route("/delete-account", methods=["DELETE"])
@jwt_required()
def delete_account():
    """Permanently delete the user's account and all associated data."""
    user_id = get_jwt_identity()
    db = get_db()

    # Delete all user data
    db.chat_history.delete_many({"user_id": user_id})
    db.lecture_chunks.delete_many({"source_id": {"$in": [
        str(r["_id"]) for r in db.sources.find({"user_id": user_id}, {"_id": 1})
    ]}})
    db.sources.delete_many({"user_id": user_id})
    db.devices.delete_many({"user_id": user_id})
    db.users.delete_one({"_id": ObjectId(user_id)})

    return jsonify({"message": "Account deleted"}), 200
