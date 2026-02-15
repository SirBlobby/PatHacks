#ifndef MICROPHONE_H
#define MICROPHONE_H

#include <Arduino.h>

// Initialize the PDM microphone via I2S
bool mic_init();

// Read a single audio sample from the microphone
// Returns the sample value, or 0 if no valid sample
int mic_read_sample();

// Test the microphone by reading samples and printing to Serial
// duration_ms: how long to run the test in milliseconds
void mic_test(unsigned long duration_ms);

#endif // MICROPHONE_H
