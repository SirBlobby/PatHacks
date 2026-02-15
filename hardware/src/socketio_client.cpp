// ============================================================
// Socket.IO Client for ESP32
// Implements Socket.IO v4 (EIO=4) over WebSocket
// Using links2004/WebSockets library
//
// Socket.IO EIO=4 packet types:
//   0  = CONNECT    (sent/received on namespace connect)
//   2  = EVENT      (JSON event: 2["name",{data}])
//   3  = ACK
//   4  = ERROR
//   40 = CONNECT to namespace "/"
//   42 = EVENT
//   45 = BINARY_EVENT (has attachments)
//   46 = BINARY_ACK
//
// Engine.IO EIO=4 packet types (over WebSocket text frames):
//   0  = open       (server sends session info)
//   2  = ping
//   3  = pong  
//   4  = message    (contains Socket.IO packet)
//   40 = SIO CONNECT (namespace /)
//   42 = SIO EVENT
// ============================================================

#include "socketio_client.h"
#include "config.h"
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

static WebSocketsClient webSocket;

// Callbacks
static SioEventCallback   _event_cb = nullptr;
static SioConnectCallback _connect_cb = nullptr;
static SioDisconnectCallback _disconnect_cb = nullptr;

// State
static bool sio_connected = false;
static bool ws_connected = false;
static unsigned long last_ping = 0;
static unsigned long ping_interval = SIO_PING_INTERVAL_MS;
static String sio_sid = "";

// Forward declarations
static void webSocketEvent(WStype_t type, uint8_t* payload, size_t length);
static void handleSioPacket(const char* payload, size_t length);

void sio_init() {
    sio_connected = false;
    ws_connected = false;
    Serial.println("[SIO] Socket.IO client initialized.");
}

bool sio_connect(const char* host, uint16_t port) {
    Serial.printf("[SIO] Connecting to ws://%s:%d%s\n", host, port, SIO_PATH);

    webSocket.begin(host, port, SIO_PATH);
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(SIO_RECONNECT_DELAY_MS);
    webSocket.enableHeartbeat(ping_interval, 5000, 2);

    return true; // async - actual connection happens in loop
}

void sio_disconnect() {
    webSocket.disconnect();
    sio_connected = false;
    ws_connected = false;
    Serial.println("[SIO] Disconnected.");
}

void sio_loop() {
    webSocket.loop();

    // Send Socket.IO ping (EIO ping = "2") periodically
    if (ws_connected && millis() - last_ping > ping_interval) {
        webSocket.sendTXT("2");
        last_ping = millis();
    }
}

bool sio_is_connected() {
    return sio_connected;
}

void sio_emit(const char* event, const char* json_payload) {
    if (!sio_connected) {
        Serial.println("[SIO] Cannot emit - not connected.");
        return;
    }

    // Format: 42["event_name",{payload}]
    String packet = "42[\"";
    packet += event;
    packet += "\",";
    packet += json_payload;
    packet += "]";

    webSocket.sendTXT(packet);
}

void sio_emit_binary(const char* event, const uint8_t* data, size_t len) {
    if (!sio_connected) return;

    // Socket.IO binary event protocol:
    // 1. Send text frame: 451-["event_name",{"_placeholder":true,"num":0}]
    //    The "1" after "45" is the number of binary attachments
    // 2. Send binary frame with the actual data

    String header = "451-[\"";
    header += event;
    header += "\",{\"_placeholder\":true,\"num\":0}]";
    webSocket.sendTXT(header);

    // Send binary payload
    webSocket.sendBIN(data, len);
}

void sio_on_event(SioEventCallback cb) {
    _event_cb = cb;
}

void sio_on_connect(SioConnectCallback cb) {
    _connect_cb = cb;
}

void sio_on_disconnect(SioDisconnectCallback cb) {
    _disconnect_cb = cb;
}

// ---- Internal handlers ----

static void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED:
            Serial.printf("[SIO] WebSocket connected to: %s\n", payload);
            ws_connected = true;
            last_ping = millis();
            break;

        case WStype_DISCONNECTED:
            Serial.println("[SIO] WebSocket disconnected.");
            ws_connected = false;
            sio_connected = false;
            if (_disconnect_cb) _disconnect_cb();
            break;

        case WStype_TEXT: {
            handleSioPacket((const char*)payload, length);
            break;
        }

        case WStype_BIN:
            // Binary frames from server (not expected in our protocol)
            Serial.printf("[SIO] Received binary: %d bytes\n", length);
            break;

        case WStype_ERROR:
            Serial.printf("[SIO] WebSocket error: %s\n", payload ? (char*)payload : "unknown");
            break;

        case WStype_PING:
            // Library handles pong automatically
            break;

        case WStype_PONG:
            break;

        default:
            break;
    }
}

static void handleSioPacket(const char* payload, size_t length) {
    if (length == 0) return;

    // Engine.IO packet type is the first character
    char eio_type = payload[0];

    switch (eio_type) {
        case '0': {
            // EIO OPEN - contains session info JSON
            // e.g. 0{"sid":"abc123","upgrades":[],"pingInterval":25000,"pingTimeout":20000}
            Serial.println("[SIO] Engine.IO OPEN received.");

            // Parse ping interval
            JsonDocument doc;
            if (deserializeJson(doc, payload + 1) == DeserializationError::Ok) {
                sio_sid = doc["sid"].as<String>();
                if (doc.containsKey("pingInterval")) {
                    ping_interval = doc["pingInterval"].as<unsigned long>();
                }
                Serial.printf("[SIO] Session: %s, pingInterval: %lu\n", sio_sid.c_str(), ping_interval);
            }

            // Send Socket.IO CONNECT to namespace "/"
            // Format: 40 (SIO type 4 = MESSAGE, SIO packet type 0 = CONNECT)
            webSocket.sendTXT("40");
            break;
        }

        case '2':
            // EIO PING -> respond with EIO PONG
            webSocket.sendTXT("3");
            break;

        case '3':
            // EIO PONG (response to our ping)
            break;

        case '4': {
            // EIO MESSAGE -> contains Socket.IO packet
            if (length < 2) break;

            char sio_type = payload[1];

            switch (sio_type) {
                case '0': {
                    // SIO CONNECT ACK - e.g. 40{"sid":"..."}
                    sio_connected = true;
                    Serial.println("[SIO] Socket.IO connected to namespace /");
                    if (_connect_cb) _connect_cb();
                    break;
                }

                case '2': {
                    // SIO EVENT - e.g. 42["event_name",{data}]
                    // Parse: skip "42" prefix, parse JSON array
                    const char* json_start = payload + 2;

                    JsonDocument doc;
                    DeserializationError err = deserializeJson(doc, json_start);
                    if (err) {
                        Serial.printf("[SIO] Failed to parse event: %s\n", err.c_str());
                        break;
                    }

                    if (doc.is<JsonArray>() && doc.size() >= 2) {
                        const char* event_name = doc[0].as<const char*>();
                        String event_data;
                        serializeJson(doc[1], event_data);

                        Serial.printf("[SIO] Event: %s\n", event_name);

                        if (_event_cb) {
                            _event_cb(event_name, event_data.c_str());
                        }
                    }
                    break;
                }

                case '4': {
                    // SIO ERROR
                    Serial.printf("[SIO] Error: %s\n", payload + 2);
                    break;
                }

                default:
                    Serial.printf("[SIO] Unknown SIO type: %c\n", sio_type);
                    break;
            }
            break;
        }

        default:
            Serial.printf("[SIO] Unknown EIO type: %c (payload: %s)\n", eio_type, payload);
            break;
    }
}
