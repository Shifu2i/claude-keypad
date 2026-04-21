# Assembly & Build Instructions

## Tools Required

### Essential
- Soldering iron (temperature controlled, 25-60W)
- Solder (lead-free rosin core, 0.6-0.8mm)
- Flush cutters (for trimming leads)
- Wire strippers (22-26 AWG)
- Multimeter (for continuity testing)
- Helping hands or PCB holder

### Optional but Recommended
- Desoldering braid/wick
- Flux pen
- Tweezers (ESD safe)
- Heat shrink tubing
- Hot glue gun (for strain relief)
- 3D printer (for case)

---

## Component Preparation

### Step 1: Test All Components

Before soldering, verify each component works:

```cpp
// Test sketch for Arduino IDE
// Upload to ESP32 before assembly

void setup() {
  Serial.begin(115200);
  
  // Test all GPIOs
  int pins[] = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 21, 22};
  
  for (int i = 0; i < 12; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], HIGH);
    delay(100);
    digitalWrite(pins[i], LOW);
    Serial.printf("Tested pin %d\n", pins[i]);
  }
  
  Serial.println("GPIO test complete");
  
  // Test speaker
  pinMode(13, OUTPUT);
  tone(13, 1000, 500);
  delay(500);
  noTone(13);
  Serial.println("Speaker test complete");
}

void loop() {}
```

---

## Assembly Steps

### Step 2: Prepare the Base/Case

#### Option A: Perfboard (Quick Build)

1. Cut perfboard to size (recommend 10cm x 15cm)
2. Mark component positions:

```
Layout (bottom view, looking through board):

    [ENC1]          [ENC2]
      ◯               ◯
      │               │
  ┌───┴───┐       ┌───┴───┐
  │ A B SW│       │ A B SW│
  └───┬───┘       └───┬───┘
      │               │
      │               │
      │    ┌─────┐    │
      │    │ OLED│    │
      │    │     │    │
      │    └─────┘    │
      │               │
   ┌──┴──┬───┬───┬───┴──┐
   │ K1  │K2 │K3 │      │
   └──┬──┴───┴───┴───┬──┘
      │               │
    ┌─┴─┐           ┌─┴─┐
    │USB│           │SPK│
    └───┘           └───┘
```

#### Option B: Custom PCB

Gerber files would be provided separately. Skip to component mounting.

#### Option C: 3D Printed Case

Recommended dimensions: 15cm x 10cm x 3cm (L x W x H)

Mount points needed:
- 2x Encoder mounting holes (7mm spacing)
- 3x Key switch cutouts (14mm x 14mm)
- 1x OLED opening (35mm x 20mm)
- 1x Speaker grille/hole
- 1x USB-C opening

---

### Step 3: Mount Encoders

1. **Position encoders** on left and right sides
2. **Mark mounting holes** and drill if using perfboard
3. **Insert encoders** and secure with nuts (finger tight for now)
4. **Do not solder yet** - wait until all components placed

**Encoder pin orientation:**
```
Looking from top of encoder:

        ┌───┐
        │Top│
        └───┘
          │
     ┌────┴────┐
     │         │
   ┌─┴─┐     ┌─┴─┐
   │CLK│     │DT │
   │ A │     │ B │
   └─┬─┘     └─┬─┘
     │         │
   ┌─┴─────────┴─┐
   │             │
   │   Button    │
   │    (SW)     │
   └──────┬──────┘
          │
         GND
```

---

### Step 4: Mount Keys

1. **Cut holes** in perfboard for key switches (14mm x 14mm)
   - Use nibbler tool or drill multiple holes and file
2. **Insert key switches** from top
3. **Secure with plate** (if using switch plate) or hot glue from bottom
4. **Leave pins unsoldered**

