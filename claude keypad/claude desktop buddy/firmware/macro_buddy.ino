#include "config.h"
#include <ArduinoJson.h>
#include <Wire.h>
#include <Encoder.h>
#include <Bounce2.h>
#include <U8g2lib.h>

// BLE includes
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLECharacteristic.h>

// ============== GLOBAL OBJECTS ==============

U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, OLED_SCL, OLED_SDA);

Encoder encoder1(ENC_1_A, ENC_1_B);
Encoder encoder2(ENC_2_A, ENC_2_B);

Bounce keys[3];
Bounce enc1Button;
Bounce enc2Button;

// BLE
BLEServer* bleServer = nullptr;
BLECharacteristic* txChar = nullptr;
BLECharacteristic* rxChar = nullptr;

bool deviceConnected = false;
bool oldDeviceConnected = false;

// State
ClaudeState currentState = STATE_SLEEP;
ClaudeState previousState = STATE_SLEEP;
unsigned long stateEntryTime = 0;
unsigned long busyTickTimer = 0;
bool approvalMode = false;

// Encoder positions
long enc1LastPos = 0;
long enc2LastPos = 0;
unsigned long enc1LastTime = 0;
unsigned long enc2LastTime = 0;

// Display
char layerName[16] = "DEFAULT";
char lastAction[32] = "Ready";
unsigned long lastDisplayUpdate = 0;

// ============== AUDIO ==============

void initAudio() {
    pinMode(SPEAKER_PIN, OUTPUT);
    digitalWrite(SPEAKER_PIN, LOW);
    ledcSetup(AUDIO_PWM_CHANNEL, AUDIO_PWM_FREQ, AUDIO_PWM_RES);
    ledcAttachPin(SPEAKER_PIN, AUDIO_PWM_CHANNEL);
}

void playTone(int freq, int duration) {
    if (freq > 0) {
        ledcWriteTone(AUDIO_PWM_CHANNEL, freq);
        ledcWrite(AUDIO_PWM_CHANNEL, 128);
    } else {
        ledcWrite(AUDIO_PWM_CHANNEL, 0);
    }
    if (duration > 0) {
        delay(duration);
        ledcWrite(AUDIO_PWM_CHANNEL, 0);
    }
}

