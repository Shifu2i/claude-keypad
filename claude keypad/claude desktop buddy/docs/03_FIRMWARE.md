# Firmware Implementation

## Overview

This firmware runs on ESP32 and implements:
1. BLE Nordic UART Service for communication with Claude Desktop
2. Input handling for keys, encoders, and encoder buttons
3. Audio generation for feedback and state notifications
4. OLED display for status visualization
5. State machine that reacts to Claude's activity

## File Structure

```
firmware/
├── macro_buddy.ino          (main entry point)
├── ble_manager.h/.cpp       (BLE setup and communication)
├── input_manager.h/.cpp     (Keys and encoder handling)
├── audio_manager.h/.cpp     (Speaker control)
├── display_manager.h/.cpp   (OLED display)
├── state_machine.h/.cpp     (Buddy state logic)
└── config.h                 (Pin definitions and settings)
```

---

## Complete Firmware Code

### config.h

```cpp
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============== HARDWARE PINS ==============

// OLED (I2C)
#define OLED_SDA        21
#define OLED_SCL        22
#define OLED_ADDRESS    0x3C  // Try 0x3D if display doesn't work

// Mechanical Keys
#define KEY_1_PIN       4
#define KEY_2_PIN       5
#define KEY_3_PIN       6

// Rotary Encoder 1
#define ENC_1_A         7     // CLK
#define ENC_1_B         8     // DT
#define ENC_1_BTN       9     // Push button

// Rotary Encoder 2
#define ENC_2_A         10    // CLK
#define ENC_2_B         11    // DT
#define ENC_2_BTN       12    // Push button

// Speaker (PWM)
#define SPEAKER_PIN     13

// ============== AUDIO CONFIG ==============

#define AUDIO_PWM_CHANNEL   0
#define AUDIO_PWM_FREQ      2000
#define AUDIO_PWM_RES       8   // 8-bit resolution (0-255)

// ============== TIMING ==============

#define DEBOUNCE_MS         10
#define ENCODER_COOLDOWN    50   // ms between encoder events
#define IDLE_TIMEOUT_MS     30000 // 30 seconds to idle state
#define DISPLAY_REFRESH_MS  100   // Screen update rate

// ============== BLE ==============

#define DEVICE_NAME         "ClaudeBuddy"
#define BLE_MTU_SIZE        512

// Nordic UART Service UUIDs
#define NUS_SERVICE_UUID    "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define NUS_TX_UUID         "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
#define NUS_RX_UUID         "6e400002-b5a3-f393-e0a9-e50e24dcca9e"

// ============== STATES ==============

enum ClaudeState {
    STATE_SLEEP,       // Not connected to desktop
    STATE_IDLE,        // Connected, no activity
    STATE_BUSY,        // Claude processing
    STATE_ATTENTION,   // Approval needed
    STATE_CELEBRATE,   // Task complete
    STATE_ERROR        // Error occurred
};

#endif
```

---

### audio_manager.h / .cpp

```cpp
// audio_manager.h
#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include "config.h"

class AudioManager {
public:
    void init();
    void playTone(int frequency, int durationMs);
    void playPattern(const char* pattern);
    void stop();
    
    // State sounds
    void playStartup();
    void playConnect();
    void playDisconnect();
    void playIdleHum();
    void playBusyTick();
    void playAttentionAlert();
    void playCelebrate();
    void playError();
    
    // Feedback sounds
    void playKeyPress(int keyId);
    void playEncoderTick();
    void playEncoderPress();
    void playLayerChange();
    void playApproval();
    void playDenial();
    
private:
    void tone(int freq, int duration);
    void noTone();
    bool isPlaying;
};

extern AudioManager audio;

#endif
```

