# Desktop Bridge Implementation

## Overview

The Desktop Bridge is a Python application that:
1. Connects to the Macro Buddy via BLE
2. Receives input events (keys, encoders)
3. Translates events into system actions
4. Monitors Claude Desktop and sends state updates to device

Two implementation approaches:
1. **BLE Only** - Uses Hardware Buddy window for state, bridge handles actions
2. **Hybrid** - Bridge monitors both Claude API and BLE

## Architecture

```
┌──────────────┐    BLE     ┌─────────────────┐     System API     ┌──────────────┐
│  MACRO BUDDY │◄──────────▶│ DESKTOP BRIDGE  │◄─────────────────▶│   CLAUDE     │
│   (Device)   │            │   (Python)      │                  │   DESKTOP    │
└──────────────┘            └─────────────────┘                  └──────────────┘
                                   │
                                   ▼ System Control
                            ┌──────────────┐
                            │  pyautogui   │ ── Keyboard/mouse simulation
                            │   subprocess │ ── Shell command execution
                            │  window mgmt │ ── Focus management
                            └──────────────┘
```

---

## Complete Bridge Implementation

### requirements.txt

```
bleak>=0.20.0
pyautogui>=0.9.54
asyncio-mqtt>=0.16.0
pynput>=1.7.6
psutil>=5.9.0
pywin32>=306; platform_system=="Windows"
applescript>=2021.2.9; platform_system=="Darwin"
```

### bridge.py

