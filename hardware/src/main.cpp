// ============================================================
// LearningBuddy - Main Firmware
// XIAO ESP32-S3 Sense
//
// State Machine:
//   BOOT -> CHECK_CONFIG -> SETUP_MODE (if not provisioned)
//                        -> WIFI_CONNECTING (if provisioned)
//   WIFI_CONNECTING -> WIFI_CONNECTED -> SIO_CONNECTING -> AUTHENTICATED -> IDLE
//   IDLE -> RECORDING (button press) -> IDLE (button press again)
//   Any state handles serial commands from Tauri setup app
// ============================================================

#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "pins.h"
#include "display.h"
#include "wifi_manager.h"
#include "serial_protocol.h"
#include "socketio_client.h"
#include "audio_stream.h"

// ---- Device States ----
enum DeviceState {
    STATE_BOOT,
    STATE_SETUP_MODE,       // Waiting for serial provisioning
    STATE_WIFI_CONNECTING,
    STATE_WIFI_CONNECTED,
    STATE_SIO_CONNECTING,
    STATE_AUTHENTICATED,
    STATE_IDLE,
    STATE_RECORDING,
    STATE_ERROR
};

static DeviceState current_state = STATE_BOOT;
static DeviceState prev_state = STATE_BOOT;

// Recording state
static unsigned long recording_start_time = 0;
static String current_recording_id = "";

// Button state
static bool button_pressed = false;
static unsigned long button_press_time = 0;
static bool button_was_pressed = false;

// Reconnect tracking
static unsigned long last_wifi_attempt = 0;
static unsigned long last_sio_attempt = 0;

// Display update throttle
static unsigned long last_display_update = 0;
#define DISPLAY_UPDATE_INTERVAL 500  // ms

// Server connection info (parsed from URL)
static char server_host[64] = {0};
static uint16_t server_port = 5000;

// Forward declarations
static void change_state(DeviceState new_state);
static void parse_server_url(const String& url);
static void on_sio_connected();
static void on_sio_disconnected();
static void on_sio_event(const char* event, const char* payload);
static void handle_button();
static void start_recording();
static void stop_recording();

// ============================================================
// Setup
// ============================================================
void setup() {
    Serial.begin(SERIAL_BAUD);

    // Wait for serial (up to 3 seconds)
    unsigned long serial_timeout = millis();
    while (!Serial && millis() - serial_timeout < 3000) {
        delay(10);
    }

    Serial.println();
    Serial.println("=============================================");
    Serial.println("  LearningBuddy Device Firmware v1.0");
    Serial.println("  MCU: XIAO ESP32-S3 Sense");
    Serial.println("=============================================");
    Serial.println();

    // Initialize display first (for visual feedback)
    display_init();
    display_message("LearningBuddy", "Booting...");

    // Initialize subsystems
    wifi_init();
    serial_proto_init();
    sio_init();

    // Setup button (BOOT button = GPIO 0, active LOW)
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // Set Socket.IO callbacks
    sio_on_connect(on_sio_connected);
    sio_on_disconnect(on_sio_disconnected);
    sio_on_event(on_sio_event);

    // Check if device is provisioned
    if (serial_proto_is_provisioned()) {
        Serial.println("[MAIN] Device is provisioned. Connecting to WiFi...");
        change_state(STATE_WIFI_CONNECTING);
    } else {
        Serial.println("[MAIN] Device not provisioned. Entering setup mode.");
        change_state(STATE_SETUP_MODE);
    }
}