```cpp
// audio_manager.cpp
#include "audio_manager.h"

AudioManager audio;

void AudioManager::init() {
    pinMode(SPEAKER_PIN, OUTPUT);
    digitalWrite(SPEAKER_PIN, LOW);
    isPlaying = false;
}

void AudioManager::tone(int freq, int duration) {
    if (freq > 0) {
        ledcWriteTone(AUDIO_PWM_CHANNEL, freq);
        ledcWrite(AUDIO_PWM_CHANNEL, 128);  // 50% duty cycle
    } else {
        noTone();
    }
}

void AudioManager::noTone() {
    ledcWrite(AUDIO_PWM_CHANNEL, 0);
}

void AudioManager::playTone(int frequency, int durationMs) {
    tone(frequency, durationMs);
    if (durationMs > 0) {
        delay(durationMs);
        noTone();
    }
}

void AudioManager::playPattern(const char* pattern) {
    // Pattern format: "freq:duration,freq:duration,..."
    // Example: "880:100,0:50,1760:200" (beep, pause, beep)
    // freq=0 means silence
    
    char buffer[128];
    strncpy(buffer, pattern, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    char* token = strtok(buffer, ",");
    while (token != NULL) {
        int freq, duration;
        if (sscanf(token, "%d:%d", &freq, &duration) == 2) {
            if (freq > 0) {
                tone(freq, duration);
                delay(duration);
            } else {
                noTone();
                delay(duration);
            }
        }
        token = strtok(NULL, ",");
    }
    noTone();
}

void AudioManager::stop() {
    noTone();
    isPlaying = false;
}

// ========== STATE SOUNDS ==========

void AudioManager::playStartup() {
    // Ascending major triad
    playTone(523, 150);  // C5
    delay(50);
    playTone(659, 150);  // E5
    delay(50);
    playTone(784, 300);  // G5
}

void AudioManager::playConnect() {
    // Success chime
    playTone(880, 100);
    delay(50);
    playTone(1760, 200);
}

void AudioManager::playDisconnect() {
    // Descending tone
    playTone(1760, 100);
    delay(50);
    playTone(880, 200);
}

void AudioManager::playIdleHum() {
    // Very quiet ambient - just a brief soft tone
    playTone(200, 50);
}

void AudioManager::playBusyTick() {
    // Subtle ticking
    playTone(400, 20);
}

void AudioManager::playAttentionAlert() {
    // Urgent repeating pattern
    for (int i = 0; i < 3; i++) {
        playTone(1200, 150);
        delay(100);
        playTone(800, 150);
        delay(100);
    }
}

void AudioManager::playCelebrate() {
    // Victory arpeggio
    int notes[] = {523, 659, 784, 1047, 784, 1047, 1319};
    int durations[] = {100, 100, 100, 200, 100, 100, 400};
    
    for (int i = 0; i < 7; i++) {
        playTone(notes[i], durations[i]);
        delay(30);
    }
}

void AudioManager::playError() {
    // Low error buzz
    playTone(150, 300);
    delay(200);
    playTone(150, 500);
}

// ========== FEEDBACK SOUNDS ==========

void AudioManager::playKeyPress(int keyId) {
    // Different pitch for each key
    int freq = 1000 + (keyId * 150);
    playTone(freq, 50);
}

void AudioManager::playEncoderTick() {
    // Very short tick
    playTone(600, 8);
}

void AudioManager::playEncoderPress() {
    playTone(1400, 80);
}

void AudioManager::playLayerChange() {
    playTone(1000, 50);
    delay(30);
    playTone(1500, 100);
}

void AudioManager::playApproval() {
    // Happy ascending
    playTone(880, 100);
    delay(50);
    playTone(1100, 150);
}

void AudioManager::playDenial() {
    // Descending
    playTone(600, 100);
    delay(50);
    playTone(400, 200);
}
```

---

### ble_manager.h / .cpp

