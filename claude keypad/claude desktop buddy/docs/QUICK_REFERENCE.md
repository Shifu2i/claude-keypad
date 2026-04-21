# Quick Reference Card

## Wiring (ESP32 GPIO)

```
OLED Display (I2C):
  VCC → 3.3V    GND → GND
  SCL → GPIO 22 SDA → GPIO 21

Keys (Input, Internal Pull-Up):
  Key 1 → GPIO 4  (GND other leg)
  Key 2 → GPIO 5  (GND other leg)
  Key 3 → GPIO 6  (GND other leg)

Encoder 1:
  CLK (A) → GPIO 7
  DT  (B) → GPIO 8
  SW (Btn) → GPIO 9  (GND other leg)

Encoder 2:
  CLK (A) → GPIO 10
  DT  (B) → GPIO 11
  SW (Btn) → GPIO 12 (GND other leg)

Speaker:
  (+) → GPIO 13 (via 100Ω resistor optional)
  (-) → GND
```

## BLE UUIDs

```
Nordic UART Service: 6e400001-b5a3-f393-e0a9-e50e24dcca9e
TX (Notify):         6e400003-b5a3-f393-e0a9-e50e24dcca9e
RX (Write):          6e400002-b5a3-f393-e0a9-e50e24dcca9e
```

## Key Mappings (Default Layer)

```
┌─────────────────────────────────────┐
│           DEFAULT LAYER            │
├─────────────────────────────────────┤
│                                     │
│    [Enc1]    Rotate: Scroll         │
│    [═══]     Push: Coding Layer     │
│                                     │
│                                     │
│   ┌───┐    ┌───┐    ┌───┐          │
│   │K1 │    │K2 │    │K3 │          │
│   │   │    │   │    │   │          │
│   │ 💬│    │👁️ │    │🆕 │          │
│   └───┘    └───┘    └───┘          │
│   Chat    Focus    New              │
│                                     │
│    [Enc2]    Rotate: Font Size      │
│    [═══]     Push: Media Layer      │
│                                     │
└─────────────────────────────────────┘
```

## Approval Mode (Auto-Activated)

```
┌─────────────────────────────────────┐
│  ⚠️ APPROVAL NEEDED ⚠️              │
├─────────────────────────────────────┤
│                                     │
│   Claude wants to...                │
│   [prompt preview]                  │
│                                     │
│   ┌───┐    ┌───┐    ┌───┐          │
│   │K1 │    │K2 │    │K3 │          │
│   │ ✅│    │ ❌│    │📄 │          │
│   └───┘    └───┘    └───┘          │
│  Approve  Deny   Details           │
│                                     │
│   Enc1: Scroll prompt               │
│   Enc2: Multiple approvals          │
│                                     │
└─────────────────────────────────────┘
```

## Device → Desktop Messages

```json
// Key press
{"type":"input","input_type":"key","id":1,"value":1}

// Encoder rotation
{"type":"encoder_turn","encoder_id":1,"delta":2}

// Encoder button
{"type":"input","input_type":"encoder_btn","id":1,"value":1}
```

## Desktop → Device Commands

```json
// Set state
{"command":"set_state","state":"attention"}

// Play sound
{"command":"play_sound","frequency":1000,"duration":200}

// Play pattern (melody)
{"command":"play_pattern","pattern":"880:100,0:50,1760:200"}

// Set layer
{"command":"set_layer","layer":"coding"}
```

## States

| State | Trigger | Audio | Display |
|-------|---------|-------|---------|
| `sleep` | Disconnected | - | Dark |
| `idle` | Connected, waiting | Chime | Idle |
| `busy` | Processing | Ticking | Busy |
| `attention` | Needs approval | Alert | NEEDS YOU |
| `celebrate` | Complete | Victory | Done! |
| `error` | Error | Buzz | Error |

## Arduino Libraries Required

```
ArduinoJson   by Benoit Blanchon
ESP32 BLE     by Neil Kolban
Encoder       by Paul Stoffregen
Bounce2       by Thomas Fredericks
U8g2          by olikraus
```

## Python Bridge Dependencies

```
bleak        >= 0.20.0
pyautogui    >= 0.9.54
pynput       >= 1.7.6
psutil       >= 5.9.0
```

## Pin Reference Table

| Component | GPIO | Pin Function |
|-----------|------|--------------|
| OLED SDA | 21 | I2C Data |
| OLED SCL | 22 | I2C Clock |
| Key 1 | 4 | Digital In (PU) |
| Key 2 | 5 | Digital In (PU) |
| Key 3 | 6 | Digital In (PU) |
| Enc1 A | 7 | Interrupt |
| Enc1 B | 8 | Interrupt |
| Enc1 Btn | 9 | Digital In (PU) |
| Enc2 A | 10 | Interrupt |
| Enc2 B | 11 | Interrupt |
| Enc2 Btn | 12 | Digital In (PU) |
| Speaker | 13 | PWM |

## Troubleshooting Quick Fixes

| Problem | Solution |
|---------|----------|
| Won't pair | Enable Developer Mode in Claude Desktop |
| No display | Try I2C address 0x3D instead of 0x3C |
| Keys dead | Check INPUT_PULLUP in code |
| Encoder skips | Add 10kΩ pullups, check A/B not swapped |
| No sound | Verify PWM pin, check polarity |
| Laggy | Move closer to computer, check BLE signal |
| Bridge crashes | Check Python dependencies installed |

## Audio Pattern Format

```
"freq:duration,freq:duration,..."

Examples:
Startup:    "523:150,0:50,659:150,0:50,784:300"
Alert:      "1200:150,0:100,800:150,0:100,1200:300"
Success:    "880:100,0:50,1100:150"
Error:      "150:300,0:200,150:500"

Note: freq=0 means silence (pause)
```

## Serial Commands (Debug)

Send via Arduino Serial Monitor (115200 baud):

```
TEST_AUDIO      - Play startup melody
TEST_KEYS       - Show key states
TEST_ENCODERS   - Show encoder positions
TEST_DISPLAY    - Fill screen patterns
TEST_BLE        - Show BLE status
RESET           - Software reset
```

## Build Steps Summary

```
1. Test all components individually
2. Wire power rails (3.3V, GND)
3. Wire I2C (OLED)
4. Wire keys and encoders
5. Wire speaker
6. Solder all connections
7. Flash test firmware
8. Test each component
9. Flash main firmware
10. Pair with desktop
11. Run bridge
12. Test complete system
```

## Component Sources

| Component | Suggested Source |
|-----------|------------------|
| ESP32-S3 | Amazon, Espressif |
| OLED | Amazon (search "1.3 SH1106") |
| Keys | NovelKeys, Drop, AliExpress |
| Encoders | Amazon (search "EC11 encoder") |
| Speaker | Amazon ("8ohm mini speaker") |
| Misc | Adafruit, SparkFun, DigiKey |

## Safety Notes

- ESP32 pins are **3.3V only** (5V damages GPIO)
- OLED needs 3.3V, not 5V
- Speaker: check polarity
- Power: USB provides 500mA max (sufficient)
- Soldering: Use ventilation, wash hands after

## Useful Links

- Reference: https://github.com/anthropics/claude-desktop-buddy
- ESP32 Pinout: https://docs.espressif.com/projects/esp-idf/
- Nordic UART: https://developer.nordicsemi.com/
- Arduino IDE: https://www.arduino.cc/en/software
