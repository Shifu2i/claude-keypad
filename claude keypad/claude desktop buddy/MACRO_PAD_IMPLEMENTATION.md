# Claude Code Macro Pad Implementation

A complete guide for building a 3-key + 2-rotary encoder + speaker macro pad that integrates with Claude Code via BLE.

## Overview

Based on the claude-desktop-buddy architecture, this implementation uses:
- **BLE GATT** with Nordic UART Service (NUS)
- **JSON protocol** over TX/RX characteristics
- **Desktop Bridge** (requires Developer Mode in Claude Desktop)

---

## Hardware Specifications

### Components
| Component | Specs |
|-----------|-------|
| Microcontroller | ESP32-S3 (recommended) or ESP32 |
| Keys | 3x Mechanical switches (Cherry MX or similar) |
| Rotary Encoders | 2x EC11 or similar (with push button) |
| Speaker | 1x Small buzzer/speaker (8Ω or piezo) |
| Power | USB-C or Battery + Charging module |

### Pin Mapping (Example)
```cpp
// Keys (with pull-up resistors or internal pull-up)
#define KEY_1_PIN  4
#define KEY_2_PIN  5
#define KEY_3_PIN  6

// Rotary Encoder 1
#define ENC_1_A    7    // CLK
#define ENC_1_B    8    // DT
#define ENC_1_BTN  9    // Push button

// Rotary Encoder 2
#define ENC_2_A    10   // CLK
#define ENC_2_B    11   // DT
#define ENC_2_BTN  12   // Push button

// Speaker
#define SPEAKER_PIN 13
```

---

## Firmware Implementation (Arduino/ESP32)

### 1. BLE Configuration

```cpp
// ble_config.h
#ifndef BLE_CONFIG_H
#define BLE_CONFIG_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLECharacteristic.h>

// Nordic UART Service UUIDs
#define NUS_SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define NUS_TX_UUID      "6e400003-b5a3-f393-e0a9-e50e24dcca9e"  // Device -> Desktop
#define NUS_RX_UUID      "6e400002-b5a3-f393-e0a9-e50e24dcca9e"  // Desktop -> Device

// Connection state
extern bool deviceConnected;
extern bool oldDeviceConnected;

// Characteristics
extern BLECharacteristic* pTxCharacteristic;
extern BLECharacteristic* pRxCharacteristic;

// Initialize BLE server
void initBLE(const char* deviceName);
void handleBLE();

// Send JSON message to desktop
void sendMessage(const JsonDocument& doc);

#endif
```

### 2. Main Firmware

