# BLE Protocol Specification

## Overview

The Macro Buddy uses **Nordic UART Service (NUS)** - a standard BLE GATT profile that emulates serial communication over Bluetooth Low Energy.

### Service UUIDs

| Role | UUID | Description |
|------|------|-------------|
| Service | `6e400001-b5a3-f393-e0a9-e50e24dcca9e` | Nordic UART Service |
| TX | `6e400003-b5a3-f393-e0a9-e50e24dcca9e` | Device → Desktop (Notify) |
| RX | `6e400002-b5a3-f393-e0a9-e50e24dcca9e` | Desktop → Device (Write) |

**Nomenclature Note:** TX/RX are from the device's perspective.
- **TX Characteristic**: Device transmits, desktop receives (via notify)
- **RX Characteristic**: Device receives, desktop transmits (via write)

---

## Communication Flow

```
DEVICE                              DESKTOP
  │                                     │
  │  ┌─────────────────────────────┐  │
  │  │      ADVERTISEMENT          │  │
  │  │ (Nordic UART Service UUID)  │──▶│  Scan for devices
  │  └─────────────────────────────┘  │
  │                                     │
  │  ┌─────────────────────────────┐  │
  │  │         CONNECT             │  │
  │  │                             │──▶│  Connect to MacroBuddy
  │  └─────────────────────────────┘  │
  │                                     │
  │◀──────────────────────────────────│  Subscribe to TX notifications
  │                                     │
  │  ┌─────────────────────────────┐  │
  │  │  TX: state_change (idle)  │──▶│  Initial state
  │  └─────────────────────────────┘  │
  │                                     │
  │◀──────────────────────────────────│  RX: set_state (busy)
  │                                     │
  │  ┌─────────────────────────────┐  │
  │  │    TX: input (key_press)  │──▶│  User pressed key
  │  └─────────────────────────────┘  │
  │                                     │
```

---

## Device → Desktop (Input Events)

Sent by the device when user interacts with hardware.

### Key Press Event

```json
{
  "type": "input",
  "input_type": "key",
  "id": 1,
  "value": 1,
  "timestamp": 12345
}
```

| Field | Type | Description |
|-------|------|-------------|
| `type` | string | Always `"input"` |
| `input_type` | string | `"key"` for mechanical keys |
| `id` | integer | Key ID (1-3) |
| `value` | integer | 1 = pressed, 0 = released |
| `timestamp` | integer | Milliseconds since boot |

### Encoder Rotation Event

```json
{
  "type": "encoder_turn",
  "encoder_id": 1,
  "delta": 2,
  "absolute": 42,
  "timestamp": 12345
}
```

| Field | Type | Description |
|-------|------|-------------|
| `type` | string | Always `"encoder_turn"` |
| `encoder_id` | integer | Encoder ID (1-2) |
| `delta` | integer | Direction and amount (+/- ticks) |
| `absolute` | integer | Absolute position since boot |
| `timestamp` | integer | Milliseconds since boot |

### Encoder Button Event

```json
{
  "type": "input",
  "input_type": "encoder_btn",
  "id": 1,
  "value": 1,
  "timestamp": 12345
}
```

| Field | Type | Description |
|-------|------|-------------|
| `id` | integer | Encoder ID (1-2) |
| `value` | integer | 1 = pressed, 0 = released |

### State Change Event

```json
{
  "type": "state_change",
  "state": "idle",
  "timestamp": 12345
}
```

Sent when device transitions between internal states (sleep, idle, busy, etc.).

### Approval Response Event

```json
{
  "type": "approval_response",
  "approved": true,
  "prompt": "Claude wants to edit file main.py",
  "timestamp": 12345
}
```

Sent when user approves or denies a permission prompt.

### Input Action Event

```json
{
  "type": "input_action",
  "action": "key1_press",
  "timestamp": 12345
}
```

Higher-level event that includes layer context. Actions:
- `key1_press`, `key2_press`, `key3_press`
- `enc1_press`, `enc2_press`
- `enc1_cw`, `enc1_ccw` (clockwise/counter-clockwise)
- `enc2_cw`, `enc2_ccw`

---

## Desktop → Device (Commands)

Sent by the desktop bridge to control device behavior.

### Set State

Changes the buddy's emotional/activity state.

```json
{
  "command": "set_state",
  "state": "attention",
  "prompt": "Claude wants to edit file main.py"
}
```

**States:**

| State | Audio | Visual |
|-------|-------|--------|
| `sleep` | Disconnect tone | Dark screen |
| `idle` | Connect tone | Default layer |
| `busy` | Ticking sound | Waiting animation |
| `attention` | Urgent alert | Approval screen |
| `celebrate` | Victory chime | Success animation |
| `error` | Error buzz | Error screen |

### Play Sound

Plays a single tone.