```python
#!/usr/bin/env python3
"""
Claude Macro Buddy - Desktop Bridge
Connects BLE hardware to system actions and Claude Desktop
"""

import asyncio
import json
import sys
import os
import subprocess
import time
from typing import Optional, Callable, Dict, Any
from dataclasses import dataclass
from enum import Enum

try:
    import pyautogui
    PYAUTOGUI_AVAILABLE = True
except ImportError:
    PYAUTOGUI_AVAILABLE = False
    print("Warning: pyautogui not available. Keyboard/mouse control disabled.")

from bleak import BleakClient, BleakScanner, BLEDevice

# ============== CONFIGURATION ==============

DEVICE_NAME = "ClaudeBuddy"
NUS_SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
NUS_TX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
NUS_RX_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"

# ============== ENUMS ==============

class ClaudeState(Enum):
    SLEEP = "sleep"
    IDLE = "idle"
    BUSY = "busy"
    ATTENTION = "attention"
    CELEBRATE = "celebrate"
    ERROR = "error"

class Layer(Enum):
    DEFAULT = "default"
    APPROVAL = "approval"
    CODING = "coding"
    DEBUG = "debug"
    MEDIA = "media"

# ============== DATA CLASSES ==============

@dataclass
class InputEvent:
    input_type: str  # "key", "encoder_btn", "encoder_turn"
    id: int
    value: int
    delta: int = 0

# ============== ACTIONS ==============

class ActionRegistry:
    """Registry for executable actions"""
    
    def __init__(self):
        self.actions: Dict[str, Callable] = {}
    
    def register(self, name: str, func: Callable):
        self.actions[name] = func
    
    def execute(self, name: str, *args, **kwargs):
        if name in self.actions:
            try:
                self.actions[name](*args, **kwargs)
            except Exception as e:
                print(f"Action '{name}' failed: {e}")
        else:
            print(f"Unknown action: {name}")

# ============== SYSTEM ACTIONS ==============

def focus_claude_window():
    """Bring Claude Desktop to foreground"""
    system = sys.platform
    
    if system == "darwin":  # macOS
        try:
            import applescript
            applescript.tell.app("Claude", "activate")
        except:
            subprocess.run(["osascript", "-e", 'tell application "Claude" to activate'])
    
    elif system == "win32":  # Windows
        try:
            import win32gui
            import win32con
            
            def callback(hwnd, extra):
                if "Claude" in win32gui.GetWindowText(hwnd):
                    win32gui.ShowWindow(hwnd, win32con.SW_RESTORE)
                    win32gui.SetForegroundWindow(hwnd)
                    return False
                return True
            
            win32gui.EnumWindows(callback, None)
        except ImportError:
            print("win32gui not available for window focus")
    
    else:  # Linux
        subprocess.run(["xdotool", "search", "--name", "Claude", "windowactivate"], 
                      capture_output=True)

def send_key_combo(keys: list):
    """Send keyboard shortcut"""
    if not PYAUTOGUI_AVAILABLE:
        return
    
    pyautogui.keyDown(*keys)
    pyautogui.keyUp(*keys)

def type_text(text: str):
    """Type text into active window"""
    if not PYAUTOGUI_AVAILABLE:
        return
    
    focus_claude_window()
    time.sleep(0.1)
    pyautogui.typewrite(text, interval=0.01)

def send_claude_command(command: str):
    """Send a slash command to Claude"""
    if not PYAUTOGUI_AVAILABLE:
        return
    
    focus_claude_window()
    time.sleep(0.1)
    
    # Ensure we're at a fresh prompt
    pyautogui.keyDown('esc')
    pyautogui.keyUp('esc')
    time.sleep(0.05)
    
    pyautogui.typewrite(command, interval=0.01)
    pyautogui.keyDown('return')
    pyautogui.keyUp('return')

def simulate_scroll(amount: int):
    """Scroll in Claude window"""
    if not PYAUTOGUI_AVAILABLE:
        return
    
    focus_claude_window()
    pyautogui.scroll(amount * 3)  # 3 lines per tick

def adjust_font(delta: int):
    """Adjust font size in Claude"""
    if delta > 0:
        send_key_combo(['ctrl', 'plus'])
    else:
        send_key_combo(['ctrl', 'minus'])

def run_shell_command(cmd: str):
    """Execute shell command"""
    try:
        subprocess.Popen(cmd, shell=True, 
                        stdout=subprocess.PIPE, 
                        stderr=subprocess.PIPE)
    except Exception as e:
        print(f"Command failed: {e}")

# ============== BRIDGE CLASS ==============

class MacroBuddyBridge:
    def __init__(self):
        self.client: Optional[BleakClient] = None
        self.connected = False
        self.current_layer = Layer.DEFAULT
        self.current_state = ClaudeState.IDLE
        self.actions = ActionRegistry()
        self.pending_approval = False
        
        self._register_default_actions()
    
    def _register_default_actions(self):
        """Register default action handlers"""
        
        # Key 1: Quick message
        self.actions.register("key1_press", 
            lambda: send_claude_command("/chat Can you help me with..."))
        
        # Key 2: Toggle focus
        self.actions.register("key2_press", focus_claude_window)
        
        # Key 3: New chat
        self.actions.register("key3_press", 
            lambda: send_claude_command("/new"))
        
        # Encoder 1: Scroll
        self.actions.register("enc1_cw", 
            lambda: simulate_scroll(5))
        self.actions.register("enc1_ccw", 
            lambda: simulate_scroll(-5))
        
        # Encoder 1 button: Toggle layer
        self.actions.register("enc1_press", self._toggle_layer)
        
        # Encoder 2: Font size
        self.actions.register("enc2_cw", 
            lambda: adjust_font(1))
        self.actions.register("enc2_ccw", 
            lambda: adjust_font(-1))
        
        # Encoder 2 button: Reset font
        self.actions.register("enc2_press", 
            lambda: send_key_combo(['ctrl', '0']))
    
    def _toggle_layer(self):
        """Cycle through layers"""
        layers = list(Layer)
        idx = layers.index(self.current_layer)
        self.current_layer = layers[(idx + 1) % len(layers)]
        print(f"Switched to layer: {self.current_layer.value}")
        
        # Update device display
        asyncio.create_task(self._send_command({
            "command": "set_layer",
            "layer": self.current_layer.value
        }))
    
    def _set_approval_layer(self, prompt: str):
        """Enter approval mode"""
        self.current_layer = Layer.APPROVAL
        self.pending_approval = True
        self.current_state = ClaudeState.ATTENTION
        
        # Update device
        asyncio.create_task(self._send_command({
            "command": "set_state",
            "state": "attention",
            "prompt": prompt[:50]  # Truncate for display
        }))
        
        print(f"\n{'='*50}")
        print(f"APPROVAL NEEDED: {prompt}")
        print(f"{'='*50}")
        print("Device keys: [K1] Approve  [K2] Deny  [K3] Details")
    
    def _handle_approval(self, approved: bool):
        """Handle approval/denial response"""
        self.pending_approval = False
        self.current_layer = Layer.DEFAULT
        self.current_state = ClaudeState.IDLE
        
        # Send to Claude Desktop
        # Note: This requires simulating keyboard interaction with Claude
        # The actual implementation depends on Claude Desktop's interface
        
        if approved:
            print("Approved!")
            # Send approval (platform specific)
            self._send_claude_approval(True)
        else:
            print("Denied!")
            self._send_claude_approval(False)
        
        # Reset device state
        asyncio.create_task(self._send_command({
            "command": "set_state",
            "state": "idle"
        }))
    
    def _send_claude_approval(self, approved: bool):
        """Send approval to Claude Desktop"""
        # This is approximate - actual method depends on Claude Desktop UI
        focus_claude_window()
        time.sleep(0.1)
        
        if approved:
            # Tab to approve button, press enter
            pyautogui.keyDown('tab')
            pyautogui.keyUp('tab')
            time.sleep(0.05)
            pyautogui.keyDown('return')
            pyautogui.keyUp('return')
        else:
            # Tab twice to deny, press enter
            pyautogui.keyDown('tab tab')
            pyautogui.keyUp('tab tab')
            time.sleep(0.05)
            pyautogui.keyDown('return')
            pyautogui.keyUp('return')
    
    async def _send_command(self, cmd: dict):
        """Send command to device"""
        if self.client and self.client.is_connected:
            try:
                payload = json.dumps(cmd).encode()
                await self.client.write_gatt_char(NUS_RX_UUID, payload)
            except Exception as e:
                print(f"Failed to send command: {e}")
    
    async def _notification_handler(self, sender: int, data: bytearray):
        """Handle incoming BLE notifications from device"""
        try:
            msg = json.loads(data.decode())
            await self._process_device_message(msg)
        except json.JSONDecodeError:
            print(f"Invalid JSON from device: {data}")
        except Exception as e:
            print(f"Error processing message: {e}")
    
    async def _process_device_message(self, msg: dict):
        """Process message from device"""
        msg_type = msg.get("type")
        
        if msg_type == "input":
            input_type = msg.get("input_type")
            input_id = msg.get("id")
            value = msg.get("value")
            
            await self._handle_input(input_type, input_id, value)
        
        elif msg_type == "encoder_turn":
            enc_id = msg.get("encoder_id")
            delta = msg.get("delta")
            await self._handle_encoder_turn(enc_id, delta)
        
        elif msg_type == "approval_response":
            # Device sent approval response
            approved = msg.get("approved")
            print(f"Device approval response: {approved}")
        
        elif msg_type == "state_change":
            state = msg.get("state")
            print(f"Device state changed to: {state}")
    
    async def _handle_input(self, input_type: str, input_id: int, value: int):
        """Handle key/button input"""
        if value == 0:  # Ignore release events
            return
        
        # Approval mode takes precedence
        if self.current_layer == Layer.APPROVAL:
            if input_id == 1:
                self._handle_approval(True)
            elif input_id == 2:
                self._handle_approval(False)
            elif input_id == 3:
                print("Show details...")
            return
        
        # Normal mode - map to actions
        action_map = {
            ("key", 1): "key1_press",
            ("key", 2): "key2_press",
            ("key", 3): "key3_press",
            ("encoder_btn", 1): "enc1_press",
            ("encoder_btn", 2): "enc2_press",
        }
        
        action = action_map.get((input_type, input_id))
        if action:
            self.actions.execute(action)
        else:
            print(f"Unhandled input: {input_type} {input_id}")
    
    async def _handle_encoder_turn(self, enc_id: int, delta: int):
        """Handle encoder rotation"""
        direction = "cw" if delta > 0 else "ccw"
        action_name = f"enc{enc_id}_{direction}"
        
        # Repeat for absolute delta value
        for _ in range(abs(delta)):
            self.actions.execute(action_name)
    
    async def find_and_connect(self):
        """Scan for and connect to Macro Buddy"""
        print(f"Scanning for {DEVICE_NAME}...")
        
        device = None
        
        async with BleakScanner() as scanner:
            await asyncio.sleep(5.0)  # Scan for 5 seconds
            
            for dev in scanner.discovered_devices:
                if dev.name == DEVICE_NAME:
                    device = dev
                    break
        
        if not device:
            print(f"Device '{DEVICE_NAME}' not found!")
            print("Make sure device is powered and advertising.")
            return False
        
        print(f"Found {device.name} at {device.address}")
        print("Connecting...")
        
        self.client = BleakClient(device)
        await self.client.connect()
        
        print("Connected!")
        self.connected = True
        
        # Subscribe to notifications
        await self.client.start_notify(NUS_TX_UUID, self._notification_handler)
        
        # Send initial state
        await self._send_command({
            "command": "set_state",
            "state": "idle"
        })
        
        return True
    
    async def run(self):
        """Main loop"""
        if not await self.find_and_connect():
            return
        
        print("\nBridge is running!")
        print(f"Current layer: {self.current_layer.value}")
        print("Press Ctrl+C to exit\n")
        
        try:
            while self.connected:
                # Monitor Claude Desktop state here
                # This would integrate with Claude's API or window monitoring
                
                await asyncio.sleep(0.1)
                
                # Check if still connected
                if not self.client.is_connected:
                    print("Disconnected!")
                    self.connected = False
                    break
                    
        except asyncio.CancelledError:
            pass
        finally:
            await self.disconnect()
    
    async def disconnect(self):
        """Clean disconnect"""
        if self.client:
            try:
                await self.client.disconnect()
            except:
                pass
        self.connected = False
        print("Disconnected from device")

# ============== MOCK CLAUDE MONITOR ==============

class MockClaudeMonitor:
    """
    Simulates Claude Desktop state changes for testing.
    In production, this would monitor actual Claude Desktop.
    """
    
    def __init__(self, bridge: MacroBuddyBridge):
        self.bridge = bridge
        self.states = [
            ("idle", None, 5),
            ("busy", None, 3),
            ("attention", "Allow editing file 'main.py'?", 10),
            ("celebrate", None, 2),
            ("idle", None, 5),
        ]
        self.index = 0
    
    async def run(self):
        """Cycle through mock states"""
        while self.bridge.connected:
            state, prompt, duration = self.states[self.index]
            
            print(f"[Mock] Claude state: {state}")
            
            if state == "attention" and prompt:
                self.bridge._set_approval_layer(prompt)
            else:
                self.bridge.current_state = ClaudeState(state)
                await self.bridge._send_command({
                    "command": "set_state",
                    "state": state
                })
            
            await asyncio.sleep(duration)
            self.index = (self.index + 1) % len(self.states)

# ============== MAIN ==============

async def main():
    print("=" * 60)
    print("  Claude Macro Buddy - Desktop Bridge")
    print("=" * 60)
    print()
    
    bridge = MacroBuddyBridge()
    
    # Optional: Start mock monitor for testing
    # monitor = MockClaudeMonitor(bridge)
    # asyncio.create_task(monitor.run())
    
    await bridge.run()

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nExiting...")
        sys.exit(0)
```

