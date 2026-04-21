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
