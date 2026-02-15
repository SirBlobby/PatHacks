use serialport::SerialPort;
use std::time::Duration;
use serde::{Serialize, Deserialize};
use std::io::{Read, Write};

// ============================================================
// Types
// ============================================================

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct SerialPortInfo {
    pub port_name: String,
    pub manufacturer: Option<String>,
}

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct DeviceStatus {
    pub wifi_connected: bool,
    pub wifi_ssid: String,
    pub wifi_ip: String,
    pub wifi_rssi: i32,
    pub server_url: String,
    pub device_key: String,
    pub provisioned: bool,
}

// ============================================================
// Port listing
// ============================================================

pub fn list_serial_ports() -> Result<Vec<SerialPortInfo>, String> {
    match serialport::available_ports() {
        Ok(ports) => {
            let mut result = Vec::new();
            for p in ports {
                let manufacturer = match p.port_type {
                    serialport::SerialPortType::UsbPort(info) => info.manufacturer,
                    _ => None,
                };
                result.push(SerialPortInfo {
                    port_name: p.port_name,
                    manufacturer,
                });
            }
            Ok(result)
        }
        Err(e) => Err(e.to_string()),
    }
}

// ============================================================
// Internal helpers
// ============================================================

/// Open a serial port at 115200 baud with the given read timeout.
fn open_port(port_name: &str, timeout_ms: u64) -> Result<Box<dyn SerialPort>, String> {
    serialport::new(port_name, 115200)
        .timeout(Duration::from_millis(timeout_ms))
        .open()
        .map_err(|e| format!("Failed to open port {}: {}", port_name, e))
}

/// Send a JSON command string (newline-terminated) and read lines back until
/// we find one that matches `success_prefix` or `fail_prefix`, or until
/// `timeout_secs` elapses. Returns the full matching line.
fn send_and_wait(
    port: &mut Box<dyn SerialPort>,
    json_cmd: &str,
    success_prefix: &str,
    fail_prefix: &str,
    timeout_secs: u64,
) -> Result<String, String> {
    let data = format!("{}\n", json_cmd);
    port.write_all(data.as_bytes()).map_err(|e| format!("Write failed: {}", e))?;
    port.flush().map_err(|e| format!("Flush failed: {}", e))?;

    let start = std::time::Instant::now();
    let mut buf = vec![0u8; 2048];
    let mut accumulated = String::new();

    loop {
        if start.elapsed().as_secs() > timeout_secs {
            return Err(format!(
                "Timeout after {}s waiting for response (got so far: {})",
                timeout_secs,
                accumulated.trim()
            ));
        }

        match port.read(buf.as_mut_slice()) {
            Ok(n) if n > 0 => {
                accumulated.push_str(&String::from_utf8_lossy(&buf[..n]));

                // Check each complete line
                while let Some(newline_pos) = accumulated.find('\n') {
                    let line = accumulated[..newline_pos].trim().to_string();
                    accumulated = accumulated[newline_pos + 1..].to_string();

                    if !line.is_empty() {
                        if line.starts_with(success_prefix) {
                            return Ok(line);
                        }
                        if line.starts_with(fail_prefix) {
                            // Extract the reason after the prefix (after the colon)
                            let reason = if line.len() > fail_prefix.len() {
                                line[fail_prefix.len()..].trim_start_matches(':').trim().to_string()
                            } else {
                                "Unknown error".to_string()
                            };
                            return Err(reason);
                        }
                        // Skip debug/log lines from ESP32 (e.g. "[SERIAL] ...")
                    }
                }
            }
            Ok(_) => {}
            Err(ref e) if e.kind() == std::io::ErrorKind::TimedOut => {}
            Err(e) => return Err(format!("Read error: {}", e)),
        }

        std::thread::sleep(Duration::from_millis(50));
    }
}

// ============================================================
// Public API — one function per protocol command
// ============================================================

/// Send PING and wait for PONG. Verifies device is connected and responsive.
pub fn ping_device(port_name: &str) -> Result<(), String> {
    let mut port = open_port(port_name, 2000)?;
    let cmd = serde_json::json!({"cmd": "PING"}).to_string();
    send_and_wait(&mut port, &cmd, "PONG", "ERROR", 5)?;
    Ok(())
}

/// Send WiFi credentials. ESP32 will save to NVS and attempt connection.
/// Returns Ok(()) on WIFI_OK, Err with reason on WIFI_FAIL.
pub fn send_wifi(port_name: &str, ssid: &str, password: &str) -> Result<(), String> {
    let mut port = open_port(port_name, 2000)?;
    let cmd = serde_json::json!({
        "cmd": "WIFI",
        "ssid": ssid,
        "password": password
    }).to_string();
    // WiFi connection can take a while — 20s timeout
    send_and_wait(&mut port, &cmd, "WIFI_OK", "WIFI_FAIL", 20)?;
    Ok(())
}

/// Send server URL and device key. ESP32 saves to NVS.
/// Returns Ok(()) on CONFIG_OK, Err with reason on CONFIG_FAIL.
pub fn send_config(port_name: &str, server_url: &str, device_key: &str) -> Result<(), String> {
    let mut port = open_port(port_name, 2000)?;
    let cmd = serde_json::json!({
        "cmd": "CONFIG",
        "server": server_url,
        "device_key": device_key
    }).to_string();
    send_and_wait(&mut port, &cmd, "CONFIG_OK", "CONFIG_FAIL", 5)?;
    Ok(())
}

/// Request device status. Returns parsed DeviceStatus.
pub fn get_status(port_name: &str) -> Result<DeviceStatus, String> {
    let mut port = open_port(port_name, 2000)?;
    let cmd = serde_json::json!({"cmd": "STATUS"}).to_string();
    let response = send_and_wait(&mut port, &cmd, "STATUS:", "ERROR", 5)?;

    // Response format: STATUS:{json}
    let json_str = response
        .strip_prefix("STATUS:")
        .ok_or("Invalid STATUS response format")?;

    serde_json::from_str::<DeviceStatus>(json_str)
        .map_err(|e| format!("Failed to parse status JSON: {} (raw: {})", e, json_str))
}

/// Factory reset — clears all NVS config on the device.
pub fn factory_reset(port_name: &str) -> Result<(), String> {
    let mut port = open_port(port_name, 2000)?;
    let cmd = serde_json::json!({"cmd": "RESET"}).to_string();
    send_and_wait(&mut port, &cmd, "RESET_OK", "ERROR", 5)?;
    Ok(())
}
