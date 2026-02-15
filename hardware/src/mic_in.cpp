// ============================================================
// Microphone Module - PDM Microphone on XIAO ESP32-S3 Sense
// Uses the onboard digital microphone via I2S in PDM mode
// ============================================================

#include "microphone.h"
#include "pins.h"
#include <I2S.h>

static bool mic_initialized = false;

bool mic_init() {
    if (mic_initialized) return true;

    Serial.println("[MIC] Initializing PDM microphone...");

    // Configure I2S pins for PDM microphone
    // setAllPins(BCLK, WS/PDM_CLK, DIN/PDM_DATA, DOUT, MCK)
    // -1 = unused pin
    I2S.setAllPins(-1, MIC_PDM_CLK, MIC_PDM_DATA, -1, -1);

    // Start I2S in PDM mono mode at 16kHz, 16-bit
    if (!I2S.begin(PDM_MONO_MODE, SAMPLE_RATE, BITS_PER_SAMPLE)) {
        Serial.println("[MIC] ERROR: Failed to initialize I2S!");
        return false;
    }

    mic_initialized = true;
    Serial.println("[MIC] PDM microphone initialized successfully.");
    return true;
}

int mic_read_sample() {
    if (!mic_initialized) return 0;

    int sample = I2S.read();

    // Filter out invalid/silence samples
    if (sample == 0 || sample == -1 || sample == 1) {
        return 0;
    }
    return sample;
}

void mic_test(unsigned long duration_ms) {
    Serial.println("[MIC] === Microphone Test ===");

    if (!mic_initialized) {
        if (!mic_init()) {
            Serial.println("[MIC] Test FAILED - could not initialize.");
            return;
        }
    }

    Serial.println("[MIC] Reading samples for " + String(duration_ms / 1000) + " seconds...");
    Serial.println("[MIC] Speak into the microphone to see values.");

    unsigned long start = millis();
    int sample_count = 0;
    int max_sample = 0;
    int min_sample = 0;

    while (millis() - start < duration_ms) {
        int sample = mic_read_sample();
        if (sample != 0) {
            sample_count++;
            if (sample > max_sample) max_sample = sample;
            if (sample < min_sample) min_sample = sample;

            // Print every 100th valid sample to avoid flooding serial
            if (sample_count % 100 == 0) {
                Serial.println("[MIC] Sample: " + String(sample));
            }
        }
    }

    // Stop I2S after test so speaker can use it
    I2S.end();
    mic_initialized = false;

    Serial.println("[MIC] Test complete.");
    Serial.println("[MIC] Total valid samples: " + String(sample_count));
    Serial.println("[MIC] Range: [" + String(min_sample) + ", " + String(max_sample) + "]");

    if (sample_count > 0) {
        Serial.println("[MIC] Test PASSED");
    } else {
        Serial.println("[MIC] Test FAILED - no valid samples received.");
    }
}