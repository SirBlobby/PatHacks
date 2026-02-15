from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from app.core.config import settings
from app.core.database import db_instance
import logging

# Import routers
from app.api import auth, dashboard, devices, recordings, chat, device_routes

# Set up logging
logging.basicConfig(level=logging.INFO)

app = FastAPI(
    title=settings.PROJECT_NAME,
    openapi_url=f"{settings.API_V1_STR}/openapi.json"
)

# CORS Middleware
origins = [
    "http://localhost",
    "http://localhost:5173", # Vite default
    "http://127.0.0.1:5173",
    # Add other origins as needed
]

app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.on_event("startup")
async def startup_db_client():
    """Start up the database connection."""
    db_instance.connect()

@app.on_event("shutdown")
async def shutdown_db_client():
    """Shut down the database connection."""
    db_instance.close()

# Include routers
app.include_router(auth.router, prefix=f"{settings.API_V1_STR}/auth", tags=["auth"])
app.include_router(dashboard.router, prefix=f"{settings.API_V1_STR}/dashboard", tags=["dashboard"])
app.include_router(devices.router, prefix=f"{settings.API_V1_STR}/devices", tags=["devices"])
app.include_router(recordings.router, prefix=f"{settings.API_V1_STR}/recordings", tags=["recordings"])
app.include_router(chat.router, prefix=f"{settings.API_V1_STR}/chat", tags=["chat"])
# Device routes usually don't need 'api/v1' prefix if they are for iot, or maybe they do.
# Let's keep them under api/v1/device
app.include_router(device_routes.router, prefix=f"{settings.API_V1_STR}/device", tags=["device_routes"])

@app.get("/")
def read_root():
    return {"message": "Welcome to the Learning Buddy Backend API"}