### advanced_bridge.py

For more sophisticated integration with Claude Desktop:

```python
#!/usr/bin/env python3
"""
Advanced Bridge with Claude Desktop Integration
Monitors Claude Desktop window and API for state changes.
"""

import asyncio
import json
import re
from typing import Optional
from dataclasses import dataclass
from datetime import datetime

import pyautogui
from bleak import BleakClient, BleakScanner
from PIL import ImageGrab  # For screen capture analysis

# ============== CLAUDE DESKTOP MONITOR ==============

class ClaudeDesktopMonitor:
    """
    Monitors Claude Desktop application for state changes.
    Uses multiple strategies: window title, UI detection, OCR.
    """
    
    def __init__(self, bridge):
        self.bridge = bridge
        self.last_state = None
        self.running = False
    
    def detect_state(self) -> Optional[str]:
        """
        Detect current Claude state by analyzing window/screen.
        Returns: "idle", "busy", "attention", or None
        """
        # Strategy 1: Window title analysis
        title = self._get_window_title()
        
        if "Claude" not in title:
            return "sleep"  # Claude not running
        
        # Strategy 2: Screen region detection
        # Look for approval dialog indicators
        if self._detect_approval_dialog():
            return "attention"
        
        # Strategy 3: Activity indicators
        if self._detect_processing_indicator():
            return "busy"
        
        return "idle"
    
    def _get_window_title(self) -> str:
        """Get active window title"""
        import sys
        if sys.platform == "win32":
            import win32gui
            return win32gui.GetWindowText(win32gui.GetForegroundWindow())
        elif sys.platform == "darwin":
            # Use AppleScript
            import subprocess
            result = subprocess.run(
                ['osascript', '-e', 
                 'tell application "System Events" to get name of first application process whose frontmost is true'],
                capture_output=True, text=True
            )
            return result.stdout.strip()
        return ""
    
    def _detect_approval_dialog(self) -> bool:
        """Check if approval dialog is visible"""
        # Screenshot and look for "Allow" or "Claude wants to" text
        # This is a simplified version - production would use proper OCR
        return False
    
    def _detect_processing_indicator(self) -> bool:
        """Check if Claude is processing (loading spinner, etc.)"""
        return False
    
    async def run(self):
        """Monitor loop"""
        self.running = True
        
        while self.running and self.bridge.connected:
            state = self.detect_state()
            
            if state and state != self.last_state:
                print(f"[Monitor] Claude state: {state}")
                
                if state == "attention":
                    self.bridge._set_approval_layer("Claude is requesting approval")
                else:
                    await self.bridge._send_command({
                        "command": "set_state",
                        "state": state
                    })
                
                self.last_state = state
            
            await asyncio.sleep(0.5)  # Poll every 500ms
    
    def stop(self):
        self.running = False

# ============== LAYER CONFIGURATION ==============

LAYER_CONFIGS = {
    "default": {
        "key1": {
            "name": "Quick Chat",
            "action": lambda b: b.send_claude_command("/chat ")
        },
        "key2": {
            "name": "Focus Toggle", 
            "action": lambda b: b.focus_claude()
        },
        "key3": {
            "name": "New Chat",
            "action": lambda b: b.send_claude_command("/new")
        },
        "enc1": {
            "name": "Scroll",
            "cw": lambda b: b.scroll(3),
            "ccw": lambda b: b.scroll(-3)
        },
        "enc2": {
            "name": "Font Size",
            "cw": lambda b: b.adjust_font(1),
            "ccw": lambda b: b.adjust_font(-1)
        }
    },
    
    "coding": {
        "key1": {
            "name": "Build",
            "action": lambda b: b.run_command("npm run build")
        },
        "key2": {
            "name": "Test",
            "action": lambda b: b.run_command("npm test")
        },
        "key3": {
            "name": "Fix",
            "action": lambda b: b.send_claude_command("/fix")
        },
        "enc1": {
            "name": "Errors",
            "cw": lambda b: b.send_key(["f8"]),  # Next error
            "ccw": lambda b: b.send_key(["shift", "f8"])  # Previous error
        },
        "enc2": {
            "name": "Files",
            "cw": lambda b: b.send_key(["ctrl", "tab"]),
            "ccw": lambda b: b.send_key(["ctrl", "shift", "tab"])
        }
    },
    
    "debug": {
        "key1": {
            "name": "Debug Start",
            "action": lambda b: b.send_key(["f5"])
        },
        "key2": {
            "name": "Breakpoint",
            "action": lambda b: b.send_key(["f9"])
        },
        "key3": {
            "name": "Step Over",
            "action": lambda b: b.send_key(["f10"])
        },
        "enc1": {
            "name": "Step",
            "cw": lambda b: b.send_key(["f11"]),  # Step into
            "ccw": lambda b: b.send_key(["shift", "f11"])  # Step out
        },
        "enc2": {
            "name": "Continue",
            "cw": lambda b: b.send_key(["f5"]),
            "ccw": lambda b: b.send_key(["shift", "f5"])  # Stop
        }
    },
    
    "media": {
        "key1": {
            "name": "Play/Pause",
            "action": lambda b: b.send_key(["playpause"])
        },
        "key2": {
            "name": "Prev",
            "action": lambda b: b.send_key(["prevtrack"])
        },
        "key3": {
            "name": "Next",
            "action": lambda b: b.send_key(["nexttrack"])
        },
        "enc1": {
            "name": "Volume",
            "cw": lambda b: b.send_key(["volumeup"]),
            "ccw": lambda b: b.send_key(["volumedown"])
        },
        "enc2": {
            "name": "Seek",
            "cw": lambda b: b.send_key(["right"]),
            "ccw": lambda b: b.send_key(["left"])
        }
    },
    
    "approval": {
        "key1": {
            "name": "Approve",
            "action": lambda b: b._handle_approval(True)
        },
        "key2": {
            "name": "Deny",
            "action": lambda b: b._handle_approval(False)
        },
        "key3": {
            "name": "Details",
            "action": lambda b: print("Show details...")
        },
        "enc1": {
            "name": "Scroll Prompt",
            "cw": lambda b: None,
            "ccw": lambda b: None
        },
        "enc2": {
            "name": "Multiple",
            "cw": lambda b: None,
            "ccw": lambda b: None
        }
    }
}

# Usage in bridge class:
# config = LAYER_CONFIGS.get(self.current_layer, LAYER_CONFIGS["default"])
# action = config.get(f"key{key_id}")
# if action: action["action"](self)
```

