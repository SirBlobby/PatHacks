#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include <Arduino.h>

// Initialize the PDM microphone for streaming
bool audio_stream_init();

// Start capturing and streaming audio via Socket.IO
void audio_stream_start();

// Stop capturing audio
void audio_stream_stop();

// Must be called in loop() - reads mic data and sends over Socket.IO
void audio_stream_loop();

// Check if currently streaming
bool audio_stream_is_active();

#endif // AUDIO_STREAM_H
