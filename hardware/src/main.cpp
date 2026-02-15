// ============================================================
// PatHacks Hardware - Central Test Program
// Sequentially tests each hardware component:
//   1. Display (SPI OLED)
//   2. Speaker (I2S MAX98357 Amp)
//   3. Microphone (PDM onboard)
// ============================================================

#include <Arduino.h>
#include "pins.h"
// #include "display.h"
#include "speaker.h"
#include "microphone.h"

void setup() {
    Serial.begin(115200);

    // Wait for serial connection (useful for debugging via USB)
    unsigned long serial_timeout = millis();
    while (!Serial && millis() - serial_timeout < 3000) {
        delay(10);
    }

    Serial.println();
    Serial.println("=============================================");
    Serial.println("  PatHacks Hardware Component Test");
    Serial.println("  MCU: XIAO ESP32-S3 Sense");
    Serial.println("=============================================");
    Serial.println();

    // // --- Test 1: Display ---
    // Serial.println(">>> TEST 1/3: OLED Display");
    // display_test();
    // Serial.println();

    delay(500);

    // --- Test 2: Speaker ---
    Serial.println(">>> TEST 2/3: Speaker (MAX98357)");
    // display_message("Testing...", "Speaker");
    spk_test();
    Serial.println();

    delay(500);

    // --- Test 3: Microphone ---
    Serial.println(">>> TEST 3/3: PDM Microphone");
    // display_message("Testing...", "Microphone (5s)");
    mic_test(5000);  // 5 second microphone test
    Serial.println();

    // --- Summary ---
    Serial.println("=============================================");
    Serial.println("  All tests complete!");
    Serial.println("  Check serial output for PASS/FAIL status.");
    Serial.println("=============================================");

    // display_message("All Tests", "Complete!");
}

void loop() {
    // Keep speaker stream moving
    // This handles buffering automatically
    spk_loop();
    delay(1);
}
