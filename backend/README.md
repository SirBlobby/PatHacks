# Backend

Backend
This is the backend of our project. It supports the Learning Buddy device ecosystem, including user authentication, device management, audio recording uploads, transcription, AI summarization, and a chat-based Q&A system using retrieval augmented generation (RAG). Transcripts are chunked and embedded into a vector database for fast semantic search and accurate AI responses.
The backend is responsible for handling secure user sessions, processing recordings through an AI pipeline, and exposing APIs that the frontend dashboard and hardware device can interact with.
Core Features

The backend must support user registration, login, and session management for the frontend panel system.
Functionality:
User registration (email/username + password)
User login (JWT)
Logout (token invalidation)
Password hashing (argon2)
User profile retrieval and update
The backend should provide summarized panel data for the dashboard page.
Functionality:
Total recordings count
Total devices count
Recent recordings list
Recording processing statuses (queued/transcribing/indexing/ready)
The backend must support registering and managing Learning Buddy hardware devices.
Functionality:
Create/register a new device


Assign device ownership to a user
Update device name and metadata
Track last sync / online status
Generate pairing tokens or device API keys
The backend must chunk transcript text and store embeddings in a vector database for retrieval.
Functionality:
Chunk transcript into readable sections
Generate embeddings for each chunk
Store chunk vectors in a vector database (Pinecone/Weaviate/Chroma/pgvector)
Allow re-indexing if transcript is updated


The backend must store lecture recordings uploaded by the device or user.
Functionality:
Create a recording entry with metadata
Upload audio files (wav/mp3/m4a)
Store recordings in local storage or S3-compatible storage
Stream/download audio for playback
Delete recordings

The backend must convert uploaded audio into text transcripts using a transcription engine.
Functionality:
Start transcription jobs
Track transcription status (queued/processing/done/failed)
Store transcript output in the database
Support timestamped transcript segments (optional)
After transcription, the backend should generate AI-based summaries for the lecture.
Functionality:
Generate short summary and detailed summary
Generate bullet point notes / key takeaways
Store summary output for quick retrieval

The backend must support user-specific preferences that control AI behavior.
Functionality:
Save AI summary preferences (short vs long)
Save auto-transcription / auto-summary toggles
Save preferred AI model options (optional)
The backend must support a chat interface where users can ask questions about a specific lecture.
Functionality:
Store chat history per recording
Convert user questions into embeddings
Retrieve relevant transcript chunks from the vector database
Use retrieved chunks as context for AI responses (RAG)
Return responses with transcript citations (recommended)

The backend must provide secure endpoints specifically for the Learning Buddy device.
Functionality:
Authenticate device uploads
Upload recordings directly from device
Send device heartbeat (online/offline tracking)
Store metadata (lecture name, course name, timestamp)
Job Queue system
Functionality:
Background processing queue (Redis + BullMQ / Celery / RabbitMQ)
Job state tracking
Retry failed jobs
Store pipeline state for each recording

