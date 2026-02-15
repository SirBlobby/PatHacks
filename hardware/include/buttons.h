#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

// Initialize button GPIOs with internal pull-ups
void buttons_init();

// Call in loop() to update debounce state
void buttons_update();

// Returns true on the tick the button transitions to pressed (single-shot)
bool btn_a_pressed();
bool btn_b_pressed();

// Returns true on the tick the button transitions to released (single-shot)
bool btn_a_released();
bool btn_b_released();

// Returns true while the button is held down (continuous)
bool btn_a_held();
bool btn_b_held();

#endif // BUTTONS_H
