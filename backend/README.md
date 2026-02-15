# Backend

This is the backend of our project. It supports the Learning Buddy device ecosystem.

## Setup Instructions

### Prerequisites
1. **Python 3.10+** installed.
2. **MongoDB** installed and running locally (or use Atlas connection string).
3. **FFmpeg** installed and added to system PATH (required for audio processing).

### Installation

1. Navigate to the backend directory:
   ```bash
   cd backend
   ```

2. Create a virtual environment (optional but recommended):
   ```bash
   python -m venv venv
   source venv/bin/activate  # On Windows: venv\Scripts\activate
   ```

3. Install dependencies:
   ```bash
   pip install -r requirements.txt
   ```

4. Create a `.env` file based on the configuration in `app/core/config.py`:
   ```env
   MONGODB_URL=mongodb://localhost:27017
   DATABASE_NAME=learning_buddy_db
   SECRET_KEY=your_secret_key_here
   GOOGLE_API_KEY=your_gemini_api_key_here
   ```

### Running the Server

Start the development server:
```bash
uvicorn app.main:app --reload
```

The API will be available at `http://localhost:8000`.
API Documentation (Swagger UI) is at `http://localhost:8000/docs`.

## Core Features Implemented

- **Authentication**: User registration, login (JWT), password hashing (Argon2).
- **Dashboard**: Stats API (`/dashboard`).
- **Devices**: Device registration & management (`/devices`).
- **Recordings**: Upload, list, delete (`/recordings`).
- **Processing Pipeline**:
  - Transcription using **Faster-Whisper**.
  - Vector Embedding using **Gemini Embedding-001**.
  - Summarization using **Gemini Pro**.
- **Chat**: RAG-based Q&A (`/chat`).
- **Device API**: Dedicated endpoints for hardware (`/device/upload`, `/device/heartbeat`).

## Directory Structure

- `app/api`: API route handlers.
- `app/core`: Configuration, database, security.
- `app/models`: (Not strictly used, Pydantic schemas in `schemas`).
- `app/schemas`: Pydantic data models.
- `app/services`: Business logic (transcription, summary, vector).