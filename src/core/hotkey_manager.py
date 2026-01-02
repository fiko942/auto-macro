"""
Hotkey Manager - Core logic untuk gaming hotkey automation
Supports keyboard AND mouse triggers
Uses PYNPUT for both Listener (Input Detection) and Actor (Input sending)
"""
import time
import threading
import ctypes
from dataclasses import dataclass, field
from typing import List, Optional, Callable, Set, Dict
from enum import Enum
from pynput import keyboard, mouse
from pynput.keyboard import Key, KeyCode

from src.core.direct_input import DirectInputSender

# VK Constants for manual mapping if needed
VK_MAP = {
    0x08: 'backspace', 0x09: 'tab', 0x0D: 'enter', 0x1B: 'esc', 0x20: 'space',
    0x21: 'pageup', 0x22: 'pagedown', 0x23: 'end', 0x24: 'home',
    0x25: 'left', 0x26: 'up', 0x27: 'right', 0x28: 'down',
    0x2C: 'printscreen', 0x2D: 'insert', 0x2E: 'delete',
    0x70: 'f1', 0x71: 'f2', 0x72: 'f3', 0x73: 'f4', 0x74: 'f5', 0x75: 'f6',
    0x76: 'f7', 0x77: 'f8', 0x78: 'f9', 0x79: 'f10', 0x7A: 'f11', 0x7B: 'f12',
    0xA0: 'shift', 0xA1: 'shift', 0xA2: 'ctrl', 0xA3: 'ctrl', 
    0xA4: 'alt', 0xA5: 'alt'
}


class ActionType(Enum):
    """Tipe aksi yang bisa dilakukan"""
    KEY_PRESS = "key_press"      # Tekan dan lepas
    KEY_DOWN = "key_down"        # Hanya tekan (tidak lepas)
    KEY_UP = "key_up"            # Hanya lepas
    KEY_HOLD = "key_hold"        # Tahan selama durasi
    KEY_SEQUENCE = "key_sequence"  # Urutan tombol
    DELAY = "delay"              # Tunggu
    

@dataclass
class KeyAction:
    """Single action dalam sequence"""
    action_type: ActionType
    keys: List[str] = field(default_factory=list)
    duration: int = 0
    
    def to_dict(self) -> dict:
        return {
            "action_type": self.action_type.value,
            "keys": self.keys,
            "duration": self.duration
        }
    
    @classmethod
    def from_dict(cls, data: dict) -> 'KeyAction':
        return cls(
            action_type=ActionType(data["action_type"]),
            keys=data.get("keys", []),
            duration=data.get("duration", 0)
        )


@dataclass
class HotkeyBinding:
    """Single hotkey binding dengan multiple triggers"""
    id: str
    name: str
    trigger_keys: List[str] = field(default_factory=list)
    actions: List[KeyAction] = field(default_factory=list)
    enabled: bool = True
    repeat: bool = False
    repeat_delay: int = 100
    block_input: bool = False
    
    def to_dict(self) -> dict:
        return {
            "id": self.id,
            "name": self.name,
            "trigger_keys": self.trigger_keys,
            "actions": [a.to_dict() for a in self.actions],
            "enabled": self.enabled,
            "repeat": self.repeat,
            "repeat_delay": self.repeat_delay,
            "block_input": self.block_input
        }
    
    @classmethod
    def from_dict(cls, data: dict) -> 'HotkeyBinding':
        trigger_keys = data.get("trigger_keys", [])
        if not trigger_keys and "trigger_key" in data:
            trigger_keys = [data["trigger_key"]]
        
        return cls(
            id=data["id"],
            name=data["name"],
            trigger_keys=trigger_keys,
            actions=[KeyAction.from_dict(a) for a in data.get("actions", [])],
            enabled=data.get("enabled", True),
            repeat=data.get("repeat", False),
            repeat_delay=data.get("repeat_delay", 100),
            block_input=data.get("block_input", False)
        )


