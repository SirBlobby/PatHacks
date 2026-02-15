// ============================================================
// Display Module - Updated for LearningBuddy status UI
// 128x64 SPI OLED (SSD1306) via U8g2
// ============================================================

#include "display.h"
#include "pins.h"
#include <U8g2lib.h>
#include <SPI.h>

// U8g2 constructor for SSD1306 128x64 SPI
static U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2(
    U8G2_R0,
    OLED_CS,
    OLED_DC,
    OLED_RST
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
    Serial.println("[DSP] Display initialized.");
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

    // Title in larger font
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
    u8g2.setFont(u8g2_font_6x10_tf);
}

void display_test_pattern() {
    if (!display_initialized) return;

    u8g2.clearBuffer();
    u8g2.drawFrame(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(20, 28, "PatHacks Device");
    u8g2.sendBuffer();
    delay(1500);

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
    display_message("PatHacks", "Display Test");
    delay(1500);
    display_test_pattern();
    display_message("Display", "Test PASSED");
    delay(1000);

    Serial.println("[DSP] Test PASSED.");
}

// ---- LearningBuddy Status Screens ----

void display_status(const char* line1, const char* line2, const char* line3) {
    if (!display_initialized) return;

    u8g2.clearBuffer();

    // Header bar
    u8g2.setFont(u8g2_font_7x14B_tf);
    u8g2.drawStr(4, 0, "LearningBuddy");
    u8g2.drawHLine(0, 16, SCREEN_WIDTH);

    // Status lines
    u8g2.setFont(u8g2_font_6x10_tf);
    if (line1) u8g2.drawStr(4, 20, line1);
    if (line2) u8g2.drawStr(4, 34, line2);
    if (line3) u8g2.drawStr(4, 48, line3);

    u8g2.sendBuffer();
}

void display_recording(unsigned long elapsed_seconds) {
    if (!display_initialized) return;

    u8g2.clearBuffer();

    // Recording header with dot indicator
    u8g2.setFont(u8g2_font_7x14B_tf);
    u8g2.drawStr(4, 0, "RECORDING");

    // Blinking record dot
    if ((millis() / 500) % 2 == 0) {
        u8g2.drawDisc(100, 7, 5);
    } else {
        u8g2.drawCircle(100, 7, 5);
    }

    u8g2.drawHLine(0, 16, SCREEN_WIDTH);

    // Timer
    u8g2.setFont(u8g2_font_logisoso22_tn);
    char timer[9];
    unsigned long mins = elapsed_seconds / 60;
    unsigned long secs = elapsed_seconds % 60;
    snprintf(timer, sizeof(timer), "%02lu:%02lu", mins, secs);
    int tw = u8g2.getStrWidth(timer);
    u8g2.drawStr((SCREEN_WIDTH - tw) / 2, 25, timer);

    // Audio level bar placeholder
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(4, 54, "Streaming audio...");

    u8g2.sendBuffer();
}

void display_wifi_connecting(const char* ssid) {
    if (!display_initialized) return;
    char msg[32];
    snprintf(msg, sizeof(msg), "%.20s", ssid);
    display_status("Connecting WiFi...", msg, "Please wait...");
}

void display_setup_mode() {
    display_status("Setup Mode", "Connect USB cable", "Run setup app");
}

void display_idle(const char* ssid, const char* ip) {
    if (!display_initialized) return;

    char line2[32], line3[32];
    snprintf(line2, sizeof(line2), "WiFi: %.16s", ssid);
    snprintf(line3, sizeof(line3), "IP: %s", ip);
    display_status("Ready", line2, line3);
}

void display_error(const char* error_msg) {
    display_status("ERROR", error_msg, "Check serial log");
}

// ---- Voice Call Screens ----

void display_voice_listening() {
    if (!display_initialized) return;

    u8g2.clearBuffer();

    // Header
    u8g2.setFont(u8g2_font_7x14B_tf);
    u8g2.drawStr(4, 0, "VOICE CALL");
    u8g2.drawHLine(0, 16, SCREEN_WIDTH);

    // Mic icon (simple circle with lines)
    int cx = SCREEN_WIDTH / 2;
    int cy = 36;
    u8g2.drawCircle(cx, cy, 8);
    u8g2.drawVLine(cx, cy + 8, 6);       // stem
    u8g2.drawHLine(cx - 4, cy + 14, 9);  // base

    // Animated sound waves (alternate based on millis)
    int phase = (millis() / 300) % 3;
    if (phase >= 1) u8g2.drawCircle(cx, cy, 12, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
    if (phase >= 2) u8g2.drawCircle(cx, cy, 16, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);

    // Label
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(30, 54, "Listening...");

    u8g2.sendBuffer();
}

void display_voice_thinking() {
    if (!display_initialized) return;

    u8g2.clearBuffer();

    // Header
    u8g2.setFont(u8g2_font_7x14B_tf);
    u8g2.drawStr(4, 0, "VOICE CALL");
    u8g2.drawHLine(0, 16, SCREEN_WIDTH);

    // Animated dots: "Processing" with rotating dots
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(24, 30, "Processing");

    int dots = (millis() / 400) % 4;
    char dot_str[4] = {0};
    for (int i = 0; i < dots; i++) dot_str[i] = '.';
    u8g2.drawStr(84, 30, dot_str);

    // Spinning indicator
    int phase = (millis() / 200) % 4;
    const char* spinner[] = {"|", "/", "-", "\\"};
    u8g2.setFont(u8g2_font_7x14B_tf);
    u8g2.drawStr(60, 44, spinner[phase]);

    u8g2.sendBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
}

void display_voice_playing() {
    if (!display_initialized) return;

    u8g2.clearBuffer();

    // Header
    u8g2.setFont(u8g2_font_7x14B_tf);
    u8g2.drawStr(4, 0, "VOICE CALL");
    u8g2.drawHLine(0, 16, SCREEN_WIDTH);

    // Speaker icon (triangle + sound waves)
    int sx = 40;
    int sy = 36;
    // Speaker body
    u8g2.drawTriangle(sx, sy - 6, sx, sy + 6, sx + 10, sy);
    u8g2.drawBox(sx - 6, sy - 4, 6, 8);

    // Animated sound waves from speaker
    int phase = (millis() / 250) % 3;
    if (phase >= 0) u8g2.drawCircle(sx + 12, sy, 4, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_LOWER_RIGHT);
    if (phase >= 1) u8g2.drawCircle(sx + 12, sy, 8, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_LOWER_RIGHT);
    if (phase >= 2) u8g2.drawCircle(sx + 12, sy, 12, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_LOWER_RIGHT);

    // Label
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(32, 54, "Speaking...");

    u8g2.sendBuffer();
}

void display_voice_ready() {
    if (!display_initialized) return;
    display_status("Voice Call", "Hold B to talk", "Long-press to end");
}
