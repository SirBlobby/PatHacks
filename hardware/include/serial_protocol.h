#ifndef SERIAL_PROTOCOL_H
#define SERIAL_PROTOCOL_H

#include <Arduino.h>

// Initialize serial protocol handler
void serial_proto_init();

// Process incoming serial data (call in loop)
// Returns true if a command was processed
bool serial_proto_loop();

// Check if device is fully provisioned (has WiFi + server + device key)
bool serial_proto_is_provisioned();

// Get stored server URL
String serial_proto_get_server();

// Get stored device key
String serial_proto_get_device_key();

// Send a status message back over serial
void serial_proto_send(const char* message);

// Clear all stored configuration (factory reset)
void serial_proto_factory_reset();

#endif // SERIAL_PROTOCOL_H
