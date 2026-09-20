# Multi-Trigger & Hardware Modifier Combo Engine Specification

**Status:** Completed & Approved  
**Date:** 2026-09-21  
**Module:** `c_src/core/input_hook.c`, `c_src/core/input_hook.h`, `c_src/ui/ui_dialogs.c`, `c_src/ui/main_window.c`  
**Target:** Win32 Native C11 (`WH_KEYBOARD_LL`, `WH_MOUSE_LL`, GDI Double-Buffered HUD)  

---

## 1. Overview & Objectives

Modern gaming and professional automation workflows require complex key combinations (e.g. `Ctrl + Left Click`, `Win + Left Click`, `Ctrl + Alt + X`) and multiple independent triggers routing to a single macro routine (e.g., triggering a recoil compensation macro from either `Mouse Left` or `NumPad 1`).

Standard Windows messaging cannot capture combinations involving the Windows Key (`VK_LWIN`/`VK_RWIN`) without opening the Windows Start Menu, and asynchronous key state polling (`GetAsyncKeyState`) exhibits race conditions against low-level hook threads.

This specification defines:
1. **Atomic Modifier Bitmask State Tracking** inside kernel hook callbacks.
2. **Real-Time Live HUD Capture Synchronization** with zero Start Menu focus stealing.
3. **Compound Multi-Trigger Data Structures** supporting up to 8 concurrent activation vectors per macro profile.
4. **Grammar & Lexical Parser** for mouse buttons and keyboard triggers combined with arbitrary modifier sequences.

---

## 2. Multi-Trigger Architecture & Data Model

### 2.1 Multi-Trigger Data Structures (`c_src/common/types.h`)
Each `HotkeyBinding` encapsulates an array of up to `MAX_TRIGGERS_PER_BINDING` (4) independent trigger key combinations, while the master application trigger supports up to `MAX_MASTER_TRIGGERS` (8):

```c
#define MAX_BINDINGS 64
#define MAX_TRIGGERS_PER_BINDING 4
#define MAX_MASTER_TRIGGERS 8
#define MAX_KEY_NAME_LEN 32

typedef struct {
    char id[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    
    // Multi-Trigger Array
    char trigger_keys[MAX_TRIGGERS_PER_BINDING][MAX_KEY_NAME_LEN];
    int trigger_count;
    
    KeyAction actions[MAX_ACTIONS_PER_BINDING];
    int action_count;
    bool enabled;
    bool repeat;
    int repeat_delay;         // In milliseconds
    bool block_input;         // Selective input suppression
} HotkeyBinding;

typedef struct {
    HotkeyBinding bindings[MAX_BINDINGS];
    int binding_count;
    char master_triggers[MAX_MASTER_TRIGGERS][MAX_KEY_NAME_LEN];
    int master_trigger_count;
    bool active;
} AppConfig;
```

---

## 3. Atomic Modifier Tracking & Hook Mechanics

### 3.1 Modifier Bitmask Constants
```c
#define MODIFIER_NONE  0x00
#define MODIFIER_CTRL  0x01
#define MODIFIER_SHIFT 0x02
#define MODIFIER_ALT   0x04
#define MODIFIER_WIN   0x08
```

### 3.2 Atomic State Synchronization in `LowLevelKeyboardProc`
To eliminate race conditions between the low-level hook thread and asynchronous UI polling, an atomic volatile variable `g_hook_modifiers` is updated immediately inside `LowLevelKeyboardProc`:

```c
static volatile uint8_t g_hook_modifiers = 0;

static void UpdateModifierState(WORD vk, bool is_down) {
    uint8_t flag = 0;
    if (vk == VK_CONTROL || vk == VK_LCONTROL || vk == VK_RCONTROL) flag = MODIFIER_CTRL;
    else if (vk == VK_SHIFT || vk == VK_LSHIFT || vk == VK_RSHIFT) flag = MODIFIER_SHIFT;
    else if (vk == VK_MENU || vk == VK_LMENU || vk == VK_RMENU) flag = MODIFIER_ALT;
    else if (vk == VK_LWIN || vk == VK_RWIN) flag = MODIFIER_WIN;

    if (flag) {
        if (is_down) {
            g_hook_modifiers |= flag;
        } else {
            // Verify with GetAsyncKeyState to ensure key is completely released
            bool still_down = false;
            if (flag == MODIFIER_CTRL) {
                still_down = ((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0) ||
                             ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0) ||
                             ((GetAsyncKeyState(VK_RCONTROL) & 0x8000) != 0);
            } else if (flag == MODIFIER_SHIFT) {
                still_down = ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0) ||
                             ((GetAsyncKeyState(VK_LSHIFT) & 0x8000) != 0) ||
                             ((GetAsyncKeyState(VK_RSHIFT) & 0x8000) != 0);
            } else if (flag == MODIFIER_ALT) {
                still_down = ((GetAsyncKeyState(VK_MENU) & 0x8000) != 0) ||
                             ((GetAsyncKeyState(VK_LMENU) & 0x8000) != 0) ||
                             ((GetAsyncKeyState(VK_RMENU) & 0x8000) != 0);
            } else if (flag == MODIFIER_WIN) {
                still_down = ((GetAsyncKeyState(VK_LWIN) & 0x8000) != 0) ||
                             ((GetAsyncKeyState(VK_RWIN) & 0x8000) != 0);
            }
            if (!still_down) {
                g_hook_modifiers &= ~flag;
            }
        }
    }
}
```

