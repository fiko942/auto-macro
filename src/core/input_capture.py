"""
Input Capture - Menggunakan pynput untuk capture keyboard dan mouse
"""
import threading
import time
from enum import Enum
from dataclasses import dataclass
from typing import Optional, Set

from pynput import keyboard, mouse
from pynput.keyboard import Key, KeyCode

from PyQt6.QtCore import QObject, pyqtSignal, QTimer


class InputType(Enum):
    KEYBOARD = "keyboard"
    MOUSE = "mouse"
    CONTROLLER = "controller"


@dataclass
class CapturedInput:
    """Represents a captured input"""
    input_type: InputType
    key_name: str
    display_name: str
    
    def __str__(self):
        return self.display_name


# Mapping pynput special keys ke nama string
SPECIAL_KEY_MAP = {
    Key.space: 'space',
    Key.enter: 'enter',
    Key.tab: 'tab',
    Key.backspace: 'backspace',
    Key.delete: 'delete',
    Key.esc: 'esc',
    Key.insert: 'insert',
    Key.home: 'home',
    Key.end: 'end',
    Key.page_up: 'page up',
    Key.page_down: 'page down',
    Key.left: 'left',
    Key.right: 'right',
    Key.up: 'up',
    Key.down: 'down',
    Key.caps_lock: 'caps lock',
    Key.num_lock: 'num lock',
    Key.scroll_lock: 'scroll lock',
    Key.print_screen: 'print screen',
    Key.pause: 'pause',
    Key.f1: 'f1', Key.f2: 'f2', Key.f3: 'f3', Key.f4: 'f4',
    Key.f5: 'f5', Key.f6: 'f6', Key.f7: 'f7', Key.f8: 'f8',
    Key.f9: 'f9', Key.f10: 'f10', Key.f11: 'f11', Key.f12: 'f12',
    Key.ctrl: 'ctrl', Key.ctrl_l: 'ctrl', Key.ctrl_r: 'ctrl',
    Key.shift: 'shift', Key.shift_l: 'shift', Key.shift_r: 'shift',
    Key.alt: 'alt', Key.alt_l: 'alt', Key.alt_r: 'alt', Key.alt_gr: 'alt',
    Key.cmd: 'win', Key.cmd_l: 'win', Key.cmd_r: 'win',
}

MOUSE_BUTTON_MAP = {
    mouse.Button.left: ('mouse_left', '🖱️ Left Click'),
    mouse.Button.right: ('mouse_right', '🖱️ Right Click'),
    mouse.Button.middle: ('mouse_middle', '🖱️ Middle Click'),
}


class UnifiedInputCapture(QObject):
    """
    Capture input dari keyboard dan mouse menggunakan pynput
    """
    
    inputCaptured = pyqtSignal(object)
    
    def __init__(self, parent=None):
        super().__init__(parent)
        self._capturing = False
        self._keyboard_listener = None
        self._mouse_listener = None
        
        # Track pressed modifiers
        self._modifiers: Set[str] = set()
        self._captured = False
    
    def start_capture(self):
        """Start capturing"""
        if self._capturing:
            return
        
        self._capturing = True
        self._captured = False
        self._modifiers.clear()
        
        # Keyboard listener
        self._keyboard_listener = keyboard.Listener(
            on_press=self._on_key_press,
            on_release=self._on_key_release
        )
        self._keyboard_listener.start()
        
        # Mouse listener
        self._mouse_listener = mouse.Listener(
            on_click=self._on_mouse_click
        )
        self._mouse_listener.start()
    
    def stop_capture(self):
        """Stop capturing"""
        self._capturing = False
        
        if self._keyboard_listener:
            self._keyboard_listener.stop()
            self._keyboard_listener = None
        
        if self._mouse_listener:
            self._mouse_listener.stop()
            self._mouse_listener = None
    
    def _on_key_press(self, key):
        """Handle keyboard press"""
        if not self._capturing or self._captured:
            return
        
        key_name = self._get_key_name(key)
        if not key_name:
            return
        
        # Check if modifier
        if key_name in ('ctrl', 'shift', 'alt', 'win'):
            self._modifiers.add(key_name)
            return
        
        # Build full key name with modifiers
        parts = []
        if 'ctrl' in self._modifiers:
            parts.append('ctrl')
        if 'shift' in self._modifiers:
            parts.append('shift')
        if 'alt' in self._modifiers:
            parts.append('alt')
        if 'win' in self._modifiers:
            parts.append('win')
        parts.append(key_name)
        
        full_key = '+'.join(parts)
        display = full_key.upper().replace('+', ' + ')
        
        self._emit_result(CapturedInput(
            input_type=InputType.KEYBOARD,
            key_name=full_key,
            display_name=f"⌨️ {display}"
        ))
    
    def _on_key_release(self, key):
        """Handle keyboard release"""
        key_name = self._get_key_name(key)
        if key_name in ('ctrl', 'shift', 'alt', 'win'):
            self._modifiers.discard(key_name)
    
    def _on_mouse_click(self, x, y, button, pressed):
        """Handle mouse click"""
        if not self._capturing or self._captured or not pressed:
            return
        
        if button in MOUSE_BUTTON_MAP:
            key_name, display = MOUSE_BUTTON_MAP[button]
            
            # Add modifiers
            parts = []
            if 'ctrl' in self._modifiers:
                parts.append('ctrl')
            if 'shift' in self._modifiers:
                parts.append('shift')
            if 'alt' in self._modifiers:
                parts.append('alt')
            
            if parts:
                key_name = '+'.join(parts) + '+' + key_name
                display = ' + '.join(p.upper() for p in parts) + ' + ' + display
            
            self._emit_result(CapturedInput(
                input_type=InputType.MOUSE,
                key_name=key_name,
                display_name=display
            ))
    
    def _get_key_name(self, key) -> Optional[str]:
        """Convert pynput key to string name"""
        # Special keys
        if key in SPECIAL_KEY_MAP:
            return SPECIAL_KEY_MAP[key]
        
        # Regular character keys
        if isinstance(key, KeyCode):
            if key.char:
                return key.char.lower()
            elif key.vk:
                # Virtual key code - try to get name
                vk = key.vk
                # Function keys via vk
                if 112 <= vk <= 123:
                    return f'f{vk - 111}'
                # Numpad
                if 96 <= vk <= 105:
                    return f'num{vk - 96}'
        
        return None
    
    def _emit_result(self, result: CapturedInput):
        """Emit result and stop capturing"""
        if self._captured:
            return
        
        self._captured = True
        self.stop_capture()
        
        # Use QTimer for thread safety
        QTimer.singleShot(0, lambda: self.inputCaptured.emit(result))


