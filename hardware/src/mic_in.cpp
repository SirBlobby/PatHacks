// ============================================================
// Microphone Module - PDM Microphone on XIAO ESP32-S3 Sense
// Uses pschatzmann/arduino-audio-tools for PDM I2S Input
// ============================================================

#include "AudioTools.h"
#include "microphone.h"
#include "pins.h"

// Define audio format for PDM mic (16kHz, 1 channel, 16-bit)
AudioInfo mic_info(16000, 1, 16);

I2SStream i2sStream;                // Access I2S as stream
CsvOutput<int16_t> csvOutput(Serial); // Output as CSV to Serial
StreamCopy micCopier(csvOutput, i2sStream); // Copies audio from I2S to CSV output

static bool mic_initialized = false;

bool mic_init() {
    if (mic_initialized) return true;
    
    Serial.println("[MIC] Initializing PDM microphone (AudioTools)...");

    // Configure I2S for PDM RX Mode
    auto config = i2sStream.defaultConfig(RX_MODE);
    config.copyFrom(mic_info);
    
    config.signal_type = PDM;
    // Critical PDM Settings for ESP32-S3
    // On ESP32 PDM: Clock is WS pin, Data is DATA pin
    config.pin_ws = MIC_PDM_CLK;        // GPIO 42 (CLK)
    config.pin_data = MIC_PDM_DATA;     // GPIO 41 (DATA)
    config.is_master = true;
    config.pin_bck = -1;                // BCLK unused in PDM
    
    if (!i2sStream.begin(config)) {
        Serial.println("[MIC] ERROR: Failed to initialize I2S stream!");
        return false;
    }

    // Initialize CSV Output
    csvOutput.begin(mic_info);
    
    mic_initialized = true;
    Serial.println("[MIC] Microphone initialized. streaming CSV data...");
    return true;
}

void mic_test(unsigned long duration_ms) {
    if (!mic_initialized) mic_init();
    
    Serial.println("--- START MIC DATA ---");
    
    unsigned long start = millis();
    while (millis() - start < duration_ms) {
        Serial.print(">mic:");
        micCopier.copy();
    }
    
    Serial.println("--- END MIC DATA ---");
}

void mic_loop() {
    if (mic_initialized) {
        micCopier.copy();
    }
}
