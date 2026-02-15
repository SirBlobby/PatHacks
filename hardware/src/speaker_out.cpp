// ============================================================
// Speaker Module - using pschatzmann/arduino-audio-tools
// Generates audio tones via I2SStream and SineWaveGenerator
// ============================================================

#include "speaker.h"
#include "pins.h"
#include "AudioTools.h"

// Define audio format
AudioInfo info(16000, 2, 16);

// Components: Generator -> Stream -> I2S
SineWaveGenerator<int16_t> sineWave(32000);                // Subclass of SoundGenerator with max amplitude 32000
GeneratedSoundStream<int16_t> sound(sineWave);             // Stream generated from sine wave
I2SStream out; 
StreamCopy copier(out, sound);                             // Copies sound into i2s

// Tone management
static unsigned long tone_end_time = 0;
static bool is_playing = false;
static bool spk_initialized = false;

bool spk_init() {
    if (spk_initialized) return true;
    
    Serial.println("[SPK] Initializing I2S speaker output (AudioTools)...");

    // Configure I2S Output
    auto config = out.defaultConfig(TX_MODE);
    config.copyFrom(info); 
    config.pin_bck = SPK_BCLK;
    config.pin_ws = SPK_LRC;
    config.pin_data = SPK_DIN; 
    
    // Start I2S
    if (!out.begin(config)) {
        Serial.println("[SPK] ERROR: Failed to initialize I2S stream!");
        return false;
    }

    // Initialize Sine Wave (start silent)
    sineWave.begin(info, 0);
    sineWave.setAmplitude(0);
    
    spk_initialized = true;
    Serial.println("[SPK] Speaker initialized.");
    return true;
}

void spk_loop() {
    if (!spk_initialized) return;

    // Check if tone duration has passed
    if (is_playing && millis() > tone_end_time) {
        sineWave.setAmplitude(0); // Silence
        is_playing = false;
    }

    // Always copy data to keep I2S clock running and buffer full (silence or tone)
    copier.copy();
}

void spk_play_tone(float freq, unsigned long duration_ms, float volume) {
    if (!spk_initialized) spk_init();

    // scale volume (0.0 - 1.0) to int16_t range (max ~32000)
    int16_t amp = (int16_t)(volume * 32000); 
    
    sineWave.setFrequency(freq);
    sineWave.setAmplitude(amp);
    
    tone_end_time = millis() + duration_ms;
    is_playing = true;
    
    // Serial.println("[SPK] Tone: " + String(freq) + "Hz");
}

void spk_stop() {
    if (spk_initialized) {
        sineWave.setAmplitude(0);
        is_playing = false;
        // We don't call out.end() usually because restarting I2S can be glitchy
    }
}

void spk_test() {
    Serial.println("[SPK] === Speaker Test ===");
    spk_init();
    
    // C Major Triad
    spk_play_tone(650, 500, 0.5); // C4
    unsigned long start = millis();
    while(millis() - start < 600) spk_loop(); 
    
    spk_play_tone(700, 500, 0.5); // E4
    start = millis();
    while(millis() - start < 600) spk_loop();

    spk_play_tone(720, 500, 0.5); // G4
    start = millis();
    while(millis() - start < 600) spk_loop();

    spk_play_tone(800, 1000, 0.5); // C5
    start = millis();
    while(millis() - start < 1100) spk_loop();
    
    Serial.println("[SPK] Test complete.");
}
