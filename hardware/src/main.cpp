// ============================================================
// LearningBuddy - Main Firmware
// XIAO ESP32-S3 Sense
//
// State Machine:
//   BOOT -> CHECK_CONFIG -> SETUP_MODE (if not provisioned)
//                        -> WIFI_CONNECTING (if provisioned)
//   WIFI_CONNECTING -> WIFI_CONNECTED -> SIO_CONNECTING -> AUTHENTICATED -> IDLE
//   IDLE -> RECORDING (Button A short press) -> IDLE (Button A short press)
//   IDLE -> VOICE_LISTENING (Button B press) -> VOICE_THINKING (Button B release)
//        -> VOICE_PLAYING (response received) -> VOICE_READY (playback done)
//        -> VOICE_LISTENING (Button B press again) or IDLE (Button B long press)
//   Any state: Button A long press = factory reset
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
#include "speaker_stream.h"

// ---- Device States ----
enum DeviceState {
    STATE_BOOT,
    STATE_SETUP_MODE,           // Waiting for serial provisioning
    STATE_WIFI_CONNECTING,
    STATE_WIFI_CONNECTED,
    STATE_SIO_CONNECTING,
    STATE_AUTHENTICATED,
    STATE_IDLE,
    STATE_RECORDING,            // Button A: streaming mic to backend for recording
    STATE_VOICE_READY,          // Voice call active, waiting for push-to-talk
    STATE_VOICE_LISTENING,      // Button B held: capturing mic for voice call
    STATE_VOICE_THINKING,       // Button B released: backend processing STT->LLM->TTS
    STATE_VOICE_PLAYING,        // Playing back TTS audio from backend
    STATE_ERROR
};

static DeviceState current_state = STATE_BOOT;
static DeviceState prev_state = STATE_BOOT;

// Recording state
static unsigned long recording_start_time = 0;
static String current_recording_id = "";

// Button A state (GPIO 8 — recording toggle / factory reset)
static bool btn_a_was_pressed = false;
static unsigned long btn_a_press_time = 0;

