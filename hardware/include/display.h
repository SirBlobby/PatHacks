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

#endif // DISPLAY_H
