# Hardware - ESP32 Firmware

Firmware for the Learning Buddy ESP32 device. Streams live lecture audio to the backend server over WiFi using Socket.IO, and is provisioned over USB serial from the Tauri desktop app.

## Hardware

- **MCU**: Seeed Studio XIAO ESP32-S3 Sense
- **Microphone**: Onboard PDM mic (Sense expansion board) - GPIO 42 (CLK), GPIO 41 (DATA)
- **Speaker**: Adafruit MAX98357 I2S amp - GPIO 2 (BCLK), 3 (LRC), 4 (DIN)
- **Display**: SSD1306 128x64 monochrome OLED over SPI - GPIO 7 (CLK), 10 (MOSI), 1 (DC), 5 (CS), 6 (RST)
- **Button**: BOOT button (GPIO 0) - short press toggles recording, long press (>2s) factory resets

I2S is shared between mic and speaker and cannot run simultaneously.

## Build

Requires [PlatformIO](https://platformio.org/). The project targets the `seeed_xiao_esp32s3` board with Arduino framework.

```bash
# Build
pio run

# Upload
pio run --target upload

# Serial monitor
pio device monitor -b 115200
```

## Architecture

### State Machine

```
BOOT -> CHECK_CONFIG -> SETUP_MODE (if not provisioned)
                     -> WIFI_CONNECTING (if provisioned)
WIFI_CONNECTING -> WIFI_CONNECTED -> SIO_CONNECTING -> AUTHENTICATED -> IDLE
IDLE -> RECORDING (button press) -> IDLE (button press again)
```

On boot, the device checks NVS for stored WiFi credentials, server URL, and device key. If all are present, it connects automatically. Otherwise it enters Setup Mode and waits for provisioning over USB serial.

### Serial Provisioning Protocol

The Tauri desktop app communicates over USB serial at 115200 baud using JSON commands:

| Command | Payload | Success | Failure |
|---|---|---|---|
| `PING` | `{"cmd":"PING"}` | `PONG` | - |
| `WIFI` | `{"cmd":"WIFI","ssid":"...","password":"..."}` | `WIFI_OK` | `WIFI_FAIL:<reason>` |
| `CONFIG` | `{"cmd":"CONFIG","server":"...","device_key":"..."}` | `CONFIG_OK` | `CONFIG_FAIL:<reason>` |
| `STATUS` | `{"cmd":"STATUS"}` | `STATUS:{json}` | - |
| `RESET` | `{"cmd":"RESET"}` | `RESET_OK` | - |

All values are persisted in ESP32 NVS (non-volatile storage) and survive reboots.

### Socket.IO Backend Protocol

Once connected to WiFi, the device connects to the backend via WebSocket using Socket.IO v4 (EIO=4):

1. **Auth**: emit `"auth"` with `{"key":"ABC123"}` -> receive `"auth_ok"` or `"auth_error"`
2. **Start recording**: emit `"rec_start"` with `{"title":"Lecture Recording"}` -> receive `"rec_started"` with `{"recording_id":"..."}`
3. **Stream audio**: emit `"audio_data"` as binary (raw 16-bit PCM chunks) - fire-and-forget
4. **Stop recording**: emit `"rec_stop"` -> receive `"rec_stopped"`

Audio format: 16kHz, 16-bit, mono PCM. Chunks are ~1024 bytes sent every ~32ms.

The Socket.IO client is a custom implementation using the `links2004/WebSockets` library, handling the EIO=4 handshake, binary frame protocol (two-frame encoding for binary events), and ping/pong keepalive.

### Display

The OLED shows contextual status screens: boot splash, setup mode (waiting for provisioning), WiFi connecting, idle (connected, ready), recording (with elapsed timer), and error states.

## Project Structure

```
hardware/
  platformio.ini          # PlatformIO config, libs, build flags
  include/
    pins.h                # GPIO pin definitions
    config.h              # NVS keys, timeouts, audio params, button config
    wifi_manager.h        # WiFi connection + NVS credential storage
    serial_protocol.h     # USB serial JSON command protocol
    socketio_client.h     # Socket.IO v4 client (WebSocket-based)
    audio_stream.h        # PDM mic -> binary Socket.IO streaming
    display.h             # OLED display status screens
    speaker.h             # Speaker output (reference/testing)
    microphone.h          # Microphone input (reference/testing)
  src/
    main.cpp              # State machine, button handling, main loop
    wifi_manager.cpp       # WiFi + NVS Preferences implementation
    serial_protocol.cpp   # JSON command parser
    socketio_client.cpp   # EIO=4 WebSocket Socket.IO client
    audio_stream.cpp      # ESP-IDF I2S PDM RX -> Socket.IO binary
    display.cpp           # OLED rendering for all status screens
    speaker_out.cpp       # Speaker test (not used in production)
    mic_in.cpp            # Mic test (not used in production)
```

## Dependencies

Managed by PlatformIO (`lib_deps` in `platformio.ini`):

- `olikraus/U8g2` - OLED display driver
- `pschatzmann/arduino-audio-tools` - Audio I2S helpers (reference code)
- `bblanchon/ArduinoJson` - JSON parsing for serial protocol
- `links2004/WebSockets` - WebSocket client for Socket.IO
