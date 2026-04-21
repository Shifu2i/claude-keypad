# Hardware Specification

## Component List

### Required Components

| Component | Qty | Specifications | Purpose |
|-----------|-----|----------------|---------|
| ESP32 DevKit | 1 | ESP32-S3 or ESP32-WROOM | Main controller |
| OLED Display | 1 | 1.3" SH1106 or SSD1306, 128x64, I2C | Visual feedback |
| Cherry MX Switches | 3 | Any color/style | Primary keys |
| Keycaps | 3 | 1U size | Key labels |
| Rotary Encoders | 2 | EC11 or KY-040 with push button | Dual function inputs |
| Encoder Knobs | 2 | 6mm D-shaft | Grip/control |
| Speaker | 1 | 8Ω 0.5W or Piezo buzzer | Audio feedback |
| USB-C Module | 1 | Breakout board or built into ESP32 | Power/data |
| Perfboard/PCB | 1 | Minimum 10x15cm or custom PCB | Assembly |
| Wires | Various | 22-26 AWG | Connections |

### Optional Components

| Component | Purpose |
|-----------|---------|
| LED Indicators (3-5x) | Visual layer/status indication |
| 10kΩ Resistors | Pull-downs for encoder pins |
| Tactile switches | Additional side buttons |
| LiPo Battery + Charger | Wireless operation |
| 3D Printed Case | Enclosure |

---

## Pin Assignment

### ESP32 GPIO Mapping

```
ESP32 DevKit
├── GPIO 21 (SDA) ─────────────► OLED SDA
├── GPIO 22 (SCL) ─────────────► OLED SCL
│
├── GPIO 4  ───────────────────► KEY 1 (with pull-up)
├── GPIO 5  ───────────────────► KEY 2 (with pull-up)
├── GPIO 6  ───────────────────► KEY 3 (with pull-up)
│
├── GPIO 7  ───────────────────► ENCODER 1 - Pin A (CLK)
├── GPIO 8  ───────────────────► ENCODER 1 - Pin B (DT)
├── GPIO 9  ───────────────────► ENCODER 1 - Button
│
├── GPIO 10 ───────────────────► ENCODER 2 - Pin A (CLK)
├── GPIO 11 ───────────────────► ENCODER 2 - Pin B (DT)
├── GPIO 12 ───────────────────► ENCODER 2 - Button
│
├── GPIO 13 ───────────────────► SPEAKER (PWM)
│
├── 3.3V  ─────────────────────► OLED VCC, Encoder VCC
└── GND   ─────────────────────► Common ground
```

### Detailed Pin Table

| Function | GPIO | Direction | Pull-Up | Notes |
|----------|------|-----------|---------|-------|
| OLED SDA | 21 | I2C Data | - | Built-in I2C |
| OLED SCL | 22 | I2C Clock | - | Built-in I2C |
| Key 1 | 4 | Digital In | Internal | Active LOW |
| Key 2 | 5 | Digital In | Internal | Active LOW |
| Key 3 | 6 | Digital In | Internal | Active LOW |
| Encoder 1 A | 7 | Digital In | Internal | Interrupt capable |
| Encoder 1 B | 8 | Digital In | Internal | Interrupt capable |
| Encoder 1 Btn | 9 | Digital In | Internal | Active LOW |
| Encoder 2 A | 10 | Digital In | Internal | Interrupt capable |
| Encoder 2 B | 11 | Digital In | Internal | Interrupt capable |
| Encoder 2 Btn | 12 | Digital In | Internal | Active LOW |
| Speaker | 13 | PWM Out | - | 8-bit resolution |

---

## Wiring Diagram

### OLED Display (I2C)

```
OLED Pin    ESP32 Pin
--------    ---------
VCC    ────── 3.3V
GND    ────── GND
SCL    ────── GPIO 22
SDA    ────── GPIO 21
```

*Note: Some OLED modules have 4 pins, others have 7. Only VCC, GND, SCL, SDA are needed.*

### Mechanical Keys

```
Key Switch Wiring:

GPIO Pin ──────┬──────────┬──────────┐
               │          │          │
            ┌──┴──┐    ┌─┴──┐    ┌─┴──┐
            │Key 1│    │Key 2│    │Key 3│
            └──┬──┘    └─┬──┘    └─┬──┘
               │          │          │
               └──────────┴──────────┴── GND

One side of each key connects to GPIO
Other side connects to common GND
```

### Rotary Encoders

