"""
Direct Input Sender - PYNPUT (Modern Method)
Menggunakan library modern pynput yang menangani abstraction input system
dengan lebih bersih dan kompatibel.
"""
import time
from pynput.keyboard import Key, Controller as KeyboardController
from pynput.mouse import Button, Controller as MouseController

# Initialize controllers
keyboard = KeyboardController()
mouse = MouseController()

class DirectInputSender:
    """Input sender wrapper menggunakan pynput"""
    
    # Mapping string key ke pynput Key object
    SPECIAL_KEYS = {
        'space': Key.space,
        'enter': Key.enter,
        'return': Key.enter,
        'esc': Key.esc,
        'escape': Key.esc,
        'backspace': Key.backspace,
        'tab': Key.tab,
        'capslock': Key.caps_lock,
        'shift': Key.shift,
        'lshift': Key.shift_l,
        'rshift': Key.shift_r,
        'ctrl': Key.ctrl,
        'lctrl': Key.ctrl_l,
        'rctrl': Key.ctrl_r,
        'alt': Key.alt,
        'lalt': Key.alt_l,
        'ralt': Key.alt_r,
        'win': Key.cmd,
        'up': Key.up,
        'down': Key.down,
        'left': Key.left,
        'right': Key.right,
        'pageup': Key.page_up,
        'pagedown': Key.page_down,
        'home': Key.home,
        'end': Key.end,
        'insert': Key.insert,
        'delete': Key.delete,
        'f1': Key.f1, 'f2': Key.f2, 'f3': Key.f3, 'f4': Key.f4,
        'f5': Key.f5, 'f6': Key.f6, 'f7': Key.f7, 'f8': Key.f8,
        'f9': Key.f9, 'f10': Key.f10, 'f11': Key.f11, 'f12': Key.f12
    }

    @staticmethod
    def _get_key_object(key_str: str):
        """Convert string key to pynput key object"""
        key_str = key_str.lower().strip()
        if '+' in key_str:
            key_str = key_str.split('+')[-1]
            
        # Check special keys first
        if key_str in DirectInputSender.SPECIAL_KEYS:
            return DirectInputSender.SPECIAL_KEYS[key_str]
            
        # Return single char
        if len(key_str) == 1:
            return key_str
            
        return None

    @staticmethod
    def key_down(key: str):
        """Tekan tombol"""
        key_obj = DirectInputSender._get_key_object(key)
        if key_obj:
            try:
                keyboard.press(key_obj)
            except Exception as e:
                print(f"Error pressing {key}: {e}")

    @staticmethod
    def key_up(key: str):
        """Lepas tombol"""
        key_obj = DirectInputSender._get_key_object(key)
        if key_obj:
            try:
                keyboard.release(key_obj)
            except Exception as e:
                print(f"Error releasing {key}: {e}")

    @staticmethod
    def press(key: str, duration: float = 0.01):
        """Tekan dan lepas dengan delay"""
        print(f"[Pynput] Pressing: {key}")
        DirectInputSender.key_down(key)
        time.sleep(duration)
        DirectInputSender.key_up(key)

    @staticmethod
    def mouse_click(button: str = 'left'):
        """Klik mouse"""
        btn = Button.left
        if button == 'right':
            btn = Button.right
        elif button == 'middle':
            btn = Button.middle
            
        mouse.press(btn)
        time.sleep(0.02)
        mouse.release(btn)
        return True
