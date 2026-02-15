// ============================================================
// Display Module - 128x64 SPI OLED (SSD1306) via U8g2
// Handles all display output for the PatHacks device
// ============================================================

#include "display.h"
#include "pins.h"
#include <U8g2lib.h>
#include <SPI.h>

// U8g2 constructor for SSD1306 128x64 SPI
// Using full buffer mode (F) for flicker-free updates
// Constructor: U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI(rotation, cs, dc, reset)
static U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2(
    U8G2_R0,      // No rotation
    OLED_CS,      // Chip Select
    OLED_DC,      // Data/Command
    OLED_RST      // Reset
);

static bool display_initialized = false;

bool display_init() {
    if (display_initialized) return true;

    Serial.println("[DSP] Initializing SPI OLED display...");

    if (!u8g2.begin()) {
        Serial.println("[DSP] ERROR: Failed to initialize display!");
        return false;
    }

    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setFontRefHeightExtendedText();
    u8g2.setFontPosTop();
    u8g2.clearBuffer();
    u8g2.sendBuffer();

    display_initialized = true;
    Serial.println("[DSP] Display initialized successfully.");
    return true;
}

void display_clear() {
    if (!display_initialized) return;
    u8g2.clearBuffer();
    u8g2.sendBuffer();
}

void display_text(const char* text, int x, int y) {
    if (!display_initialized) return;
    u8g2.drawStr(x, y, text);
    u8g2.sendBuffer();
}

void display_message(const char* title, const char* message) {
    if (!display_initialized) return;

    u8g2.clearBuffer();

    // Title in a larger font
    u8g2.setFont(u8g2_font_7x14B_tf);
    int title_width = u8g2.getStrWidth(title);
    u8g2.drawStr((SCREEN_WIDTH - title_width) / 2, 5, title);

    // Divider line
    u8g2.drawHLine(0, 22, SCREEN_WIDTH);

    // Message in smaller font
    u8g2.setFont(u8g2_font_6x10_tf);
    int msg_width = u8g2.getStrWidth(message);
    u8g2.drawStr((SCREEN_WIDTH - msg_width) / 2, 30, message);

    u8g2.sendBuffer();

    // Restore default font
    u8g2.setFont(u8g2_font_6x10_tf);
}

void display_test_pattern() {
    if (!display_initialized) return;

    // Pattern 1: Border rectangle
    u8g2.clearBuffer();
    u8g2.drawFrame(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(20, 28, "PatHacks Device");
    u8g2.sendBuffer();
    delay(1500);

    // Pattern 2: Checkerboard
    u8g2.clearBuffer();
    for (int y = 0; y < SCREEN_HEIGHT; y += 8) {
        for (int x = 0; x < SCREEN_WIDTH; x += 8) {
            if ((x / 8 + y / 8) % 2 == 0) {
                u8g2.drawBox(x, y, 8, 8);
            }
        }
    }
    u8g2.sendBuffer();
    delay(1000);

    // Pattern 3: Concentric circles
    u8g2.clearBuffer();
    for (int r = 5; r <= 30; r += 5) {
        u8g2.drawCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, r);
    }
    u8g2.sendBuffer();
    delay(1000);
}

void display_test() {
    Serial.println("[DSP] === Display Test ===");

    if (!display_initialized) {
        if (!display_init()) {
            Serial.println("[DSP] Test FAILED - could not initialize.");
            return;
        }
    }

    Serial.println("[DSP] Showing test patterns...");

    // Show welcome message
    display_message("PatHacks", "Display Test");
    delay(1500);

    // Show test patterns
    display_test_pattern();

    // Show result
    display_message("Display", "Test PASSED");
    delay(1000);

    Serial.println("[DSP] Test PASSED - verify patterns appeared on screen.");
}