// ============================================================
// Main Loop
// ============================================================
void loop() {
    // Always process serial commands (allows re-provisioning anytime)
    bool serial_cmd = serial_proto_loop();

    // If we got a serial command and we're in setup mode,
    // check if we're now provisioned
    if (serial_cmd && current_state == STATE_SETUP_MODE) {
        if (serial_proto_is_provisioned()) {
            Serial.println("[MAIN] Provisioning complete! Connecting...");
            change_state(STATE_WIFI_CONNECTING);
        }
    }

    // If WiFi command came through while already connected, allow re-provisioning
    if (serial_cmd && current_state != STATE_SETUP_MODE
        && current_state != STATE_WIFI_CONNECTING
        && current_state != STATE_RECORDING) {
        if (serial_proto_is_provisioned() && !wifi_is_connected()) {
            change_state(STATE_WIFI_CONNECTING);
        }
    }

    // State machine
    switch (current_state) {
        case STATE_BOOT:
            // Should not stay here
            break;

        case STATE_SETUP_MODE:
            // Show setup screen, just wait for serial commands
            if (millis() - last_display_update > DISPLAY_UPDATE_INTERVAL) {
                display_setup_mode();
                last_display_update = millis();
            }
            break;

        case STATE_WIFI_CONNECTING: {
            if (wifi_is_connected()) {
                change_state(STATE_WIFI_CONNECTED);
                break;
            }

            // Attempt connection with throttling
            if (millis() - last_wifi_attempt > WIFI_RETRY_DELAY_MS) {
                display_wifi_connecting(wifi_get_ssid());
                last_wifi_attempt = millis();

                if (wifi_connect()) {
                    change_state(STATE_WIFI_CONNECTED);
                } else {
                    Serial.println("[MAIN] WiFi connection failed. Retrying...");
                    display_error("WiFi failed");
                }
            }
            break;
        }

        case STATE_WIFI_CONNECTED: {
            Serial.println("[MAIN] WiFi connected. Connecting to backend...");

            // Parse server URL for Socket.IO connection
            String url = serial_proto_get_server();
            parse_server_url(url);

            change_state(STATE_SIO_CONNECTING);
            break;
        }

        case STATE_SIO_CONNECTING: {
            // Attempt Socket.IO connection with throttling
            if (millis() - last_sio_attempt > SIO_RECONNECT_DELAY_MS) {
                last_sio_attempt = millis();

                if (!wifi_is_connected()) {
                    change_state(STATE_WIFI_CONNECTING);
                    break;
                }

                if (!sio_is_connected()) {
                    display_status("Connecting...", server_host, "Socket.IO");
                    sio_connect(server_host, server_port);
                }
            }

            // Process WebSocket
            sio_loop();

            // sio_on_connect callback will change state to AUTHENTICATED
            break;
        }

        case STATE_AUTHENTICATED: {
            // We've authenticated. Transition to idle
            change_state(STATE_IDLE);
            break;
        }

        case STATE_IDLE: {
            // Process Socket.IO
            sio_loop();

            // Check WiFi
            if (!wifi_is_connected()) {
                change_state(STATE_WIFI_CONNECTING);
                break;
            }

            // Check Socket.IO
            if (!sio_is_connected()) {
                change_state(STATE_SIO_CONNECTING);
                break;
            }

            // Handle button press to start recording
            handle_button();

            // Update display
            if (millis() - last_display_update > DISPLAY_UPDATE_INTERVAL) {
                display_idle(wifi_get_ssid(), wifi_get_ip().c_str());
                last_display_update = millis();
            }
            break;
        }

        case STATE_RECORDING: {
            // Process Socket.IO
            sio_loop();

            // Stream audio
            audio_stream_loop();

            // Check connections
            if (!wifi_is_connected() || !sio_is_connected()) {
                Serial.println("[MAIN] Connection lost during recording!");
                stop_recording();
                change_state(STATE_WIFI_CONNECTING);
                break;
            }

            // Handle button press to stop recording
            handle_button();

            // Update display with timer
            if (millis() - last_display_update > DISPLAY_UPDATE_INTERVAL) {
                unsigned long elapsed = (millis() - recording_start_time) / 1000;
                display_recording(elapsed);
                last_display_update = millis();
            }
            break;
        }

        case STATE_ERROR:
            // Display error, wait for serial commands or button press to retry
            handle_button();
            if (button_pressed) {
                if (serial_proto_is_provisioned()) {
                    change_state(STATE_WIFI_CONNECTING);
                } else {
                    change_state(STATE_SETUP_MODE);
                }
            }
            break;
    }

    delay(1);  // Small yield
}

// ============================================================
// State Management
// ============================================================
static void change_state(DeviceState new_state) {
    if (new_state == current_state) return;

    Serial.printf("[MAIN] State: %d -> %d\n", current_state, new_state);
    prev_state = current_state;
    current_state = new_state;
    last_display_update = 0; // Force display update
}

// ============================================================
// Server URL Parser
// ============================================================
static void parse_server_url(const String& url) {
    // Parse "http://host:port" or "http://host"
    String working = url;

    // Strip protocol
    if (working.startsWith("http://")) {
        working = working.substring(7);
    } else if (working.startsWith("https://")) {
        working = working.substring(8);
    }

    // Strip trailing slash
    if (working.endsWith("/")) {
        working = working.substring(0, working.length() - 1);
    }

    // Split host:port
    int colon = working.indexOf(':');
    if (colon > 0) {
        working.substring(0, colon).toCharArray(server_host, sizeof(server_host));
        server_port = working.substring(colon + 1).toInt();
    } else {
        working.toCharArray(server_host, sizeof(server_host));
        server_port = 80;
    }

    Serial.printf("[MAIN] Server: %s:%d\n", server_host, server_port);
}

