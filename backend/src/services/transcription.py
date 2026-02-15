"""
Audio/Video Transcription Service using faster-whisper.
"""

import os
import subprocess
from pathlib import Path


def extract_audio_from_video(video_path: str, output_dir: str = None) -> str:
    """
    Extract audio track from a video file using ffmpeg.
    Returns path to the extracted .wav file.
    """
    if output_dir is None:
        output_dir = os.path.dirname(video_path)

    stem = Path(video_path).stem
    audio_path = os.path.join(output_dir, f"{stem}_audio.wav")

    try:
        subprocess.run(
            [
                "ffmpeg", "-y", "-i", video_path,
                "-vn", "-acodec", "pcm_s16le",
                "-ar", "16000", "-ac", "1",
                audio_path,
            ],
            capture_output=True,
            check=True,
        )
        return audio_path
    except FileNotFoundError:
        raise RuntimeError("ffmpeg is not installed. Install it to process video files.")
    except subprocess.CalledProcessError as e:
        raise RuntimeError(f"ffmpeg failed: {e.stderr.decode()}")


def transcribe_audio(file_path: str, model_size: str = "base") -> dict:
    """
    Transcribe an audio file using faster-whisper.

    Args:
        file_path: Path to audio file (.mp3, .wav, .m4a, etc.)
        model_size: Whisper model size (tiny, base, small, medium, large-v3)

    Returns:
        Dict with 'text', 'segments', 'language', 'duration'
    """
    try:
        from faster_whisper import WhisperModel
    except ImportError:
        raise ImportError("faster-whisper is required: pip install faster-whisper")

    print(f"[Whisper] Loading model '{model_size}'...")
    model = WhisperModel(model_size, compute_type="int8", device="cpu")

    print(f"[Whisper] Transcribing {os.path.basename(file_path)}...")
    segments_gen, info = model.transcribe(file_path, beam_size=5)

    segments = []
    full_text_parts = []

    for segment in segments_gen:
        segments.append({
            "start": round(segment.start, 2),
            "end": round(segment.end, 2),
            "text": segment.text.strip(),
        })
        full_text_parts.append(segment.text.strip())

    full_text = " ".join(full_text_parts)
    duration = info.duration if info.duration else 0

    print(f"[Whisper] Done. Language: {info.language}, Duration: {duration:.1f}s, Characters: {len(full_text)}")

    return {
        "text": full_text,
        "segments": segments,
        "language": info.language,
        "duration": round(duration, 2),
    }


def transcribe_file(file_path: str, model_size: str = "base") -> dict:
    """
    Transcribe an audio or video file.
    For video files, extracts audio first.

    Returns:
        Dict with 'text', 'segments', 'language', 'duration'
    """
    video_extensions = {".mp4", ".mkv", ".avi", ".mov", ".webm"}
    ext = Path(file_path).suffix.lower()

    audio_path = file_path

    if ext in video_extensions:
        print(f"[Transcribe] Extracting audio from video...")
        audio_path = extract_audio_from_video(file_path)

    try:
        result = transcribe_audio(audio_path, model_size=model_size)
        return result
    finally:
        # Clean up extracted audio if we created one
        if audio_path != file_path and os.path.exists(audio_path):
            os.remove(audio_path)
