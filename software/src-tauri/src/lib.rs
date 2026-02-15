// Learn more about Tauri commands at https://tauri.app/develop/calling-rust/
use networking::{self, DeviceStatus, SerialPortInfo};
use back_end::{self, WifiNetwork};

#[tauri::command]
fn list_ports() -> Result<Vec<SerialPortInfo>, String> {
    networking::list_serial_ports()
}

#[tauri::command]
fn get_wifi_list() -> Result<Vec<WifiNetwork>, String> {
    back_end::scan_wifi()
}

/// Ping the ESP32 to verify serial communication is working.
#[tauri::command]
async fn ping_device(port: String) -> Result<(), String> {
    tauri::async_runtime::spawn_blocking(move || {
        networking::ping_device(&port)
    }).await.map_err(|e| e.to_string())?
}

/// Send WiFi credentials to the ESP32. It will save them to NVS and connect.
#[tauri::command]
async fn send_wifi_config(port: String, ssid: String, password: String) -> Result<(), String> {
    tauri::async_runtime::spawn_blocking(move || {
        networking::send_wifi(&port, &ssid, &password)
    }).await.map_err(|e| e.to_string())?
}

/// Send server URL and device key to the ESP32. Saved to NVS.
#[tauri::command]
async fn send_server_config(port: String, server_url: String, device_key: String) -> Result<(), String> {
    tauri::async_runtime::spawn_blocking(move || {
        networking::send_config(&port, &server_url, &device_key)
    }).await.map_err(|e| e.to_string())?
}

/// Get the current status of the ESP32 device.
#[tauri::command]
async fn get_device_status(port: String) -> Result<DeviceStatus, String> {
    tauri::async_runtime::spawn_blocking(move || {
        networking::get_status(&port)
    }).await.map_err(|e| e.to_string())?
}

/// Factory reset the ESP32 — clears all stored config from NVS.
#[tauri::command]
async fn factory_reset_device(port: String) -> Result<(), String> {
    tauri::async_runtime::spawn_blocking(move || {
        networking::factory_reset(&port)
    }).await.map_err(|e| e.to_string())?
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .invoke_handler(tauri::generate_handler![
            list_ports,
            get_wifi_list,
            ping_device,
            send_wifi_config,
            send_server_config,
            get_device_status,
            factory_reset_device
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
