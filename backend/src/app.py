"""
LearningBuddy Backend - Flask Application Factory.
"""

import os
import logging
from pathlib import Path
from datetime import timedelta

from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
from flask_jwt_extended import JWTManager
from flask_socketio import SocketIO

from src.config import Config
from src.db import init_db, close_db

# Module-level SocketIO instance so run.py and recordings can access it
socketio = SocketIO()


class APIOnlyFilter(logging.Filter):
    """Only log /api/* requests, suppress static asset noise."""

    def filter(self, record: logging.LogRecord) -> bool:
        msg = record.getMessage()
        # Always show non-request log lines
        if '"GET ' not in msg and '"POST ' not in msg and '"PUT ' not in msg and '"DELETE ' not in msg:
            return True
        # Only show API requests
        if "/api/" in msg:
            return True
        return False


def create_app() -> Flask:
    app = Flask(
        __name__,
        static_folder=Config.FRONTEND_BUILD_DIR,
        static_url_path="",
    )

    # ── Suppress static asset logging ──
    werkzeug_logger = logging.getLogger("werkzeug")
    werkzeug_logger.addFilter(APIOnlyFilter())

    # ── Configuration ──
    app.config["SECRET_KEY"] = Config.SECRET_KEY
    app.config["JWT_SECRET_KEY"] = Config.JWT_SECRET_KEY
    app.config["JWT_ACCESS_TOKEN_EXPIRES"] = timedelta(seconds=Config.JWT_ACCESS_TOKEN_EXPIRES)
    app.config["MAX_CONTENT_LENGTH"] = Config.MAX_CONTENT_LENGTH

    # ── Extensions ──
    CORS(app, resources={r"/api/*": {"origins": "*"}})
    JWTManager(app)
    socketio.init_app(app, cors_allowed_origins="*", async_mode="threading")

    # ── Ensure upload directory exists ──
    os.makedirs(Config.UPLOAD_FOLDER, exist_ok=True)

    # ── Initialize database (graceful if unavailable) ──
    with app.app_context():
        try:
            init_db()
        except Exception as e:
            print(f"[WARNING] Database init failed: {e}")
            print("[WARNING] API endpoints will fail until MongoDB is configured.")

    # ── Register API Blueprints ──
    from src.routes.auth import auth_bp
    from src.routes.dashboard import dashboard_bp
    from src.routes.devices import devices_bp
    from src.routes.sources import sources_bp
    from src.routes.chat import chat_bp
    from src.routes.profile import profile_bp
    from src.routes.settings import settings_bp
    from src.routes.voice import voice_bp, general_voice_bp
    from src.routes.recordings import recordings_bp, register_socketio_events
    from src.routes.voice_call import register_voice_call_events
    from src.routes.general_chat import general_chat_bp

    app.register_blueprint(auth_bp)
    app.register_blueprint(dashboard_bp)
    app.register_blueprint(devices_bp)
    app.register_blueprint(sources_bp)
    app.register_blueprint(chat_bp)
    app.register_blueprint(profile_bp)
    app.register_blueprint(settings_bp)
    app.register_blueprint(voice_bp)
    app.register_blueprint(recordings_bp)
    app.register_blueprint(general_chat_bp)
    app.register_blueprint(general_voice_bp)

    # ── Register Socket.IO events for device audio streaming ──
    register_socketio_events(socketio)

    # ── Register Socket.IO events for voice calls ──
    register_voice_call_events(socketio)

    # ── Global error handlers (return JSON, not HTML) ──
    @app.errorhandler(Exception)
    def handle_exception(e):
        """Catch-all: return JSON for API routes, let frontend errors fall through."""
        import traceback
        traceback.print_exc()
        return {"error": str(e)}, 500

    @app.errorhandler(404)
    def not_found(e):
        # If it's an API route, return JSON 404
        if request.path.startswith("/api/"):
            return {"error": "Endpoint not found"}, 404
        # Otherwise fall through to serve_frontend
        return serve_frontend(request.path)

    # ── Health check ──
    @app.route("/api/health")
    def health():
        return {"status": "ok", "service": "LearningBuddy API"}, 200

    # ── Serve SvelteKit frontend build ──
    @app.route("/", defaults={"path": ""})
    @app.route("/<path:path>")
    def serve_frontend(path):
        """
        Serve the built SvelteKit frontend.
        If a matching static file exists, serve it; otherwise serve index.html
        for SPA client-side routing.
        """
        build_dir = Config.FRONTEND_BUILD_DIR

        if path and os.path.isfile(os.path.join(build_dir, path)):
            return send_from_directory(build_dir, path)

        # Fallback to index.html for SPA routing
        index_path = os.path.join(build_dir, "index.html")
        if os.path.isfile(index_path):
            return send_from_directory(build_dir, "index.html")

        # If no build exists, return a helpful message
        return {
            "message": "Frontend build not found. Run 'bun run build' in the frontend directory.",
            "api_docs": {
                "health": "GET /api/health",
                "auth": "POST /api/auth/register, POST /api/auth/login, GET /api/auth/me",
                "dashboard": "GET /api/dashboard",
                "devices": "GET|POST /api/devices, GET|PUT|DELETE /api/devices/:id",
                "sources": "GET|POST /api/sources, GET|DELETE /api/sources/:id",
                "chat": "POST /api/sources/:id/chat, POST /api/sources/:id/chat/stream",
                "profile": "GET|PUT /api/profile",
                "settings": "GET|PUT /api/settings",
            },
        }, 200

    # ── Teardown ──
    @app.teardown_appcontext
    def teardown(exception):
        pass  # Connection pooling handles cleanup

    return app
