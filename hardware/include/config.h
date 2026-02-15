#ifndef CONFIG_H
#define CONFIG_H

// =============================================================
// Configuration for LearningBuddy ESP32 Device
// These defaults can be overridden via serial provisioning
// and are stored in NVS (non-volatile storage)
// =============================================================

// --- NVS Namespace ---
#define NVS_NAMESPACE      "lbuddy"

// --- NVS Keys ---
#define NVS_KEY_SSID       "wifi_ssid"
#define NVS_KEY_PASS       "wifi_pass"
#define NVS_KEY_SERVER     "server_url"
#define NVS_KEY_DEVICE_KEY "device_key"
#define NVS_KEY_CONFIGURED "configured"

// --- Default Backend Server ---
#define DEFAULT_SERVER_URL "http://192.168.1.100:5000"

// --- WiFi Settings ---
#define WIFI_CONNECT_TIMEOUT_MS  15000
#define WIFI_RETRY_DELAY_MS      5000

// --- Socket.IO Settings ---
#define SIO_RECONNECT_DELAY_MS   3000
#define SIO_PING_INTERVAL_MS     25000
#define SIO_PATH               "/socket.io/?EIO=4&transport=websocket"

// --- Audio Streaming ---
#define AUDIO_CHUNK_SIZE       1024    // bytes per Socket.IO audio_data event
#define AUDIO_SEND_INTERVAL_MS 32      // ~32ms per chunk at 16kHz/16bit = 1024 bytes

// --- Serial Protocol ---
#define SERIAL_BAUD            115200
#define SERIAL_TIMEOUT_MS      100

// --- Recording Button (optional - use boot button on XIAO) ---
// The XIAO ESP32-S3 doesn't have a dedicated user button on headers,
// but the BOOT button (GPIO 0) can be used after boot
#define BUTTON_PIN             0
#define BUTTON_DEBOUNCE_MS     300
#define LONG_PRESS_MS          2000

#endif // CONFIG_H
