#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

// ============================================================
// DeskPet Expression enum — matches the web frontend expressions
// ============================================================
enum PetExpression {
    EXPR_NEUTRAL,     // Default: rounded-rect eyes, flat eyebrows
    EXPR_HAPPY,       // Curved arc eyes (smile-eyes), blush cheeks
    EXPR_SURPRISED,   // Large circular eyes, no eyebrows
    EXPR_SLEEPY,      // Half-closed eyes (thin horizontal slits)
    EXPR_LOVE,        // Heart-shaped eyes, blush cheeks
    EXPR_CONFUSED,    // Normal eyes, tilted eyebrows (one up, one down)
    EXPR_BLUSH,       // Normal eyes, blush cheeks
};

// Initialize the SPI OLED display
bool display_init();

// Clear the display
void display_clear();

// ---- Face Rendering (DeskPet) ----

// Draw the DeskPet face with the given expression and a one-line status label.
// The face occupies the top ~52px, the label sits at the bottom.
// Call this in your main loop at your desired refresh rate.
void display_face(PetExpression expr, const char* label = nullptr);

// Advance the blink animation state machine. Call once per loop iteration.
// Returns true if a blink is currently active (for refresh coordination).
void display_blink_tick();

// ---- Legacy Status Screens (for non-face screens) ----

// Show a title and message (two-line layout)
void display_message(const char* title, const char* message);

// Show 3-line status screen with header "LearningBuddy"
void display_status(const char* line1, const char* line2, const char* line3);

// Show recording screen with elapsed timer + face
void display_recording(unsigned long elapsed_seconds);

// Show WiFi connecting screen
void display_wifi_connecting(const char* ssid);

// Show setup/provisioning mode screen
void display_setup_mode();

// Show idle/ready screen with WiFi info
void display_idle(const char* ssid, const char* ip);

// Show error screen
void display_error(const char* error_msg);

// ---- Voice Call Screens (with face) ----

void display_voice_listening();
void display_voice_thinking();
void display_voice_playing();
void display_voice_ready();

// ---- Test functions ----
void display_text(const char* text, int x, int y);
void display_test_pattern();
void display_test();

#endif // DISPLAY_H
