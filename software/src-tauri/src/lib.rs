// Learn more about Tauri commands at https://tauri.app/develop/calling-rust/
use networking::{self, SerialPortInfo};
use back_end::{self, WifiNetwork};

#[tauri::command]
fn list_ports() -> Result<Vec<SerialPortInfo>, String> {
    networking::list_serial_ports()
}

#[tauri::command]
fn get_wifi_list() -> Result<Vec<WifiNetwork>, String> {
    back_end::scan_wifi()
}

#[tauri::command]
async fn connect_device(port: String, ssid: String, password: String) -> Result<(), String> {
    // Run blocking serial/network code in a separate thread
    let res = tauri::async_runtime::spawn_blocking(move || {
        networking::send_wifi_credentials(&port, &ssid, &password)
    }).await.map_err(|e| e.to_string())?;
    
    res
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .invoke_handler(tauri::generate_handler![list_ports, get_wifi_list, connect_device])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