```json
{
  "command": "play_sound",
  "frequency": 880,
  "duration": 200
}
```

| Field | Type | Description |
|-------|------|-------------|
| `frequency` | integer | Hz (20 - 20000) |
| `duration` | integer | Milliseconds |

### Play Pattern

Plays a sequence of tones (melody/chime).

```json
{
  "command": "play_pattern",
  "pattern": "880:100,0:50,1760:200,0:50,880:100"
}
```

**Pattern format:** `freq:duration,freq:duration,...`
- `freq=0` = silence/pause
- Example patterns:
  - Startup: `"523:150,0:50,659:150,0:50,784:300"`
  - Alert: `"1200:150,0:100,800:150,0:100,1200:300"`

### Set Layer

Changes the macro pad's active layer.

```json
{
  "command": "set_layer",
  "layer": "approval"
}
```

**Layers:**
- `default` - Normal macro functions
- `approval` - Approval mode (K1=Approve, K2=Deny)
- `coding` - Build/test/fix commands
- `debug` - Debug controls
- `media` - Media controls

### Set LED (Optional)

If LEDs are added to hardware.

```json
{
  "command": "set_led",
  "led_id": 1,
  "state": true,
  "color": "#FF0000",
  "brightness": 128
}
```

### Display Text (Optional)

Shows custom message on display.

```json
{
  "command": "display_text",
  "line1": "Custom Message",
  "line2": "Second line",
  "duration": 3000
}
```

### Ping/Heartbeat

Keepalive to verify connection.

```json
{
  "command": "ping"
}
```

Device should respond:
```json
{
  "type": "pong",
  "timestamp": 12345
}
```

---

## State Machine Diagram

```
                    ┌─────────┐
    Power On ──────▶│  SLEEP  │
                    └────┬────┘
                         │ BLE Connect
                         ▼
                    ┌─────────┐
            ┌───────│  IDLE   │◀────────┐
            │       └────┬────┘         │
            │            │               │
            │   Request  │               │ Complete
            │            ▼               │
            │       ┌─────────┐          │
            │       │  BUSY   │──────────┘
            │       └────┬────┘
            │            │ Needs Approval
            │            ▼
            │       ┌───────────┐
            │  Approve│ ATTENTION │
            │ Deny   └─────┬─────┘
            │       ┌──────┴──────┐
            │       │             │
            ▼       ▼             │
       ┌─────────┐          ┌─────┴────┐
       │CELEBRATE│          │  ERROR   │
       │  (if    │          │ (on error)│
       │ success)│          └─────┬─────┘
       └────┬────┘                │
            │                     │
            └──────────┬──────────┘
                       ▼
                  ┌─────────┐
                  │  IDLE   │
                  └─────────┘
```

---

## Message Size Limits

| Direction | Max Size | Notes |
|-----------|----------|-------|
| Device → Desktop | 512 bytes | BLE MTU limit |
| Desktop → Device | 512 bytes | BLE MTU limit |

**JSON Guidelines:**
- Keep messages under 256 bytes when possible
- Use abbreviated keys for high-frequency events
- Batch multiple events if needed

---

## Error Handling

### Invalid JSON

If device receives malformed JSON:
1. Log error to serial
2. Optionally send error response:
```json
{
  "type": "error",
  "error": "invalid_json",
  "message": "Could not parse command"
}
```

### Unknown Command

```json
{
  "type": "error",
  "error": "unknown_command",
  "command": "foo"
}
```

### Rate Limiting

Device should throttle encoder events:
- Minimum 50ms between encoder notifications
- Drop intermediate ticks, send accumulated delta

---

## Implementation Reference

### Python (Desktop Bridge)

```python
import asyncio
from bleak import BleakClient
import json

NUS_TX = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
NUS_RX = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"

async def send_command(client, cmd_dict):
    """Send command to device"""
    payload = json.dumps(cmd_dict).encode()
    await client.write_gatt_char(NUS_RX, payload)

async def read_notifications(client, callback):
    """Subscribe to device events"""
    await client.start_notify(NUS_TX, 
        lambda s, d: callback(json.loads(d.decode())))

# Example: Set attention state
await send_command(client, {
    "command": "set_state",
    "state": "attention",
    "prompt": "Allow file edit?"
})
```

### Arduino (Device)

```cpp
void sendInputEvent(const char* type, int id, int value) {
    StaticJsonDocument<256> doc;
    doc["type"] = "input";
    doc["input_type"] = type;
    doc["id"] = id;
    doc["value"] = value;
    doc["timestamp"] = millis();
    
    char buffer[256];
    size_t len = serializeJson(doc, buffer);
    
    txCharacteristic->setValue((uint8_t*)buffer, len);
    txCharacteristic->notify();
}
```

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2024-XX-XX | Initial protocol specification |
