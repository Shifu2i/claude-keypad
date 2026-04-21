# Claude Desktop Buddy + Macro Pad - Overview

## Vision

A hybrid hardware device that combines:
- **Desktop Buddy**: Reacts to Claude's state with audio/visual feedback
- **Macro Pad**: Physical controls for common Claude Code actions

## Core Concept

The device sits on your desk and:
1. **Monitors Claude's activity** via BLE connection to Claude Desktop
2. **Plays audio cues** when Claude is ready, thinking, or needs approval
3. **Provides instant physical controls** for common actions without reaching for keyboard/mouse

## Architecture

```
┌─────────────────┐      BLE (Nordic UART)       ┌──────────────────────┐
│   MACRO BUDDY   │ ◄──────────────────────────► │   CLAUDE DESKTOP     │
│   (ESP32 +      │      JSON over GATT          │   Developer Mode     │
│    Peripherals) │                               │   + Hardware Buddy   │
└─────────────────┘                               └──────────┬───────────┘
         │                                                    │
         │ Audio Cues                                         │ Claude API
         ▼                                                    ▼
    ┌─────────┐                                         ┌────────────┐
    │ Speaker │                                         │ Claude Code│
    └─────────┘                                         └────────────┘
```

## Key Features

| Feature | Implementation |
|---------|----------------|
| **State Awareness** | Receives `idle`, `busy`, `attention`, `celebrate` states from Claude |
| **Audio Feedback** | Different tones/patterns for each state |
| **Physical Input** | 3 keys + 2 rotary encoders (with push) |
| **Layer System** | Context-aware functions based on Claude's state |
| **Visual Display** | OLED showing current state and available actions |

## State-Driven Behavior

### Claude States → Audio Cues

| State | Trigger | Audio Response |
|-------|---------|----------------|
| `idle` | Connected, waiting | Soft ambient hum |
| `busy` | Processing request | "Thinking" ticking pattern |
| `attention` | **Approval needed** | **Urgent alert tone** |
| `celebrate` | Task completed | Victory chime |
| `error` | Error occurred | Low error tone |

### Macro Functions by Context

**Normal Mode** (Claude idle):
- Keys: Quick prompts, new chat, focus toggle
- Encoders: Scroll, font adjust

**Approval Mode** (permission pending):
- Keys: **Approve / Deny / Details**
- Encoders: Scroll options, select multiple approvals

**Coding Mode** (manual toggle):
- Keys: Build, Test, Fix
- Encoders: Navigate errors/files

## Required Components

1. **ESP32 microcontroller** (WiFi/BLE capable)
2. **1.3" OLED display** (128x64, I2C)
3. **3x Mechanical key switches** (Cherry MX style)
4. **2x Rotary encoders** (with push button)
5. **Speaker/Buzzer** (8Ω or piezo)
6. **USB-C connection** (power + optional serial)

## Dependencies

- Claude Desktop with Developer Mode enabled
- Hardware Buddy window for initial pairing
- Python bridge running on desktop (optional, for extended features)

## File Structure

```
docs/
├── 01_OVERVIEW.md         (this file)
├── 02_HARDWARE.md         (components & wiring)
├── 03_FIRMWARE.md         (Arduino code)
├── 04_PROTOCOL.md         (BLE communication spec)
├── 05_DESKTOP_BRIDGE.md   (Python integration)
├── 06_FUNCTION_MAPS.md    (key mappings by layer)
└── 07_ASSEMBLY.md         (build instructions)
```