// Button B state (GPIO 44 — voice call push-to-talk)
static bool btn_b_was_pressed = false;
static unsigned long btn_b_press_time = 0;
static bool btn_b_long_handled = false;  // prevent repeat long-press triggers

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
static void on_sio_binary(const char* event, const uint8_t* data, size_t len);
static void handle_button_a();
static void handle_button_b();
static void start_recording();
static void stop_recording();
static void voice_call_start();
static void voice_call_start_listening();
static void voice_call_stop_listening();
static void voice_call_end();
static void switch_to_mic();
static void switch_to_speaker();

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
    Serial.println("  LearningBuddy Device Firmware v2.0");
    Serial.println("  MCU: XIAO ESP32-S3 Sense");
    Serial.println("  Features: Recording + Voice Call");
    Serial.println("=============================================");
    Serial.println();

    // Initialize display first (for visual feedback)
    display_init();
    display_message("LearningBuddy", "Booting...");

    // Initialize subsystems
    wifi_init();
    serial_proto_init();
    sio_init();
    spk_stream_init();

    // Setup buttons (active LOW with internal pullup)
    pinMode(BUTTON_A_PIN, INPUT_PULLUP);
    pinMode(BUTTON_B_PIN, INPUT_PULLUP);

    // Set Socket.IO callbacks
    sio_on_connect(on_sio_connected);
    sio_on_disconnect(on_sio_disconnected);
    sio_on_event(on_sio_event);
    sio_on_binary(on_sio_binary);

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
    // Advance the blink animation state machine every frame
    display_blink_tick();

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
        && current_state != STATE_RECORDING
        && current_state != STATE_VOICE_LISTENING
        && current_state != STATE_VOICE_PLAYING) {
        if (serial_proto_is_provisioned() && !wifi_is_connected()) {
            change_state(STATE_WIFI_CONNECTING);
        }
    }

    // State machine
    switch (current_state) {
        case STATE_BOOT:
            break;

        case STATE_SETUP_MODE:
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

            String url = serial_proto_get_server();
            parse_server_url(url);

            change_state(STATE_SIO_CONNECTING);
            break;
        }

        case STATE_SIO_CONNECTING: {
            if (millis() - last_sio_attempt > SIO_RECONNECT_DELAY_MS) {
                last_sio_attempt = millis();

                if (!wifi_is_connected()) {
                    change_state(STATE_WIFI_CONNECTING);
                    break;
                }

                if (!sio_is_connected()) {
                    display_face(EXPR_CONFUSED, "Connecting...");
                    sio_connect(server_host, server_port);
                }
            }

            sio_loop();
            break;
        }

        case STATE_AUTHENTICATED: {
            change_state(STATE_IDLE);
            break;
        }

        case STATE_IDLE: {
            sio_loop();

            if (!wifi_is_connected()) {
                change_state(STATE_WIFI_CONNECTING);
                break;
            }
            if (!sio_is_connected()) {
                change_state(STATE_SIO_CONNECTING);
                break;
            }

            // Handle both buttons
            handle_button_a();
            handle_button_b();

            if (millis() - last_display_update > DISPLAY_UPDATE_INTERVAL) {
                display_idle(wifi_get_ssid(), wifi_get_ip().c_str());
                last_display_update = millis();
            }
            break;
        }

        case STATE_RECORDING: {
            sio_loop();
            audio_stream_loop();

            if (!wifi_is_connected() || !sio_is_connected()) {
                Serial.println("[MAIN] Connection lost during recording!");
                stop_recording();
                change_state(STATE_WIFI_CONNECTING);
                break;
            }

            // Button A toggles recording off
            handle_button_a();

            if (millis() - last_display_update > DISPLAY_UPDATE_INTERVAL) {
                unsigned long elapsed = (millis() - recording_start_time) / 1000;
                display_recording(elapsed);
                last_display_update = millis();
            }
            break;
        }

        case STATE_VOICE_READY: {
            sio_loop();

            if (!wifi_is_connected() || !sio_is_connected()) {
                Serial.println("[MAIN] Connection lost during voice call!");
                voice_call_end();
                change_state(STATE_WIFI_CONNECTING);
                break;
            }

            // Button B: press to start listening, long press to end call
            handle_button_b();

            if (millis() - last_display_update > DISPLAY_UPDATE_INTERVAL) {
                display_voice_ready();
                last_display_update = millis();
            }
            break;
        }

        case STATE_VOICE_LISTENING: {
            sio_loop();
            audio_stream_loop();  // stream mic audio to backend as call_audio

            if (!wifi_is_connected() || !sio_is_connected()) {
                voice_call_end();
                change_state(STATE_WIFI_CONNECTING);
                break;
            }

            // Button B release = stop listening
            handle_button_b();

            if (millis() - last_display_update > DISPLAY_UPDATE_INTERVAL) {
                display_voice_listening();
                last_display_update = millis();
            }
            break;
        }

        case STATE_VOICE_THINKING: {
            sio_loop();

            if (!wifi_is_connected() || !sio_is_connected()) {
                voice_call_end();
                change_state(STATE_WIFI_CONNECTING);
                break;
            }

            if (millis() - last_display_update > DISPLAY_UPDATE_INTERVAL) {
                display_voice_thinking();
                last_display_update = millis();
            }
            break;
        }

        case STATE_VOICE_PLAYING: {
            sio_loop();
            spk_stream_loop();  // drain ring buffer to I2S

            if (!wifi_is_connected() || !sio_is_connected()) {
                spk_stream_stop();
                voice_call_end();
                change_state(STATE_WIFI_CONNECTING);
                break;
            }

            // Check if playback is done
            if (!spk_stream_is_playing()) {
                Serial.println("[MAIN] TTS playback finished.");
                // Switch back from speaker to mic-ready state
                spk_stream_stop();
                change_state(STATE_VOICE_READY);
                break;
            }

            if (millis() - last_display_update > DISPLAY_UPDATE_INTERVAL) {
                display_voice_playing();
                last_display_update = millis();
            }
            break;
        }

        case STATE_ERROR:
            // Button A press to retry connection
            handle_button_a();
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
    String working = url;

    if (working.startsWith("http://")) {
        working = working.substring(7);
    } else if (working.startsWith("https://")) {
        working = working.substring(8);
    }

    if (working.endsWith("/")) {
        working = working.substring(0, working.length() - 1);
    }

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

    String key = serial_proto_get_device_key();
    String payload = "{\"key\":\"" + key + "\"}";
    sio_emit("auth", payload.c_str());

    display_face(EXPR_NEUTRAL, "Authenticating...");
}

