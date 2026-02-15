#ifndef PINS_H
#define PINS_H

// =============================================================
// Pin Definitions for LearningBuddy Hardware
// MCU: Seeed Studio XIAO ESP32-S3 Sense
// Power: 3.7V LiPo -> onboard regulator -> 3.3V for all peripherals
// =============================================================

// --- Onboard PDM Microphone (Sense Expansion Board) ---
// These are internal to the expansion board, not on headers
#define MIC_PDM_CLK   42   // PDM Clock
#define MIC_PDM_DATA  41   // PDM Data

// --- I2S Speaker Amp (Adafruit MAX98357) ---
#define SPK_BCLK       2   // D1 - I2S Bit Clock
#define SPK_LRC        3   // D2 - I2S Word Select (Left/Right Clock)
#define SPK_DIN        4   // D3 - I2S Data Out

// --- SPI OLED Display (128x64 SSD1306) ---
#define OLED_CLK       7   // D8  - SPI Clock (SCK)
#define OLED_MOSI     10   // D10 - SPI Data (MOSI)
#define OLED_DC        1   // D0  - Data/Command select
#define OLED_CS        5   // D4  - Chip Select
#define OLED_RST       6   // D5  - Reset

// --- Push Buttons (active LOW with internal pullup) ---
#define BUTTON_A_PIN   8   // D9 (MISO) - Recording toggle
#define BUTTON_B_PIN  44   // D7 (RX)   - Voice call push-to-talk

// --- Display Parameters ---
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64

// --- Audio Parameters ---
#define SAMPLE_RATE   16000
#define BITS_PER_SAMPLE  16

#endif // PINS_H
