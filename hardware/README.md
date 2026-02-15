# Hardware - ESP32 Firmware

Firmware for the Learning Buddy ESP32 device. Streams live lecture audio to the backend server over WiFi using Socket.IO, supports push-to-talk AI voice calls, and is provisioned over USB serial from the Tauri desktop app.

## Hardware

- **MCU**: Seeed Studio XIAO ESP32-S3 Sense
- **Microphone**: Onboard PDM mic (Sense expansion board) - GPIO 42 (CLK), GPIO 41 (DATA)
- **Speaker**: Adafruit MAX98357 I2S amp - GPIO 2 (BCLK), 3 (LRC), 4 (DIN)
- **Display**: SSD1306 128x64 monochrome OLED over SPI - GPIO 7 (CLK), 10 (MOSI), 1 (DC), 5 (CS), 6 (RST)
- **Button A**: GPIO 8 (D9/MISO) - short press toggles recording, long press (>2s) factory resets
- **Button B**: GPIO 44 (D7/RX) - push-to-talk for AI voice calls (press=listen, release=get response, long press=end call)

### Pin Map

| GPIO | Label | Function | Component |
|------|-------|----------|-----------|
| 2 | D1 | SPK_BCLK (I2S Bit Clock) | MAX98357 Speaker Amp |
| 3 | D2 | SPK_LRC (I2S Word Select) | MAX98357 Speaker Amp |
| 4 | D3 | SPK_DIN (I2S Data Out) | MAX98357 Speaker Amp |
| 1 | D0 | OLED_DC (Data/Command) | SPI OLED Display |
| 5 | D4 | OLED_CS (Chip Select) | SPI OLED Display |
| 6 | D5 | OLED_RST (Reset) | SPI OLED Display |
| 7 | D8 (SCK) | OLED_CLK (SPI Clock) | SPI OLED Display |
| 8 | D9 (MISO) | Button A (INPUT_PULLUP) | Recording toggle |
| 10 | D10 (MOSI) | OLED_MOSI (SPI Data) | SPI OLED Display |
| 41 | Internal | PDM Mic DATA | Onboard Mic |
| 42 | Internal | PDM Mic CLK | Onboard Mic |
| 44 | D7 (RX) | Button B (INPUT_PULLUP) | Voice call push-to-talk |

I2S is shared between mic (PDM RX) and speaker (standard TX) and cannot run simultaneously. The firmware switches between them for half-duplex voice calls.

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

Recording (Button A):
  IDLE -> RECORDING (A short press) -> IDLE (A short press again)

Voice Call (Button B):
  IDLE -> VOICE_LISTENING (B press) -> VOICE_THINKING (B release)
       -> VOICE_PLAYING (response received) -> VOICE_READY (playback done)
       -> VOICE_LISTENING (B press again) or IDLE (B long press)

Factory Reset:
  Any state -> Button A long press (>2s) -> BOOT (ESP.restart())
```

On boot, the device checks NVS for stored WiFi credentials, server URL, and device key. If all are present, it connects automatically. Otherwise it enters Setup Mode and waits for provisioning over USB serial.

### Button Controls

**Button A (GPIO 8)** - Recording:
- Short press in IDLE: start recording (mic streams `audio_data` to backend)
- Short press while RECORDING: stop recording
- Long press (>2s): factory reset all NVS data and restart

**Button B (GPIO 44)** - Voice Call:
- Press in IDLE: start voice call (`call_start`), begin listening
- Press in VOICE_READY: resume listening (switch I2S back to mic)
- Release while VOICE_LISTENING: stop listening (`call_stop_listening`), wait for AI response
- Long press (>2s): end voice call (`call_end`), return to IDLE

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

**Authentication:**
1. Emit `"auth"` with `{"key":"ABC123"}` -> receive `"auth_ok"` or `"auth_error"`

**Recording (Button A):**
1. Emit `"rec_start"` with `{"title":"Lecture Recording"}` -> receive `"rec_started"` with `{"recording_id":"..."}`
2. Emit `"audio_data"` as binary (raw 16-bit PCM chunks) - fire-and-forget
3. Emit `"rec_stop"` -> receive `"rec_stopped"`

**Voice Call (Button B):**
1. Emit `"call_start"` -> receive `"call_started"`
2. Emit `"call_audio"` as binary PCM (while Button B held)
3. Emit `"call_stop_listening"` (on Button B release) -> backend processes STT -> LLM -> TTS
4. Receive `"call_response"` as binary event (TTS audio PCM) -> play through speaker
5. Emit `"call_end"` (Button B long press) -> receive `"call_ended"`

Audio format: 16kHz, 16-bit, mono PCM. Recording chunks are ~1024 bytes sent every ~32ms. Voice call TTS responses arrive as a single binary frame.

### I2S Bus Sharing (Half-Duplex Voice)

The mic (PDM RX) and speaker (standard I2S TX) both use `I2S_NUM_0`. They cannot run simultaneously:

- **Recording**: mic I2S installed, speaker not used
- **Voice listening**: mic I2S installed, capturing `call_audio`
- **Voice playing**: mic I2S uninstalled (`audio_stream_teardown()`), speaker I2S installed (`spk_stream_start()`)
- **Resume listening**: speaker I2S uninstalled (`spk_stream_stop()`), mic I2S reinstalled

### Display

The OLED shows contextual status screens: boot splash, setup mode (waiting for provisioning), WiFi connecting, idle (connected, ready with button hints), recording (with elapsed timer), voice call states (listening with mic animation, processing with spinner, speaking with speaker animation), and error states.

## Dependencies

Managed by PlatformIO (`lib_deps` in `platformio.ini`):

- `olikraus/U8g2` - OLED display driver
- `pschatzmann/arduino-audio-tools` - Audio I2S helpers (reference code)
- `bblanchon/ArduinoJson` - JSON parsing for serial protocol + Socket.IO events
- `links2004/WebSockets` - WebSocket client for Socket.IO