```cpp
// ble_manager.h
#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include "config.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLECharacteristic.h>
#include <ArduinoJson.h>

class BLEManager {
public:
    void init();
    void handle();
    bool isConnected();
    void sendEvent(const char* type, JsonDocument& payload);
    void sendStateChange(const char* state);
    void sendInputEvent(const char* inputType, int id, int value = 0);
    
    typedef void (*MessageHandler)(JsonDocument& doc);
    void onMessage(MessageHandler handler);
    
private:
    BLEServer* server;
    BLECharacteristic* txChar;
    BLECharacteristic* rxChar;
    bool connected;
    bool oldConnected;
    MessageHandler msgHandler;
    
    friend class BLEServerCallbacks;
    friend class BLECallbacks;
};

extern BLEManager ble;

#endif
```

```cpp
// ble_manager.cpp
#include "ble_manager.h"
#include "audio_manager.h"

BLEManager ble;

class BLEServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        ble.connected = true;
        Serial.println("BLE: Client connected");
        audio.playConnect();
    }
    
    void onDisconnect(BLEServer* pServer) {
        ble.connected = false;
        Serial.println("BLE: Client disconnected");
        audio.playDisconnect();
        BLEDevice::startAdvertising();
    }
};

class BLECallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0 && ble.msgHandler) {
            Serial.printf("BLE RX: %s\n", value.c_str());
            
            StaticJsonDocument<512> doc;
            DeserializationError err = deserializeJson(doc, value);
            
            if (!err) {
                ble.msgHandler(doc);
            } else {
                Serial.printf("JSON parse error: %s\n", err.c_str());
            }
        }
    }
};

void BLEManager::init() {
    connected = false;
    oldConnected = false;
    msgHandler = nullptr;
    
    BLEDevice::init(DEVICE_NAME);
    
    server = BLEDevice::createServer();
    server->setCallbacks(new BLEServerCallbacks());
    
    BLEService* service = server->createService(NUS_SERVICE_UUID);
    
    // TX - Device to Desktop (Notify)
    txChar = service->createCharacteristic(
        NUS_TX_UUID,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    txChar->addDescriptor(new BLE2902());
    
    // RX - Desktop to Device (Write)
    rxChar = service->createCharacteristic(
        NUS_RX_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    rxChar->setCallbacks(new BLECallbacks());
    
    service->start();
    
    BLEAdvertising* advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(NUS_SERVICE_UUID);
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x06);
    advertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    Serial.println("BLE: Advertising started");
}

void BLEManager::handle() {
    // Reconnection logic handled in disconnect callback
}

bool BLEManager::isConnected() {
    return connected;
}

void BLEManager::sendEvent(const char* type, JsonDocument& payload) {
    if (!connected || !txChar) return;
    
    payload["type"] = type;
    payload["timestamp"] = millis();
    
    char buffer[512];
    size_t len = serializeJson(payload, buffer);
    
    txChar->setValue((uint8_t*)buffer, len);
    txChar->notify();
    
    Serial.printf("BLE TX: %s\n", buffer);
}

void BLEManager::sendStateChange(const char* state) {
    StaticJsonDocument<256> doc;
    doc["state"] = state;
    sendEvent("state_change", doc);
}

void BLEManager::sendInputEvent(const char* inputType, int id, int value) {
    StaticJsonDocument<256> doc;
    doc["input_type"] = inputType;
    doc["id"] = id;
    if (value != 0) doc["value"] = value;
    sendEvent("input", doc);
}

void BLEManager::onMessage(MessageHandler handler) {
    msgHandler = handler;
}
```

---

### input_manager.h / .cpp

```cpp
// input_manager.h
#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "config.h"
#include <Bounce2.h>
#include <Encoder.h>

typedef void (*InputCallback)(int id, int value);

class InputManager {
public:
    void init();
    void update();
    
    void setKeyCallback(InputCallback cb);
    void setEncoderCallback(InputCallback cb);
    void setEncoderButtonCallback(InputCallback cb);
    
    int getEncoderPosition(int encoderId);
    void resetEncoder(int encoderId);
    
private:
    Bounce keys[3];
    Bounce enc1Button;
    Bounce enc2Button;
    
    Encoder* enc1;
    Encoder* enc2;
    
    long enc1LastPos;
    long enc2LastPos;
    unsigned long enc1LastTime;
    unsigned long enc2LastTime;
    
    InputCallback keyCallback;
    InputCallback encoderCallback;
    InputCallback encoderButtonCallback;
};

extern InputManager inputs;

#endif
```