**Key spacing:** Standard 19mm (0.75") centers
```
   ┌─────┬─────┬─────┐
   │ K1  │ K2  │ K3  │
   │ 1U  │ 1U  │ 1U  │
   │     │     │     │
   │     │     │     │
   └─────┴─────┴─────┘
    19mm  19mm
```

---

### Step 5: Mount OLED Display

1. **Cut opening** for display (approx 35mm x 20mm)
2. **Mount display** with hot glue or standoffs
3. **Keep header pins accessible** for soldering

**I2C pins:**
- VCC → 3.3V
- GND → GND
- SCL → GPIO 22
- SDA → GPIO 21

---

### Step 6: Mount Speaker

1. **Drill speaker hole** (if using enclosed speaker)
2. **Mount with hot glue** or double-sided tape
3. **Keep polarity in mind** (+ to GPIO, - to GND)

**Optional volume control:**
Add 100Ω resistor in series with speaker positive for quieter operation.

---

### Step 7: Wire Everything

#### Wiring Order (recommended)

1. **Power rails first**
   - Run 3.3V and GND lines to all components
   - Use colored wire (red = 3.3V, black = GND)

2. **I2C bus** (OLED)
   - Yellow = SCL (GPIO 22)
   - Green = SDA (GPIO 21)

3. **Digital inputs** (keys, encoder buttons)
   - Use short wire runs
   - Keep away from speaker wires (can cause noise)

4. **Encoder signals** (A/B pins)
   - Use twisted pair if possible (reduces noise)
   - Keep wire lengths equal for A and B

5. **Speaker** (last, to avoid accidental shorts)

#### Detailed Wiring Table

| From | To | Wire Color | Notes |
|------|-----|------------|-------|
| ESP32 GPIO 4 | Key 1 (one leg) | Any | Other leg to GND |
| ESP32 GPIO 5 | Key 2 (one leg) | Any | Other leg to GND |
| ESP32 GPIO 6 | Key 3 (one leg) | Any | Other leg to GND |
| ESP32 GPIO 7 | Enc1 CLK (A) | Yellow | |
| ESP32 GPIO 8 | Enc1 DT (B) | Green | |
| ESP32 GPIO 9 | Enc1 Button | Any | To GND when pressed |
| ESP32 GPIO 10 | Enc2 CLK (A) | Orange | |
| ESP32 GPIO 11 | Enc2 DT (B) | Blue | |
| ESP32 GPIO 12 | Enc2 Button | Any | To GND when pressed |
| ESP32 GPIO 13 | Speaker (+) | Red | Via 100Ω resistor |
| ESP32 GPIO 21 | OLED SDA | Green | I2C data |
| ESP32 GPIO 22 | OLED SCL | Yellow | I2C clock |
| ESP32 3.3V | OLED VCC | Red | |
| ESP32 3.3V | Enc1 VCC | Red | If encoder has LED |
| ESP32 3.3V | Enc2 VCC | Red | If encoder has LED |
| ESP32 GND | Common GND | Black | All grounds connected |

---

### Step 8: Solder Connections

1. **Start with power rails**
   - Solder all 3.3V and GND connections first
   - Test continuity with multimeter

2. **Solder signal wires**
   - Work from one component to next
   - Keep solder joints clean and shiny

3. **Trim excess leads** with flush cutters

4. **Inspect for shorts**
   - Check adjacent pins aren't bridged
   - Test continuity between power and ground

#### Soldering Tips

```
Good solder joint:        Bad solder joint:
    ┌───┐                  ┌───┐
   ╱     ╲                ╱  ╱╲╲
  │  ●●●  │              │ ●   ●│
  │ ●●●●● │              │●  ●●●│
  └──┬┬┬──┘              └──┬┬┬──┘
     │││                    │││

Shiny, smooth,           Dull, blobby,
concave surface          excess solder
```

---

### Step 9: Mount ESP32

1. **Connect ESP32** via USB for power/programming
2. **Option A:** Solder directly to perfboard (not recommended - hard to remove)
3. **Option B:** Use female headers (recommended - removable)
4. **Option C:** Use pin headers and Dupont connections (most flexible)

#### Recommended: Female Headers

```
ESP32 sits in female headers on perfboard:

Perfboard:          ESP32:
┌────────────┐      ┌────────────┐
│ ░░░░░░░░░░ │      │ ▓▓▓▓▓▓▓▓▓▓ │
│ ░░░░░░░░░░ │◀────▶│ ▓▓▓▓▓▓▓▓▓▓ │
│ ░░░░░░░░░░ │      │ ▓▓▓▓▓▓▓▓▓▓ │
└────────────┘      └────────────┘
  Female               Male
  headers              pins
  (soldered            (on ESP32)
  to board)
```

---

### Step 10: Initial Power-On Test

**Before connecting all peripherals:**

1. **Connect ESP32 USB** to computer
2. **Open Serial Monitor** at 115200 baud
3. **Upload test firmware** from Step 1
4. **Verify:**
   - ESP32 boots (blue LED flashes)
   - Serial output appears
   - No smoke or burning smell

**If ESP32 doesn't boot:**
- Check USB cable (some are charge-only)
- Verify BOOT button not shorted
- Check for solder bridges on ESP32 pins

---

### Step 11: Component Testing

Upload comprehensive test sketch:

```cpp
// comprehensive_test.ino
#include <Wire.h>
#include <U8g2lib.h>

// Pin definitions
const int KEYS[] = {4, 5, 6};
const int ENC_1_A = 7, ENC_1_B = 8, ENC_1_BTN = 9;
const int ENC_2_A = 10, ENC_2_B = 11, ENC_2_BTN = 12;
const int SPEAKER = 13;

U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0);

void setup() {
  Serial.begin(115200);
  Serial.println("=== COMPREHENSIVE TEST ===");
  
  // Initialize pins
  for (int i = 0; i < 3; i++) {
    pinMode(KEYS[i], INPUT_PULLUP);
  }
  pinMode(ENC_1_BTN, INPUT_PULLUP);
  pinMode(ENC_2_BTN, INPUT_PULLUP);
  
  // Test display
  display.begin();
  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tf);
  display.drawStr(0, 10, "Testing...");
  display.sendBuffer();
  
  Serial.println("1. Display: OK");
  delay(500);
  
  // Test speaker
  tone(SPEAKER, 1000, 200);
  Serial.println("2. Speaker: OK");
  delay(300);
  
  display.drawStr(0, 25, "Ready for input test");
  display.sendBuffer();
}

void loop() {
  // Test keys
  for (int i = 0; i < 3; i++) {
    if (digitalRead(KEYS[i]) == LOW) {
      Serial.printf("Key %d pressed\n", i + 1);
      tone(SPEAKER, 1000 + (i * 200), 100);
      delay(200);
    }
  }
  
  // Test encoder buttons
  if (digitalRead(ENC_1_BTN) == LOW) {
    Serial.println("Enc1 button pressed");
    tone(SPEAKER, 1500, 100);
    delay(200);
  }
  
  if (digitalRead(ENC_2_BTN) == LOW) {
    Serial.println("Enc2 button pressed");
    tone(SPEAKER, 1700, 100);
    delay(200);
  }
  
  delay(10);
}
```

**Expected behavior:**
- Display shows "Testing..." then "Ready for input test"
- Speaker plays tones
- Pressing keys/encoders shows messages and plays tones

---

### Step 12: Final Assembly

1. **Mount ESP32** in final position
2. **Secure all wiring** with:
   - Hot glue strain relief at stress points
   - Zip ties for cable management
   - Heat shrink on exposed connections

3. **Attach keycaps** to switches
4. **Attach encoder knobs**
5. **Fit into case** (if using)

### Step 13: Firmware Upload

1. **Install required libraries** (see 03_FIRMWARE.md)
2. **Upload main firmware** (macro_buddy.ino)
3. **Verify BLE advertising** - device should appear as "ClaudeBuddy"
4. **Test with bridge** (see 05_DESKTOP_BRIDGE.md)

---

## Troubleshooting Assembly

### Display Not Working

**Symptom:** Blank screen or garbled pixels

**Check:**
- [ ] I2C address (0x3C vs 0x3D) - try both in code
- [ ] SDA/SCL not swapped
- [ ] 3.3V present at OLED (not 5V!)
- [ ] Display initialization in code

**Fix:**
```cpp
// Try alternative display constructor
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0);  // SSD1306 variant
```

### Keys Not Responding

**Symptom:** No input detected

**Check:**
- [ ] Continuity from key leg to GPIO
- [ ] Other key leg to GND
- [ ] `INPUT_PULLUP` mode set in code
- [ ] Keys oriented correctly (check datasheet)

### Encoders Skipping/Not Working

**Symptom:** Erratic or no rotation detection

**Check:**
- [ ] A and B pins not swapped
- [ ] Interrupt pins (GPIO 7-12 support interrupts on ESP32)
- [ ] Encoder mechanically seated
- [ ] Add pull-up resistors if internal weak pullups insufficient

**Fix:** Add external 10kΩ pull-ups to A and B pins:
```
GPIO ──┬── Encoder Pin
       │
     ┌─┴─┐
     │10k│
     └─┬─┘
       │
      GND
```

### Speaker Not Working

**Symptom:** No sound or very quiet

**Check:**
- [ ] Correct GPIO pin (must support PWM)
- [ ] Polarity correct
- [ ] Volume resistor not too large
- [ ] `ledc` functions used (ESP32 PWM)

### BLE Not Advertising

**Symptom:** Device doesn't appear in scan

**Check:**
- [ ] ESP32 has BLE support (most do)
- [ ] Correct board selected in Arduino IDE
- [ ] `BLEDevice::init()` called
- [ ] `startAdvertising()` called

---

## Case Design (3D Printed)

### Basic Enclosure Dimensions

```
External: 150mm x 100mm x 35mm (L x W x H)
Internal: 144mm x 94mm x 30mm
Wall thickness: 3mm

Component Cutouts:
- Encoders: 7mm diameter, 14mm spacing for mounting
- Keys: 14mm x 14mm square (x3, 19mm spacing)
- OLED: 35mm x 20mm rectangle
- Speaker: Ø25mm or grille pattern
- USB: 12mm x 8mm rectangle
```

### Recommended Print Settings
- **Material:** PLA or PETG
- **Layer height:** 0.2mm
- **Infill:** 20%
- **Supports:** Yes, for key cutouts and mounting posts

---

## Bill of Materials

| Item | Qty | Est. Cost (USD) | Source |
|------|-----|-----------------|--------|
| ESP32-S3-DevKitC-1 | 1 | $12 | Amazon, Espressif |
| 1.3" OLED (SH1106) | 1 | $6 | Amazon |
| Cherry MX Switches | 3 | $3 | NovelKeys, AliExpress |
| EC11 Rotary Encoders | 2 | $3 | Amazon |
| Keycaps (1U, 3x) | 3 | $6 | Various |
| 8Ω Speaker | 1 | $2 | Amazon |
| Perfboard 10x15cm | 1 | $4 | Amazon |
| Female Headers 2.54mm | 40 pins | $2 | Amazon |
| Jumper wires (M-M, M-F) | 40 each | $5 | Amazon |
| USB-C cable | 1 | $5 | Any |
| M3 Screws & standoffs | 4 sets | $3 | Amazon |
| **Total** | | **~$51** | |

---

## Post-Assembly Checklist

- [ ] All solder joints inspected (shiny, no cold joints)
- [ ] No shorts between power and ground
- [ ] USB connection reliable
- [ ] All keys register correctly
- [ ] Both encoders work smoothly
- [ ] Speaker plays all tones
- [ ] Display shows clearly
- [ ] BLE pairs with computer
- [ ] Bridge software connects
- [ ] All layers functional
- [ ] Audio cues working
- [ ] Case assembled (if applicable)

---

## Next Steps

1. **Configure the bridge** (05_DESKTOP_BRIDGE.md)
2. **Customize function mappings** (06_FUNCTION_MAPS.md)
3. **Test all features**
4. **Iterate on firmware** for your workflow