---

## Configuration File

### bridge_config.json

```json
{
  "device_name": "ClaudeBuddy",
  "connection": {
    "scan_timeout": 10,
    "reconnect_attempts": 3,
    "reconnect_delay": 5
  },
  "layers": {
    "default": {
      "key1": {
        "type": "claude_command",
        "command": "/chat "
      },
      "key2": {
        "type": "system",
        "action": "focus_window"
      },
      "key3": {
        "type": "claude_command",
        "command": "/new"
      },
      "encoder1": {
        "type": "scroll",
        "sensitivity": 3
      },
      "encoder2": {
        "type": "font_size",
        "step": 1
      }
    },
    "coding": {
      "key1": {
        "type": "shell",
        "command": "npm run build"
      },
      "key2": {
        "type": "shell",
        "command": "npm test"
      },
      "key3": {
        "type": "claude_command",
        "command": "/fix"
      },
      "encoder1": {
        "type": "keys",
        "cw": ["f8"],
        "ccw": ["shift", "f8"]
      }
    }
  },
  "audio": {
    "enabled": true,
    "volume": 0.5,
    "patterns": {
      "startup": "523:150,0:50,659:150,0:50,784:300",
      "attention": "1200:150,0:100,800:150,0:100,1200:300"
    }
  },
  "claude_desktop": {
    "auto_detect": true,
    "window_title": "Claude",
    "approval_detection": {
      "enabled": true,
      "poll_interval": 0.5
    }
  }
}
```