```cpp
// macro_pad.ino
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Encoder.h>
#include <Bounce2.h>
#include "ble_config.h"

// ============== CONFIGURATION ==============
const char* DEVICE_NAME = "ClaudeMacroPad";

// Key pins
const int KEY_PINS[] = {4, 5, 6};
const int KEY_COUNT = 3;

// Encoder pins
Encoder enc1(7, 8);
Encoder enc2(10, 11);
const int ENC_1_BTN = 9;
const int ENC_2_BTN = 12;

// Speaker
const int SPEAKER_PIN = 13;

// ============== STATE ==============
Bounce keys[KEY_COUNT];
Bounce enc1Btn;
Bounce enc2Btn;

long enc1LastPos = 0;
long enc2LastPos = 0;

unsigned long lastActivity = 0;
bool isIdle = false;

// ============== SETUP ==============
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting Claude Macro Pad...");

  // Initialize keys with debouncing
  for (int i = 0; i < KEY_COUNT; i++) {
    keys[i].attach(KEY_PINS[i], INPUT_PULLUP);
    keys[i].interval(10);
  }

  // Initialize encoder buttons
  enc1Btn.attach(ENC_1_BTN, INPUT_PULLUP);
  enc1Btn.interval(10);
  enc2Btn.attach(ENC_2_BTN, INPUT_PULLUP);
  enc2Btn.interval(10);

  // Initialize speaker
  pinMode(SPEAKER_PIN, OUTPUT);
  digitalWrite(SPEAKER_PIN, LOW);

  // Initialize BLE
  initBLE(DEVICE_NAME);

  // Startup tone
  tone(SPEAKER_PIN, 880, 200);
  delay(100);
  tone(SPEAKER_PIN, 1760, 300);

  lastActivity = millis();
}

// ============== MAIN LOOP ==============
void loop() {
  handleBLE();
  handleKeys();
  handleEncoders();
  handleIdleState();
}

// ============== INPUT HANDLERS ==============

void handleKeys() {
  for (int i = 0; i < KEY_COUNT; i++) {
    keys[i].update();

    if (keys[i].fell()) {  // Key pressed
      StaticJsonDocument<256> doc;
      doc["type"] = "key_press";
      doc["key_id"] = i + 1;  // 1-indexed
      doc["timestamp"] = millis();
      sendMessage(doc);

      activityDetected();
      playFeedbackTone(1000 + (i * 200), 50);
    }

    if (keys[i].rose()) {  // Key released
      StaticJsonDocument<256> doc;
      doc["type"] = "key_release";
      doc["key_id"] = i + 1;
      doc["duration"] = keys[i].duration();
      sendMessage(doc);
    }
  }
}

void handleEncoders() {
  // Encoder 1
  long pos1 = enc1.read();
  if (pos1 != enc1LastPos) {
    int delta = pos1 - enc1LastPos;

    StaticJsonDocument<256> doc;
    doc["type"] = "encoder_turn";
    doc["encoder_id"] = 1;
    doc["delta"] = delta;
    doc["absolute"] = pos1;
    sendMessage(doc);

    enc1LastPos = pos1;
    activityDetected();
    playTickSound();
  }

  // Encoder 2
  long pos2 = enc2.read();
  if (pos2 != enc2LastPos) {
    int delta = pos2 - enc2LastPos;

    StaticJsonDocument<256> doc;
    doc["type"] = "encoder_turn";
    doc["encoder_id"] = 2;
    doc["delta"] = delta;
    doc["absolute"] = pos2;
    sendMessage(doc);

    enc2LastPos = pos2;
    activityDetected();
    playTickSound();
  }

  // Encoder buttons
  enc1Btn.update();
  if (enc1Btn.fell()) {
    StaticJsonDocument<256> doc;
    doc["type"] = "encoder_press";
    doc["encoder_id"] = 1;
    sendMessage(doc);
    activityDetected();
    playFeedbackTone(1500, 80);
  }

  enc2Btn.update();
  if (enc2Btn.fell()) {
    StaticJsonDocument<256> doc;
    doc["type"] = "encoder_press";
    doc["encoder_id"] = 2;
    sendMessage(doc);
    activityDetected();
    playFeedbackTone(1700, 80);
  }
}

// ============== UTILITY FUNCTIONS ==============

void activityDetected() {
  lastActivity = millis();
  if (isIdle) {
    isIdle = false;
    // Send wake message
    StaticJsonDocument<128> doc;
    doc["type"] = "state_change";
    doc["state"] = "active";
    sendMessage(doc);
  }
}

void handleIdleState() {
  const unsigned long IDLE_TIMEOUT = 30000;  // 30 seconds

  if (!isIdle && millis() - lastActivity > IDLE_TIMEOUT) {
    isIdle = true;
    StaticJsonDocument<128> doc;
    doc["type"] = "state_change";
    doc["state"] = "idle";
    sendMessage(doc);
  }
}

void playFeedbackTone(int frequency, int duration) {
  if (!isIdle) {  // Don't play during idle
    tone(SPEAKER_PIN, frequency, duration);
  }
}

void playTickSound() {
  // Short subtle tick
  tone(SPEAKER_PIN, 800, 10);
}

// Handle incoming commands from desktop
void onDesktopMessage(const JsonDocument& doc) {
  const char* cmd = doc["command"];

  if (strcmp(cmd, "beep") == 0) {
    int freq = doc["frequency"] | 1000;
    int dur = doc["duration"] | 100;
    tone(SPEAKER_PIN, freq, dur);
  }
  else if (strcmp(cmd, "play_pattern") == 0) {
    playPattern(doc["pattern"]);
  }
  else if (strcmp(cmd, "set_led") == 0) {
    // If you add LEDs
    int ledId = doc["led_id"];
    bool state = doc["state"];
    // digitalWrite(LED_PINS[ledId], state);
  }
}

void playPattern(const char* pattern) {
  // Pattern: sequence of "freq:duration," e.g., "1000:100,0:50,1500:100"
  // 0 = silence
}
```

### 3. BLE Implementation

```cpp
// ble_config.cpp
#include "ble_config.h"

bool deviceConnected = false;
bool oldDeviceConnected = false;
BLECharacteristic* pTxCharacteristic = nullptr;
BLECharacteristic* pRxCharacteristic = nullptr;

class ServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("BLE Client Connected");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("BLE Client Disconnected");
  }
};

class RxCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
      Serial.print("RX: ");
      Serial.println(value.c_str());

      // Parse incoming JSON
      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, value);

      if (!error) {
        onDesktopMessage(doc);
      }
    }
  }
};

void initBLE(const char* deviceName) {
  BLEDevice::init(deviceName);

  BLEServer* pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService* pService = pServer->createService(NUS_SERVICE_UUID);

  // TX Characteristic (Notify) - Device to Desktop
  pTxCharacteristic = pService->createCharacteristic(
    NUS_TX_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pTxCharacteristic->addDescriptor(new BLE2902());

  // RX Characteristic (Write) - Desktop to Device
  pRxCharacteristic = pService->createCharacteristic(
    NUS_RX_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  pRxCharacteristic->setCallbacks(new RxCallbacks());

  pService->start();

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(NUS_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);

  BLEDevice::startAdvertising();
  Serial.println("BLE Advertising started");
}

void handleBLE() {
  // Handle reconnection
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    BLEDevice::startAdvertising();
    Serial.println("Restarting advertising");
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
}

void sendMessage(const JsonDocument& doc) {
  if (!deviceConnected || !pTxCharacteristic) return;

  char buffer[512];
  size_t len = serializeJson(doc, buffer);

  pTxCharacteristic->setValue((uint8_t*)buffer, len);
  pTxCharacteristic->notify();

  Serial.print("TX: ");
  Serial.println(buffer);
}
```