```cpp
// input_manager.cpp
#include "input_manager.h"
#include "audio_manager.h"

InputManager inputs;

void InputManager::init() {
    // Initialize keys
    keys[0].attach(KEY_1_PIN, INPUT_PULLUP);
    keys[1].attach(KEY_2_PIN, INPUT_PULLUP);
    keys[2].attach(KEY_3_PIN, INPUT_PULLUP);
    
    for (int i = 0; i < 3; i++) {
        keys[i].interval(DEBOUNCE_MS);
    }
    
    // Initialize encoder buttons
    enc1Button.attach(ENC_1_BTN, INPUT_PULLUP);
    enc1Button.interval(DEBOUNCE_MS);
    enc2Button.attach(ENC_2_BTN, INPUT_PULLUP);
    enc2Button.interval(DEBOUNCE_MS);
    
    // Initialize encoders
    enc1 = new Encoder(ENC_1_A, ENC_1_B);
    enc2 = new Encoder(ENC_2_A, ENC_2_B);
    
    enc1LastPos = enc1->read();
    enc2LastPos = enc2->read();
    enc1LastTime = 0;
    enc2LastTime = 0;
    
    keyCallback = nullptr;
    encoderCallback = nullptr;
    encoderButtonCallback = nullptr;
}

void InputManager::update() {
    // Update keys
    for (int i = 0; i < 3; i++) {
        keys[i].update();
        
        if (keys[i].fell()) {
            audio.playKeyPress(i + 1);
            if (keyCallback) keyCallback(i + 1, 1);  // pressed
        }
        if (keys[i].rose()) {
            if (keyCallback) keyCallback(i + 1, 0);  // released
        }
    }
    
    // Update encoder 1
    long pos1 = enc1->read();
    if (pos1 != enc1LastPos) {
        unsigned long now = millis();
        if (now - enc1LastTime > ENCODER_COOLDOWN) {
            int delta = pos1 - enc1LastPos;
            audio.playEncoderTick();
            if (encoderCallback) encoderCallback(1, delta);
            enc1LastTime = now;
        }
        enc1LastPos = pos1;
    }
    
    // Update encoder 2
    long pos2 = enc2->read();
    if (pos2 != enc2LastPos) {
        unsigned long now = millis();
        if (now - enc2LastTime > ENCODER_COOLDOWN) {
            int delta = pos2 - enc2LastPos;
            audio.playEncoderTick();
            if (encoderCallback) encoderCallback(2, delta);
            enc2LastTime = now;
        }
        enc2LastPos = pos2;
    }
    
    // Update encoder buttons
    enc1Button.update();
    if (enc1Button.fell()) {
        audio.playEncoderPress();
        if (encoderButtonCallback) encoderButtonCallback(1, 1);
    }
    
    enc2Button.update();
    if (enc2Button.fell()) {
        audio.playEncoderPress();
        if (encoderButtonCallback) encoderButtonCallback(2, 1);
    }
}

void InputManager::setKeyCallback(InputCallback cb) {
    keyCallback = cb;
}

void InputManager::setEncoderCallback(InputCallback cb) {
    encoderCallback = cb;
}

void InputManager::setEncoderButtonCallback(InputCallback cb) {
    encoderButtonCallback = cb;
}

int InputManager::getEncoderPosition(int encoderId) {
    if (encoderId == 1) return enc1->read();
    if (encoderId == 2) return enc2->read();
    return 0;
}

void InputManager::resetEncoder(int encoderId) {
    if (encoderId == 1) {
        enc1->write(0);
        enc1LastPos = 0;
    } else if (encoderId == 2) {
        enc2->write(0);
        enc2LastPos = 0;
    }
}
```

