# Function Mappings & Layers

## Overview

The Macro Buddy uses a **layer system** where the same physical inputs perform different actions depending on the active layer. Layers can be switched manually or automatically based on Claude's state.

## Layer Types

| Layer | Trigger | Purpose |
|-------|---------|---------|
| **DEFAULT** | Default | General purpose, navigation, common actions |
| **APPROVAL** | Auto on permission prompt | Handle Claude approval requests |
| **CODING** | Manual (Enc1 press) | Development workflow (build, test, fix) |
| **DEBUG** | Manual (Enc1 double-press) | Debugging controls |
| **MEDIA** | Manual (Enc2 press) | Media playback controls |

---

## Default Layer

**Activated by:** Default, or return from other layers

**Display:** Shows "DEFAULT" and last action

### Key Functions

| Key | Action | Claude Code Equivalent | Notes |
|-----|--------|------------------------|-------|
| **K1** | Quick Chat | Type `/chat` + space | Opens new message, ready for input |
| **K2** | Toggle Focus | Focus Claude window | Press again to return to previous window |
| **K3** | New Chat | Execute `/new` | Starts fresh conversation |

### Encoder Functions

| Input | Action | Effect |
|-------|--------|--------|
| **Enc1 CW** | Scroll Up | Scroll transcript up |
| **Enc1 CCW** | Scroll Down | Scroll transcript down |
| **Enc1 Push** | **Layer Toggle** | Cycle to next layer (CODING) |
| **Enc2 CW** | Font Increase | `Ctrl + Plus` |
| **Enc2 CCW** | Font Decrease | `Ctrl + Minus` |
| **Enc2 Push** | Reset Font | `Ctrl + 0` |

---

## Approval Layer

**Activated by:** Automatically when Claude requests permission

**Deactivated by:** Approval/denial, or timeout after 2 minutes

**Audio:** Urgent alert tone on activation

**Display:** Shows "NEEDS YOU" and approval prompt preview

### Key Functions (Approval Priority)

| Key | Action | System Response |
|-----|--------|-----------------|
| **K1** | **APPROVE** | Allows Claude's requested action |
| **K2** | **DENY** | Rejects the request |
| **K3** | Show Details | Cycle through full permission text |

### Encoder Functions

| Input | Action |
|-------|--------|
| **Enc1** | Scroll through permission details |
| **Enc2** | Navigate multiple pending approvals |

### Approval Flow

```
Claude requests permission
         │
         ▼
┌─────────────────┐
│  Audio: Alert   │
│  Display: Show  │
│  prompt preview │
└────────┬────────┘
         │
    ┌────┴────┐
    ▼         ▼
┌───────┐  ┌───────┐
│  K1   │  │  K2   │
│Approve│  │ Deny  │
└───┬───┘  └───┬───┘
    │          │
    ▼          ▼
Audio: Happy  Audio: Sad
Send response  Send response
Return to      Return to
previous       previous
layer          layer
```

---

## Coding Layer

**Activated by:** Press Enc1 in Default layer

**Display:** Shows "CODING"

**Audio:** Layer change chime

### Key Functions

| Key | Action | Typical Binding |
|-----|--------|-----------------|
| **K1** | Run Build | `npm run build` or `make` |
| **K2** | Run Tests | `npm test` or `pytest` |
| **K3** | Quick Fix | Claude `/fix` command |

### Encoder Functions

| Input | Action | VS Code Equivalent |
|-------|--------|-------------------|
| **Enc1 CW** | Next Error | `F8` |
| **Enc1 CCW** | Previous Error | `Shift + F8` |
| **Enc1 Push** | Quick Fix | `Ctrl + .` |
| **Enc2 CW** | Next File | `Ctrl + Tab` |
| **Enc2 CCW** | Previous File | `Ctrl + Shift + Tab` |
| **Enc2 Push** | Close File | `Ctrl + W` |

### Extended Actions (Hold Key)

| Key | Hold (1s) | Action |
|-----|-----------|--------|
| **K1** | Run with Args | Opens dialog for custom command |
| **K2** | Run Single Test | Runs test at cursor |
| **K3** | Explain Error | Claude explains selected error |

---

## Debug Layer

**Activated by:** Double-press Enc1 in Default layer (within 500ms)

**Display:** Shows "DEBUG"

### Key Functions

| Key | Action | Standard Key |
|-----|--------|--------------|
| **K1** | Start Debugging | `F5` |
| **K2** | Toggle Breakpoint | `F9` |
| **K3** | Step Over | `F10` |

### Encoder Functions

| Input | Action | Standard Key |
|-------|--------|--------------|
| **Enc1 CW** | Step Into | `F11` |
| **Enc1 CCW** | Step Out | `Shift + F11` |
| **Enc1 Push** | Run to Cursor | `Ctrl + F10` |
| **Enc2 CW** | Continue | `F5` |
| **Enc2 CCW** | Stop Debugging | `Shift + F5` |
| **Enc2 Push** | Pause | `Ctrl + Break` |