void playPattern(const char* pattern) {
    char buf[128];
    strncpy(buf, pattern, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char* token = strtok(buf, ",");
    while (token != NULL) {
        int freq, duration;
        if (sscanf(token, "%d:%d", &freq, &duration) == 2) {
            if (freq > 0) {
                playTone(freq, duration);
            } else {
                delay(duration);
            }
        }
        token = strtok(NULL, ",");
    }
    playTone(0, 0);
}

// State sounds
void playStartup() {
    playPattern("523:150,0:50,659:150,0:50,784:300");
}

void playConnect() {
    playPattern("880:100,0:50,1760:200");
}

void playDisconnect() {
    playPattern("1760:100,0:50,880:200");
}

void playBusyTick() {
    playTone(400, 20);
}

void playAttentionAlert() {
    for (int i = 0; i < 3; i++) {
        playTone(1200, 150);
        delay(100);
        playTone(800, 150);
        delay(100);
    }
}

void playCelebrate() {
    int notes[] = {523, 659, 784, 1047, 784, 1047, 1319};
    int durations[] = {100, 100, 100, 200, 100, 100, 400};
    for (int i = 0; i < 7; i++) {
        playTone(notes[i], durations[i]);
        delay(30);
    }
}

void playError() {
    playTone(150, 300);
    delay(200);
    playTone(150, 500);
}

void playKeyPress(int keyId) {
    playTone(1000 + (keyId * 150), 50);
}

void playEncoderTick() {
    playTone(600, 8);
}

void playEncoderPress() {
    playTone(1400, 80);
}

void playApprovalSound() {
    playPattern("880:100,0:50,1100:150");
}

void playDenialSound() {
    playPattern("600:100,0:50,400:200");
}

// ============== DISPLAY ==============

void initDisplay() {
    display.begin();
    display.clearBuffer();
    display.setFont(u8g2_font_6x10_tf);
    display.setFontRefHeightExtendedText();
    display.setDrawColor(1);
    display.setFontPosTop();
}

void updateDisplay() {
    unsigned long now = millis();
    if (now - lastDisplayUpdate < DISPLAY_REFRESH_MS) return;
    lastDisplayUpdate = now;

    display.clearBuffer();

    // Top bar - Connection status
    if (currentState == STATE_SLEEP) {
        display.drawStr(0, 0, "[ SLEEP ]");
    } else {
        display.drawStr(0, 0, deviceConnected ? "[ ONLINE ]" : "[ OFFLINE ]");
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
    int strWidth = display.getStrWidth(stateText);
    display.drawStr(128 - strWidth, 0, stateText);

    // Separator line
    display.drawLine(0, 11, 128, 11);

    // Main content
    display.setFont(u8g2_font_7x13_tf);
    display.drawStr(0, 15, layerName);

    display.setFont(u8g2_font_6x10_tf);
    display.drawStr(0, 30, "Last:");
    display.drawStr(30, 30, lastAction);

    // Key hints
    display.drawStr(0, 50, "K1  K2  K3");

    display.sendBuffer();
}

void setLayer(const char* name) {
    strncpy(layerName, name, 15);
    layerName[15] = '\0';
}

void setLastAction(const char* action) {
    strncpy(lastAction, action, 31);
    lastAction[31] = '\0';
}

void setClaudeState(ClaudeState state) {
    if (state == currentState) return;

    currentState = state;
    stateEntryTime = millis();

    switch (state) {
        case STATE_SLEEP:
            playDisconnect();
            setLayer("SLEEP");
            break;
        case STATE_IDLE:
            playConnect();
            setLayer("DEFAULT");
            break;
        case STATE_BUSY:
            setLayer("WAITING");
            break;
        case STATE_ATTENTION:
            playAttentionAlert();
            setLayer("APPROVAL");
            break;
        case STATE_CELEBRATE:
            playCelebrate();
            setLayer("COMPLETE");
            break;
        case STATE_ERROR:
            playError();
            setLayer("ERROR");
            break;
    }
}

// ============== BLE ==============

void sendMessage(const char* type, JsonDocument& doc) {
    if (!deviceConnected || !txChar) return;

    doc["type"] = type;
    doc["timestamp"] = millis();

    char buffer[512];
    size_t len = serializeJson(doc, buffer);

    txChar->setValue((uint8_t*)buffer, len);
    txChar->notify();

    Serial.printf("TX: %s\n", buffer);
}

void onBLECommand(JsonDocument& doc);

class BLEServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("BLE: Client connected");
        playConnect();
    }

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("BLE: Client disconnected");
        playDisconnect();
        BLEDevice::startAdvertising();
    }
};

class BLECallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            Serial.printf("RX: %s\n", value.c_str());

            StaticJsonDocument<512> doc;
            DeserializationError err = deserializeJson(doc, value);

            if (!err) {
                onBLECommand(doc);
            }
        }
    }
};