---

### display_manager.h / .cpp

```cpp
// display_manager.h
#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "config.h"
#include <Wire.h>
#include <U8g2lib.h>

enum DisplayMode {
    MODE_NORMAL,
    MODE_APPROVAL,
    MODE_CODING,
    MODE_DEBUG
};

class DisplayManager {
public:
    void init();
    void update();
    
    void setClaudeState(ClaudeState state);
    void setLayer(const char* layerName);
    void setLastAction(const char* action);
    void showApprovalPrompt(const char* prompt);
    void clearApproval();
    
private:
    U8G2_SH1106_128X64_NONAME_F_HW_I2C* u8g2;
    ClaudeState currentState;
    char layerName[16];
    char lastAction[32];
    char approvalPrompt[64];
    bool showApproval;
    unsigned long lastUpdate;
};

extern DisplayManager display;

#endif
```

```cpp
// display_manager.cpp
#include "display_manager.h"

DisplayManager display;

void DisplayManager::init() {
    u8g2 = new U8G2_SH1106_128X64_NONAME_F_HW_I2C(U8G2_R0, OLED_SCL, OLED_SDA);
    u8g2->begin();
    u8g2->setFont(u8g2_font_6x10_tf);
    u8g2->setFontRefHeightExtendedText();
    u8g2->setDrawColor(1);
    u8g2->setFontPosTop();
    
    currentState = STATE_SLEEP;
    strcpy(layerName, "DEFAULT");
    strcpy(lastAction, "Ready");
    approvalPrompt[0] = '\0';
    showApproval = false;
    lastUpdate = 0;
}

void DisplayManager::update() {
    unsigned long now = millis();
    if (now - lastUpdate < DISPLAY_REFRESH_MS) return;
    lastUpdate = now;
    
    u8g2->clearBuffer();
    
    // Top bar - Connection status
    if (currentState == STATE_SLEEP) {
        u8g2->drawStr(0, 0, "[ SLEEP ]");
    } else {
        u8g2->drawStr(0, 0, ble.isConnected() ? "[ ONLINE ]" : "[ OFFLINE ]");
    }
    
    // State indicator
    const char* stateText = "SLEEP";
    switch (currentState) {
        case STATE_IDLE: stateText = "IDLE"; break;
        case STATE_BUSY: stateText = "BUSY"; break;
        case STATE_ATTENTION: stateText = "NEEDS YOU"; break;
        case STATE_CELEBRATE: stateText = "DONE!"; break;
        case STATE_ERROR: stateText = "ERROR"; break;
        default: break;
    }
    u8g2->drawStr(90, 0, stateText);
    
    // Separator line
    u8g2->drawLine(0, 11, 128, 11);
    
    if (showApproval) {
        // Approval mode display
        u8g2->setFont(u8g2_font_7x13B_tf);
        u8g2->drawStr(0, 15, "APPROVAL NEEDED");
        u8g2->setFont(u8g2_font_6x10_tf);
        
        // Truncate and wrap prompt
        char buf[64];
        strncpy(buf, approvalPrompt, 63);
        buf[63] = '\0';
        
        // Simple truncation for display
        if (strlen(buf) > 21) {
            buf[21] = '\0';
            strcat(buf, "...");
        }
        u8g2->drawStr(0, 30, buf);
        
        // Key hints
        u8g2->drawStr(0, 50, "[K1]Approve [K2]Deny");
    } else {
        // Normal mode
        u8g2->setFont(u8g2_font_7x13_tf);
        u8g2->drawStr(0, 15, layerName);
        
        u8g2->setFont(u8g2_font_6x10_tf);
        u8g2->drawStr(0, 30, "Last:");
        u8g2->drawStr(30, 30, lastAction);
        
        // Key hints based on layer
        u8g2->drawStr(0, 50, "K1  K2  K3");
    }
    
    u8g2->sendBuffer();
}

void DisplayManager::setClaudeState(ClaudeState state) {
    currentState = state;
}

void DisplayManager::setLayer(const char* name) {
    strncpy(layerName, name, 15);
    layerName[15] = '\0';
}

void DisplayManager::setLastAction(const char* action) {
    strncpy(lastAction, action, 31);
    lastAction[31] = '\0';
}

void DisplayManager::showApprovalPrompt(const char* prompt) {
    strncpy(approvalPrompt, prompt, 63);
    approvalPrompt[63] = '\0';
    showApproval = true;
}

void DisplayManager::clearApproval() {
    showApproval = false;
    approvalPrompt[0] = '\0';
}
```

