"""
Auth Routes - Register, Login, Token Refresh
"""

from flask import Blueprint, request, jsonify
from flask_jwt_extended import (
    create_access_token,
    jwt_required,
    get_jwt_identity,
)
import bcrypt
from datetime import datetime
from src.db import get_db

auth_bp = Blueprint("auth", __name__, url_prefix="/api/auth")


@auth_bp.route("/register", methods=["POST"])
def register():
    data = request.get_json()

    name = data.get("name", "").strip()
    email = data.get("email", "").strip().lower()
    password = data.get("password", "")

    if not name or not email or not password:
        return jsonify({"error": "Name, email, and password are required"}), 400

    if len(password) < 6:
        return jsonify({"error": "Password must be at least 6 characters"}), 400

    db = get_db()

    # Check if email already exists
    if db.users.find_one({"email": email}):
        return jsonify({"error": "Email already registered"}), 409

    hashed = bcrypt.hashpw(password.encode("utf-8"), bcrypt.gensalt())

    user_doc = {
        "name": name,
        "email": email,
        "password": hashed.decode("utf-8"),
        "plan": "free",
        "created_at": datetime.utcnow(),
        "settings": {
            "theme": "dark",
        },
    }

    result = db.users.insert_one(user_doc)
    user_id = str(result.inserted_id)

    access_token = create_access_token(identity=user_id)

    return jsonify({
        "message": "Account created successfully",
        "access_token": access_token,
        "user": {
            "id": user_id,
            "name": name,
            "email": email,
            "plan": "free",
        },
    }), 201


@auth_bp.route("/login", methods=["POST"])
def login():
    data = request.get_json()

    email = data.get("email", "").strip().lower()
    password = data.get("password", "")

    if not email or not password:
        return jsonify({"error": "Email and password are required"}), 400

    db = get_db()
    user = db.users.find_one({"email": email})

    if not user:
        return jsonify({"error": "Invalid email or password"}), 401

    if not bcrypt.checkpw(password.encode("utf-8"), user["password"].encode("utf-8")):
        return jsonify({"error": "Invalid email or password"}), 401

    user_id = str(user["_id"])
    access_token = create_access_token(identity=user_id)

    return jsonify({
        "access_token": access_token,
        "user": {
            "id": user_id,
            "name": user["name"],
            "email": user["email"],
            "plan": user.get("plan", "free"),
        },
    }), 200


@auth_bp.route("/me", methods=["GET"])
@jwt_required()
def me():
    """Get the currently authenticated user's info."""
    user_id = get_jwt_identity()
    db = get_db()

    from bson import ObjectId
    user = db.users.find_one({"_id": ObjectId(user_id)})

    if not user:
        return jsonify({"error": "User not found"}), 404

    return jsonify({
        "id": str(user["_id"]),
        "name": user["name"],
        "email": user["email"],
        "plan": user.get("plan", "free"),
        "settings": user.get("settings", {}),
        "created_at": user["created_at"].isoformat() if user.get("created_at") else None,
    }), 200