class HotkeyManager:
    """Manager untuk semua hotkey bindings - supports keyboard and mouse triggers via PYNPUT"""
    
    def __init__(self):
        self.bindings: List[HotkeyBinding] = []
        self._active = False
        self.on_status_changed: Optional[Callable[[bool], None]] = None
        self.on_binding_triggered: Optional[Callable[[HotkeyBinding], None]] = None
        
        # Modifier tracking
        self._pressed_keys: Set[str] = set()
        self._pressed_mouse: Set[str] = set()
        
        # Master Toggle Keys
        self.master_trigger_keys: List[str] = []
        
        # Pynput Listeners
        self._keyboard_listener = None
        self._mouse_listener = None
        
        # Repeat logic
        self._repeat_threads = {}
        self._stop_repeat = {}

    def _stop_listeners(self):
        """Stop low-level listeners"""
        if self._keyboard_listener:
            try: self._keyboard_listener.stop()
            except: pass
            self._keyboard_listener = None
            
        if self._mouse_listener:
            try: self._mouse_listener.stop()
            except: pass
            self._mouse_listener = None

    def restart_service(self):
        """Restart background listeners (force refresh context)"""
        print("[DEBUG] Restarting Input Service...")
        self._stop_listeners()
        # Brief pause to ensure OS unhooks
        time.sleep(0.1)
        self.start_listeners()

    def start_listeners(self):
        """Start listeners (Background service)"""
        # Only start if not already running to prevent duplication
        if self._keyboard_listener and self._keyboard_listener.running:
             return

        # NOTE: win32_event_filter only works on Windows and allows us to BLOCK input
        self._keyboard_listener = keyboard.Listener(
            on_press=self._on_key_press,
            on_release=self._on_key_release,
            win32_event_filter=self._win32_event_filter) # Hook for blocking
            
        self._keyboard_listener.start()
        
        self._mouse_listener = mouse.Listener(
            on_click=self._on_mouse_click)
        self._mouse_listener.start()
        
        print("[DEBUG] Pynput Listeners started (Service).")

    def start(self):
        """Aktifkan listeners & logic macro"""
        self.start_listeners() # Ensure listeners are running
        
        if self._active:
            return
        
        print("[DEBUG] HotkeyManager activating...")
        self._active = True
        
        # Reset tracking
        self._pressed_keys.clear()
        self._pressed_mouse.clear()
        
        if self.on_status_changed:
            self.on_status_changed(True)
    
    def stop(self):
        """Matikan/Pause macro execution (Listeners tetap jalan untuk detect toggle)"""
        print("[DEBUG] HotkeyManager stopping (pausing macros)...")
        self._active = False
        
        # Stop repeat threads
        for binding_id in self._stop_repeat:
            self._stop_repeat[binding_id] = True
            
        # Notify UI
        if self.on_status_changed:
            self.on_status_changed(False)
            
    def set_master_triggers(self, keys: List[str]):
        """Set key yang digunakan untuk toggle On/Off global"""
        self.master_trigger_keys = keys
        print(f"[DEBUG] Master triggers set: {keys}")
        # Force restart service to ensure new keys are picked up by the hook
        self.restart_service()
            


    def _vk_to_str(self, vk: int) -> Optional[str]:
        """Convert Virtual Key code to string"""
        if vk in VK_MAP:
            return VK_MAP[vk]
        
        # Try character
        try:
            k = KeyCode.from_vk(vk)
            if k.char:
                return k.char.lower()
        except:
            pass
        
        # Fallback for some common alphanumeric
        if 48 <= vk <= 57: # 0-9
            return chr(vk)
        if 65 <= vk <= 90: # A-Z
            return chr(vk).lower()
            
        return None

    def _win32_event_filter(self, msg, data):
        """
        Low-level hook filter to allow blocking input.
        Returns False to suppress event, None to allow.
        """
        # WM_KEYDOWN=0x0100, WM_SYSKEYDOWN=0x0104
        if msg not in (0x0100, 0x0104):
            return None
            
        vk = data.vkCode
        key_name = self._vk_to_str(vk)
        
        if not key_name:
            return None
            
        # Get physical modifier state
        user32 = ctypes.windll.user32
        mods = []
        if user32.GetAsyncKeyState(0x11) & 0x8000: mods.append('ctrl')  # VK_CONTROL
        if user32.GetAsyncKeyState(0x10) & 0x8000: mods.append('shift') # VK_SHIFT
        if user32.GetAsyncKeyState(0x12) & 0x8000: mods.append('alt')   # VK_MENU
        
        # Build combo
        if key_name in mods:
            combo = key_name
        else:
            if mods:
                combo = '+'.join(mods) + '+' + key_name
            else:
                combo = key_name
                
        # DEBUG: Print what we see
        print(f"[FILTER] Combo: {combo}, Masters: {self.master_trigger_keys}")
                
        # 1. CHECK MASTER TOGGLE (Highest Priority)
        # Check if matched ANY master trigger
        if combo in self.master_trigger_keys:
            # Toggle Active State
            self._active = not self._active
            print(f"[Master] Toggle Active State -> {self._active}")
            if self.on_status_changed:
                self.on_status_changed(self._active)
            return False # Consume/Block the toggle key
            
        # If macros are paused, no further processing (except master toggle above)
        if not self._active:
            return None
                
        # Check matching blocking bindings
        for binding in self.bindings:
            if not binding.enabled or not binding.block_input:
                continue
            
            # Check exact match
            if combo in binding.trigger_keys:
                # Found a blocking binding!
                # Execute it (in thread to avoid blocking hook)
                print(f"[BlockInput] Blocked original input for: {combo}")
                self._execute_binding(binding)
                return False # BLOCK
        
        return None  # ALLOW

    def _get_key_name(self, key):
        """Normalisasi nama key dari pynput object"""
        if hasattr(key, 'char') and key.char:
            return key.char.lower()
        if hasattr(key, 'name'):
            return key.name.lower()
        # Fallback for weird keys
        return str(key).replace('Key.', '')

    def _build_current_combo(self, trigger_key_name: str) -> str:
        """Bangun combo string seperti 'ctrl+mouse_left'"""
        modifiers = []
        # Urutan standar: ctrl -> shift -> alt
        if 'ctrl' in self._pressed_keys or 'ctrl_l' in self._pressed_keys or 'ctrl_r' in self._pressed_keys:
            modifiers.append('ctrl')
        if 'shift' in self._pressed_keys or 'shift_l' in self._pressed_keys or 'shift_r' in self._pressed_keys:
            modifiers.append('shift')
        if 'alt' in self._pressed_keys or 'alt_l' in self._pressed_keys or 'alt_r' in self._pressed_keys:
            modifiers.append('alt')
            
        # Hindari duplikasi jika trigger key adalah modifier itu sendiri
        if trigger_key_name in modifiers:
            return trigger_key_name
            
        if modifiers:
            return '+'.join(modifiers) + '+' + trigger_key_name
        return trigger_key_name

    def _check_bindings(self, trigger: str):
        """Check apakah trigger match dengan binding apapun"""
            
        # Fallback check for Master Toggle (in case win32 filter didn't catch it / mouse trigger)
        if trigger in self.master_trigger_keys:
             self._active = not self._active
             print(f"[Master-Check] Toggle Active State -> {self._active}")
             if self.on_status_changed:
                self.on_status_changed(self._active)
             return
             
        if not self._active:
            return
            
        print(f"[DEBUG] Trigger detected: {trigger}")
        
        for binding in self.bindings:
            if not binding.enabled:
                continue
            
            # If blocking is enabled, we already handled it in win32_event_filter!
            # So we should SKIP it here to prevent double execution.
            if binding.block_input:
                continue
            
            # 1. Exact Match
            if trigger in binding.trigger_keys:
                print(f"[DEBUG] EXECUTE: {binding.name} (Trigger: {trigger})")
                self._execute_binding(binding)
                return
            
            # 2. Loose Match for Mouse (e.g. trigger 'ctrl+mouse_left' matches 'mouse_left' binding)
            if 'mouse_' in trigger:
                base_mouse = trigger.split('+')[-1] # Get 'mouse_left' from 'ctrl+mouse_left'
                if base_mouse in binding.trigger_keys:
                     print(f"[DEBUG] EXECUTE (Loose): {binding.name} (Base: {base_mouse})")
                     self._execute_binding(binding)
                     return

    def _on_key_press(self, key):
        try:
            key_name = self._get_key_name(key)
            self._pressed_keys.add(key_name)
            
            # Build trigger combo
            combo = self._build_current_combo(key_name)
            self._check_bindings(combo)
            
        except AttributeError:
            pass

    def _on_key_release(self, key):
        try:
            key_name = self._get_key_name(key)
            self._pressed_keys.discard(key_name)
            
            # Handle modifiers cleanup (ctrl_l vs ctrl)
            if 'ctrl' in key_name: self._pressed_keys.discard('ctrl') 
            if 'shift' in key_name: self._pressed_keys.discard('shift')
            if 'alt' in key_name: self._pressed_keys.discard('alt')
            
        except AttributeError:
            pass

    def _on_mouse_click(self, x, y, button, pressed):
        btn_name = f"mouse_{button.name}" # mouse_left, mouse_right
        
        if pressed:
            self._pressed_mouse.add(btn_name)
            combo = self._build_current_combo(btn_name)
            self._check_bindings(combo)
        else:
            self._pressed_mouse.discard(btn_name)

    def _execute_binding(self, binding: HotkeyBinding):
        """Execute binding actions"""
        def run_in_thread():
            if self.on_binding_triggered:
                self.on_binding_triggered(binding)
            
            if binding.repeat:
                self._start_repeat(binding)
            else:
                self._execute_actions(binding.actions)
        
        threading.Thread(target=run_in_thread, daemon=True).start()
    
    def _execute_actions(self, actions: List[KeyAction]):
        """Execute logic menggunakan DirectInputSender (Pynput based)"""
        for action in actions:
            if action.action_type == ActionType.KEY_PRESS:
                for key in action.keys:
                    if 'mouse_' in key:
                         btn = key.replace('mouse_', '')
                         DirectInputSender.mouse_click(btn)
                    else:
                        DirectInputSender.press(key)
            elif action.action_type == ActionType.DELAY:
                time.sleep(action.duration / 1000.0)
            # Add other action types as needed (key_hold, etc) similar to before
            # For brevity & PB focus, key_press & delay are primary

    def _start_repeat(self, binding: HotkeyBinding):
        # ... logic repeat sama ...
        self._stop_repeat[binding.id] = False
        def repeat_loop():
            while not self._stop_repeat.get(binding.id, True):
                # Is Trigger Still Pressed?
                # Check pynput active keys
                # This is harder with pynput (no easy is_pressed), so we rely on _pressed_keys set
                is_pressed = False
                for t in binding.trigger_keys:
                    if 'mouse_' in t:
                        if t.split('+')[-1] in self._pressed_mouse:
                            is_pressed = True
                    else:
                        if t in self._pressed_keys:
                            is_pressed = True
                
                if not is_pressed:
                     self._stop_repeat[binding.id] = True
                     break
                
                self._execute_actions(binding.actions)
                time.sleep(binding.repeat_delay / 1000.0)
                
        t = threading.Thread(target=repeat_loop, daemon=True)
        self._repeat_threads[binding.id] = t
        t.start()
        
    def add_binding(self, binding: HotkeyBinding):
        # Prevent duplicate bindings
        self.bindings = [b for b in self.bindings if b.id != binding.id]
        self.bindings.append(binding)
        # Pynput doesn't need explicit register like 'keyboard' lib
        return True
    
    def update_binding(self, binding: HotkeyBinding):
        self.add_binding(binding)
        
    def remove_binding(self, binding_id: str):
        self.bindings = [b for b in self.bindings if b.id != binding_id]
        
    def toggle_binding(self, binding_id: str, enabled: bool):
        binding = self.get_binding(binding_id)
        if binding:
            binding.enabled = enabled
        
    def get_binding(self, binding_id: str) -> Optional[HotkeyBinding]:
        for b in self.bindings:
            if b.id == binding_id:
                return b
        return None
    
    def clear_bindings(self):
        self.bindings.clear()

    def to_dict(self) -> dict:
        return {"bindings": [b.to_dict() for b in self.bindings]}
    
    def from_dict(self, data: dict):
        self.stop()
        self.bindings = [HotkeyBinding.from_dict(b) for b in data.get("bindings", [])]

    @property
    def is_active(self) -> bool:
        return self._active
