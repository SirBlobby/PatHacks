#ifndef MICROPHONE_H
#define MICROPHONE_H

#include <Arduino.h>

// Initialize the PDM microphone via I2S
bool mic_init();

// Process microphone data loop (copy to output)
void mic_loop();

// Test the microphone by outputting CSV data to Serial for a duration
void mic_test(unsigned long duration_ms);

#endif // MICROPHONE_H
