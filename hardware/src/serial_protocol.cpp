// ============================================================
// Serial Protocol Handler
// Receives JSON commands over USB serial from the Tauri app
// Protocol:
//   Tauri -> ESP: {"cmd":"PING"}                              -> ESP replies: PONG
//   Tauri -> ESP: {"cmd":"WIFI","ssid":"...","password":"..."} -> ESP replies: WIFI_OK or WIFI_FAIL:reason
//   Tauri -> ESP: {"cmd":"CONFIG","server":"...","device_key":"..."} -> ESP replies: CONFIG_OK or CONFIG_FAIL:reason
//   Tauri -> ESP: {"cmd":"STATUS"}                            -> ESP replies: STATUS:{json}
//   Tauri -> ESP: {"cmd":"RESET"}                             -> ESP replies: RESET_OK (clears NVS)
// ============================================================

#include "serial_protocol.h"
#include "config.h"
#include "wifi_manager.h"
#include <ArduinoJson.h>
#include <Preferences.h>

static String serial_buffer = "";
static Preferences config_prefs;

// Stored config
static char server_url[128] = {0};
static char device_key[16] = {0};
static bool config_loaded = false;

void serial_proto_init() {
    // Load saved config from NVS
    config_prefs.begin(NVS_NAMESPACE, true); // read-only
    String srv = config_prefs.getString(NVS_KEY_SERVER, "");
    String key = config_prefs.getString(NVS_KEY_DEVICE_KEY, "");
    config_prefs.end();

    if (srv.length() > 0) {
        srv.toCharArray(server_url, sizeof(server_url));
        Serial.printf("[SERIAL] Loaded server URL: %s\n", server_url);
    }
    if (key.length() > 0) {
        key.toCharArray(device_key, sizeof(device_key));
        Serial.printf("[SERIAL] Loaded device key: %s\n", device_key);
    }

    config_loaded = (srv.length() > 0 && key.length() > 0);
    Serial.println("[SERIAL] Protocol handler initialized.");
}

bool serial_proto_is_provisioned() {
    return wifi_has_credentials() && config_loaded;
}

String serial_proto_get_server() {
    return String(server_url);
}

String serial_proto_get_device_key() {
    return String(device_key);
}

void serial_proto_send(const char* message) {
    Serial.println(message);
}

void serial_proto_factory_reset() {
    Preferences p;
    p.begin(NVS_NAMESPACE, false);
    p.clear();
    p.end();

    memset(server_url, 0, sizeof(server_url));
    memset(device_key, 0, sizeof(device_key));
    config_loaded = false;

    Serial.println("[SERIAL] Factory reset complete. All config cleared.");
}

static void handle_command(const char* json_str) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json_str);

    if (err) {
        serial_proto_send("ERROR:Invalid JSON");
        return;
    }

    const char* cmd = doc["cmd"] | "";

    // --- PING ---
    if (strcmp(cmd, "PING") == 0) {
        serial_proto_send("PONG");
        return;
    }

    // --- WIFI ---
    if (strcmp(cmd, "WIFI") == 0) {
        const char* ssid = doc["ssid"] | "";
        const char* password = doc["password"] | "";

        if (strlen(ssid) == 0) {
            serial_proto_send("WIFI_FAIL:Missing SSID");
            return;
        }

        // Save credentials
        wifi_save_credentials(ssid, password);

        // Attempt connection
        if (wifi_connect()) {
            serial_proto_send("WIFI_OK");
        } else {
            serial_proto_send("WIFI_FAIL:Connection timed out");
        }
        return;
    }

    // --- CONFIG (server URL + device key) ---
    if (strcmp(cmd, "CONFIG") == 0) {
        const char* srv = doc["server"] | "";
        const char* key = doc["device_key"] | "";

        if (strlen(srv) == 0 || strlen(key) == 0) {
            serial_proto_send("CONFIG_FAIL:Missing server or device_key");
            return;
        }

        // Save to NVS
        config_prefs.begin(NVS_NAMESPACE, false);
        config_prefs.putString(NVS_KEY_SERVER, srv);
        config_prefs.putString(NVS_KEY_DEVICE_KEY, key);
        config_prefs.end();

        strncpy(server_url, srv, sizeof(server_url) - 1);
        strncpy(device_key, key, sizeof(device_key) - 1);
        config_loaded = true;

        Serial.printf("[SERIAL] Config saved - Server: %s, Key: %s\n", server_url, device_key);
        serial_proto_send("CONFIG_OK");
        return;
    }

    // --- STATUS ---
    if (strcmp(cmd, "STATUS") == 0) {
        JsonDocument resp;
        resp["wifi_connected"] = wifi_is_connected();
        resp["wifi_ssid"] = wifi_get_ssid();
        resp["wifi_ip"] = wifi_get_ip();
        resp["wifi_rssi"] = wifi_get_rssi();
        resp["server_url"] = server_url;
        resp["device_key"] = device_key;
        resp["provisioned"] = serial_proto_is_provisioned();

        String out;
        serializeJson(resp, out);
        Serial.print("STATUS:");
        Serial.println(out);
        return;
    }

    // --- RESET ---
    if (strcmp(cmd, "RESET") == 0) {
        serial_proto_factory_reset();
        serial_proto_send("RESET_OK");
        return;
    }

    serial_proto_send("ERROR:Unknown command");
}

bool serial_proto_loop() {
    bool processed = false;

    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (serial_buffer.length() > 0) {
                handle_command(serial_buffer.c_str());
                serial_buffer = "";
                processed = true;
            }
        } else {
            serial_buffer += c;
            // Safety limit
            if (serial_buffer.length() > 512) {
                serial_buffer = "";
                serial_proto_send("ERROR:Buffer overflow");
            }
        }
    }

    return processed;
}
