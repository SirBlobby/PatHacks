#ifndef SPEAKER_STREAM_H
#define SPEAKER_STREAM_H

#include <Arduino.h>

// Initialize the I2S TX speaker stream (does NOT install I2S driver yet)
bool spk_stream_init();

// Start I2S TX output mode for speaker playback.
// IMPORTANT: Caller must ensure mic I2S is stopped/uninstalled first!
bool spk_stream_start();

// Stop I2S TX and uninstall driver (frees bus for mic reuse)
void spk_stream_stop();

// Write raw PCM data (16kHz, 16-bit, mono) to the speaker.
// Data is buffered internally and written to I2S in spk_stream_loop().
// Returns number of bytes accepted into the ring buffer.
size_t spk_stream_write(const uint8_t* data, size_t len);

// Must be called in loop() — drains ring buffer to I2S DMA
void spk_stream_loop();

// Returns true if there is still audio data being played
bool spk_stream_is_playing();

// Flush any remaining buffered audio and mark playback complete
void spk_stream_flush();

#endif // SPEAKER_STREAM_H