### 3.3 Real-Time Live Modifier Query API
External UI dialogs and timers query the instantaneous union of hook and hardware key states via:
```c
uint8_t InputHook_GetLiveModifiers(void) {
    uint8_t mods = g_hook_modifiers;
    if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) || (GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000)) mods |= MODIFIER_CTRL;
    if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) || (GetAsyncKeyState(VK_LSHIFT) & 0x8000) || (GetAsyncKeyState(VK_RSHIFT) & 0x8000))   mods |= MODIFIER_SHIFT;
    if ((GetAsyncKeyState(VK_MENU) & 0x8000) || (GetAsyncKeyState(VK_LMENU) & 0x8000) || (GetAsyncKeyState(VK_RMENU) & 0x8000))    mods |= MODIFIER_ALT;
    if ((GetAsyncKeyState(VK_LWIN) & 0x8000) || (GetAsyncKeyState(VK_RWIN) & 0x8000)) mods |= MODIFIER_WIN;
    return mods;
}
```

---

## 4. Windows Key Suppression & Modal Capture Isolation

### 4.1 Start Menu Interception
When the interactive DirectInput Capture HUD is active (`g_bCapturing == true`):
- `VK_LWIN` and `VK_RWIN` key-down and key-up messages return `1` to Windows OS, suppressing the default Start Menu invocation.
- Modifiers are accumulated into `g_hook_modifiers`.
- The capture modal window is notified (`WM_USER + 102`) to trigger an immediate GDI repaint of the live modifier badge chips.

### 4.2 Escape Cancellation
Pressing `VK_ESCAPE` during interactive capture immediately aborts the modal without writing changes:
```c
if (vk == VK_ESCAPE) {
    InputHook_StopCapture();
    if (g_capture_hwnd) {
        PostMessageW(g_capture_hwnd, WM_USER + 101, 0, 0); // Trigger Cancel
    }
    return 1;
}
```

---

## 5. String Grammar & Lexical Parsing Engine

The `ParseTriggerString()` function supports flexible human-readable and canonical formats:

### 5.1 Grammar Rules
```
<TriggerCombo> ::= [<ModifierPrefix> "+"] <BaseKeyOrMouse>
<ModifierPrefix> ::= <Modifier> ["+" <Modifier>]*
<Modifier>      ::= "ctrl" | "lctrl" | "rctrl" | "control"
                  | "shift" | "lshift" | "rshift"
                  | "alt" | "lalt" | "ralt" | "menu"
                  | "win" | "lwin" | "rwin" | "super"
<BaseKeyOrMouse>::= <MouseToken> | <KeyToken>
<MouseToken>    ::= "mouse_left" | "left" | "click"
                  | "mouse_right" | "right" | "rclick"
                  | "mouse_middle" | "middle" | "mclick"
                  | "mouse_x1" | "x1"
                  | "mouse_x2" | "x2"
<KeyToken>      ::= "a".."z" | "0".."9" | "f1".."f24" | "space" | "return" | ...
```

### 5.2 Canonical Normalization Table

| Input String | Parsed Type | Modifiers Bitmask | Base VK / Button | Canonical String |
|---|---|---|---|---|
| `ctrl + left click` | `TRIGGER_TYPE_MOUSE` | `MODIFIER_CTRL` | `Button 1` | `ctrl+mouse_left` |
| `win + left` | `TRIGGER_TYPE_KEYBOARD` | `MODIFIER_WIN` | `VK_LEFT` | `win+left` |
| `ctrl + alt + left click` | `TRIGGER_TYPE_MOUSE` | `MODIFIER_CTRL \| MODIFIER_ALT` | `Button 1` | `ctrl+alt+mouse_left` |
| `shift + f5` | `TRIGGER_TYPE_KEYBOARD` | `MODIFIER_SHIFT` | `VK_F5` | `shift+f5` |
| `mouse_x1` | `TRIGGER_TYPE_MOUSE` | `MODIFIER_NONE` | `Button 4` | `mouse_x1` |

---

## 6. UI & Interactive Visual Feedback

### 6.1 Real-Time HUD Modifier Chips
When the capture modal is active:
- A 30ms timer (`SetTimer(hwnd, 1, 30, NULL)`) polls `InputHook_GetLiveModifiers()`.
- Active modifiers are drawn as highlighted vector badges:
  - `[ CTRL ]` in Cyan `#00F0FF`
  - `[ ALT ]` in Amber `#FFA500`
  - `[ SHIFT ]` in Violet `#BD00FF`
  - `[ WIN ]` in Cyber Blue `#0078D7`
- Inactive modifiers or pending keys display `[ PRESS KEY / MOUSE ]` in muted Gray `#5A6275`.

### 6.2 Bento Card 1: Multi-Trigger List & Dynamic Management
- **Listbox Height:** Expanded to 58px with `WS_VSCROLL | LBS_OWNERDRAWFIXED | LBS_NOTIFY`.
- **Owner-Drawn Rendering:** Custom `WM_DRAWITEM` draws:
  - `ICON_TARGET` vector icon.
  - Numbered trigger index badge (`Trigger #1: ctrl+mouse_left`, `Trigger #2: numpad1`).
  - Active selection background (`#00F0FF` cyan border, `#0A2540` fill).
- **Controls:**
  - `+ Add Trigger` button: Appends up to 8 triggers with automatic duplication prevention.
  - `- Remove Trigger` button: Deletes selected trigger index while keeping at least one trigger active.
  - `[ Re-Capture ]` button: Launches interactive HUD for the selected trigger.

### 6.3 Main Dashboard Bento Card Display
The primary macro card renders all configured triggers separated by high-contrast `/` badges:
- `TRIGGER: [ ctrl+mouse_left ] / [ numpad1 ] / [ win+left ]`
- Green `ACTIVE` status badge with Left-Click safety shield indicator when mouse triggers are present.
