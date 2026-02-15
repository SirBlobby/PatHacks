"""
Device Routes - CRUD for user devices and data ingestion.
"""

import random
import string
from flask import Blueprint, request, jsonify
from flask_jwt_extended import jwt_required, get_jwt_identity
from bson import ObjectId
from datetime import datetime
from src.db import get_db

devices_bp = Blueprint("devices", __name__, url_prefix="/api/devices")


def generate_key():
    """Generate a random 6-character alphanumeric key."""
    return ''.join(random.choices(string.ascii_uppercase + string.digits, k=6))


@devices_bp.route("", methods=["GET"])
@jwt_required()
def list_devices():
    """List all devices for the authenticated user and their recent data."""
    user_id = get_jwt_identity()
    db = get_db()

    devices = list(db.devices.find({"user_id": user_id}).sort("created_at", -1))
    
    result = []
    for d in devices:
        latest_data = db.device_data.find_one(
            {"device_id": str(d["_id"])},
            sort=[("timestamp", -1)]
        )
        
        result.append({
            "id": str(d["_id"]),
            "name": d.get("name", "Unnamed Device"),
            "key": d.get("key", ""),
            "device_type": d.get("device_type", "Learning Buddy"),
            "status": d.get("status", "offline"),
            "last_seen": d.get("last_seen").isoformat() if d.get("last_seen") else None,
            "created_at": d.get("created_at").isoformat() if d.get("created_at") else None,
            "latest_data": latest_data.get("payload") if latest_data else None
        })

    return jsonify(result), 200


@devices_bp.route("/setup", methods=["POST"])
def setup_device():
    """
    Called by the DEVICE on first boot.
    Generates a 6-digit KEY for the device to display on screen.
    Registers the device as 'available' (unclaimed).
    No JWT required.
    """
    db = get_db()
    
    # Generate unique key
    attempts = 0
    while True:
        key = generate_key()
        if not db.devices.find_one({"key": key}):
            break
        attempts += 1
        if attempts > 10:
            return jsonify({"error": "Failed to generate key"}), 500

    now = datetime.utcnow()
    device_doc = {
        "key": key,
        "user_id": None,  # Not linked to any user yet
        "name": None,
        "device_type": "Learning Buddy",
        "status": "available",
        "last_seen": now,
        "created_at": now,
    }
    
    db.devices.insert_one(device_doc)
    
    return jsonify({
        "message": "Start successful",
        "key": key,
        "status": "available"
    }), 201


@devices_bp.route("", methods=["POST"])
@jwt_required()
def register_device():
    """
    Called by the USER to CLAIM an existing device using its KEY.
    """
    user_id = get_jwt_identity()
    data = request.get_json()
    db = get_db()

    key = data.get("key", "").strip().upper()
    name = data.get("name", "").strip()

    if not key:
        return jsonify({"error": "Device Key is required"}), 400
    if not name:
        return jsonify({"error": "Device Name is required"}), 400

    # Find the device by key
    device = db.devices.find_one({"key": key})
    
    if not device:
        return jsonify({"error": "Invalid Device Key. Check device screen."}), 404
        
    if device.get("user_id"):
        return jsonify({"error": "This device is already registered to a user."}), 409
        
    # Claim the device
    updated_fields = {
        "user_id": user_id,
        "name": name,
        "status": "registered",
        "last_seen": datetime.utcnow()
    }
    
    db.devices.update_one(
        {"_id": device["_id"]},
        {"$set": updated_fields}
    )

    return jsonify({
        "message": "Device paired successfully",
        "device": {
            "id": str(device["_id"]),
            "name": name,
            "key": key,
            "status": "registered",
        },
    }), 200


@devices_bp.route("/ingest", methods=["POST"])
def ingest_data():
    """
    Device sends data here. 
    Authentication is via the 'key' in the payload.
    """
    data = request.get_json()
    if not data:
        return jsonify({"error": "Invalid JSON"}), 400

    key = data.get("key", "").strip().upper()
    payload = data.get("data")

    if not key:
        return jsonify({"error": "Missing 'key'"}), 400
    if payload is None:
        return jsonify({"error": "Missing 'data'"}), 400

    db = get_db()
    device = db.devices.find_one({"key": key})

    if not device:
        return jsonify({"error": "Device not registered or invalid key"}), 401

    timestamp = datetime.utcnow()
    
    # Store data
    log_entry = {
        "device_id": str(device["_id"]),
        "timestamp": timestamp,
        "payload": payload
    }
    db.device_data.insert_one(log_entry)

    # Update status
    db.devices.update_one(
        {"_id": device["_id"]},
        {"$set": {
            "last_seen": timestamp,
            "status": "online"
        }}
    )

    return jsonify({"status": "success", "timestamp": timestamp.isoformat()}), 201


@devices_bp.route("/<device_id>/data", methods=["GET"])
@jwt_required()
def get_device_data(device_id):
    """Get stored data for a device."""
    user_id = get_jwt_identity()
    db = get_db()

    try:
        device = db.devices.find_one({"_id": ObjectId(device_id), "user_id": user_id})
    except Exception:
        return jsonify({"error": "Invalid device ID"}), 400

    if not device:
        return jsonify({"error": "Device not found"}), 404

    logs = list(db.device_data.find(
        {"device_id": str(device["_id"])}
    ).sort("timestamp", -1).limit(50))

    return jsonify([{
        "timestamp": log["timestamp"].isoformat(),
        "data": log["payload"]
    } for log in logs]), 200


@devices_bp.route("/<device_id>", methods=["GET"])
@jwt_required()
def get_device(device_id):
    """Get a single device details."""
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
        "key": device.get("key"),
        "status": device.get("status"),
        "last_seen": device.get("last_seen").isoformat() if device.get("last_seen") else None,
        "created_at": device.get("created_at").isoformat() if device.get("created_at") else None,
    }), 200


@devices_bp.route("/<device_id>", methods=["PUT"])
@jwt_required()
def update_device(device_id):
    """Update a device's name."""
    user_id = get_jwt_identity()
    data = request.get_json()
    db = get_db()

    try:
        device = db.devices.find_one({"_id": ObjectId(device_id), "user_id": user_id})
    except Exception:
        return jsonify({"error": "Invalid device ID"}), 400

    if not device:
        return jsonify({"error": "Device not found"}), 404

    if "name" in data:
        db.devices.update_one(
            {"_id": ObjectId(device_id)}, 
            {"$set": {"name": data["name"].strip()}}
        )

    return jsonify({"message": "Device updated"}), 200


@devices_bp.route("/<device_id>", methods=["DELETE"])
@jwt_required()
def delete_device(device_id):
    """Unpair/Unclaim a device."""
    user_id = get_jwt_identity()
    db = get_db()
    
    try:
        # Check ownership
        device = db.devices.find_one({"_id": ObjectId(device_id), "user_id": user_id})
        if not device:
            return jsonify({"error": "Device not found"}), 404
            
        # Unclaim device (reset user_id to None, status to available)
        result = db.devices.update_one(
            {"_id": ObjectId(device_id)},
            {"$set": {"user_id": None, "status": "available", "name": None}}
        )
    except Exception:
        return jsonify({"error": "Invalid device ID"}), 400

    return jsonify({"message": "Device unpaired"}), 200
