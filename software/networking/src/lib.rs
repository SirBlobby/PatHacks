use serialport::SerialPort;
use std::time::Duration;
use serde::{Serialize, Deserialize};
use std::io::{Read, Write};

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct SerialPortInfo {
    pub port_name: String,
    pub manufacturer: Option<String>,
}

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

pub fn send_wifi_credentials(port_name: &str, ssid: &str, password: &str) -> Result<(), String> {
    let mut port = serialport::new(port_name, 115200)
        .timeout(Duration::from_millis(10000))
        .open()
        .map_err(|e| e.to_string())?;

    let payload = serde_json::json!({
        "ssid": ssid,
        "password": password
    });
    
    let data = payload.to_string() + "\n";
    
    port.write_all(data.as_bytes()).map_err(|e| e.to_string())?;
    
    let mut serial_buf: Vec<u8> = vec![0; 1000];
    let start = std::time::Instant::now();
    loop {
        if start.elapsed().as_secs() > 15 {
            return Err("Timeout waiting for response".to_string());
        }
        match port.read(serial_buf.as_mut_slice()) {
            Ok(t) => {
                if t > 0 {
                    let response = String::from_utf8_lossy(&serial_buf[..t]);
                    if response.contains("CONNECTED") || response.contains("SUCCESS") {
                        return Ok(());
                    } else if response.contains("FAIL") {
                        return Err("Failed to connect".to_string());
                    }
                }
            }
            Err(ref e) if e.kind() == std::io::ErrorKind::TimedOut => (),
            Err(e) => return Err(e.to_string()),
        }
        std::thread::sleep(Duration::from_millis(100));
    }
}
