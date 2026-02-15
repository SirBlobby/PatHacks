// ============================================================
// Button Module - Two Push Buttons using Bounce2 Library
// Active-low with internal pull-ups (pressed = LOW)
// ============================================================

#include <Arduino.h>
#include <Bounce2.h>
#include "buttons.h"
#include "pins.h"

static Bounce btnA;
static Bounce btnB;

void buttons_init() {
    btnA.attach(BUTTON_A_PIN, INPUT_PULLUP);
    btnA.interval(25);  // 25ms debounce

    btnB.attach(BUTTON_B_PIN, INPUT_PULLUP);
    btnB.interval(25);

    Serial.println("[BTN] Buttons initialized (A=GPIO" + String(BUTTON_A_PIN) +
                   ", B=GPIO" + String(BUTTON_B_PIN) + ")");
}

void buttons_update() {
    btnA.update();
    btnB.update();
}

// --- Edge detection: pressed (fell = HIGH→LOW on active-low button) ---
bool btn_a_pressed() { return btnA.fell(); }
bool btn_b_pressed() { return btnB.fell(); }

// --- Edge detection: released (rose = LOW→HIGH on active-low button) ---
bool btn_a_released() { return btnA.rose(); }
bool btn_b_released() { return btnB.rose(); }

// --- Level: held down (read LOW = pressed) ---
bool btn_a_held() { return btnA.read() == LOW; }
bool btn_b_held() { return btnB.read() == LOW; }
