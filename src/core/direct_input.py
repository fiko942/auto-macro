"""
Direct Input Sender - reliable Windows input backend.

Uses SendInput on Windows for better game compatibility, with a pynput
fallback for unsupported environments or unusual keys.
"""
import os
import time
import atexit
import threading
import ctypes
from ctypes import wintypes

from pynput.keyboard import Key, Controller as KeyboardController
from pynput.mouse import Button, Controller as MouseController

# Fallback controllers
keyboard = KeyboardController()
mouse = MouseController()

if os.name == "nt":
    try:
        _winmm = ctypes.WinDLL("winmm")
        _timeBeginPeriod = _winmm.timeBeginPeriod
        _timeEndPeriod = _winmm.timeEndPeriod
        _timeBeginPeriod.argtypes = (wintypes.UINT,)
        _timeBeginPeriod.restype = wintypes.UINT
        _timeEndPeriod.argtypes = (wintypes.UINT,)
        _timeEndPeriod.restype = wintypes.UINT
        _timeBeginPeriod(1)
        atexit.register(lambda: _timeEndPeriod(1))
    except Exception:
        pass


if os.name == "nt":
    class _KEYBDINPUT(ctypes.Structure):
        _fields_ = [
            ("wVk", wintypes.WORD),
            ("wScan", wintypes.WORD),
            ("dwFlags", wintypes.DWORD),
            ("time", wintypes.DWORD),
            ("dwExtraInfo", ctypes.c_size_t),
        ]


    class _MOUSEINPUT(ctypes.Structure):
        _fields_ = [
            ("dx", wintypes.LONG),
            ("dy", wintypes.LONG),
            ("mouseData", wintypes.DWORD),
            ("dwFlags", wintypes.DWORD),
            ("time", wintypes.DWORD),
            ("dwExtraInfo", ctypes.c_size_t),
        ]


    class _HARDWAREINPUT(ctypes.Structure):
        _fields_ = [
            ("uMsg", wintypes.DWORD),
            ("wParamL", wintypes.WORD),
            ("wParamH", wintypes.WORD),
        ]


    class _INPUTUNION(ctypes.Union):
        _fields_ = [
            ("ki", _KEYBDINPUT),
            ("mi", _MOUSEINPUT),
            ("hi", _HARDWAREINPUT),
        ]


    class _INPUT(ctypes.Structure):
        _anonymous_ = ("u",)
        _fields_ = [
            ("type", wintypes.DWORD),
            ("u", _INPUTUNION),
        ]


