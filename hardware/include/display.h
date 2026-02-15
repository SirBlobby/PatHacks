#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

// Initialize the SPI OLED display
bool display_init();

// Clear the display
void display_clear();

// Show a text message on the display
// text: string to display
// x, y: pixel position
void display_text(const char* text, int x, int y);

// Show a title and message (two-line layout)
void display_message(const char* title, const char* message);

// Show a progress-style test screen
void display_test_pattern();

// Test the display by showing text and patterns
void display_test();

// ---- LearningBuddy Status Screens ----

// Show 3-line status screen with header "LearningBuddy"
void display_status(const char* line1, const char* line2, const char* line3);

// Show recording screen with elapsed timer
void display_recording(unsigned long elapsed_seconds);

// Show WiFi connecting screen
void display_wifi_connecting(const char* ssid);

// Show setup/provisioning mode screen
void display_setup_mode();

// Show idle/ready screen with WiFi info
void display_idle(const char* ssid, const char* ip);

// Show error screen
void display_error(const char* error_msg);

// ---- Voice Call Screens ----

// Show "Listening..." screen with mic icon (user is speaking)
void display_voice_listening();

// Show "Processing..." screen (backend is doing STT -> LLM -> TTS)
void display_voice_thinking();

// Show "Speaking..." screen with speaker icon (AI response playing)
void display_voice_playing();

// Show "Voice Call" idle/ready screen (call active, waiting for push-to-talk)
void display_voice_ready();

#endif // DISPLAY_H
