FROM python:3.12-slim

WORKDIR /app

# Install system dependencies (gcc for building wheels, ffmpeg for audio processing)
RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc \
    python3-dev \
    ffmpeg \
    && rm -rf /var/lib/apt/lists/*

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy source code
COPY . .

# Create uploads directory
RUN mkdir -p /app/uploads

# Expose port (Flask + SocketIO)
EXPOSE 5000

# Run the application via SocketIO
CMD ["python", "run.py"]