// ============================================================
// Socket.IO Callbacks
// ============================================================
static void on_sio_connected() {
    Serial.println("[MAIN] Socket.IO connected. Sending auth...");

    // Send auth event with device key
    String key = serial_proto_get_device_key();
    String payload = "{\"key\":\"" + key + "\"}";
    sio_emit("auth", payload.c_str());

    display_status("Connected", "Authenticating...", "");
}

static void on_sio_disconnected() {
    Serial.println("[MAIN] Socket.IO disconnected.");

    // If we were recording, stop
    if (current_state == STATE_RECORDING) {
        audio_stream_stop();
        current_recording_id = "";
    }

    // Go back to reconnecting (but don't change state in callback directly)
    // The main loop will detect sio_is_connected() == false
}

static void on_sio_event(const char* event, const char* payload) {
    Serial.printf("[MAIN] Event: %s -> %s\n", event, payload);

    // --- auth_ok ---
    if (strcmp(event, "auth_ok") == 0) {
        Serial.println("[MAIN] Authenticated successfully!");
        display_status("Authenticated!", "Ready to record", "Press button");
        change_state(STATE_AUTHENTICATED);
        return;
    }

    // --- auth_error ---
    if (strcmp(event, "auth_error") == 0) {
        Serial.printf("[MAIN] Auth error: %s\n", payload);
        display_error("Auth failed");
        change_state(STATE_ERROR);
        return;
    }

    // --- rec_started ---
    if (strcmp(event, "rec_started") == 0) {
        // Parse recording_id from payload
        JsonDocument doc;
        if (deserializeJson(doc, payload) == DeserializationError::Ok) {
            current_recording_id = doc["recording_id"].as<String>();
            Serial.printf("[MAIN] Recording started: %s\n", current_recording_id.c_str());
        }
        return;
    }

    // --- rec_stopped ---
    if (strcmp(event, "rec_stopped") == 0) {
        Serial.println("[MAIN] Recording stopped (server confirmed).");
        current_recording_id = "";
        return;
    }

    // --- rec_error ---
    if (strcmp(event, "rec_error") == 0) {
        Serial.printf("[MAIN] Recording error: %s\n", payload);
        audio_stream_stop();
        current_recording_id = "";
        change_state(STATE_IDLE);
        display_error("Recording error");
        return;
    }
}

// ============================================================
// Button Handling
// ============================================================
static void handle_button() {
    bool pressed = (digitalRead(BUTTON_PIN) == LOW); // Active LOW

    button_pressed = false; // Reset edge detection

    if (pressed && !button_was_pressed) {
        // Button just pressed
        button_press_time = millis();
        button_was_pressed = true;
    } else if (!pressed && button_was_pressed) {
        // Button just released
        unsigned long held = millis() - button_press_time;
        button_was_pressed = false;

        if (held > BUTTON_DEBOUNCE_MS) {
            button_pressed = true; // Valid press detected

            if (held > LONG_PRESS_MS) {
                // Long press: factory reset
                Serial.println("[MAIN] Long press detected - factory reset!");
                serial_proto_factory_reset();
                display_message("Factory Reset", "Restarting...");
                delay(1000);
                ESP.restart();
            } else {
                // Short press: toggle recording
                if (current_state == STATE_IDLE) {
                    start_recording();
                } else if (current_state == STATE_RECORDING) {
                    stop_recording();
                }
            }
        }
    }
}

// ============================================================
// Recording Control
// ============================================================
static void start_recording() {
    Serial.println("[MAIN] Starting recording...");

    // Initialize audio if needed
    if (!audio_stream_init()) {
        Serial.println("[MAIN] Failed to init audio!");
        display_error("Mic init failed");
        return;
    }

    // Tell backend to start recording
    sio_emit("rec_start", "{\"title\":\"Lecture Recording\"}");

    // Start streaming audio
    audio_stream_start();
    recording_start_time = millis();

    change_state(STATE_RECORDING);
    Serial.println("[MAIN] Recording in progress...");
}

static void stop_recording() {
    Serial.println("[MAIN] Stopping recording...");

    // Stop streaming audio
    audio_stream_stop();

    // Tell backend to stop recording
    sio_emit("rec_stop", "{}");

    change_state(STATE_IDLE);
    display_status("Recording saved", "Processing...", "");
}