---

### state_machine.h / .cpp

```cpp
// state_machine.h
#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "config.h"

class StateMachine {
public:
    void init();
    void update();
    
    void setClaudeState(ClaudeState newState);
    ClaudeState getClaudeState();
    
    void onApprovalStart(const char* prompt);
    void onApprovalEnd(bool approved);
    
    void handleInput(int inputId, int value);
    
private:
    ClaudeState currentState;
    ClaudeState previousState;
    unsigned long stateEntryTime;
    unsigned long busyTickTimer;
    
    bool approvalMode;
    char pendingApproval[256];
    
    void enterState(ClaudeState state);
    void exitState(ClaudeState state);
    void handleNormalInput(int inputId, int value);
    void handleApprovalInput(int inputId, int value);
};

extern StateMachine stateMachine;

#endif
```

```cpp
// state_machine.cpp
#include "state_machine.h"
#include "audio_manager.h"
#include "display_manager.h"
#include "ble_manager.h"

StateMachine stateMachine;

void StateMachine::init() {
    currentState = STATE_SLEEP;
    previousState = STATE_SLEEP;
    stateEntryTime = millis();
    busyTickTimer = 0;
    approvalMode = false;
    pendingApproval[0] = '\0';
}

void StateMachine::update() {
    // Periodic busy tick sound
    if (currentState == STATE_BUSY) {
        unsigned long now = millis();
        if (now - busyTickTimer > 500) {  // Tick every 500ms
            audio.playBusyTick();
            busyTickTimer = now;
        }
    }
    
    display.update();
}

void StateMachine::setClaudeState(ClaudeState newState) {
    if (newState == currentState) return;
    
    exitState(currentState);
    previousState = currentState;
    currentState = newState;
    stateEntryTime = millis();
    
    enterState(newState);
    display.setClaudeState(newState);
}

ClaudeState StateMachine::getClaudeState() {
    return currentState;
}

void StateMachine::enterState(ClaudeState state) {
    switch (state) {
        case STATE_SLEEP:
            audio.playDisconnect();
            display.setLayer("SLEEP");
            break;
            
        case STATE_IDLE:
            audio.playConnect();
            display.setLayer("DEFAULT");
            break;
            
        case STATE_BUSY:
            audio.playBusyTick();
            display.setLayer("WAITING");
            break;
            
        case STATE_ATTENTION:
            audio.playAttentionAlert();
            display.setLayer("APPROVAL");
            break;
            
        case STATE_CELEBRATE:
            audio.playCelebrate();
            display.setLayer("COMPLETE");
            break;
            
        case STATE_ERROR:
            audio.playError();
            display.setLayer("ERROR");
            break;
    }
}

void StateMachine::exitState(ClaudeState state) {
    // Cleanup when leaving state
    if (state == STATE_ATTENTION) {
        approvalMode = false;
        display.clearApproval();
    }
}

void StateMachine::onApprovalStart(const char* prompt) {
    strncpy(pendingApproval, prompt, 255);
    pendingApproval[255] = '\0';
    approvalMode = true;
    display.showApprovalPrompt(prompt);
    setClaudeState(STATE_ATTENTION);
}

void StateMachine::onApprovalEnd(bool approved) {
    approvalMode = false;
    pendingApproval[0] = '\0';
    display.clearApproval();
    
    // Send approval response
    StaticJsonDocument<256> doc;
    doc["approved"] = approved;
    doc["prompt"] = pendingApproval;
    ble.sendEvent("approval_response", doc);
    
    if (approved) {
        audio.playApproval();
    } else {
        audio.playDenial();
    }
}

void StateMachine::handleInput(int inputId, int value) {
    if (approvalMode) {
        handleApprovalInput(inputId, value);
    } else {
        handleNormalInput(inputId, value);
    }
}

void StateMachine::handleNormalInput(int inputId, int value) {
    // Map inputs and send to desktop
    // inputId: 1-3 = keys, 11-12 = enc1-2 buttons
    // value: 1 = pressed, 0 = released (for keys)
    // or delta value (for encoders)
    
    if (value == 0) return;  // Only handle press, not release
    
    const char* action = nullptr;
    
    switch (inputId) {
        case 1: action = "key1_press"; break;
        case 2: action = "key2_press"; break;
        case 3: action = "key3_press"; break;
        case 11: action = "enc1_press"; break;
        case 12: action = "enc2_press"; break;
        default: action = "unknown"; break;
    }
    
    if (action) {
        StaticJsonDocument<256> doc;
        doc["action"] = action;
        ble.sendEvent("input_action", doc);
        display.setLastAction(action);
    }
}

void StateMachine::handleApprovalInput(int inputId, int value) {
    if (value == 0) return;
    
    if (inputId == 1) {
        // Key 1 = Approve
        onApprovalEnd(true);
    } else if (inputId == 2) {
        // Key 2 = Deny
        onApprovalEnd(false);
    } else if (inputId == 3) {
        // Key 3 = Show details (could cycle through text)
        audio.playTone(1000, 50);
    }
}
```

