# Claude Desktop Buddy + Macro Pad Documentation

Complete documentation for building a hybrid hardware device that functions as both a **Desktop Buddy** (reactive companion for Claude Code) and a **Macro Pad** (physical controls for common actions).

## What Is This?

A custom hardware controller featuring:
- **3 mechanical keys** for instant actions
- **2 rotary encoders** (with push buttons) for navigation and control
- **OLED display** for status visualization
- **Speaker** for audio cues and feedback
- **ESP32** for BLE connectivity to Claude Desktop

## Key Features

| Feature | Description |
|---------|-------------|
| **State Awareness** | Knows when Claude is idle, busy, or needs approval |
| **Audio Cues** | Different sounds for different states (busy ticking, urgent alert for approvals, celebration chimes) |
| **Context Layers** | Keys do different things based on context (coding, debugging, media control) |
| **Physical Approvals** | Press button to approve/deny without touching mouse/keyboard |
| **Customizable** | Fully programmable actions and sounds |

## Documentation Index

| File | Purpose |
|------|---------|
| [**01_OVERVIEW.md**](01_OVERVIEW.md) | Vision, architecture, and core concepts |
| [**02_HARDWARE.md**](02_HARDWARE.md) | Component list, pin assignments, wiring diagrams |
| [**03_FIRMWARE.md**](03_FIRMWARE.md) | Complete Arduino/ESP32 firmware with all modules |
| [**04_PROTOCOL.md**](04_PROTOCOL.md) | BLE communication specification and JSON protocol |
| [**05_DESKTOP_BRIDGE.md**](05_DESKTOP_BRIDGE.md) | Python desktop bridge implementation |
| [**06_FUNCTION_MAPS.md**](06_FUNCTION_MAPS.md) | Layer system and key mapping reference |
| [**07_ASSEMBLY.md**](07_ASSEMBLY.md) | Step-by-step build instructions |

## Quick Start

### 1. Hardware (See 02_HARDWARE.md, 07_ASSEMBLY.md)
```
Components needed:
- ESP32 development board
- 1.3" OLED display (I2C)
- 3x Cherry MX switches + keycaps
- 2x EC11 rotary encoders + knobs
- Small speaker/buzzer
- Perfboard or custom PCB
```

### 2. Firmware (See 03_FIRMWARE.md)
```bash
# Install Arduino libraries
- ArduinoJson
- ESP32 BLE Arduino
- Encoder
- Bounce2
- U8g2

# Upload firmware/macro_buddy.ino to ESP32
```

### 3. Desktop Bridge (See 05_DESKTOP_BRIDGE.md)
```bash
# Install Python dependencies
pip install -r requirements.txt

# Run bridge
python bridge.py
```

### 4. Pair (See 01_OVERVIEW.md)
```
1. Enable Developer Mode in Claude Desktop
2. Open Hardware Buddy window
3. Pair with "ClaudeBuddy" device
```

## Architecture Overview

```
┌──────────────┐     BLE (NUS)      ┌──────────────┐     API/Shortcuts    ┌──────────┐
│ MACRO BUDDY  │◄──────────────────►│    BRIDGE    │◄───────────────────▶│  CLAUDE  │
│   Device     │   JSON Protocol    │    (Python)  │   pyautogui/shell  │  DESKTOP │
└──────────────┘                    └──────────────┘                    └──────────┘
     │                                    │
     │ Audio Cues                         │ System Actions
     ▼                                    ▼
┌──────────┐                        ┌──────────┐
│ Speaker  │                        │ Keyboard │
│ Display  │                        │ Commands │
└──────────┘                        └──────────┘
```

## Default Key Functions

| Input | Normal Mode | Approval Mode |
|-------|-------------|---------------|
| **Key 1** | Quick Chat | **APPROVE** |
| **Key 2** | Toggle Focus | **DENY** |
| **Key 3** | New Chat | Details |
| **Encoder 1** | Scroll | Scroll prompt |
| **Encoder 2** | Font size | Multiple approvals |

## Audio Cues

| State | Sound | When |
|-------|-------|------|
| `idle` | Connect chime | Connected to desktop |
| `busy` | Periodic ticking | Claude processing request |
| `attention` | Urgent alert | Approval needed |
| `celebrate` | Victory melody | Task completed |
| `key press` | Brief tone | Any key pressed |
| `encoder turn` | Soft tick | Encoder rotated |

## Customization

All functions are programmable:

```python
# Example: Custom key binding
actions.register("key1_press", lambda: run_shell_command("make"))

# Example: Custom audio
audio.playPattern("880:100,0:50,1760:200")  # Custom melody

# Example: New layer
LAYER_CONFIGS["rust"] = {
    "key1": {"type": "shell", "command": "cargo build"},
    "key2": {"type": "shell", "command": "cargo test"},
    "key3": {"type": "shell", "command": "cargo run"}
}
```

## Project Structure

```
claude-macro-buddy/
├── docs/
│   ├── 01_OVERVIEW.md
│   ├── 02_HARDWARE.md
│   ├── 03_FIRMWARE.md
│   ├── 04_PROTOCOL.md
│   ├── 05_DESKTOP_BRIDGE.md
│   ├── 06_FUNCTION_MAPS.md
│   ├── 07_ASSEMBLY.md
│   └── README.md (this file)
├── firmware/
│   ├── config.h
│   ├── macro_buddy.ino
│   ├── audio_manager.cpp
│   ├── ble_manager.cpp
│   ├── input_manager.cpp
│   ├── display_manager.cpp
│   └── state_machine.cpp
├── bridge/
│   ├── bridge.py
│   ├── advanced_bridge.py
│   ├── requirements.txt
│   └── bridge_config.json
└── hardware/
    ├── schematic.pdf
    └── case.stl (optional 3D print)
```

## Troubleshooting

### Device won't pair
- Ensure Developer Mode enabled in Claude Desktop
- Check BLE enabled on computer
- Verify device appears as "ClaudeBuddy"

### Keys not working
- Check INPUT_PULLUP mode in firmware
- Verify GPIO pin numbers match wiring
- Test with multimeter for continuity

### No audio
- Verify speaker polarity
- Check PWM pin capability (GPIO 13)
- Test with simple tone sketch first

### Display blank
- Try alternate I2C address (0x3C vs 0x3D)
- Verify SDA/SCL connections
- Check 3.3V supply (not 5V)

## References

- [claude-desktop-buddy](https://github.com/anthropics/claude-desktop-buddy) - Reference implementation
- [Nordic UART Service](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.0.0%2Fble_sdk_app_nus_eval.html)
- [ESP32 BLE Arduino](https://github.com/nkolban/ESP32_BLE_Arduino)

## License

This project documentation is provided as-is for educational purposes.

## Contributing

Feel free to modify and extend:
- Add new layers for your workflow
- Create custom audio patterns
- Design alternative enclosures
- Extend the protocol

## Support

For issues:
1. Check [07_ASSEMBLY.md](07_ASSEMBLY.md) troubleshooting section
2. Verify wiring against [02_HARDWARE.md](02_HARDWARE.md)
3. Test components individually
4. Review serial monitor output
