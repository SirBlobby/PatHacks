# Pinout & Wiring Guide

## Microcontroller: Seeed Studio XIAO ESP32-S3 Sense

All components connect to the XIAO ESP32-S3 Sense. The onboard PDM microphone is internal to the Sense expansion board and requires no external wiring.

---

## GPIO Pin Assignments

| GPIO | Board Pin | Function          | Component        |
|------|-----------|-------------------|------------------|
| 1    | D0        | OLED D/C          | SPI OLED Display |
| 2    | D1        | Speaker BCLK      | MAX98357 Amp     |
| 3    | D2        | Speaker LRC       | MAX98357 Amp     |
| 4    | D3        | Speaker DIN       | MAX98357 Amp     |
| 5    | D4        | OLED CS           | SPI OLED Display |
| 6    | D5        | OLED RST          | SPI OLED Display |
| 7    | D8 (SCK)  | OLED CLK          | SPI OLED Display |
| 8    | D9 (MISO) | Button A (INPUT)  | Push Button A    |
| 10   | D10 (MOSI)| OLED MOSI         | SPI OLED Display |
| 41   | Internal  | PDM Mic DATA      | Onboard Mic      |
| 42   | Internal  | PDM Mic CLK       | Onboard Mic      |
| 43   | D6 (TX)   | *Available (UART)*| —                |
| 44   | D7 (RX)   | Button B (INPUT)  | Push Button B    |

---

## Component 1: PDM Microphone (Onboard)

The digital PDM microphone is built into the XIAO ESP32-S3 Sense expansion board. No external wiring required.

| Microphone Signal | ESP32-S3 GPIO |
|-------------------|---------------|
| PDM CLK           | GPIO42        |
| PDM DATA          | GPIO41        |

**Protocol:** I2S (PDM Mono Mode)
**Sample Rate:** 16 kHz
**Bit Depth:** 16-bit

---

## Component 2: Speaker Amp (Adafruit MAX98357)

The MAX98357 is an I2S Class-D mono amplifier. It receives digital audio over I2S and drives the speaker directly.

### Wiring

```
MAX98357         XIAO ESP32-S3
--------         ----------------
Vin  ---------- 3V3
GND  ---------- GND
BCLK ---------- GPIO2  (D1)
LRC  ---------- GPIO3  (D2)
DIN  ---------- GPIO4  (D3)
GAIN            (leave floating = 9dB)
SD              (leave floating = stereo avg via onboard 1MΩ)
```

> **3.3V Operation:** The MAX98357 accepts 2.5V–5.5V on Vin. At 3.3V into a 4Ω speaker, max output is ~1.3W (vs 3W at 5V). This is sufficient for most small speaker applications.

### Speaker Connection

Connect a **4Ω or higher** impedance speaker to the `+` and `-` screw terminals on the MAX98357 breakout. The output is bridge-tied — **neither speaker wire should touch ground**.

**Protocol:** I2S (Philips Standard Mode)

---

## Component 3: SPI OLED Display (128x64 SSD1306)

The 128x64 monochrome OLED uses SPI communication. If using the Adafruit STEMMA QT version, **cut solder jumpers J1 and J2** on the back of the board to switch from I2C to SPI mode.

### Wiring

```
OLED Display     XIAO ESP32-S3
-----------      ----------------
Vin  ---------- 3V3
GND  ---------- GND
CLK  ---------- GPIO7  (D8 / SCK)
DATA ---------- GPIO10 (D10 / MOSI)
D/C  ---------- GPIO1  (D0)
CS   ---------- GPIO5  (D4)
RST  ---------- GPIO6  (D5)
```

**Protocol:** SPI (Hardware SPI)
**Library:** U8g2

> **Note:** GPIO7, GPIO8, and GPIO10 are shared with the onboard SD card slot on the Sense expansion board. If you need simultaneous SD card access, use software SPI on different pins for the OLED or manage chip-select lines carefully.