void initBLE() {
    BLEDevice::init(DEVICE_NAME);

    bleServer = BLEDevice::createServer();
    bleServer->setCallbacks(new BLEServerCallbacks());

    BLEService* service = bleServer->createService(NUS_SERVICE_UUID);

    txChar = service->createCharacteristic(
        NUS_TX_UUID, BLECharacteristic::PROPERTY_NOTIFY);
    txChar->addDescriptor(new BLE2902());

    rxChar = service->createCharacteristic(
        NUS_RX_UUID, BLECharacteristic::PROPERTY_WRITE);
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

void onBLECommand(JsonDocument& doc) {
    const char* cmd = doc["command"];
    if (!cmd) return;

    if (strcmp(cmd, "set_state") == 0) {
        const char* state = doc["state"];
        if (strcmp(state, "idle") == 0) setClaudeState(STATE_IDLE);
        else if (strcmp(state, "busy") == 0) setClaudeState(STATE_BUSY);
        else if (strcmp(state, "attention") == 0) setClaudeState(STATE_ATTENTION);
        else if (strcmp(state, "celebrate") == 0) setClaudeState(STATE_CELEBRATE);
        else if (strcmp(state, "error") == 0) setClaudeState(STATE_ERROR);
        else if (strcmp(state, "sleep") == 0) setClaudeState(STATE_SLEEP);
    }
    else if (strcmp(cmd, "play_sound") == 0) {
        int freq = doc["frequency"] | 1000;
        int dur = doc["duration"] | 100;
        playTone(freq, dur);
    }
    else if (strcmp(cmd, "play_pattern") == 0) {
        const char* pattern = doc["pattern"];
        if (pattern) playPattern(pattern);
    }
    else if (strcmp(cmd, "set_layer") == 0) {
        const char* layer = doc["layer"];
        if (layer) setLayer(layer);
    }
}

// ============== INPUTS ==============

void initInputs() {
    keys[0].attach(KEY_1_PIN, INPUT_PULLUP);
    keys[1].attach(KEY_2_PIN, INPUT_PULLUP);
    keys[2].attach(KEY_3_PIN, INPUT_PULLUP);
    for (int i = 0; i < 3; i++) {
        keys[i].interval(DEBOUNCE_MS);
    }

    enc1Button.attach(ENC_1_BTN, INPUT_PULLUP);
    enc1Button.interval(DEBOUNCE_MS);
    enc2Button.attach(ENC_2_BTN, INPUT_PULLUP);
    enc2Button.interval(DEBOUNCE_MS);

    enc1LastPos = encoder1.read();
    enc2LastPos = encoder2.read();
}

void handleInputs() {
    // Keys
    for (int i = 0; i < 3; i++) {
        keys[i].update();
        if (keys[i].fell()) {
            playKeyPress(i + 1);

            StaticJsonDocument<256> doc;
            doc["input_type"] = "key";
            doc["id"] = i + 1;
            doc["value"] = 1;
            sendMessage("input", doc);

            char action[32];
            snprintf(action, sizeof(action), "Key%d Press", i + 1);
            setLastAction(action);
        }
        if (keys[i].rose()) {
            StaticJsonDocument<256> doc;
            doc["input_type"] = "key";
            doc["id"] = i + 1;
            doc["value"] = 0;
            sendMessage("input", doc);
        }
    }

    // Encoder 1 rotation
    long pos1 = encoder1.read();
    if (pos1 != enc1LastPos) {
        unsigned long now = millis();
        if (now - enc1LastTime > ENCODER_COOLDOWN) {
            int delta = pos1 - enc1LastPos;
            playEncoderTick();

            StaticJsonDocument<256> doc;
            doc["encoder_id"] = 1;
            doc["delta"] = delta;
            doc["absolute"] = pos1;
            sendMessage("encoder_turn", doc);

            enc1LastTime = now;
        }
        enc1LastPos = pos1;
    }

    // Encoder 2 rotation
    long pos2 = encoder2.read();
    if (pos2 != enc2LastPos) {
        unsigned long now = millis();
        if (now - enc2LastTime > ENCODER_COOLDOWN) {
            int delta = pos2 - enc2LastPos;
            playEncoderTick();

            StaticJsonDocument<256> doc;
            doc["encoder_id"] = 2;
            doc["delta"] = delta;
            doc["absolute"] = pos2;
            sendMessage("encoder_turn", doc);

            enc2LastTime = now;
        }
        enc2LastPos = pos2;
    }

    // Encoder buttons
    enc1Button.update();
    if (enc1Button.fell()) {
        playEncoderPress();

        StaticJsonDocument<256> doc;
        doc["input_type"] = "encoder_btn";
        doc["id"] = 1;
        doc["value"] = 1;
        sendMessage("input", doc);

        setLastAction("Enc1 Button");
    }

    enc2Button.update();
    if (enc2Button.fell()) {
        playEncoderPress();

        StaticJsonDocument<256> doc;
        doc["input_type"] = "encoder_btn";
        doc["id"] = 2;
        doc["value"] = 1;
        sendMessage("input", doc);

        setLastAction("Enc2 Button");
    }
}

// ============== STATE MACHINE ==============

void handleStateMachine() {
    // Busy ticking
    if (currentState == STATE_BUSY) {
        unsigned long now = millis();
        if (now - busyTickTimer > 500) {
            playBusyTick();
            busyTickTimer = now;
        }
    }
}

// ============== SETUP & LOOP ==============

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("╔══════════════════════════════════╗");
    Serial.println("║   Claude Desktop Buddy + Macro   ║");
    Serial.println("║           Firmware v1.0            ║");
    Serial.println("╚══════════════════════════════════╝");

    initAudio();
    initDisplay();
    initInputs();
    initBLE();

    playStartup();
    setLastAction("Ready");

    Serial.println("Setup complete.");
    Serial.println("Waiting for BLE connection...");
}

void loop() {
    handleInputs();
    handleStateMachine();
    updateDisplay();
    delay(1);
}