---

## Protocol Specification

### Device → Desktop (Input Events)

```json
// Key press
{
  "type": "key_press",
  "key_id": 1,
  "timestamp": 12345
}

// Key release
{
  "type": "key_release",
  "key_id": 1,
  "duration": 150
}

// Encoder rotation
{
  "type": "encoder_turn",
  "encoder_id": 1,
  "delta": 2,
  "absolute": 42
}

// Encoder button press
{
  "type": "encoder_press",
  "encoder_id": 1
}

// State change
{
  "type": "state_change",
  "state": "active"
}
```

### Desktop → Device (Feedback)

```json
// Audio feedback
{
  "command": "beep",
  "frequency": 1000,
  "duration": 100
}

// Pattern playback
{
  "command": "play_pattern",
  "pattern": "880:100,0:50,1760:200"
}

// LED control (if implemented)
{
  "command": "set_led",
  "led_id": 1,
  "state": true,
  "color": "#FF0000"
}
```

---

## Function Mappings

### Default Layer (General Controls)

| Input | Function | Claude Code Action |
|-------|----------|-------------------|
| **Key 1** | Send Message | Sends predefined message to Claude |
| **Key 2** | Toggle Focus | Focus/defocus Claude Code window |
| **Key 3** | New Chat | Start a new conversation |
| **Encoder 1 Turn** | Scroll History | Scroll up/down through transcript |
| **Encoder 1 Push** | Toggle Scroll Mode | Switch between fine/coarse scrolling |
| **Encoder 2 Turn** | Adjust Font | Increase/decrease UI font size |
| **Encoder 2 Push** | Reset Font | Reset to default font size |

### Approval Layer (Auto-activated on permission prompt)

| Input | Function |
|-------|----------|
| **Key 1** | **Approve** - Allow the action |
| **Key 2** | **Deny** - Reject the action |
| **Key 3** | **View Details** - Show full permission text |
| **Encoder 1** | Scroll permission details |
| **Encoder 2** | Scroll through multiple pending approvals |

### Coding Layer (Manual toggle with Key 1 + Encoder 1 push)

| Input | Function | Action |
|-------|----------|--------|
| **Key 1** | Run Build | Execute build command |
| **Key 2** | Run Tests | Execute test suite |
| **Key 3** | Quick Fix | Trigger Claude's quick fix |
| **Encoder 1** | Navigate Errors | Jump between errors/warnings |
| **Encoder 2** | Navigate Files | Scroll through open files |

### Debug Layer

| Input | Function |
|-------|----------|
| **Key 1** | Start Debugging |
| **Key 2** | Set Breakpoint |
| **Key 3** | Step Over |
| **Encoder 1** | Step In/Out |
| **Encoder 2** | Continue/Stop |

---

## Desktop Integration

### Python Bridge (Reference Implementation)

