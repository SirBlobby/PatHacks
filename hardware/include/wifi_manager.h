#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

// Initialize WiFi manager (loads saved credentials from NVS)
void wifi_init();

// Check if WiFi credentials are stored in NVS
bool wifi_has_credentials();

// Save WiFi credentials to NVS
bool wifi_save_credentials(const char* ssid, const char* password);

// Connect to WiFi using stored credentials
// Returns true if connected within timeout
bool wifi_connect();

// Disconnect from WiFi
void wifi_disconnect();

// Check if currently connected
bool wifi_is_connected();

// Get current SSID
const char* wifi_get_ssid();

// Get local IP as string
String wifi_get_ip();

// Get RSSI (signal strength)
int wifi_get_rssi();

#endif // WIFI_MANAGER_H
