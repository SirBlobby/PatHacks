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
#define DEFAULT_SERVER_URL "https://buddy.sirblob.co"

// --- WiFi Settings ---
#define WIFI_CONNECT_TIMEOUT_MS  15000
#define WIFI_RETRY_DELAY_MS      5000

// --- Socket.IO Settings ---
#define SIO_RECONNECT_DELAY_MS   3000
#define SIO_PING_INTERVAL_MS     25000
#define SIO_PATH               "/socket.io/?EIO=4&transport=websocket"

// --- Audio Streaming (Recording) ---
#define AUDIO_CHUNK_SIZE       1024    // bytes per Socket.IO audio_data event
#define AUDIO_SEND_INTERVAL_MS 32      // ~32ms per chunk at 16kHz/16bit = 1024 bytes

// --- Speaker Playback (Voice Call) ---
#define SPK_BUFFER_SIZE        8192    // Ring buffer for incoming TTS audio
#define SPK_I2S_DMA_BUF_COUNT  8
#define SPK_I2S_DMA_BUF_LEN    256

// --- Serial Protocol ---
#define SERIAL_BAUD            115200
#define SERIAL_TIMEOUT_MS      100

// --- Button A: Recording Toggle (GPIO 8) ---
#define BUTTON_A_DEBOUNCE_MS   300
#define BUTTON_A_LONG_PRESS_MS 2000    // Long press = factory reset

// --- Button B: Voice Call Push-to-Talk (GPIO 44) ---
#define BUTTON_B_DEBOUNCE_MS   200
#define BUTTON_B_LONG_PRESS_MS 2000    // Long press = end call

#endif // CONFIG_H