---

## Media Layer

**Activated by:** Press Enc2 in Default layer

**Display:** Shows "MEDIA"

**Note:** Works globally, not just in Claude

### Key Functions

| Key | Action | Media Key |
|-----|--------|-----------|
| **K1** | Play/Pause | `Play/Pause` |
| **K2** | Previous Track | `Previous Track` |
| **K3** | Next Track | `Next Track` |

### Encoder Functions

| Input | Action |
|-------|--------|
| **Enc1 CW** | Volume Up |
| **Enc1 CCW** | Volume Down |
| **Enc1 Push** | Mute |
| **Enc2 CW** | Seek Forward (10s) |
| **Enc2 CCW** | Seek Backward (10s) |
| **Enc2 Push** | Seek to Beginning |

---

## Layer Transitions

### Manual Switching

```
[DEFAULT] ──Enc1 press──▶ [CODING] ──Enc1 press──▶ [DEBUG]
    │                        │                        │
    │                        │                        │
    └──────Enc2 press───────▶[MEDIA]◀───────────────────┘
                              │
                              └─Any press (not Enc)──▶ [DEFAULT]
```

### Automatic Switching

```
Any Layer ──Claude needs approval──▶ [APPROVAL]
                                              │
                                              │
Any Layer ◀────────Approval/Denial─────────────┘
```

---

## Custom Function Mapping

Users can customize mappings by editing `bridge_config.json`:

### Example: Custom Coding Layer

```json
{
  "layers": {
    "coding_custom": {
      "key1": {
        "type": "shell",
        "command": "cargo build --release",
        "description": "Rust Release Build"
      },
      "key2": {
        "type": "shell",
        "command": "cargo test",
        "description": "Run Tests"
      },
      "key3": {
        "type": "shell",
        "command": "cargo fmt && cargo clippy",
        "description": "Format & Lint"
      },
      "encoder1": {
        "type": "shell",
        "cw": {
          "command": "wmctrl -r ':ACTIVE:' -b add,above",
          "description": "Float window up"
        },
        "ccw": {
          "command": "wmctrl -r ':ACTIVE:' -b remove,above",
          "description": "Unfloat window"
        }
      }
    }
  }
}
```

---

## Action Types Reference

| Type | Description | Example |
|------|-------------|---------|
| `claude_command` | Type `/command` in Claude | `/new`, `/chat`, `/fix` |
| `shell` | Execute shell command | `npm run build` |
| `keys` | Send keyboard shortcut | `["ctrl", "tab"]` |
| `focus_window` | Focus specific window | Focus Claude |
| `scroll` | Scroll current window | +/- N lines |
| `font_size` | Adjust UI font | +/- 1 step |
| `callback` | Python function call | Custom function |
| `macro` | Sequence of actions | Multiple steps |

---

## Visual Feedback Summary

| Input Type | Visual | Audio |
|------------|--------|-------|
| Key press | Brief flash on display | Tone by key # |
| Encoder turn | Value shown temporarily | Soft tick |
| Encoder push | Layer name displayed | Chime |
| Layer change | New layer name | Arpeggio |
| Approval needed | Prompt preview + "NEEDS YOU" | Urgent alert |
| Approval given | Checkmark + return | Happy tone |
| Approval denied | X + return | Sad tone |
| Claude busy | Animation (dots) | Periodic tick |
| Task complete | Success animation | Victory chime |
| Error | Error icon | Error buzz |

---

## Quick Reference Card

```
┌─────────────────────────────────────────────┐
│           MACRO BUDDY - QUICK REF          │
├─────────────────────────────────────────────┤
│                                             │
│  ┌─────┐      DEFAULT LAYER               │
│  │Enc 1│  ◀──  Scroll Transcript          │
│  │ [═] │      Push → Coding Layer          │
│  └─────┘                                   │
│                                             │
│     [K1]      [K2]      [K3]               │
│   Quick     Focus      New                 │
│   Chat      Toggle     Chat                │
│                                             │
│  ┌─────┐                                   │
│  │Enc 2│  ◀──  Font Size                  │
│  │ [═] │      Push → Media Layer           │
│  └─────┘                                   │
│                                             │
│  ═══════════════════════════════════════   │
│                                             │
│  APPROVAL MODE (Auto on prompt)            │
│                                             │
│     [K1]      [K2]      [K3]               │
│   APPROVE    DENY     Details              │
│                                             │
│  ═══════════════════════════════════════   │
│                                             │
│  CODING LAYER (Enc1 push)                  │
│                                             │
│     [K1]      [K2]      [K3]               │
│   Build      Test      Fix                 │
│                                             │
│  Enc1: Next/Prev Error  Enc2: Next/Prev File│
│                                             │
└─────────────────────────────────────────────┘
```
