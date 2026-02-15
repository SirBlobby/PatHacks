#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include <Arduino.h>

// Initialize the PDM microphone for streaming
bool audio_stream_init();

// Start capturing and streaming audio via Socket.IO
// event_name: Socket.IO event to emit (e.g. "audio_data" for recording, "call_audio" for voice call)
void audio_stream_start(const char* event_name = "audio_data");

// Stop capturing audio (I2S remains installed)
void audio_stream_stop();

// Fully teardown mic I2S driver (uninstall, free bus for speaker)
void audio_stream_teardown();

// Must be called in loop() - reads mic data and sends over Socket.IO
void audio_stream_loop();

// Check if currently streaming
bool audio_stream_is_active();

#endif // AUDIO_STREAM_H