# XInput controller support (optional)
try:
    import XInput
    XINPUT_AVAILABLE = True
    
    XBOX_BUTTON_MAP = {
        'DPAD_UP': ('dpad_up', '🎮 D-Pad Up'),
        'DPAD_DOWN': ('dpad_down', '🎮 D-Pad Down'),
        'DPAD_LEFT': ('dpad_left', '🎮 D-Pad Left'),
        'DPAD_RIGHT': ('dpad_right', '🎮 D-Pad Right'),
        'START': ('start', '🎮 Start'),
        'BACK': ('back', '🎮 Back'),
        'LEFT_THUMB': ('left_stick', '🎮 Left Stick'),
        'RIGHT_THUMB': ('right_stick', '🎮 Right Stick'),
        'LEFT_SHOULDER': ('lb', '🎮 LB'),
        'RIGHT_SHOULDER': ('rb', '🎮 RB'),
        'A': ('xbox_a', '🎮 A'),
        'B': ('xbox_b', '🎮 B'),
        'X': ('xbox_x', '🎮 X'),
        'Y': ('xbox_y', '🎮 Y'),
    }
    
    class ControllerCapture(QObject):
        """Capture controller input"""
        
        inputCaptured = pyqtSignal(object)
        
        def __init__(self, parent=None):
            super().__init__(parent)
            self._capturing = False
            self._thread = None
            self._stop = False
        
        def start_capture(self):
            if self._capturing:
                return
            self._capturing = True
            self._stop = False
            self._thread = threading.Thread(target=self._poll, daemon=True)
            self._thread.start()
        
        def stop_capture(self):
            self._capturing = False
            self._stop = True
        
        def _poll(self):
            prev_buttons = {}
            
            while not self._stop:
                try:
                    connected = XInput.get_connected()
                    for i in range(4):
                        if connected[i]:
                            state = XInput.get_state(i)
                            buttons = XInput.get_button_values(state)
                            
                            for name, pressed in buttons.items():
                                key = f"{i}_{name}"
                                was_pressed = prev_buttons.get(key, False)
                                
                                if pressed and not was_pressed and name in XBOX_BUTTON_MAP:
                                    key_name, display = XBOX_BUTTON_MAP[name]
                                    result = CapturedInput(
                                        input_type=InputType.CONTROLLER,
                                        key_name=key_name,
                                        display_name=display
                                    )
                                    self._stop = True
                                    self._capturing = False
                                    QTimer.singleShot(0, lambda r=result: self.inputCaptured.emit(r))
                                    return
                                
                                prev_buttons[key] = pressed
                except:
                    pass
                
                time.sleep(0.02)
                
except ImportError:
    XINPUT_AVAILABLE = False
    ControllerCapture = None
