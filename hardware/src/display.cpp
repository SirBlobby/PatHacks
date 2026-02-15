// ============================================================
// Display Module - DeskPet Face + Status UI
// 128x64 SPI OLED (SSD1306) via U8g2
//
// Ports the web DeskPet's 7 expressions to monochrome 128x64:
//   neutral, happy, surprised, sleepy, love, confused, blush
//
// Layout: Face occupies top ~52px, status label at bottom 12px.
// Eyes are ~14px tall rounded rects, spaced 30px apart, centered.
// Eyebrows are 16px wide rectangles above the eyes.
// Cheeks are small filled circles beside the eyes.
// Hearts (love) are drawn with two overlapping circles + triangle.
// Blink is a 4-state machine matching the web DeskPet timing.
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

// ---- Blink State Machine ----
// Matches web DeskPet: 0=open, 1=closing, 2=closed, 3=opening
static uint8_t blink_state = 0;
static unsigned long blink_counter = 0;
static const unsigned long BLINK_INTERVAL = 3000; // ms between blinks
static const unsigned long BLINK_FRAME_MS = 50;   // ms per blink frame
static unsigned long last_blink_time = 0;
static bool blink_active = false;

// ============================================================
// Init
// ============================================================
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

// ============================================================
// Blink State Machine
// ============================================================
void display_blink_tick() {
    unsigned long now = millis();

    if (!blink_active) {
        // Wait for blink interval (with some randomness)
        if (now - last_blink_time > BLINK_INTERVAL + (now % 1500)) {
            blink_active = true;
            blink_state = 1; // start closing
            blink_counter = now;
        }
        return;
    }

    // Advance blink frames
    if (now - blink_counter > BLINK_FRAME_MS) {
        blink_counter = now;
        if (blink_state == 1) {
            blink_state = 2; // closed
        } else if (blink_state == 2) {
            blink_state = 3; // opening
        } else if (blink_state == 3) {
            blink_state = 0; // open
            blink_active = false;
            last_blink_time = now;
        }
    }
}

// ============================================================
// Primitive Helpers
// ============================================================

// Draw a heart at (cx, cy) with given half-width.
// Uses two filled circles + a filled triangle for monochrome.
static void draw_heart(int cx, int cy, int half_w) {
    int r = half_w / 2;
    if (r < 2) r = 2;

    // Two circles for the top bumps
    u8g2.drawDisc(cx - r, cy - r / 2, r);
    u8g2.drawDisc(cx + r, cy - r / 2, r);

    // Triangle for the bottom point
    u8g2.drawTriangle(
        cx - half_w, cy,
        cx + half_w, cy,
        cx, cy + half_w + r / 2
    );
}

// Draw a rounded rectangle eye at (cx, cy) with given width and height.
// cx,cy is the center of the eye.
static void draw_rounded_rect_eye(int cx, int cy, int w, int h, int r) {
    u8g2.drawRBox(cx - w / 2, cy - h / 2, w, h, r);
}

// Draw a flat eyebrow bar centered at (cx, cy), rotated.
// Since U8g2 doesn't support rotation, we approximate tilted eyebrows
// by drawing at slightly different y positions on left/right.
static void draw_eyebrow(int cx, int cy, int w, int h) {
    u8g2.drawBox(cx - w / 2, cy - h / 2, w, h);
}

// Draw blush cheek (two small circles with a pattern for "pink" on mono)
static void draw_cheek(int cx, int cy, int r) {
    // Dithered circle: draw outline + inner dots for blush effect
    u8g2.drawCircle(cx, cy, r);
    u8g2.drawCircle(cx, cy, r - 1);
    // Inner dither pattern (every other pixel)
    for (int dy = -r + 2; dy <= r - 2; dy += 2) {
        for (int dx = -r + 2; dx <= r - 2; dx += 2) {
            if (dx * dx + dy * dy <= (r - 1) * (r - 1)) {
                u8g2.drawPixel(cx + dx, cy + dy);
            }
        }
    }
}

