"""
Profile Routes - View and update user profile.
"""

from flask import Blueprint, request, jsonify
from flask_jwt_extended import jwt_required, get_jwt_identity
from bson import ObjectId
from datetime import datetime
from src.db import get_db

profile_bp = Blueprint("profile", __name__, url_prefix="/api/profile")


@profile_bp.route("", methods=["GET"])
@jwt_required()
def get_profile():
    """Get the authenticated user's profile."""
    user_id = get_jwt_identity()
    db = get_db()

    user = db.users.find_one({"_id": ObjectId(user_id)})
    if not user:
        return jsonify({"error": "User not found"}), 404

    # Get stats
    source_count = db.sources.count_documents({"user_id": user_id})

    return jsonify({
        "id": str(user["_id"]),
        "name": user.get("name", ""),
        "email": user.get("email", ""),
        "plan": user.get("plan", "free"),
        "source_count": source_count,
        "created_at": user["created_at"].isoformat() if user.get("created_at") else None,
    }), 200


@profile_bp.route("", methods=["PUT"])
@jwt_required()
def update_profile():
    """Update the authenticated user's profile (name, email)."""
    user_id = get_jwt_identity()
    data = request.get_json()
    db = get_db()

    update_fields = {}
    if "name" in data:
        name = data["name"].strip()
        if not name:
            return jsonify({"error": "Name cannot be empty"}), 400
        update_fields["name"] = name

    if "email" in data:
        email = data["email"].strip().lower()
        if not email:
            return jsonify({"error": "Email cannot be empty"}), 400
        # Check uniqueness
        existing = db.users.find_one({"email": email, "_id": {"$ne": ObjectId(user_id)}})
        if existing:
            return jsonify({"error": "Email already in use"}), 409
        update_fields["email"] = email

    if not update_fields:
        return jsonify({"error": "No fields to update"}), 400

    update_fields["updated_at"] = datetime.utcnow()
    db.users.update_one({"_id": ObjectId(user_id)}, {"$set": update_fields})

    return jsonify({"message": "Profile updated"}), 200
