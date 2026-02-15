# Software - Tauri Desktop App

Desktop application for provisioning the Learning Buddy ESP32 device. Connects to the device over USB serial and walks the user through a multi-step setup wizard: device detection, WiFi configuration, and server/device key registration.

Built with [Tauri 2](https://tauri.app/) (Rust backend) and [SvelteKit](https://svelte.dev/) (Svelte 5 frontend).

## Setup

### Prerequisites

- [Rust](https://rustup.rs/) (1.70+)
- [Bun](https://bun.sh/) (or Node.js)
- Linux system dependencies for Tauri:
  ```bash
  sudo apt install pkg-config libglib2.0-dev libgtk-3-dev libwebkit2gtk-4.1-dev \
    libjavascriptcoregtk-4.1-dev libsoup-3.0-dev libayatana-appindicator3-dev libudev-dev
  ```

### Install & Run

```bash
bun install
bun run tauri dev
```

### Build

```bash
bun run tauri build
```

## Provisioning Flow

The app guides users through 4 steps:

1. **Device Detection** - Scans USB serial ports and WiFi networks, then pings the ESP32 to verify communication
2. **WiFi Configuration** - User selects a WiFi network and enters the password. Credentials are sent to the ESP32 which connects and confirms
3. **Server Configuration** - User enters the backend server URL (defaults to `https://learningbuddy.tech`) and the 6-character device key from their web dashboard
4. **Done** - Device is fully provisioned and ready. WiFi, server URL, and device key are stored in ESP32 NVS and persist across reboots

### Serial Protocol

Communicates with the ESP32 at 115200 baud using newline-terminated JSON commands:

| Command | Tauri Sends | ESP32 Replies |
|---|---|---|
| Ping | `{"cmd":"PING"}` | `PONG` |
| WiFi | `{"cmd":"WIFI","ssid":"...","password":"..."}` | `WIFI_OK` / `WIFI_FAIL:<reason>` |
| Config | `{"cmd":"CONFIG","server":"...","device_key":"..."}` | `CONFIG_OK` / `CONFIG_FAIL:<reason>` |
| Status | `{"cmd":"STATUS"}` | `STATUS:{json}` |
| Reset | `{"cmd":"RESET"}` | `RESET_OK` |

## Project Structure

```
software/
  front_end/                    # SvelteKit frontend (custom directory, not src/)
    routes/
      +page.svelte              # Multi-step provisioning wizard UI
      +layout.ts                # SPA layout config
    app.html                    # HTML template
  src-tauri/                    # Tauri main app crate
    src/
      lib.rs                    # Tauri commands (ping, wifi, config, status, reset)
      main.rs                   # Entry point
    Cargo.toml
    capabilities/default.json   # Tauri permissions
  networking/                   # Serial communication crate
    src/
      lib.rs                    # Serial port listing + protocol functions
    Cargo.toml
  back_end/                     # System utilities crate
    src/
      lib.rs                    # WiFi network scanning (host machine)
    Cargo.toml
  svelte.config.js              # SvelteKit config (adapter-static, custom dirs)
  package.json
```

### Rust Crates

| Crate | Purpose | Key Dependencies |
|---|---|---|
| `src-tauri` (software) | Main Tauri app, exposes commands to frontend | `tauri`, `networking`, `back_end` |
| `networking` | USB serial communication with ESP32 | `serialport`, `serde_json` |
| `back_end` | Host WiFi scanning for network selection | `wifiscanner` |

### Tauri Commands

| Command | Description |
|---|---|
| `list_ports` | List available USB serial ports |
| `get_wifi_list` | Scan WiFi networks on the host machine |
| `ping_device` | Verify ESP32 is connected and responsive |
| `send_wifi_config` | Send WiFi SSID + password to ESP32 |
| `send_server_config` | Send server URL + device key to ESP32 |
| `get_device_status` | Query device status (WiFi, server, provisioning state) |
| `factory_reset_device` | Clear all stored config from ESP32 NVS |

## Tech Stack

- **Rust** - Tauri 2, serialport, wifiscanner
- **Svelte 5** - Runes (`$state`, `$derived`), SvelteKit with adapter-static
- **TypeScript**
