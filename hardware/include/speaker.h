#ifndef SPEAKER_H
#define SPEAKER_H

#include <Arduino.h>

// Initialize the I2S output for the MAX98357 speaker amp
bool spk_init();

// Play a sine wave tone at the given frequency for a duration
// freq: frequency in Hz
// duration_ms: duration in milliseconds
// volume: amplitude 0.0 - 1.0
void spk_play_tone(float freq, unsigned long duration_ms, float volume);

// Stop I2S output
void spk_stop();

// Test the speaker by playing a sequence of tones
void spk_test();

#endif // SPEAKER_H
