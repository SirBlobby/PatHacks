# LearningBuddy

A full-stack AI learning platform that combines a web dashboard with a physical ESP32 recording device. Students can upload lecture materials, chat with AI about their content (text and voice), and capture live lectures using a hardware companion that streams audio in real-time.

https://buddy.sirblob.co/

## Architecture

```
frontend/          SvelteKit 2 + Svelte 5 + Tailwind CSS v4
backend/           Python Flask + Flask-SocketIO + MongoDB
hardware/          ESP32-S3 Sense firmware (PlatformIO)
```

The frontend builds to static files (`backend/build/`) via `@sveltejs/adapter-static`. Flask serves these alongside the API, so a single server handles everything in production.

## Features

- **Source Management** -- Upload PDFs, DOCX, text, and other documents. Content is chunked and embedded for vector search.
- **AI Chat** -- Ask questions about your sources with context-grounded answers powered by Google Gemini.
- **Voice Chat** -- Have a spoken conversation with your AI study buddy via ElevenLabs Conversational AI, with RAG context from your sources.
- **Live Recording** -- Stream audio from an ESP32 device over WebSocket. The backend assembles PCM chunks into WAV, transcribes with faster-whisper, and creates a new source automatically.
- **Device Management** -- Pair physical devices using a 6-character key. Monitor device status and view recordings from the dashboard.
- **DeskPet** -- An interactive digital companion that reacts to your learning activity.

## Quick Start

### Prerequisites

- Python 3.12+
- Bun (frontend package manager)
- MongoDB (local or Atlas)
- Google Gemini API key
- ElevenLabs API key + Agent ID (for voice chat)

### Environment Variables

Create `backend/.env`:

```bash
GEMINI_API_KEY="your_gemini_api_key"
MONGODB_URI="mongodb://localhost:27017/learningbuddy"
ELEVENLABS_API_KEY="your_elevenlabs_api_key"
ELEVENLABS_AGENT_ID="your_elevenlabs_agent_id"
```

### Run Locally

```bash
# Backend
cd backend
pip install -r requirements.txt
python run.py

# Frontend (development)
cd frontend
bun install
bun run dev

# Frontend (production build → served by Flask)
cd frontend
bun run build
# Then just run the backend — it serves the built frontend
```

### Run with Docker

```bash
# Set environment variables
export GEMINI_API_KEY="..."
export ELEVENLABS_API_KEY="..."
export ELEVENLABS_AGENT_ID="..."

docker compose up --build
```

## Tech Stack

| Layer | Technology |
|-------|-----------|
| Frontend | SvelteKit 2, Svelte 5 (runes), Tailwind CSS v4, TypeScript |
| Backend | Flask, Flask-SocketIO, Flask-JWT-Extended |
| AI | Google Gemini (chat + embeddings), ElevenLabs (voice), faster-whisper (transcription) |
| Database | MongoDB (PyMongo) |
| Hardware | ESP32-S3 Sense, PDM microphone, PlatformIO |
| Icons | Iconify (`@iconify/svelte`) |