class DirectInputSender:
    """Input sender wrapper with a serialized backend."""

    _lock = threading.RLock()
    _is_windows = os.name == "nt"
    _user32 = ctypes.windll.user32 if _is_windows else None

    # Windows SendInput constants
    _INPUT_MOUSE = 0
    _INPUT_KEYBOARD = 1
    _KEYEVENTF_EXTENDEDKEY = 0x0001
    _KEYEVENTF_KEYUP = 0x0002
    _KEYEVENTF_SCANCODE = 0x0008
    _MAPVK_VK_TO_VSC = 0

    # Better defaults for game input reliability.
    DEFAULT_PRESS_DURATION = 0.01
    DEFAULT_INTER_KEY_DELAY = 0.001

    # Virtual-key codes for the keys we use in the app.
    VK_CODES = {
        "backspace": 0x08,
        "tab": 0x09,
        "enter": 0x0D,
        "return": 0x0D,
        "esc": 0x1B,
        "escape": 0x1B,
        "space": 0x20,
        "pageup": 0x21,
        "pagedown": 0x22,
        "end": 0x23,
        "home": 0x24,
        "left": 0x25,
        "up": 0x26,
        "right": 0x27,
        "down": 0x28,
        "printscreen": 0x2C,
        "insert": 0x2D,
        "delete": 0x2E,
        "shift": 0x10,
        "lshift": 0xA0,
        "rshift": 0xA1,
        "ctrl": 0x11,
        "lctrl": 0xA2,
        "rctrl": 0xA3,
        "alt": 0x12,
        "lalt": 0xA4,
        "ralt": 0xA5,
        "win": 0x5B,
        "f1": 0x70,
        "f2": 0x71,
        "f3": 0x72,
        "f4": 0x73,
        "f5": 0x74,
        "f6": 0x75,
        "f7": 0x76,
        "f8": 0x77,
        "f9": 0x78,
        "f10": 0x79,
        "f11": 0x7A,
        "f12": 0x7B,
    }

    EXTENDED_VKS = {
        0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
        0x2D, 0x2E, 0x90, 0x91, 0xA3, 0xA5
    }

    if _is_windows:
        _KEYBDINPUT = _KEYBDINPUT
        _MOUSEINPUT = _MOUSEINPUT
        _HARDWAREINPUT = _HARDWAREINPUT
        _INPUTUNION = _INPUTUNION
        _INPUT = _INPUT
        _SendInput = _user32.SendInput
        _SendInput.argtypes = (wintypes.UINT, ctypes.POINTER(_INPUT), ctypes.c_int)
        _SendInput.restype = wintypes.UINT
        _MapVirtualKeyW = _user32.MapVirtualKeyW
        _MapVirtualKeyW.argtypes = (wintypes.UINT, wintypes.UINT)
        _MapVirtualKeyW.restype = wintypes.UINT
        _VkKeyScanW = _user32.VkKeyScanW
        _VkKeyScanW.argtypes = (wintypes.WCHAR,)
        _VkKeyScanW.restype = wintypes.SHORT

    @staticmethod
    def _normalize_key(key: str) -> str:
        key = key.lower().strip()
        if "+" in key:
            key = key.split("+")[-1]
        return key

    @classmethod
    def _resolve_vk(cls, key: str):
        """Return a Windows VK code for the key if possible."""
        key = cls._normalize_key(key)

        if key in cls.VK_CODES:
            return cls.VK_CODES[key]

        if key.startswith("num") and len(key) == 4 and key[3].isdigit():
            return 0x60 + int(key[3])

        if len(key) == 1:
            if cls._is_windows:
                vk_scan = cls._VkKeyScanW(key)
                if vk_scan != -1:
                    return vk_scan & 0xFF
            return ord(key.upper())

        return None

    @classmethod
    def _send_key_event(cls, vk: int, is_key_up: bool):
        """Send a keyboard event using SendInput."""
        if not cls._is_windows:
            return False

        scan = cls._MapVirtualKeyW(vk, cls._MAPVK_VK_TO_VSC)
        flags = cls._KEYEVENTF_SCANCODE
        if is_key_up:
            flags |= cls._KEYEVENTF_KEYUP
        if vk in cls.EXTENDED_VKS:
            flags |= cls._KEYEVENTF_EXTENDEDKEY

        inp = cls._INPUT(
            type=cls._INPUT_KEYBOARD,
            ki=cls._KEYBDINPUT(
                wVk=0,
                wScan=scan,
                dwFlags=flags,
                time=0,
                dwExtraInfo=0,
            ),
        )
        sent = cls._SendInput(1, ctypes.byref(inp), ctypes.sizeof(inp))
        return sent == 1

    @classmethod
    def _send_mouse_event(cls, button: str):
        """Send a mouse click using SendInput."""
        if not cls._is_windows:
            return False

        button = cls._normalize_key(button)
        mapping = {
            "left": (0x0002, 0x0004),
            "right": (0x0008, 0x0010),
            "middle": (0x0020, 0x0040),
        }

        if button not in mapping:
            return False

        down_flag, up_flag = mapping[button]

        def make_input(flags):
            return cls._INPUT(
                type=cls._INPUT_MOUSE,
                mi=cls._MOUSEINPUT(
                    dx=0,
                    dy=0,
                    mouseData=0,
                    dwFlags=flags,
                    time=0,
                    dwExtraInfo=0,
                ),
            )

        down = make_input(down_flag)
        up = make_input(up_flag)
        down_sent = cls._SendInput(1, ctypes.byref(down), ctypes.sizeof(down))
        up_sent = cls._SendInput(1, ctypes.byref(up), ctypes.sizeof(up))
        return down_sent == 1 and up_sent == 1

    @classmethod
    def _fallback_key_down(cls, key_obj):
        try:
            keyboard.press(key_obj)
            return True
        except Exception as exc:
            print(f"Error pressing {key_obj}: {exc}")
            return False

    @classmethod
    def _fallback_key_up(cls, key_obj):
        try:
            keyboard.release(key_obj)
            return True
        except Exception as exc:
            print(f"Error releasing {key_obj}: {exc}")
            return False

    @staticmethod
    def _pynput_key_object(key: str):
        key = key.lower().strip()
        if "+" in key:
            key = key.split("+")[-1]

        special = {
            "space": Key.space,
            "enter": Key.enter,
            "return": Key.enter,
            "esc": Key.esc,
            "escape": Key.esc,
            "backspace": Key.backspace,
            "tab": Key.tab,
            "shift": Key.shift,
            "lshift": Key.shift_l,
            "rshift": Key.shift_r,
            "ctrl": Key.ctrl,
            "lctrl": Key.ctrl_l,
            "rctrl": Key.ctrl_r,
            "alt": Key.alt,
            "lalt": Key.alt_l,
            "ralt": Key.alt_r,
            "win": Key.cmd,
            "up": Key.up,
            "down": Key.down,
            "left": Key.left,
            "right": Key.right,
            "pageup": Key.page_up,
            "pagedown": Key.page_down,
            "home": Key.home,
            "end": Key.end,
            "insert": Key.insert,
            "delete": Key.delete,
            "f1": Key.f1,
            "f2": Key.f2,
            "f3": Key.f3,
            "f4": Key.f4,
            "f5": Key.f5,
            "f6": Key.f6,
            "f7": Key.f7,
            "f8": Key.f8,
            "f9": Key.f9,
            "f10": Key.f10,
            "f11": Key.f11,
            "f12": Key.f12,
        }

        if key in special:
            return special[key]

        if len(key) == 1:
            return key

        return None

    @classmethod
    def sleep_precise(cls, seconds: float):
        """Sleep with better accuracy for short waits on Windows."""
        if seconds <= 0:
            return

        if not cls._is_windows:
            time.sleep(seconds)
            return

        target = time.perf_counter() + seconds
        # For very short waits, spin so the delay stays close to the target.
        if seconds <= 0.01:
            while time.perf_counter() < target:
                pass
            return

        while True:
            remaining = target - time.perf_counter()
            if remaining <= 0:
                return
            if remaining > 0.003:
                time.sleep(remaining - 0.001)
            else:
                time.sleep(0)

    @classmethod
    def key_down(cls, key: str):
        """Press a key without releasing it."""
        with cls._lock:
            vk = cls._resolve_vk(key)
            if vk is not None and cls._is_windows:
                if cls._send_key_event(vk, is_key_up=False):
                    return

            key_obj = cls._pynput_key_object(key)
            if key_obj is not None:
                cls._fallback_key_down(key_obj)

    @classmethod
    def key_up(cls, key: str):
        """Release a key."""
        with cls._lock:
            vk = cls._resolve_vk(key)
            if vk is not None and cls._is_windows:
                if cls._send_key_event(vk, is_key_up=True):
                    return

            key_obj = cls._pynput_key_object(key)
            if key_obj is not None:
                cls._fallback_key_up(key_obj)

    @classmethod
    def press(cls, key: str, duration: float = None):
        """Tap a key with a slightly longer hold to improve game detection."""
        if duration is None:
            duration = cls.DEFAULT_PRESS_DURATION

        print(f"[Input] Pressing: {key}")
        with cls._lock:
            cls.key_down(key)
            cls.sleep_precise(duration)
            cls.key_up(key)

    @classmethod
    def mouse_click(cls, button: str = "left"):
        """Click a mouse button."""
        button = button.lower().strip()
        with cls._lock:
            if cls._is_windows and cls._send_mouse_event(button):
                return True

            btn = Button.left
            if button == "right":
                btn = Button.right
            elif button == "middle":
                btn = Button.middle

            try:
                mouse.press(btn)
                cls.sleep_precise(0.005)
                mouse.release(btn)
                return True
            except Exception as exc:
                print(f"Error clicking {button}: {exc}")
                return False
