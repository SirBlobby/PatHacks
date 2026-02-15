import asyncio
from concurrent.futures import ThreadPoolExecutor
from faster_whisper import WhisperModel
from app.core.config import settings
import logging

# Initialize model (lazy loading recommended to save resources)
model = None

def get_model():
    global model
    if model is None:
        logging.info(f"Loading Whisper model: {settings.WHISPER_MODEL_SIZE}")
        # Use 'cuda' if available, else 'cpu'
        # 'int8' compute type is usually faster on CPU
        compute_type = "int8"
        device = "cuda" if False else "cpu" # Check for CUDA properly in prod
        model = WhisperModel(settings.WHISPER_MODEL_SIZE, device=device, compute_type=compute_type)
        logging.info("Whisper model loaded")
    return model

def transcribe_audio_sync(file_path: str):
    """
    Synchronous function to transcribe audio using Faster Whisper.
    """
    model = get_model()
    segments, info = model.transcribe(file_path, beam_size=5)
    
    transcribed_segments = []
    full_text = []
    
    for segment in segments:
        transcribed_segments.append({
            "start": segment.start,
            "end": segment.end,
            "text": segment.text
        })
        full_text.append(segment.text)
        
    return {
        "text": " ".join(full_text),
        "segments": transcribed_segments,
        "language": info.language,
        "duration": info.duration
    }

async def transcribe_audio(file_path: str):
    """
    Async wrapper for transcription.
    """
    loop = asyncio.get_event_loop()
    # Run CPU-bound task in a thread pool
    with ThreadPoolExecutor() as pool:
        result = await loop.run_in_executor(pool, transcribe_audio_sync, file_path)
    return result
