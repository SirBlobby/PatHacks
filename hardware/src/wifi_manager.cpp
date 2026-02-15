// ============================================================
// WiFi Manager - Handles WiFi connection + NVS credential storage
// ============================================================

#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>
#include <Preferences.h>

static Preferences prefs;
static char stored_ssid[64] = {0};
static char stored_pass[64] = {0};
static bool has_creds = false;

void wifi_init() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    // Load saved credentials from NVS
    prefs.begin(NVS_NAMESPACE, true); // read-only
    String ssid = prefs.getString(NVS_KEY_SSID, "");
    String pass = prefs.getString(NVS_KEY_PASS, "");
    prefs.end();

    if (ssid.length() > 0) {
        ssid.toCharArray(stored_ssid, sizeof(stored_ssid));
        pass.toCharArray(stored_pass, sizeof(stored_pass));
        has_creds = true;
        Serial.printf("[WIFI] Found saved credentials for SSID: %s\n", stored_ssid);
    } else {
        Serial.println("[WIFI] No saved WiFi credentials found.");
    }
}

bool wifi_has_credentials() {
    return has_creds;
}

bool wifi_save_credentials(const char* ssid, const char* password) {
    prefs.begin(NVS_NAMESPACE, false); // read-write
    prefs.putString(NVS_KEY_SSID, ssid);
    prefs.putString(NVS_KEY_PASS, password);
    prefs.end();

    strncpy(stored_ssid, ssid, sizeof(stored_ssid) - 1);
    strncpy(stored_pass, password, sizeof(stored_pass) - 1);
    has_creds = true;

    Serial.printf("[WIFI] Credentials saved for SSID: %s\n", ssid);
    return true;
}

bool wifi_connect() {
    if (!has_creds) {
        Serial.println("[WIFI] No credentials to connect with.");
        return false;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[WIFI] Already connected.");
        return true;
    }

    Serial.printf("[WIFI] Connecting to '%s'...\n", stored_ssid);
    WiFi.begin(stored_ssid, stored_pass);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > WIFI_CONNECT_TIMEOUT_MS) {
            Serial.println("[WIFI] Connection timed out.");
            WiFi.disconnect();
            return false;
        }
        delay(250);
        Serial.print(".");
    }

    Serial.println();
    Serial.printf("[WIFI] Connected! IP: %s, RSSI: %d dBm\n",
                  WiFi.localIP().toString().c_str(), WiFi.RSSI());
    return true;
}

void wifi_disconnect() {
    WiFi.disconnect();
    Serial.println("[WIFI] Disconnected.");
}

bool wifi_is_connected() {
    return WiFi.status() == WL_CONNECTED;
}

const char* wifi_get_ssid() {
    return stored_ssid;
}

String wifi_get_ip() {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.localIP().toString();
    }
    return "0.0.0.0";
}

int wifi_get_rssi() {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.RSSI();
    }
    return 0;
}
