// ============================================================
// Speaker Stream Module
// Plays raw PCM audio (16kHz, 16-bit, mono) through MAX98357
// via I2S TX using ESP-IDF driver/i2s.h directly.
//
// IMPORTANT: This shares I2S_NUM_0 with the PDM microphone.
// The caller must stop/uninstall mic I2S before calling
// spk_stream_start(), and stop/uninstall speaker I2S before
// restarting the mic.
// ============================================================

#include "speaker_stream.h"
#include "config.h"
#include "pins.h"

#include <driver/i2s.h>

// I2S port (shared with mic — only one active at a time)
#define I2S_SPK_PORT I2S_NUM_0

// Ring buffer for incoming PCM data
static uint8_t ring_buffer[SPK_BUFFER_SIZE];
static volatile size_t rb_head = 0;   // write position
static volatile size_t rb_tail = 0;   // read position
static volatile size_t rb_count = 0;  // bytes in buffer

static bool spk_initialized = false;
static bool spk_active = false;
static bool playback_done = false;  // set after flush when buffer drains

bool spk_stream_init() {
    // Just mark as initialized — actual I2S install happens in start()
    spk_initialized = true;
    rb_head = 0;
    rb_tail = 0;
    rb_count = 0;
    playback_done = false;
    Serial.println("[SPKSTRM] Speaker stream initialized.");
    return true;
}

bool spk_stream_start() {
    if (spk_active) return true;

    Serial.println("[SPKSTRM] Starting I2S TX for speaker...");

    // Clear ring buffer
    rb_head = 0;
    rb_tail = 0;
    rb_count = 0;
    playback_done = false;

    // Configure I2S for TX (standard mode, not PDM)
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = SPK_I2S_DMA_BUF_COUNT,
        .dma_buf_len = SPK_I2S_DMA_BUF_LEN,
        .use_apll = false,
        .tx_desc_auto_clear = true,   // Auto-clear TX DMA on underrun (silence)
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = SPK_BCLK,       // GPIO 2
        .ws_io_num = SPK_LRC,         // GPIO 3
        .data_out_num = SPK_DIN,      // GPIO 4
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    esp_err_t err = i2s_driver_install(I2S_SPK_PORT, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("[SPKSTRM] ERROR: i2s_driver_install failed: %d\n", err);
        return false;
    }

    err = i2s_set_pin(I2S_SPK_PORT, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("[SPKSTRM] ERROR: i2s_set_pin failed: %d\n", err);
        i2s_driver_uninstall(I2S_SPK_PORT);
        return false;
    }

    i2s_zero_dma_buffer(I2S_SPK_PORT);
    i2s_start(I2S_SPK_PORT);

    spk_active = true;
    Serial.println("[SPKSTRM] I2S TX started.");
    return true;
}

void spk_stream_stop() {
    if (!spk_active) return;

    i2s_stop(I2S_SPK_PORT);
    i2s_driver_uninstall(I2S_SPK_PORT);

    spk_active = false;
    rb_head = 0;
    rb_tail = 0;
    rb_count = 0;
    playback_done = false;

    Serial.println("[SPKSTRM] I2S TX stopped and uninstalled.");
}

size_t spk_stream_write(const uint8_t* data, size_t len) {
    if (!spk_initialized) return 0;

    size_t written = 0;
    for (size_t i = 0; i < len; i++) {
        if (rb_count >= SPK_BUFFER_SIZE) {
            // Buffer full — drop remaining
            break;
        }
        ring_buffer[rb_head] = data[i];
        rb_head = (rb_head + 1) % SPK_BUFFER_SIZE;
        rb_count++;
        written++;
    }

    return written;
}

void spk_stream_loop() {
    if (!spk_active || rb_count == 0) {
        // If we flushed and buffer is empty, mark playback as done
        if (spk_active && playback_done && rb_count == 0) {
            // All data has been written to I2S
        }
        return;
    }

    // Write chunks from ring buffer to I2S
    // Read up to 512 bytes at a time
    uint8_t chunk[512];
    size_t to_write = (rb_count < sizeof(chunk)) ? rb_count : sizeof(chunk);

    for (size_t i = 0; i < to_write; i++) {
        chunk[i] = ring_buffer[rb_tail];
        rb_tail = (rb_tail + 1) % SPK_BUFFER_SIZE;
    }
    rb_count -= to_write;

    // Write to I2S (blocking, with short timeout)
    size_t bytes_written = 0;
    i2s_write(I2S_SPK_PORT, chunk, to_write, &bytes_written, 10);

    // If not all bytes were written, put them back
    if (bytes_written < to_write) {
        size_t leftover = to_write - bytes_written;
        // Move tail back
        if (rb_tail >= leftover) {
            rb_tail -= leftover;
        } else {
            rb_tail = SPK_BUFFER_SIZE - (leftover - rb_tail);
        }
        rb_count += leftover;
    }
}

bool spk_stream_is_playing() {
    if (!spk_active) return false;
    // Still playing if there's data in the ring buffer
    // or if we haven't been flushed yet
    return (rb_count > 0) || !playback_done;
}

void spk_stream_flush() {
    // Signal that no more data will be written.
    // spk_stream_is_playing() will return false once the buffer drains.
    playback_done = true;
    Serial.printf("[SPKSTRM] Flush called. %d bytes remaining.\n", rb_count);
}