---

## Running the Bridge

### Basic Usage

```bash
# Install dependencies
pip install -r requirements.txt

# Run bridge
python bridge.py
```

### With Systemd (Linux)

```ini
# /etc/systemd/system/claude-macro-bridge.service
[Unit]
Description=Claude Macro Buddy Bridge
After=bluetooth.target

[Service]
Type=simple
User=%I
WorkingDirectory=/home/%I/claude-macro-buddy
ExecStart=/home/%I/.local/bin/python bridge.py
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

Enable: `sudo systemctl enable claude-macro-bridge@username`
Start: `sudo systemctl start claude-macro-bridge@username`

### With LaunchAgent (macOS)

```xml
<!-- ~/Library/LaunchAgents/com.claude.macro-bridge.plist -->
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>com.claude.macro-bridge</string>
    <key>ProgramArguments</key>
    <array>
        <string>/usr/local/bin/python3</string>
        <string>/Users/username/claude-macro-buddy/bridge.py</string>
    </array>
    <key>RunAtLoad</key>
    <true/>
    <key>KeepAlive</key>
    <true/>
</dict>
</plist>
```

Load: `launchctl load ~/Library/LaunchAgents/com.claude.macro-bridge.plist`

---

## Troubleshooting

### BLE Connection Issues

```bash
# List BLE devices
python -m bleakscan

# Check Bluetooth service
# Linux:
sudo systemctl status bluetooth

# macOS:
sudo launchctl list | grep bluetooth

# Reset BLE adapter
# Linux:
sudo hciconfig hci0 reset
```

### Permission Errors

```bash
# Linux: Add user to bluetooth group
sudo usermod -aG bluetooth $USER

# macOS: Grant Bluetooth permission in System Preferences
# -> Security & Privacy -> Privacy -> Bluetooth
```

### Claude Desktop Not Detected

1. Ensure Claude Desktop is running
2. Check window title contains "Claude"
3. Try manual focus: press key 2 to toggle focus

### Laggy Response

1. Check BLE signal strength (move closer)
2. Reduce encoder sensitivity in config
3. Close other BLE devices