---

### macro_buddy.ino (Main)

```cpp
// macro_buddy.ino
#include "config.h"
#include "audio_manager.h"
#include "input_manager.h"
#include "ble_manager.h"
#include "display_manager.h"
#include "state_machine.h"

// Forward declarations
void onBLEMessage(JsonDocument& doc);
void onKeyInput(int keyId, int value);
void onEncoderTurn(int encId, int delta);
void onEncoderButton(int encId, int value);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("╔══════════════════════════════════╗");
    Serial.println("║   Claude Desktop Buddy + Macro   ║");
    Serial.println("║           Firmware v1.0            ║");
    Serial.println("╚══════════════════════════════════╝");
    
    // Initialize subsystems
    audio.init();
    display.init();
    inputs.init();
    ble.init();
    stateMachine.init();
    
    // Register callbacks
    ble.onMessage(onBLEMessage);
    inputs.setKeyCallback(onKeyInput);
    inputs.setEncoderCallback(onEncoderTurn);
    inputs.setEncoderButtonCallback(onEncoderButton);
    
    // Startup sequence
    audio.playStartup();
    display.setLayer("STARTING");
    display.setLastAction("System Ready");
    
    Serial.println("Setup complete. Waiting for BLE connection...");
}

void loop() {
    // Handle BLE
    ble.handle();
    
    // Handle inputs
    inputs.update();
    
    // Handle state machine
    stateMachine.update();
    
    // Small delay to prevent tight loop
    delay(1);
}

// ========== CALLBACKS ==========

void onBLEMessage(JsonDocument& doc) {
    const char* cmd = doc["command"];
    
    if (strcmp(cmd, "set_state") == 0) {
        const char* state = doc["state"];
        ClaudeState newState = STATE_SLEEP;
        
        if (strcmp(state, "idle") == 0) newState = STATE_IDLE;
        else if (strcmp(state, "busy") == 0) newState = STATE_BUSY;
        else if (strcmp(state, "attention") == 0) newState = STATE_ATTENTION;
        else if (strcmp(state, "celebrate") == 0) newState = STATE_CELEBRATE;
        else if (strcmp(state, "error") == 0) newState = STATE_ERROR;
        else if (strcmp(state, "sleep") == 0) newState = STATE_SLEEP;
        
        stateMachine.setClaudeState(newState);
        
        // If attention, check for prompt
        if (newState == STATE_ATTENTION && doc["prompt"]) {
            stateMachine.onApprovalStart(doc["prompt"]);
        }
    }
    else if (strcmp(cmd, "play_sound") == 0) {
        int freq = doc["frequency"] | 1000;
        int dur = doc["duration"] | 100;
        audio.playTone(freq, dur);
    }
    else if (strcmp(cmd, "play_pattern") == 0) {
        const char* pattern = doc["pattern"];
        if (pattern) audio.playPattern(pattern);
    }
    else if (strcmp(cmd, "set_layer") == 0) {
        const char* layer = doc["layer"];
        if (layer) display.setLayer(layer);
    }
}

void onKeyInput(int keyId, int value) {
    // value: 1 = pressed, 0 = released
    stateMachine.handleInput(keyId, value);
    
    // Also send raw event
    ble.sendInputEvent("key", keyId, value);
}

void onEncoderTurn(int encId, int delta) {
    // Map to input IDs: Encoder 1 = 21, Encoder 2 = 22
    int inputId = 20 + encId;
    stateMachine.handleInput(inputId, delta);
    
    // Send raw event
    StaticJsonDocument<256> doc;
    doc["delta"] = delta;
    doc["encoder_id"] = encId;
    ble.sendEvent("encoder_turn", doc);
}

void onEncoderButton(int encId, int value) {
    // Map to input IDs: Encoder 1 btn = 11, Encoder 2 btn = 12
    int inputId = 10 + encId;
    stateMachine.handleInput(inputId, value);
    
    ble.sendInputEvent("encoder_btn", encId, value);
}
```

