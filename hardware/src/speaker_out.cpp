// ============================================================
// Speaker Module - I2S output to Adafruit MAX98357 Amp
// Generates audio tones via I2S for speaker playback
// ============================================================

#include "speaker.h"
#include "pins.h"
#include <I2S.h>
#include <math.h>

static bool spk_initialized = false;

bool spk_init() {
    if (spk_initialized) return true;

    Serial.println("[SPK] Initializing I2S speaker output...");

    // Configure I2S pins for speaker output
    // setAllPins(BCLK, WS/LRC, DIN(unused for output), DOUT/DIN_amp, MCK)
    I2S.setAllPins(SPK_BCLK, SPK_LRC, -1, SPK_DIN, -1);

    // Start I2S in standard stereo mode for the MAX98357
    // The amp expects standard I2S format
    if (!I2S.begin(I2S_PHILIPS_MODE, SAMPLE_RATE, BITS_PER_SAMPLE)) {
        Serial.println("[SPK] ERROR: Failed to initialize I2S!");
        return false;
    }

    spk_initialized = true;
    Serial.println("[SPK] Speaker output initialized successfully.");
    return true;
}

void spk_play_tone(float freq, unsigned long duration_ms, float volume) {
    if (!spk_initialized) {
        Serial.println("[SPK] Not initialized!");
        return;
    }

    // Clamp volume
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    Serial.println("[SPK] Playing " + String((int)freq) + " Hz for " + String(duration_ms) + " ms");

    unsigned long start = millis();
    float samples_per_cycle = (float)SAMPLE_RATE / freq;
    int sample_index = 0;

    while (millis() - start < duration_ms) {
        // Generate sine wave sample
        float angle = 2.0f * PI * (float)sample_index / samples_per_cycle;
        int16_t sample = (int16_t)(sinf(angle) * 32767.0f * volume);

        // Write sample (both L and R channels in Philips mode)
        I2S.write(sample);
        I2S.write(sample);

        sample_index++;
        if (sample_index >= (int)samples_per_cycle) {
            sample_index = 0;
        }
    }
}

void spk_stop() {
    if (spk_initialized) {
        I2S.end();
        spk_initialized = false;
        Serial.println("[SPK] Speaker stopped.");
    }
}

void spk_test() {
    Serial.println("[SPK] === Speaker Test ===");

    if (!spk_initialized) {
        if (!spk_init()) {
            Serial.println("[SPK] Test FAILED - could not initialize.");
            return;
        }
    }

    Serial.println("[SPK] Playing test tones...");

    // Play a scale of tones: C4, E4, G4, C5
    float tones[] = {261.63f, 329.63f, 392.00f, 523.25f};
    const char* names[] = {"C4", "E4", "G4", "C5"};

    for (int i = 0; i < 4; i++) {
        Serial.println("[SPK] Tone: " + String(names[i]) +
                       " (" + String((int)tones[i]) + " Hz)");
        spk_play_tone(tones[i], 500, 0.5f);
        delay(100);  // Brief pause between tones
    }

    // Stop I2S after test so mic can use it
    spk_stop();

    Serial.println("[SPK] Test PASSED - verify you heard 4 ascending tones.");
}