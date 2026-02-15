# LearningBuddy Backend

The API service for LearningBuddy, powered by Python Flask, Google Gemini, and Flask-SocketIO.

## Features

- **AI Chat** -- Context-grounded Q&A using Google Gemini with vector search (RAG).
- **Voice Chat** -- ElevenLabs Conversational AI integration with signed URL generation and system prompt injection from source context.
- **Live Recording** -- WebSocket-based audio streaming from ESP32 devices. Receives 16kHz/16-bit/mono PCM chunks, assembles into WAV, transcribes with faster-whisper, and creates sources automatically.
- **Device Provisioning** -- Physical device self-registration via 6-character key pairing.
- **Source Management** -- Upload and process PDFs, DOCX, text files. Content is chunked, embedded, and indexed for vector search.
- **Frontend Serving** -- Serves the SvelteKit static build via Flask (single-server deployment).

## API Endpoints

| Blueprint | Routes | Description |
|-----------|--------|-------------|
| `auth` | `POST /api/auth/register`, `POST /api/auth/login`, `GET /api/auth/me` | Authentication |
| `devices` | `GET\|POST /api/devices`, `GET\|PUT\|DELETE /api/devices/:id`, `POST /api/devices/setup` | Device management |
| `sources` | `GET\|POST /api/sources`, `GET\|DELETE /api/sources/:id` | Source CRUD + upload |
| `chat` | `POST /api/sources/:id/chat`, `POST /api/sources/:id/chat/stream` | AI chat (text) |
| `voice` | `GET /api/sources/:id/voice/signed-url` | ElevenLabs voice session |
| `recordings` | `GET /api/devices/:id/recordings`, `GET /api/recordings/:id`, `DELETE /api/recordings/:id` | Recording management |
| `dashboard` | `GET /api/dashboard` | Dashboard stats |
| `profile` | `GET\|PUT /api/profile` | User profile |
| `settings` | `GET\|PUT /api/settings` | User settings |

### WebSocket Events (Socket.IO)

| Event | Direction | Description |
|-------|-----------|-------------|
| `auth` | Client -> Server | Authenticate with `{ device_key }` |
| `auth_response` | Server -> Client | `{ status, device_id, message }` |
| `rec_start` | Client -> Server | Begin a recording session |
| `rec_start_ack` | Server -> Client | `{ status, recording_id }` |
| `audio_data` | Client -> Server | Binary PCM chunk (16kHz, 16-bit, mono) |
| `rec_stop` | Client -> Server | End recording, triggers transcription pipeline |
| `rec_stop_ack` | Server -> Client | `{ status, recording_id }` |

## Setup

1. **Environment Variables** -- Create a `.env` file in this directory:
   ```bash
   GEMINI_API_KEY="your_api_key"
   MONGODB_URI="mongodb://localhost:27017/learningbuddy"
   ELEVENLABS_API_KEY="your_elevenlabs_api_key"
   ELEVENLABS_AGENT_ID="your_elevenlabs_agent_id"
   ```

2. **Install Dependencies**:
   ```bash
   pip install -r requirements.txt
   ```

3. **Run Server**:
   ```bash
   python run.py
   ```
   The server binds to `0.0.0.0:5000` with SocketIO (threading async mode).

## Database

MongoDB collections: `users`, `devices`, `sources`, `chat_history`, `device_data`, `recordings`.

Indexes are created automatically on startup (`src/db.py`).

## Key Dependencies

- `flask-socketio` + `simple-websocket` -- WebSocket server for device audio streaming
- `faster-whisper` -- Local speech-to-text transcription
- `google-genai` -- Gemini API for chat, summaries, and embeddings
- `requests` -- ElevenLabs signed URL generation