---

## Arduino Libraries Required

Install these via Arduino Library Manager:

| Library | Version | Purpose |
|---------|---------|---------|
| `ArduinoJson` | 6.x | JSON serialization |
| `ESP32 BLE Arduino` | Latest | BLE stack |
| `Encoder` | Latest | Rotary encoder handling |
| `Bounce2` | Latest | Button debouncing |
| `U8g2` | Latest | OLED display driver |

## Build Configuration

### Board Settings (Arduino IDE)

| Setting | Value |
|---------|-------|
| Board | "ESP32S3 Dev Module" or "ESP32 Dev Module" |
| Flash Mode | QIO |
| Flash Size | 4MB (32Mb) |
| Partition Scheme | Default 4MB with spiffs |
| Upload Speed | 921600 |
| CPU Frequency | 240MHz |

### PlatformIO (platformio.ini)

```ini
[env:esp32-s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200

lib_deps =
    bblanchon/ArduinoJson@^6.21.0
    https://github.com/nkolban/ESP32_BLE_Arduino.git
    paulstoffregen/Encoder@^1.4.2
    thomasfredericks/Bounce2@^2.71
    olikraus/U8g2@^2.34.22

build_flags =
    -DCORE_DEBUG_LEVEL=0
    -DCONFIG_BT_ENABLED=1
```

---

## Upload Instructions

1. **Connect ESP32 via USB**
2. **Select correct COM port** in Arduino IDE
3. **Hold BOOT button** while clicking Upload (if needed)
4. **Release BOOT** when "Connecting..." appears
5. **Monitor serial output** at 115200 baud

## Serial Debug Output

Expected startup output:
```
╔══════════════════════════════════╗
║   Claude Desktop Buddy + Macro   ║
║           Firmware v1.0            ║
╚══════════════════════════════════╝
BLE: Advertising started
Setup complete. Waiting for BLE connection...
```

On BLE connection:
```
BLE: Client connected
BLE TX: {"type":"state_change","timestamp":12345,"state":"idle"}
```
