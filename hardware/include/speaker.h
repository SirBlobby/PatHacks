#ifndef SPEAKER_H
#define SPEAKER_H

#include <Arduino.h>

// Initialize the speaker and I2S
bool spk_init();

// Process audio stream (must be called in main loop)
void spk_loop();

// Play a tone
// freq: Frequency in Hz
// duration_ms: Duration in milliseconds
// volume: Amplitude (0.0 to 1.0)
void spk_play_tone(float freq, unsigned long duration_ms, float volume = 0.5);

// Stop speaker (silence)
void spk_stop();

// Test sequence
void spk_test();

#endif // SPEAKER_H