---

## Component 4: Push Buttons (x2)

Two momentary push buttons for user input. Connected between the GPIO pin and GND, using the ESP32-S3's internal pull-up resistors (no external resistors needed). Active-low: pressed = LOW, released = HIGH.

### Wiring

```
Button A         XIAO ESP32-S3
--------         ----------------
Pin 1  --------- GPIO8  (D9)
Pin 2  --------- GND

Button B         XIAO ESP32-S3
--------         ----------------
Pin 1  --------- GPIO44 (D7)
Pin 2  --------- GND
```

> **Debounce:** Software debounce (50ms) is applied in firmware. No external RC filter needed.

---

## Power

The system is powered by a **3.7V LiPo battery** connected to the BAT pads on the bottom of the XIAO ESP32-S3. The onboard BQ25101 charge controller handles charging via USB-C.

| Pin       | Description                                       |
|-----------|---------------------------------------------------|
| BAT+/BAT- | 3.7V LiPo battery pads (bottom of board)          |
| 3V3       | Regulated 3.3V output — powers OLED & speaker amp |
| 5V        | USB 5V (only available when USB-C is connected)   |
| GND       | Common ground for all components                  |

### Battery Notes

- LiPo voltage range: **3.0V** (empty) to **4.2V** (full)
- The onboard regulator provides stable **3.3V** from the battery
- 3V3 pin max current: **700mA** (shared across all peripherals)
- Battery charges automatically when USB-C is plugged in
- **Do not exceed 700mA** combined draw on 3V3 (amp ~350mA peak + OLED ~40mA = ~390mA — well within limit)

---

## Wiring Diagram (Text)

```
                    XIAO ESP32-S3 Sense
                   ┌──────────────────┐
                   │      USB-C       │
                   ├──────────────────┤
  OLED D/C    ←──  │ D0  (GPIO1)      │
  SPK BCLK    ←──  │ D1  (GPIO2)      │
  SPK LRC     ←──  │ D2  (GPIO3)      │
  SPK DIN     ←──  │ D3  (GPIO4)      │
  OLED CS     ←──  │ D4  (GPIO5)      │
  OLED RST    ←──  │ D5  (GPIO6)      │
  (UART TX)        │ D6  (GPIO43)     │
  Button B    ──→  │ D7  (GPIO44)     │
  OLED CLK    ←──  │ D8  (GPIO7/SCK)  │
  Button A    ──→  │ D9  (GPIO8/MISO) │
  OLED MOSI   ←──  │ D10 (GPIO10/MOSI)│
                   ├──────────────────┤
                   │ 3V3   5V   GND   │
                   └──┬─────┬─────┬───┘
                      │           │
                      ├── OLED Vin (3V3)
                      ├── MAX98357 Vin (3V3)
                      └── Common GND

         Battery (bottom pads):
           BAT+ ← 3.7V LiPo +
           BAT- ← 3.7V LiPo -

         Internal (Sense Expansion Board):
           GPIO42 → PDM Mic CLK
           GPIO41 → PDM Mic DATA
```

---

## Notes

- The system runs entirely at **3.3V** when on battery power. The 5V pin is only live when USB-C is connected.
- The XIAO ESP32-S3 operates at **3.3V logic**. Both the MAX98357 and the SPI OLED accept 3.3V logic and power.
- The MAX98357 at 3.3V delivers up to **1.3W into 4Ω** (vs 3W at 5V). Adequate for a small speaker.
- The I2S peripheral on the ESP32-S3 is shared. The microphone and speaker **cannot operate simultaneously** — the test program runs them sequentially.
- Keep I2S wires (BCLK, LRC, DIN) short to minimize noise.
- Keep SPI wires (CLK, MOSI) short for reliable display communication.
- Total 3V3 current budget: ~700mA. Estimated peak draw: speaker amp ~350mA + OLED ~40mA + MCU ~100mA = ~490mA.
