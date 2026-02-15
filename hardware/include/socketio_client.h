#ifndef SOCKETIO_CLIENT_H
#define SOCKETIO_CLIENT_H

#include <Arduino.h>

// Callback types
typedef void (*SioEventCallback)(const char* event, const char* payload);
typedef void (*SioBinaryCallback)(const char* event, const uint8_t* data, size_t len);
typedef void (*SioConnectCallback)();
typedef void (*SioDisconnectCallback)();

// Initialize Socket.IO client
void sio_init();

// Connect to the backend server
// server_url: e.g. "buddy.sirblob.co", port: e.g. 443
bool sio_connect(const char* host, uint16_t port);

// Disconnect
void sio_disconnect();

// Must be called in loop() to process WebSocket frames
void sio_loop();

// Check if connected to Socket.IO server
bool sio_is_connected();

// Emit a JSON event: 42["event_name",{json_payload}]
void sio_emit(const char* event, const char* json_payload);

// Emit binary data with event name (Socket.IO binary frame)
// Uses msgpack-style binary attachment: 451-["event_name",{"_placeholder":true,"num":0}] + binary frame
void sio_emit_binary(const char* event, const uint8_t* data, size_t len);

// Set callbacks
void sio_on_event(SioEventCallback cb);
void sio_on_binary(SioBinaryCallback cb);
void sio_on_connect(SioConnectCallback cb);
void sio_on_disconnect(SioDisconnectCallback cb);

#endif // SOCKETIO_CLIENT_H
