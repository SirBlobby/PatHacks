use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct WifiNetwork {
    pub ssid: String,
    pub bssid: String, // MAC address
    pub rssi: i32,     // Signal strength
}

pub fn scan_wifi() -> Result<Vec<WifiNetwork>, String> {
    // Implement real WiFi scanning
    match wifiscanner::scan() {
        Ok(networks) => {
            let mut result = Vec::new();
            for net in networks {
                result.push(WifiNetwork {
                    ssid: net.ssid,
                    bssid: net.mac,
                    rssi: net.signal_level.parse().unwrap_or(0),
                });
            }
            // Sort by signal strength
            result.sort_by(|a, b| b.rssi.cmp(&a.rssi));
            Ok(result)
        }
        Err(e) => {
            // Fallback for development/testing if scan fails (e.g. no wifi adapter)
            // or return error. Since user wants "execute it", returning error might be better
            // but let's return a specific error message.
            // However, maybe we should keep the demo data if it fails?
            // "Double check any work" -> Real implementation is better.
            Err(format!("Failed to scan wifi: {:?}", e))
        }
    }
}
