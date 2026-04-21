#!/usr/bin/env python3
"""
Claude Desktop Buddy - Desktop Bridge
Connects BLE hardware to system actions and Claude Desktop

Usage:
    python bridge.py

Requirements:
    pip install -r requirements.txt

Note:
    Claude Desktop Developer Mode must be enabled:
    Help -> Troubleshooting -> Enable Developer Mode
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
            subprocess.run(
                ["osascript", "-e", 'tell application "Claude" to activate'],
                capture_output=True
            )
        except Exception as e:
            print(f"Failed to focus Claude: {e}")

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
            print("win32gui not available. Install pywin32 for window focus.")

    else:  # Linux
        subprocess.run(
            ["xdotool", "search", "--name", "Claude", "windowactivate"],
            capture_output=True
        )

def send_claude_command(command: str):
    """Send a slash command to Claude"""
    if not PYAUTOGUI_AVAILABLE:
        return

    focus_claude_window()
    time.sleep(0.1)

    # Clear any existing text
    pyautogui.keyDown('esc')
    pyautogui.keyUp('esc')
    time.sleep(0.05)

    pyautogui.typewrite(command, interval=0.01)
    pyautogui.keyDown('return')
    pyautogui.keyUp('return')

def simulate_scroll(amount: int):
    """Scroll in active window"""
    if not PYAUTOGUI_AVAILABLE:
        return

    focus_claude_window()
    pyautogui.scroll(amount * 3)

def adjust_font(delta: int):
    """Adjust font size"""
    if delta > 0:
        pyautogui.keyDown('ctrl')
        pyautogui.keyDown('plus')
        pyautogui.keyUp('plus')
        pyautogui.keyUp('ctrl')
    else:
        pyautogui.keyDown('ctrl')
        pyautogui.keyDown('minus')
        pyautogui.keyUp('minus')
        pyautogui.keyUp('ctrl')

def run_shell_command(cmd: str):
    """Execute shell command"""
    try:
        subprocess.Popen(
            cmd, shell=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
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
            lambda: send_claude_command("/chat "))

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
            lambda: pyautogui.keyDown('ctrl') or pyautogui.keyDown('0') or pyautogui.keyUp('0') or pyautogui.keyUp('ctrl'))

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

    def _handle_approval(self, approved: bool):
        """Handle approval/denial"""
        self.pending_approval = False
        self.current_layer = Layer.DEFAULT
        self.current_state = ClaudeState.IDLE

        print("Approved!" if approved else "Denied!")

        # Send to device
        asyncio.create_task(self._send_command({
            "command": "set_state",
            "state": "idle"
        }))

        if approved:
            asyncio.create_task(self._send_command({
                "command": "play_pattern",
                "pattern": "880:100,0:50,1100:150"
            }))
        else:
            asyncio.create_task(self._send_command({
                "command": "play_pattern",
                "pattern": "600:100,0:50,400:200"
            }))

    async def _send_command(self, cmd: dict):
        """Send command to device"""
        if self.client and self.client.is_connected:
            try:
                payload = json.dumps(cmd).encode()
                await self.client.write_gatt_char(NUS_RX_UUID, payload)
            except Exception as e:
                print(f"Failed to send command: {e}")

    async def _notification_handler(self, sender: int, data: bytearray):
        """Handle incoming BLE notifications"""
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

    async def _handle_input(self, input_type: str, input_id: int, value: int):
        """Handle key/button input"""
        if value == 0:
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

        # Normal mode
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

        for _ in range(abs(delta)):
            self.actions.execute(action_name)

    async def find_and_connect(self):
        """Scan for and connect to Macro Buddy"""
        print(f"Scanning for {DEVICE_NAME}...")

        device = None

        async with BleakScanner() as scanner:
            await asyncio.sleep(5.0)

            for dev in scanner.discovered_devices:
                if dev.name == DEVICE_NAME:
                    device = dev
                    break

        if not device:
            print(f"Device '{DEVICE_NAME}' not found!")
            return False

        print(f"Found {device.name} at {device.address}")
        print("Connecting...")

        self.client = BleakClient(device)
        await self.client.connect()

        print("Connected!")
        self.connected = True

        await self.client.start_notify(NUS_TX_UUID, self._notification_handler)

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
                await asyncio.sleep(0.1)

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

# ============== MAIN ==============

async def main():
    print("=" * 60)
    print("  Claude Macro Buddy - Desktop Bridge")
    print("=" * 60)
    print()

    bridge = MacroBuddyBridge()
    await bridge.run()

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nExiting...")
        sys.exit(0)