// ============================================================
// Eye Drawing (per expression + blink state)
// ============================================================
static void draw_eye(int cx, int cy, PetExpression expr) {
    // Eye dimensions
    const int EYE_W = 16;   // width of rounded-rect eye
    const int EYE_H = 14;   // height of rounded-rect eye
    const int EYE_R = 4;    // corner radius

    switch (expr) {
        case EXPR_LOVE: {
            // Pulsing heart eyes — slight size variation
            int pulse = 6 + ((millis() / 150) % 3); // 6-8 half-width
            draw_heart(cx, cy - 2, pulse);
            break;
        }

        case EXPR_HAPPY: {
            // Curved arc eyes (smile-eyes): upward arc
            // Draw a thick arc above center — like ^^ eyes
            u8g2.drawCircle(cx, cy + 6, 9, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
            u8g2.drawCircle(cx, cy + 6, 8, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
            u8g2.drawCircle(cx, cy + 6, 7, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
            break;
        }

        case EXPR_SURPRISED: {
            // Large circular eyes
            u8g2.drawDisc(cx, cy, 9);
            // Hollow center for "wide open" look
            u8g2.setDrawColor(0);
            u8g2.drawDisc(cx, cy, 5);
            u8g2.setDrawColor(1);
            // Small pupil dot
            u8g2.drawDisc(cx, cy, 2);
            break;
        }

        case EXPR_SLEEPY: {
            // Half-closed eyes — thin horizontal slit
            int slit_h = 4;
            // Draw a line-like eye (thin rounded rect)
            u8g2.drawRBox(cx - EYE_W / 2, cy - slit_h / 2, EYE_W, slit_h, 2);
            // Droop line above (eyelid)
            u8g2.drawLine(cx - EYE_W / 2, cy - slit_h / 2,
                          cx + EYE_W / 2, cy - slit_h / 2 - 2);
            break;
        }

        default: {
            // neutral, confused, blush — standard rounded-rect eyes
            int h = EYE_H;

            // Apply blink animation
            if (blink_state == 2) {
                h = 3;  // fully closed
            } else if (blink_state == 1 || blink_state == 3) {
                h = EYE_H / 2;  // half closed
            }

            draw_rounded_rect_eye(cx, cy, EYE_W, h, EYE_R);
            break;
        }
    }
}

// ============================================================
// Eyebrow Drawing (per expression)
// ============================================================
static void draw_eyebrows(int left_cx, int right_cx, int cy, PetExpression expr) {
    const int BW = 16;  // eyebrow width
    const int BH = 3;   // eyebrow height

    switch (expr) {
        case EXPR_NEUTRAL:
        case EXPR_BLUSH: {
            // Flat horizontal eyebrows
            draw_eyebrow(left_cx, cy, BW, BH);
            draw_eyebrow(right_cx, cy, BW, BH);
            break;
        }

        case EXPR_CONFUSED: {
            // Left eyebrow tilted up, right tilted down
            // Approximate by shifting y: left higher on outside, right higher on inside
            // Left eyebrow: angled up-left (higher on left side)
            u8g2.drawLine(left_cx - BW / 2, cy + 2, left_cx + BW / 2, cy - 2);
            u8g2.drawLine(left_cx - BW / 2, cy + 3, left_cx + BW / 2, cy - 1);
            // Right eyebrow: angled up-right (higher on right side)
            u8g2.drawLine(right_cx - BW / 2, cy - 2, right_cx + BW / 2, cy + 2);
            u8g2.drawLine(right_cx - BW / 2, cy - 1, right_cx + BW / 2, cy + 3);
            break;
        }

        case EXPR_SURPRISED: {
            // Raised eyebrows (higher than normal, small arcs)
            u8g2.drawLine(left_cx - BW / 2, cy, left_cx + BW / 2, cy);
            u8g2.drawLine(left_cx - BW / 2, cy + 1, left_cx + BW / 2, cy + 1);
            u8g2.drawLine(right_cx - BW / 2, cy, right_cx + BW / 2, cy);
            u8g2.drawLine(right_cx - BW / 2, cy + 1, right_cx + BW / 2, cy + 1);
            break;
        }

        default:
            // happy, love, sleepy — no eyebrows
            break;
    }
}

// ============================================================
// Full Face Draw
// ============================================================
void display_face(PetExpression expr, const char* label) {
    if (!display_initialized) return;

    u8g2.clearBuffer();

    // ---- Face Layout Constants ----
    // Screen: 128 x 64
    // Face area: top 52 pixels
    // Label area: bottom 12 pixels (y=52..63)

    const int FACE_CENTER_X = 64;
    const int FACE_CENTER_Y = 24;    // vertical center of face area
    const int EYE_SPACING = 24;      // half-distance between eye centers
    const int EYEBROW_OFFSET_Y = 16; // eyebrow is this far above face center

    int left_eye_x  = FACE_CENTER_X - EYE_SPACING;
    int right_eye_x = FACE_CENTER_X + EYE_SPACING;
    int eye_y = FACE_CENTER_Y;

    // ---- Draw cheeks (behind eyes) ----
    if (expr == EXPR_BLUSH || expr == EXPR_LOVE || expr == EXPR_HAPPY) {
        draw_cheek(left_eye_x - 14, eye_y + 10, 5);
        draw_cheek(right_eye_x + 14, eye_y + 10, 5);
    }

    // ---- Draw eyes ----
    draw_eye(left_eye_x, eye_y, expr);
    draw_eye(right_eye_x, eye_y, expr);

    // ---- Draw eyebrows ----
    int brow_y = FACE_CENTER_Y - EYEBROW_OFFSET_Y;
    draw_eyebrows(left_eye_x, right_eye_x, brow_y, expr);

    // ---- Small mouth for some expressions ----
    if (expr == EXPR_HAPPY) {
        // Small smile arc below eyes
        u8g2.drawCircle(FACE_CENTER_X, eye_y + 6, 6,
                        U8G2_DRAW_LOWER_LEFT | U8G2_DRAW_LOWER_RIGHT);
    } else if (expr == EXPR_SURPRISED) {
        // Small "O" mouth
        u8g2.drawCircle(FACE_CENTER_X, eye_y + 14, 3);
    }

    // ---- Divider line above label ----
    u8g2.drawHLine(0, 52, 128);

    // ---- Status label at bottom ----
    if (label && label[0] != '\0') {
        u8g2.setFont(u8g2_font_6x10_tf);
        int lw = u8g2.getStrWidth(label);
        u8g2.drawStr((128 - lw) / 2, 54, label);
    }

    u8g2.sendBuffer();
}

// ============================================================
// Legacy / Status Screens (unchanged API, now with face)
// ============================================================

void display_message(const char* title, const char* message) {
    if (!display_initialized) return;

    u8g2.clearBuffer();

    // Title in larger font
    u8g2.setFont(u8g2_font_7x14B_tf);
    int title_width = u8g2.getStrWidth(title);
    u8g2.drawStr((128 - title_width) / 2, 5, title);

    // Divider line
    u8g2.drawHLine(0, 22, 128);

    // Message in smaller font
    u8g2.setFont(u8g2_font_6x10_tf);
    int msg_width = u8g2.getStrWidth(message);
    u8g2.drawStr((128 - msg_width) / 2, 30, message);

    u8g2.sendBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
}

void display_status(const char* line1, const char* line2, const char* line3) {
    if (!display_initialized) return;

    u8g2.clearBuffer();

    // Header bar
    u8g2.setFont(u8g2_font_7x14B_tf);
    u8g2.drawStr(4, 0, "LearningBuddy");
    u8g2.drawHLine(0, 16, 128);

    // Status lines
    u8g2.setFont(u8g2_font_6x10_tf);
    if (line1) u8g2.drawStr(4, 20, line1);
    if (line2) u8g2.drawStr(4, 34, line2);
    if (line3) u8g2.drawStr(4, 48, line3);

    u8g2.sendBuffer();
}

// ---- Screens that now use the DeskPet face ----

void display_setup_mode() {
    display_face(EXPR_SLEEPY, "Connect USB to setup");
}

void display_wifi_connecting(const char* ssid) {
    char label[32];
    snprintf(label, sizeof(label), "WiFi: %.20s", ssid);
    display_face(EXPR_CONFUSED, label);
}

void display_idle(const char* ssid, const char* ip) {
    (void)ssid;
    (void)ip;
    display_face(EXPR_NEUTRAL, "Ready  A=Rec  B=Voice");
}

void display_error(const char* error_msg) {
    // Show confused face with the error
    char label[32];
    snprintf(label, sizeof(label), "%.21s", error_msg);
    display_face(EXPR_CONFUSED, label);
}

void display_recording(unsigned long elapsed_seconds) {
    if (!display_initialized) return;

    u8g2.clearBuffer();

    // Draw surprised/alert face in the top area but smaller to leave room for timer
    const int FACE_CX = 64;
    const int FACE_CY = 16;
    const int EYE_SPACING = 20;

    // Simplified surprised eyes (smaller)
    int lx = FACE_CX - EYE_SPACING;
    int rx = FACE_CX + EYE_SPACING;

    // Large round eyes
    u8g2.drawDisc(lx, FACE_CY, 7);
    u8g2.setDrawColor(0);
    u8g2.drawDisc(lx, FACE_CY, 4);
    u8g2.setDrawColor(1);
    u8g2.drawDisc(lx, FACE_CY, 2);

    u8g2.drawDisc(rx, FACE_CY, 7);
    u8g2.setDrawColor(0);
    u8g2.drawDisc(rx, FACE_CY, 4);
    u8g2.setDrawColor(1);
    u8g2.drawDisc(rx, FACE_CY, 2);

    // Blinking record dot
    if ((millis() / 500) % 2 == 0) {
        u8g2.drawDisc(FACE_CX, FACE_CY, 3);
    }

    // Timer below the face
    u8g2.setFont(u8g2_font_logisoso16_tn);
    char timer[9];
    unsigned long mins = elapsed_seconds / 60;
    unsigned long secs = elapsed_seconds % 60;
    snprintf(timer, sizeof(timer), "%02lu:%02lu", mins, secs);
    int tw = u8g2.getStrWidth(timer);
    u8g2.drawStr((128 - tw) / 2, 30, timer);

    // Label
    u8g2.drawHLine(0, 52, 128);
    u8g2.setFont(u8g2_font_6x10_tf);
    const char* rec_label = "REC  Streaming...";
    int lw = u8g2.getStrWidth(rec_label);
    u8g2.drawStr((128 - lw) / 2, 54, rec_label);

    u8g2.sendBuffer();
}

// ---- Voice Call Screens with Face ----

void display_voice_listening() {
    display_face(EXPR_NEUTRAL, "Listening...");
}

void display_voice_thinking() {
    // Confused face with animated dots
    char label[24];
    int dots = (millis() / 400) % 4;
    snprintf(label, sizeof(label), "Thinking");
    for (int i = 0; i < dots; i++) {
        size_t len = strlen(label);
        if (len < sizeof(label) - 1) {
            label[len] = '.';
            label[len + 1] = '\0';
        }
    }
    display_face(EXPR_CONFUSED, label);
}

void display_voice_playing() {
    display_face(EXPR_HAPPY, "Speaking...");
}

void display_voice_ready() {
    display_face(EXPR_NEUTRAL, "Hold B to talk");
}

// ============================================================
// Test Screens (legacy, unchanged)
// ============================================================

void display_test_pattern() {
    if (!display_initialized) return;

    u8g2.clearBuffer();
    u8g2.drawFrame(0, 0, 128, 64);
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(20, 28, "PatHacks Device");
    u8g2.sendBuffer();
    delay(1500);

    u8g2.clearBuffer();
    for (int y = 0; y < 64; y += 8) {
        for (int x = 0; x < 128; x += 8) {
            if ((x / 8 + y / 8) % 2 == 0) {
                u8g2.drawBox(x, y, 8, 8);
            }
        }
    }
    u8g2.sendBuffer();
    delay(1000);

    u8g2.clearBuffer();
    for (int r = 5; r <= 30; r += 5) {
        u8g2.drawCircle(64, 32, r);
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

    // Test each expression
    Serial.println("[DSP] Testing DeskPet expressions...");
    display_face(EXPR_NEUTRAL, "neutral");
    delay(1000);
    display_face(EXPR_HAPPY, "happy");
    delay(1000);
    display_face(EXPR_SURPRISED, "surprised");
    delay(1000);
    display_face(EXPR_SLEEPY, "sleepy");
    delay(1000);
    display_face(EXPR_LOVE, "love");
    delay(1000);
    display_face(EXPR_CONFUSED, "confused");
    delay(1000);
    display_face(EXPR_BLUSH, "blush");
    delay(1000);

    display_test_pattern();
    display_message("Display", "Test PASSED");
    delay(1000);

    Serial.println("[DSP] Test PASSED.");
}
