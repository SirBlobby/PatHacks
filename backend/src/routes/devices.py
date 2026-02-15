"""
Device Routes - CRUD for user devices.
"""

from flask import Blueprint, request, jsonify
from flask_jwt_extended import jwt_required, get_jwt_identity
from bson import ObjectId
from datetime import datetime
from src.db import get_db

devices_bp = Blueprint("devices", __name__, url_prefix="/api/devices")


@devices_bp.route("", methods=["GET"])
@jwt_required()
def list_devices():
    """List all devices for the authenticated user."""
    user_id = get_jwt_identity()
    db = get_db()

    devices = list(db.devices.find({"user_id": user_id}).sort("created_at", -1))

    result = []
    for d in devices:
        result.append({
            "id": str(d["_id"]),
            "name": d.get("name", "Unnamed Device"),
            "serial_number": d.get("serial_number", ""),
            "device_type": d.get("device_type", "Standard Recorder"),
            "status": d.get("status", "offline"),
            "battery": d.get("battery"),
            "last_seen": d["last_seen"].isoformat() if d.get("last_seen") else None,
            "created_at": d["created_at"].isoformat() if d.get("created_at") else None,
        })

    return jsonify(result), 200


@devices_bp.route("", methods=["POST"])
@jwt_required()
def register_device():
    """Register a new device."""
    user_id = get_jwt_identity()
    data = request.get_json()
    db = get_db()

    serial_number = data.get("serial_number", "").strip()
    name = data.get("name", "").strip()
    device_type = data.get("device_type", "Standard Recorder")

    if not serial_number:
        return jsonify({"error": "Serial number is required"}), 400
    if not name:
        return jsonify({"error": "Device name is required"}), 400

    # Check if serial number already registered
    if db.devices.find_one({"serial_number": serial_number}):
        return jsonify({"error": "Serial number already registered"}), 409

    # Check plan device limit
    user = db.users.find_one({"_id": ObjectId(user_id)})
    plan = user.get("plan", "free") if user else "free"
    device_count = db.devices.count_documents({"user_id": user_id})
    max_devices = 1 if plan == "free" else 5

    if device_count >= max_devices:
        return jsonify({
            "error": f"Device limit reached ({max_devices}). Upgrade your plan for more devices."
        }), 403

    now = datetime.utcnow()
    device_doc = {
        "user_id": user_id,
        "name": name,
        "serial_number": serial_number,
        "device_type": device_type,
        "status": "offline",
        "battery": None,
        "last_seen": None,
        "created_at": now,
    }

    result = db.devices.insert_one(device_doc)

    return jsonify({
        "message": "Device registered successfully",
        "device": {
            "id": str(result.inserted_id),
            "name": name,
            "serial_number": serial_number,
            "device_type": device_type,
            "status": "offline",
        },
    }), 201


@devices_bp.route("/<device_id>", methods=["GET"])
@jwt_required()
def get_device(device_id):
    """Get a single device by ID."""
    user_id = get_jwt_identity()
    db = get_db()

    try:
        device = db.devices.find_one({"_id": ObjectId(device_id), "user_id": user_id})
    except Exception:
        return jsonify({"error": "Invalid device ID"}), 400

    if not device:
        return jsonify({"error": "Device not found"}), 404

    return jsonify({
        "id": str(device["_id"]),
        "name": device.get("name"),
        "serial_number": device.get("serial_number"),
        "device_type": device.get("device_type"),
        "status": device.get("status"),
        "battery": device.get("battery"),
        "last_seen": device["last_seen"].isoformat() if device.get("last_seen") else None,
        "created_at": device["created_at"].isoformat() if device.get("created_at") else None,
    }), 200


@devices_bp.route("/<device_id>", methods=["PUT"])
@jwt_required()
def update_device(device_id):
    """Update a device's name or type."""
    user_id = get_jwt_identity()
    data = request.get_json()
    db = get_db()

    try:
        device = db.devices.find_one({"_id": ObjectId(device_id), "user_id": user_id})
    except Exception:
        return jsonify({"error": "Invalid device ID"}), 400

    if not device:
        return jsonify({"error": "Device not found"}), 404

    update_fields = {}
    if "name" in data:
        update_fields["name"] = data["name"].strip()
    if "device_type" in data:
        update_fields["device_type"] = data["device_type"]

    if update_fields:
        db.devices.update_one({"_id": ObjectId(device_id)}, {"$set": update_fields})

    return jsonify({"message": "Device updated"}), 200


@devices_bp.route("/<device_id>", methods=["DELETE"])
@jwt_required()
def delete_device(device_id):
    """Delete a device."""
    user_id = get_jwt_identity()
    db = get_db()

    try:
        result = db.devices.delete_one({"_id": ObjectId(device_id), "user_id": user_id})
    except Exception:
        return jsonify({"error": "Invalid device ID"}), 400

    if result.deleted_count == 0:
        return jsonify({"error": "Device not found"}), 404

    return jsonify({"message": "Device deleted"}), 200
