# DirectInput Macro Engine & Low-Level Hook Specification

**Status:** Completed & Approved  
**Date:** 2026-09-21  
**Module:** `c_src/core/` (`direct_input.c`, `input_hook.c`, `input_sender.c`, `macro_engine.c`)  

---

## 1. Overview & Objectives

Traditional Windows macro tools rely on standard Virtual-Key (`VK_...`) injection and high-level window messaging (`WM_KEYDOWN`). Modern anti-cheat systems and DirectX/Vulkan game engines read directly from DirectInput / Raw Input ring buffers, bypassing virtual key messages entirely.

The **Tobelsoft DirectInput Macro Engine** directly drives the kernel input subsystem by translating logical virtual keys and user triggers into hardware scan codes (`Set 1 / Set 2 PS/2 & USB HID scancodes`), injecting them via `KEYEVENTF_SCANCODE`.

---

## 2. Low-Level Hook Architecture (`WH_KEYBOARD_LL` & `WH_MOUSE_LL`)

### 2.1 Hook Registration
Hooks are installed on the dedicated UI/Input thread:
```c
HHOOK g_kbd_hook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandleW(NULL), 0);
HHOOK g_mouse_hook = SetWindowsHookExW(WH_MOUSE_LL, LowLevelMouseProc, GetModuleHandleW(NULL), 0);
```

### 2.2 Callback Execution & Multi-Trigger Evaluation Loop
The hook procedure executes in user space before the active application receives the message:

```c
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kbd = (KBDLLHOOKSTRUCT*)lParam;
        
        // Check for injected synthetic events to prevent infinite feedback loops
        if (kbd->flags & LLKHF_INJECTED) {
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        }

        DWORD vk = kbd->vkCode;
        bool is_down = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
        bool is_up = (wParam == WM_KEYUP || wParam == WM_SYSKEYUP);

        // Update atomic modifier tracking state
        UpdateModifierState((WORD)vk, is_down);

        // Modal Capture Interception Mode
        if (g_bCapturing) {
            if (vk == VK_ESCAPE && is_down) {
                InputHook_StopCapture();
                if (g_capture_hwnd) PostMessageW(g_capture_hwnd, WM_USER + 101, 0, 0);
                return 1;
            }
            if (vk == VK_LWIN || vk == VK_RWIN) {
                // Suppress Windows Start menu popup during capture
                return 1;
            }
            if (is_down && !IsModifierKey((WORD)vk)) {
                uint8_t mods = InputHook_GetLiveModifiers();
                char combo[64];
                BuildComboString(mods, vk, 0, combo, sizeof(combo));
                InputHook_StopCapture();
                if (g_capture_cb) g_capture_cb(combo);
                return 1;
            }
            return (vk == VK_LWIN || vk == VK_RWIN) ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam);
        }

        // Emergency Kill-Switch Evaluation
        if (vk == VK_ESCAPE && is_down) {
            EmergencyStopAllMacros();
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        }

        // Multi-Trigger Match Evaluation
        uint8_t current_mods = InputHook_GetLiveModifiers();
        MacroItem* matched = FindActiveMacroByTrigger(TRIGGER_TYPE_KEYBOARD, vk, 0, current_mods);
        if (matched) {
            if (is_down && !matched->is_executing) {
                TriggerMacroAsync(matched);
            }
            if (matched->suppress_original_input) {
                return 1; // Suppress original keystroke from reaching OS
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
```

---

## 3. DirectInput Hardware Scancode Injection

### 3.1 Scancode Translation Table
The dispatcher converts Virtual Key codes into hardware scan codes dynamically via Windows API:
```c
WORD scan_code = (WORD)MapVirtualKeyExW(vk_code, MAPVK_VK_TO_VSC_EX, GetKeyboardLayout(0));
```

### 3.2 Extended Key Handling
Certain keys (Navigation arrows, NumPad Enter, Ins, Del, Home, End, PageUp, PageDown) require the `KEYEVENTF_EXTENDEDKEY` flag to distinguish them from standard primary keys.

```c
static bool IsExtendedKey(DWORD vk) {
    switch (vk) {
        case VK_LEFT: case VK_RIGHT: case VK_UP: case VK_DOWN:
        case VK_INSERT: case VK_DELETE: case VK_HOME: case VK_END:
        case VK_PRIOR: case VK_NEXT: case VK_RCONTROL: case VK_RMENU:
            return true;
        default:
            return false;
    }
}
```

### 3.3 Hardware Dispatch Routine
```c
void DirectInput_SendKeyPress(DWORD vk, DWORD hold_ms) {
    INPUT inputs[2] = {0};
    WORD scan = (WORD)MapVirtualKeyExW(vk, MAPVK_VK_TO_VSC_EX, GetKeyboardLayout(0));
    DWORD flags = KEYEVENTF_SCANCODE | (IsExtendedKey(vk) ? KEYEVENTF_EXTENDEDKEY : 0);

    // Key Down
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wScan = scan;
    inputs[0].ki.dwFlags = flags;

    // Key Up
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wScan = scan;
    inputs[1].ki.dwFlags = flags | KEYEVENTF_KEYUP;

    if (hold_ms == 0) {
        SendInput(2, inputs, sizeof(INPUT));
    } else {
        SendInput(1, &inputs[0], sizeof(INPUT));
        HighPrecisionSleep(hold_ms);
        SendInput(1, &inputs[1], sizeof(INPUT));
    }
}
```

---

## 4. High-Precision Timing Engine

### 4.1 Hybrid Spinlock & Multimedia Timer
Windows default `Sleep()` has a standard quantum of 15.6 ms. The macro engine enforces sub-millisecond precision:

1. **Resolution Request:** `timeBeginPeriod(1)` sets the OS scheduling tick to 1.0 ms.
2. **High-Resolution QPC Timer:**
```c
void HighPrecisionSleep(double milliseconds) {
    LARGE_INTEGER freq, start, cur;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    double target_ticks = (milliseconds / 1000.0) * (double)freq.QuadPart;
    
    // For delays >= 2ms, use coarse sleep first to yield CPU
    if (milliseconds > 2.0) {
        Sleep((DWORD)(milliseconds - 1.5));
    }

    // Spin-wait for remaining sub-millisecond portion
    do {
        QueryPerformanceCounter(&cur);
    } while ((double)(cur.QuadPart - start.QuadPart) < target_ticks);
}
```

---

## 5. Left-Click Safety Guard

To prevent accidental locking of the mouse during automated macro triggers:
- Any macro mapped to Left Mouse Button (`VK_LBUTTON` / `MOUSE_TRIGGER_LCLICK`) cannot enable input suppression (`suppress_input = false` forced).
- A minimum cooldown safety threshold of 10 ms is enforced on mouse down-up sequences.
- Pressing `VK_ESCAPE` immediately terminates all mouse injection threads and releases synthetic button holds.
