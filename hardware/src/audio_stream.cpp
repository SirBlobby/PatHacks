// ============================================================
// Audio Streaming Module
// Captures PDM microphone data and streams it via Socket.IO
// as raw 16-bit PCM (16kHz, mono) binary events
// ============================================================

#include "audio_stream.h"
#include "socketio_client.h"
#include "config.h"
#include "pins.h"

#include <driver/i2s.h>

// Audio buffer
static uint8_t audio_buffer[AUDIO_CHUNK_SIZE];
static bool stream_active = false;
static bool mic_initialized = false;
static unsigned long last_send_time = 0;

// I2S port for microphone (use port 0)
#define I2S_MIC_PORT I2S_NUM_0

bool audio_stream_init() {
    if (mic_initialized) return true;

    Serial.println("[AUDIO] Initializing PDM microphone for streaming...");

    // Configure I2S for PDM RX
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 256,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_PIN_NO_CHANGE,
        .ws_io_num = MIC_PDM_CLK,      // GPIO 42 - PDM Clock
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = MIC_PDM_DATA    // GPIO 41 - PDM Data
    };

    esp_err_t err = i2s_driver_install(I2S_MIC_PORT, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("[AUDIO] ERROR: i2s_driver_install failed: %d\n", err);
        return false;
    }

    err = i2s_set_pin(I2S_MIC_PORT, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("[AUDIO] ERROR: i2s_set_pin failed: %d\n", err);
        i2s_driver_uninstall(I2S_MIC_PORT);
        return false;
    }

    // Clear DMA buffers
    i2s_zero_dma_buffer(I2S_MIC_PORT);

    mic_initialized = true;
    Serial.println("[AUDIO] PDM microphone initialized.");
    return true;
}

void audio_stream_start() {
    if (!mic_initialized) {
        if (!audio_stream_init()) return;
    }

    // Start I2S
    i2s_start(I2S_MIC_PORT);
    stream_active = true;
    last_send_time = millis();
    Serial.println("[AUDIO] Streaming started.");
}

void audio_stream_stop() {
    if (!stream_active) return;

    stream_active = false;
    i2s_stop(I2S_MIC_PORT);
    Serial.println("[AUDIO] Streaming stopped.");
}

void audio_stream_loop() {
    if (!stream_active || !sio_is_connected()) return;

    // Read audio data from I2S DMA buffer
    size_t bytes_read = 0;
    esp_err_t err = i2s_read(I2S_MIC_PORT, audio_buffer, AUDIO_CHUNK_SIZE, &bytes_read, 10);

    if (err == ESP_OK && bytes_read > 0) {
        // Send raw PCM data as binary Socket.IO event
        sio_emit_binary("audio_data", audio_buffer, bytes_read);
    }
}

bool audio_stream_is_active() {
    return stream_active;
}