```
Encoder Pinout (EC11):

        ┌─────────┐
        │  ╔═══╗  │
        │  ║Top║  │  ← Knob/Shaft
        │  ╚═══╝  │
        │         │
     ┌──┴──┬──┬───┴──┐
     │ CLK │  │  DT  │
     │(A)  │  │ (B)  │
     └──┬──┘  └──┬───┘
        │        │
      GPIO7    GPIO8
        │        │
        │   ┌────┴────┐
        │   │  SW     │  ← Push button
        │   │(Button) │
        │   └────┬────┘
        │        │
      GPIO9    GND
        │        │
     ══╧════════╧══
        COMMON GND

Repeat for Encoder 2 using GPIO 10, 11, 12
```

### Speaker

```
Speaker Pin    Connection
-----------    ----------
+ (red)    ──── GPIO 13
- (black)  ──── GND

For piezo buzzer: Same connection

Optional: Add 100Ω resistor in series for volume control
```

---

## Power Requirements

### Current Draw Estimate

| Component | Active Current | Notes |
|-----------|----------------|-------|
| ESP32 | 100-240mA | BLE active, peaks during transmit |
| OLED Display | 10-20mA | Depends on brightness |
| Mechanical Keys | ~0mA | Passive switches |
| Encoders | ~1mA | Internal logic |
| Speaker | 10-50mA | Peak during sound |
| **Total** | **~150-300mA** | Budget 500mA for USB safety |

### Power Options

**Option 1: USB Powered (Recommended)**
- Connect ESP32 via USB-C/USB-Micro
- Provides 5V → onboard 3.3V regulator
- Simple, always on when computer is on

**Option 2: Battery Powered**
- LiPo 3.7V 500-1000mAh
- Add TP4056 charging module
- Add ON/OFF switch
- Runtime: 4-8 hours depending on usage

---

## Physical Layout Suggestions

### Minimal Layout (10cm x 8cm)

```
┌─────────────────────────────┐
│  ┌─────────┐ ┌─────────┐   │
│  │ENCODER 1│ │ENCODER 2│   │
│  │  [===]  │ │  [===]  │   │
│  └─────────┘ └─────────┘   │
│                            │
│    [KEY 1] [KEY 2] [KEY 3] │
│                            │
│    ┌───────────────┐       │
│    │    OLED       │       │
│    │   128x64      │       │
│    └───────────────┘       │
└─────────────────────────────┘
```

### Recommended Layout (15cm x 10cm)

```
┌───────────────────────────────────┐
│                                   │
│   ┌─────────┐     ┌─────────┐    │
│   │ENCODER 1│     │ENCODER 2│    │
│   │  [===]  │     │  [===]  │    │
│   │ (Left)  │     │ (Right) │    │
│   └─────────┘     └─────────┘    │
│                                   │
│   ┌─────────────────────────┐    │
│   │         OLED            │    │
│   │      128x64 Display     │    │
│   │   (Shows state/layer)   │    │
│   └─────────────────────────┘    │
│                                   │
│   [K1]        [K2]        [K3]   │
│   Send      Toggle       New     │
│   Msg       Focus        Chat      │
│                                   │
│        ┌─────────────┐            │
│        │   SPEAKER   │            │
│        └─────────────┘            │
│                                   │
└───────────────────────────────────┘
```

---

## Bill of Materials (Shopping List)

### Essential (~$30-50)

| Item | Est. Price | Source |
|------|------------|--------|
| ESP32-S3 DevKit | $8-12 | Amazon, AliExpress |
| 1.3" OLED (SH1106) | $5-8 | Amazon |
| Cherry MX Switches (3x) | $3-5 | NovelKeys, Drop |
| EC11 Encoders (2x) | $2-4 | Amazon |
| Speaker 8Ω | $1-3 | Amazon |
| Perfboard 10x15cm | $3-5 | Amazon |
| Jumper wires | $3-5 | Amazon |
| Keycaps (3x 1U) | $5-15 | Various |

### Tools Needed
- Soldering iron
- Wire strippers
- Flush cutters
- Multimeter (for testing)
- 3D printer (optional, for case)

---

## Testing Checklist

Before final assembly, test each component individually:

- [ ] ESP32 powers on and connects via USB
- [ ] OLED displays test pattern
- [ ] Each key registers (use serial monitor)
- [ ] Each encoder direction registers
- [ ] Each encoder button registers
- [ ] Speaker plays test tones
- [ ] BLE advertises and pairs

---

## Troubleshooting

### OLED not displaying
- Check I2C address (0x3C vs 0x3D)
- Verify SDA/SCL not swapped
- Check 3.3V (not 5V) supply

### Keys not working
- Verify pull-up resistors enabled in code
- Test with multimeter for continuity
- Check GPIO numbers match code

### Encoder skipping steps
- Add 0.1µF capacitors between A/B and GND
- Enable internal pull-ups
- Check encoder quality (EC11 clones vary)

### Speaker too quiet
- Check PWM pin capability
- Verify speaker impedance (8Ω)
- Increase duty cycle in code