static void on_sio_disconnected() {
    Serial.println("[MAIN] Socket.IO disconnected.");

    // If we were recording, stop
    if (current_state == STATE_RECORDING) {
        audio_stream_stop();
        current_recording_id = "";
    }

    // If we were in a voice call, clean up
    if (current_state == STATE_VOICE_LISTENING ||
        current_state == STATE_VOICE_THINKING ||
        current_state == STATE_VOICE_PLAYING ||
        current_state == STATE_VOICE_READY) {
        audio_stream_stop();
        spk_stream_stop();
    }
}

static void on_sio_event(const char* event, const char* payload) {
    Serial.printf("[MAIN] Event: %s -> %s\n", event, payload);

    // --- auth_ok ---
    if (strcmp(event, "auth_ok") == 0) {
        Serial.println("[MAIN] Authenticated successfully!");
        display_face(EXPR_LOVE, "Connected!");
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

    // --- call_started ---
    if (strcmp(event, "call_started") == 0) {
        Serial.println("[MAIN] Voice call started (server confirmed).");
        return;
    }

    // --- call_response ---
    // Note: call_response with binary audio comes through on_sio_binary, not here.
    // A text-only call_response could signal metadata or errors.
    if (strcmp(event, "call_response") == 0) {
        Serial.printf("[MAIN] call_response (text): %s\n", payload);
        return;
    }

    // --- call_ended ---
    if (strcmp(event, "call_ended") == 0) {
        Serial.println("[MAIN] Voice call ended (server confirmed).");
        return;
    }

    // --- call_error ---
    if (strcmp(event, "call_error") == 0) {
        Serial.printf("[MAIN] Call error: %s\n", payload);
        audio_stream_stop();
        spk_stream_stop();
        change_state(STATE_IDLE);
        display_error("Call error");
        return;
    }
}

static void on_sio_binary(const char* event, const uint8_t* data, size_t len) {
    Serial.printf("[MAIN] Binary event: %s (%d bytes)\n", event, len);

    // --- call_response binary: TTS audio from backend ---
    if (strcmp(event, "call_response") == 0) {
        if (current_state == STATE_VOICE_THINKING) {
            // Switch I2S to speaker mode and start playback
            switch_to_speaker();
            change_state(STATE_VOICE_PLAYING);
        }

        if (current_state == STATE_VOICE_PLAYING) {
            // Write PCM data to speaker ring buffer
            size_t written = spk_stream_write(data, len);
            if (written < len) {
                Serial.printf("[MAIN] Speaker buffer overflow: lost %d bytes\n", len - written);
            }

            // If this is the last chunk, flush
            // The backend should send all audio in one binary frame,
            // so we flush immediately after receiving.
            spk_stream_flush();
        }
    }
}

// ============================================================
// I2S Bus Switching (mic and speaker share I2S_NUM_0)
// ============================================================
static void switch_to_mic() {
    spk_stream_stop();                  // uninstall speaker I2S driver
    audio_stream_init();                // install mic PDM I2S driver
    audio_stream_start("call_audio");   // start mic capture for voice call
    Serial.println("[MAIN] I2S switched to MIC (RX).");
}

static void switch_to_speaker() {
    audio_stream_stop();      // stop mic capture
    audio_stream_teardown();  // fully uninstall mic I2S driver (frees bus)
    spk_stream_start();       // install and start speaker I2S driver
    Serial.println("[MAIN] I2S switched to SPEAKER (TX).");
}

// ============================================================
// Button A Handling (GPIO 8 — Recording toggle / Factory reset)
// ============================================================
static void handle_button_a() {
    bool pressed = (digitalRead(BUTTON_A_PIN) == LOW);

    if (pressed && !btn_a_was_pressed) {
        // Just pressed
        btn_a_press_time = millis();
        btn_a_was_pressed = true;
    } else if (!pressed && btn_a_was_pressed) {
        // Just released
        unsigned long held = millis() - btn_a_press_time;
        btn_a_was_pressed = false;

        if (held < BUTTON_A_DEBOUNCE_MS) return; // ignore bounce

        if (held >= BUTTON_A_LONG_PRESS_MS) {
            // Long press: factory reset
            Serial.println("[MAIN] Button A long press - factory reset!");
            serial_proto_factory_reset();
            display_message("Factory Reset", "Restarting...");
            delay(1000);
            ESP.restart();
        } else {
            // Short press: toggle recording (only from IDLE or RECORDING states)
            if (current_state == STATE_IDLE) {
                start_recording();
            } else if (current_state == STATE_RECORDING) {
                stop_recording();
            } else if (current_state == STATE_ERROR) {
                // Retry connection
                if (serial_proto_is_provisioned()) {
                    change_state(STATE_WIFI_CONNECTING);
                } else {
                    change_state(STATE_SETUP_MODE);
                }
            }
        }
    }
}

// ============================================================
// Button B Handling (GPIO 44 — Voice call push-to-talk)
// ============================================================
static void handle_button_b() {
    bool pressed = (digitalRead(BUTTON_B_PIN) == LOW);

    if (pressed && !btn_b_was_pressed) {
        // Just pressed
        btn_b_press_time = millis();
        btn_b_was_pressed = true;
        btn_b_long_handled = false;

        // Immediate action on press (start listening)
        if (current_state == STATE_IDLE) {
            voice_call_start();
        } else if (current_state == STATE_VOICE_READY) {
            voice_call_start_listening();
        }
    } else if (pressed && btn_b_was_pressed) {
        // Still held — check for long press
        unsigned long held = millis() - btn_b_press_time;

        if (held >= BUTTON_B_LONG_PRESS_MS && !btn_b_long_handled) {
            btn_b_long_handled = true;

            // Long press: end voice call entirely
            Serial.println("[MAIN] Button B long press - ending voice call.");
            voice_call_end();
        }
    } else if (!pressed && btn_b_was_pressed) {
        // Just released
        unsigned long held = millis() - btn_b_press_time;
        btn_b_was_pressed = false;

        if (held < BUTTON_B_DEBOUNCE_MS) return; // ignore bounce

        // If long press was already handled, don't do anything else
        if (btn_b_long_handled) return;

        // Short release: stop listening, send audio for processing
        if (current_state == STATE_VOICE_LISTENING) {
            voice_call_stop_listening();
        }
    }
}

// ============================================================
// Recording Control (Button A)
// ============================================================
static void start_recording() {
    Serial.println("[MAIN] Starting recording...");

    if (!audio_stream_init()) {
        Serial.println("[MAIN] Failed to init audio!");
        display_error("Mic init failed");
        return;
    }

    sio_emit("rec_start", "{\"title\":\"Lecture Recording\"}");
    audio_stream_start();
    recording_start_time = millis();

    change_state(STATE_RECORDING);
    Serial.println("[MAIN] Recording in progress...");
}

static void stop_recording() {
    Serial.println("[MAIN] Stopping recording...");

    audio_stream_stop();
    sio_emit("rec_stop", "{}");

    change_state(STATE_IDLE);
    display_face(EXPR_HAPPY, "Recording saved!");
}

// ============================================================
// Voice Call Control (Button B)
// ============================================================
static void voice_call_start() {
    Serial.println("[MAIN] Starting voice call...");

    // Emit call_start to backend
    sio_emit("call_start", "{}");

    // Switch I2S to mic and start capturing
    if (!audio_stream_init()) {
        Serial.println("[MAIN] Failed to init mic for voice call!");
        display_error("Mic init failed");
        return;
    }
    audio_stream_start("call_audio");

    change_state(STATE_VOICE_LISTENING);
    Serial.println("[MAIN] Voice call: listening...");
}

static void voice_call_start_listening() {
    Serial.println("[MAIN] Voice call: start listening again...");

    // Switch I2S back to mic
    switch_to_mic();

    change_state(STATE_VOICE_LISTENING);
}

static void voice_call_stop_listening() {
    Serial.println("[MAIN] Voice call: stop listening, waiting for response...");

    // Stop mic capture
    audio_stream_stop();

    // Tell backend we're done speaking
    sio_emit("call_stop_listening", "{}");

    change_state(STATE_VOICE_THINKING);
}

static void voice_call_end() {
    Serial.println("[MAIN] Ending voice call...");

    // Stop everything
    audio_stream_stop();
    spk_stream_stop();

    // Tell backend the call is over
    if (sio_is_connected()) {
        sio_emit("call_end", "{}");
    }

    change_state(STATE_IDLE);
    display_face(EXPR_NEUTRAL, "Call ended");
}