```python
# macro_bridge.py
import asyncio
import json
from bleak import BleakClient, BleakScanner

NUS_SERVICE = "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
NUS_TX = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
NUS_RX = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"

class MacroPadBridge:
    def __init__(self):
        self.client = None
        self.current_layer = "default"
        self.layers = {
            "default": self.handle_default,
            "approval": self.handle_approval,
            "coding": self.handle_coding,
            "debug": self.handle_debug
        }

    async def connect(self):
        print("Scanning for Macro Pad...")
        devices = await BleakScanner.discover()

        for device in devices:
            if device.name and "Macro" in device.name:
                print(f"Found: {device.name}")
                self.client = BleakClient(device.address)
                await self.client.connect()
                await self.start_notifications()
                return True
        return False

    async def start_notifications(self):
        await self.client.start_notify(NUS_TX, self.on_notification)

    def on_notification(self, sender, data):
        try:
            msg = json.loads(data.decode())
            self.process_input(msg)
        except json.JSONDecodeError:
            print(f"Invalid JSON: {data}")

    def process_input(self, msg):
        handler = self.layers.get(self.current_layer, self.handle_default)
        handler(msg)

    # ========== LAYER HANDLERS ==========

    def handle_default(self, msg):
        msg_type = msg.get("type")

        if msg_type == "key_press":
            key = msg.get("key_id")
            actions = {
                1: lambda: self.send_to_claude("/chat Quick question..."),
                2: lambda: self.toggle_focus(),
                3: lambda: self.send_to_claude("/new")
            }
            action = actions.get(key)
            if action:
                action()

        elif msg_type == "encoder_turn":
            enc = msg.get("encoder_id")
            delta = msg.get("delta")

            if enc == 1:
                # Scroll transcript
                scroll_amount = delta * 3
                self.simulate_scroll(scroll_amount)
            elif enc == 2:
                # Adjust font
                self.adjust_font(delta)

        elif msg_type == "encoder_press":
            enc = msg.get("encoder_id")
            if enc == 1:
                self.toggle_scroll_mode()
            elif enc == 2:
                self.reset_font()

    def handle_approval(self, msg):
        """Auto-activated when permission prompt appears"""
        if msg.get("type") == "key_press":
            key = msg.get("key_id")
            if key == 1:
                self.send_approval(True)
                self.send_feedback({"command": "beep", "frequency": 1500, "duration": 100})
            elif key == 2:
                self.send_approval(False)
                self.send_feedback({"command": "beep", "frequency": 800, "duration": 100})

    def handle_coding(self, msg):
        """Manual toggle layer for coding workflows"""
        if msg.get("type") == "key_press":
            key = msg.get("key_id")
            commands = {
                1: "npm run build",
                2: "npm test",
                3: "/fix"
            }
            cmd = commands.get(key)
            if cmd:
                self.send_to_claude(cmd)

    # ========== ACTIONS ==========

    def send_to_claude(self, text):
        """Send text input to Claude Code"""
        # Integration with Claude Desktop's API
        print(f"Sending to Claude: {text}")

    def toggle_focus(self):
        """Toggle window focus"""
        print("Toggling focus")

    def simulate_scroll(self, amount):
        """Simulate mouse scroll"""
        print(f"Scrolling by {amount}")

    def adjust_font(self, delta):
        """Adjust UI font size"""
        print(f"Adjusting font by {delta}")

    def send_approval(self, approved):
        """Send approval/denial to Claude"""
        print(f"Approval: {approved}")

    def send_feedback(self, msg):
        """Send feedback to device"""
        if self.client and self.client.is_connected:
            asyncio.create_task(
                self.client.write_gatt_char(
                    NUS_RX,
                    json.dumps(msg).encode()
                )
            )

    async def set_layer(self, layer_name):
        """Switch active layer"""
        if layer_name in self.layers:
            self.current_layer = layer_name
            print(f"Switched to layer: {layer_name}")

# ========== MAIN ==========

async def main():
    bridge = MacroPadBridge()

    if await bridge.connect():
        print("Connected to Macro Pad!")

        # Keep running
        while True:
            await asyncio.sleep(1)
    else:
        print("Could not find Macro Pad")

if __name__ == "__main__":
    asyncio.run(main())
```

---

## Setup Instructions

### Prerequisites
1. **Claude Desktop** with Developer Mode enabled:
   - `Help → Troubleshooting → Enable Developer Mode`

2. **Hardware Requirements**:
   - ESP32 development board
   - 3x mechanical key switches
   - 2x rotary encoders with push buttons
   - Breadboard or PCB for assembly

### Arduino Libraries Required
```
- ArduinoJson by Benoit Blanchon
- ESP32 BLE Arduino by Neil Kolban
- Encoder by Paul Stoffregen
- Bounce2 by Thomas O Fredericks
```

### Pairing Process
1. Flash the firmware to your ESP32
2. Open Claude Desktop → Developer → Open Hardware Buddy
3. The Macro Pad should appear in the pairing list
4. Click "Pair" - the device will remember the pairing

### Testing
```python
# Simple test to verify connection
import asyncio
from bleak import BleakClient

async def test_connection(address):
    async with BleakClient(address) as client:
        services = await client.get_services()
        for service in services:
            print(f"Service: {service.uuid}")
```

---

## Extension Ideas

### LED Feedback Ring
Add an addressable LED ring around each encoder for visual feedback:
- Color indicates current layer
- Brightness shows scroll position
- Flash on approval requests

### OLED Display
Add a small 128x64 OLED to show:
- Current layer name
- Last action performed
- Connection status
- Pending approval preview

### Haptic Feedback
Replace speaker with vibration motors for silent operation.

### Battery Power
Add LiPo battery with charging circuit for wireless operation.

---

## References

- [claude-desktop-buddy](https://github.com/anthropics/claude-desktop-buddy) - Reference implementation
- [Nordic UART Service Spec](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.0.0%2Fble_sdk_app_nus_eval.html)
- [ESP32 BLE Arduino Docs](https://github.com/nkolban/ESP32_BLE_Arduino)
