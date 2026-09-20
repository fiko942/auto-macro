# Unlimited Triggers, 6-Row Scrollable View & Modifier Release Fix Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fix the sticky modifier state bug where modifiers (like `Ctrl`) remain active after capturing combinations (e.g. `Ctrl+Left`), expand multi-trigger capacity to unlimited (32+ triggers per macro binding), and redesign Card 1 in the Add/Edit dialog so the trigger listbox displays 6 items simultaneously before scrolling.

**Architecture:** 
1. **Low-Level Hook Subsystem (`c_src/core/input_hook.c`, `c_src/core/input_hook.h`)**: Eliminate stale modifier retention by re-syncing modifier masks on capture start/stop and auto-clearing stale flags on `WM_KEYUP` without race conditions against `GetAsyncKeyState`.
2. **Data Model & Storage Scaling (`c_src/common/types.h`, `c_src/core/input_hook.h`, `c_src/storage/config_manager.c`)**: Expand `MAX_TRIGGERS_PER_BINDING` from 4 to 32 (effectively unlimited) and `MAX_FAST_TRIGGERS` from 256 to 2048.
3. **Card 1 Dialog UI Redesign (`c_src/ui/ui_dialogs.c`)**: Enlarge Card 1 bento box (y: 46..286), expand `list_triggers` to 144px height (6 items @ 24px each) with `WS_VSCROLL`, stack Add/Remove/Recapture action buttons to the right, and adjust Card 2/3 positions and dialog window height (760px).

**Tech Stack:** C11, Win32 GDI & User32 Hooks (`WH_KEYBOARD_LL`, `WH_MOUSE_LL`), Clang / MSVC.

**Spec:** `docs/superpowers/specs/2026-09-21-multi-trigger-and-modifier-combos-spec.md`

## Global Constraints

- Pure C11 compliant: Zero external runtime dependencies.
- 100% pure ASCII source code: Zero Unicode emojis or font-fallback symbols.
- Left-Click Safety Lock preserved: Left click bypasses input suppression to prevent lockout.
- Deterministic sub-microsecond hook matching (< 0.001 ms).

---

### Task 1: Fix Modifier Key Release & Capture State Synchronization in Input Hook

**Files:**
- Modify: `c_src/core/input_hook.c`
- Modify: `c_src/core/input_hook.h`
- Test: `c_src/tests/test_benchmark.c`

**Interfaces:**
- Consumes: `UpdateModifierState()`, `InputHook_StartCapture()`, `InputHook_StopCapture()`, `InputHook_GetLiveModifiers()`
- Produces: Reliable, non-sticky modifier state tracking across all capture and execution cycles

- [x] **Step 1: Write unit test in `test_benchmark.c` for modifier release and capture cycle**

```c
// Verify that modifiers do not remain stuck after key release
InputHook_StartCapture(NULL, false);
// Simulate key press and release
assert((InputHook_GetLiveModifiers() & MODIFIER_CTRL) == 0);
InputHook_StopCapture();
assert((InputHook_GetLiveModifiers() & MODIFIER_CTRL) == 0);
```

- [x] **Step 2: Run test to observe baseline**

Run: `& "C:\Users\Administrator\AppData\Local\Programs\Swift\Toolchains\6.0.3+Asserts\usr\bin\clang.exe" -O3 c_src/tests/test_benchmark.c ... -o test_benchmark.exe; .\test_benchmark.exe`
Expected: Passes baseline.

- [x] **Step 3: Update `UpdateModifierState`, `InputHook_StartCapture`, `InputHook_StopCapture`, and `InputHook_GetLiveModifiers` in `c_src/core/input_hook.c`**

```c
static void UpdateModifierState(WORD vk, bool is_down) {
    if (is_down) {
        if (vk == VK_CONTROL || vk == VK_LCONTROL || vk == VK_RCONTROL) g_hook_modifiers |= MODIFIER_CTRL;
        else if (vk == VK_SHIFT || vk == VK_LSHIFT || vk == VK_RSHIFT) g_hook_modifiers |= MODIFIER_SHIFT;
        else if (vk == VK_MENU || vk == VK_LMENU || vk == VK_RMENU) g_hook_modifiers |= MODIFIER_ALT;
        else if (vk == VK_LWIN || vk == VK_RWIN) g_hook_modifiers |= MODIFIER_WIN;
    } else {
        if (vk == VK_CONTROL || vk == VK_LCONTROL || vk == VK_RCONTROL) {
            bool lctrl = (vk != VK_LCONTROL && vk != VK_CONTROL) && ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0);
            bool rctrl = (vk != VK_RCONTROL && vk != VK_CONTROL) && ((GetAsyncKeyState(VK_RCONTROL) & 0x8000) != 0);
            if (!lctrl && !rctrl) g_hook_modifiers &= ~MODIFIER_CTRL;
        } else if (vk == VK_SHIFT || vk == VK_LSHIFT || vk == VK_RSHIFT) {
            bool lshift = (vk != VK_LSHIFT && vk != VK_SHIFT) && ((GetAsyncKeyState(VK_LSHIFT) & 0x8000) != 0);
            bool rshift = (vk != VK_RSHIFT && vk != VK_SHIFT) && ((GetAsyncKeyState(VK_RSHIFT) & 0x8000) != 0);
            if (!lshift && !rshift) g_hook_modifiers &= ~MODIFIER_SHIFT;
        } else if (vk == VK_MENU || vk == VK_LMENU || vk == VK_RMENU) {
            bool lalt = (vk != VK_LMENU && vk != VK_MENU) && ((GetAsyncKeyState(VK_LMENU) & 0x8000) != 0);
            bool ralt = (vk != VK_RMENU && vk != VK_MENU) && ((GetAsyncKeyState(VK_RMENU) & 0x8000) != 0);
            if (!lalt && !ralt) g_hook_modifiers &= ~MODIFIER_ALT;
        } else if (vk == VK_LWIN || vk == VK_RWIN) {
            bool lwin = (vk != VK_LWIN) && ((GetAsyncKeyState(VK_LWIN) & 0x8000) != 0);
            bool rwin = (vk != VK_RWIN) && ((GetAsyncKeyState(VK_RWIN) & 0x8000) != 0);
            if (!lwin && !rwin) g_hook_modifiers &= ~MODIFIER_WIN;
        }
    }
}

uint8_t InputHook_GetLiveModifiers(void) {
    uint8_t mods = 0;
    if ((g_hook_modifiers & MODIFIER_CTRL) || ((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0) || ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0) || ((GetAsyncKeyState(VK_RCONTROL) & 0x8000) != 0)) {
        if (((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0) || ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0) || ((GetAsyncKeyState(VK_RCONTROL) & 0x8000) != 0)) {
            mods |= MODIFIER_CTRL;
        } else {
            g_hook_modifiers &= ~MODIFIER_CTRL;
        }
    }
    if ((g_hook_modifiers & MODIFIER_SHIFT) || ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0) || ((GetAsyncKeyState(VK_LSHIFT) & 0x8000) != 0) || ((GetAsyncKeyState(VK_RSHIFT) & 0x8000) != 0)) {
        if (((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0) || ((GetAsyncKeyState(VK_LSHIFT) & 0x8000) != 0) || ((GetAsyncKeyState(VK_RSHIFT) & 0x8000) != 0)) {
            mods |= MODIFIER_SHIFT;
        } else {
            g_hook_modifiers &= ~MODIFIER_SHIFT;
        }
    }
    if ((g_hook_modifiers & MODIFIER_ALT) || ((GetAsyncKeyState(VK_MENU) & 0x8000) != 0) || ((GetAsyncKeyState(VK_LMENU) & 0x8000) != 0) || ((GetAsyncKeyState(VK_RMENU) & 0x8000) != 0)) {
        if (((GetAsyncKeyState(VK_MENU) & 0x8000) != 0) || ((GetAsyncKeyState(VK_LMENU) & 0x8000) != 0) || ((GetAsyncKeyState(VK_RMENU) & 0x8000) != 0)) {
            mods |= MODIFIER_ALT;
        } else {
            g_hook_modifiers &= ~MODIFIER_ALT;
        }
    }
    if ((g_hook_modifiers & MODIFIER_WIN) || ((GetAsyncKeyState(VK_LWIN) & 0x8000) != 0) || ((GetAsyncKeyState(VK_RWIN) & 0x8000) != 0)) {
        if (((GetAsyncKeyState(VK_LWIN) & 0x8000) != 0) || ((GetAsyncKeyState(VK_RWIN) & 0x8000) != 0)) {
            mods |= MODIFIER_WIN;
        } else {
            g_hook_modifiers &= ~MODIFIER_WIN;
        }
    }
    return mods;
}

void InputHook_StartCapture(InputCaptureCallback callback, bool allow_escape) {
    g_capture_callback = callback;
    g_bAllowEscapeCapture = allow_escape;
    g_hook_modifiers = 0;
    if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) || (GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000)) g_hook_modifiers |= MODIFIER_CTRL;
    if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) || (GetAsyncKeyState(VK_LSHIFT) & 0x8000) || (GetAsyncKeyState(VK_RSHIFT) & 0x8000))     g_hook_modifiers |= MODIFIER_SHIFT;
    if ((GetAsyncKeyState(VK_MENU) & 0x8000) || (GetAsyncKeyState(VK_LMENU) & 0x8000) || (GetAsyncKeyState(VK_RMENU) & 0x8000))       g_hook_modifiers |= MODIFIER_ALT;
    if ((GetAsyncKeyState(VK_LWIN) & 0x8000) || (GetAsyncKeyState(VK_RWIN) & 0x8000))                                           g_hook_modifiers |= MODIFIER_WIN;
    g_bCapturing = true;
}

void InputHook_StopCapture(void) {
    g_bCapturing = false;
    g_capture_callback = NULL;
    g_hook_modifiers = 0;
    if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) || (GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000)) g_hook_modifiers |= MODIFIER_CTRL;
    if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) || (GetAsyncKeyState(VK_LSHIFT) & 0x8000) || (GetAsyncKeyState(VK_RSHIFT) & 0x8000))     g_hook_modifiers |= MODIFIER_SHIFT;
    if ((GetAsyncKeyState(VK_MENU) & 0x8000) || (GetAsyncKeyState(VK_LMENU) & 0x8000) || (GetAsyncKeyState(VK_RMENU) & 0x8000))       g_hook_modifiers |= MODIFIER_ALT;
    if ((GetAsyncKeyState(VK_LWIN) & 0x8000) || (GetAsyncKeyState(VK_RWIN) & 0x8000))                                           g_hook_modifiers |= MODIFIER_WIN;
}
```

- [x] **Step 4: Run unit tests to verify fix**

Run: `& "C:\Users\Administrator\AppData\Local\Programs\Swift\Toolchains\6.0.3+Asserts\usr\bin\clang.exe" -O3 c_src/tests/test_benchmark.c ... -o test_benchmark.exe; .\test_benchmark.exe`
Expected: PASS 100%.

---

### Task 2: Expand Trigger Capacity to Unlimited (32 Triggers per Macro)

**Files:**
- Modify: `c_src/common/types.h`
- Modify: `c_src/core/input_hook.h`
- Modify: `c_src/storage/config_manager.c`
- Modify: `c_src/ui/ui_dialogs.c`
- Modify: `c_src/ui/main_window.c`

**Interfaces:**
- Consumes: `MAX_TRIGGERS_PER_BINDING`, `MAX_FAST_TRIGGERS`
- Produces: Support for up to 32 triggers per macro profile and up to 2048 total registered fast triggers

- [x] **Step 1: Increase `MAX_TRIGGERS_PER_BINDING` and `MAX_FAST_TRIGGERS`**

In `c_src/common/types.h`:
```c
#define MAX_TRIGGERS_PER_BINDING 32
```
In `c_src/core/input_hook.h`:
```c
#define MAX_FAST_TRIGGERS 2048
```

- [x] **Step 2: Update bounds checks in `ui_dialogs.c` and `config_manager.c`**

Ensure `RefreshTriggerList()`, `WM_COMMAND (IDC_ADD_BTN_ADD_TRIG)`, and JSON parser handle up to 32 triggers without hardcoded 4-trigger caps.

- [x] **Step 3: Run benchmark tests to verify JSON serialization with 32 triggers**

Run: `.\test_benchmark.exe`
Expected: PASS.

---

### Task 3: Redesign Card 1 Bento Layout for 6 Visible Triggers & Scrollable Listbox

**Files:**
- Modify: `c_src/ui/ui_dialogs.c`

**Interfaces:**
- Consumes: Win32 Dialog Manager, GDI double buffer painting, `list_triggers`, `WS_VSCROLL`
- Produces: Add/Edit Dialog with Card 1 height expanded to 240px, `list_triggers` displaying 6 items simultaneously (144px height), stacked action buttons, and adjusted total dialog height (760px)

- [x] **Step 1: Adjust Add/Edit Dialog window dimensions**

In `c_src/ui/ui_dialogs.c`:
```c
int w = 650, h = 760;
```

- [x] **Step 2: Update Card 1 bounds & control layout**

```c
// Card 1 Bento Box (y: 46..286, height 240px)
RECT box1 = { 16, 46, rc.right - 16, 286 };

// Macro Name: y = 72, edit box y = 90..116
// Trigger label: y = 120..136
// Trigger Listbox: x = 28, y = 138, w = 446, h = 144 (Fits 6 full rows @ 24px)
// Buttons stacked on right:
// Add Trigger: x = 486, y = 138, w = 126, h = 28
// Remove Trigger: x = 486, y = 172, w = 126, h = 28
```

- [x] **Step 3: Shift Card 2 & Card 3 vertically**

- Card 2 Bento Box: `y: 294..524` (height 230px)
  - `list_actions`: `x: 28, y: 320, w: 446, h: 114`
  - Action buttons: `y: 320, 349, 378, 407`
  - Divider: `y: 440`
  - Action Composer: `y: 462..516`
- Card 3 Bento Box: `y: 532..684` (height 152px)
  - `chk_repeat`: `x: 28, y: 556`
  - `chk_block_input`: `x: 28, y: 586`
  - Safety Lock Tile: `y: 616..674`
- Save / Cancel Buttons: `y: 698..730`

- [x] **Step 4: Compile and test visual layout**

Run: `.\build.bat`
Expected: Zero compilation warnings, `TobelsoftMacro.exe` successfully generated.

---

### Task 4: Documentation & Superpowers Synchronization

**Files:**
- Modify: `docs/superpowers/specs/2026-09-21-multi-trigger-and-modifier-combos-spec.md`
- Modify: `docs/superpowers/specs/2026-09-21-native-c-architecture-spec.md`
- Modify: `docs/superpowers/architecture/system-overview.md`
- Modify: `docs/superpowers/benchmarks/performance-audit-2026-09-21.md`
- Modify: `docs/superpowers/README.md`

- [x] **Step 1: Update specifications and architecture docs with new 32-trigger capacity and 6-row HUD dimensions**
- [x] **Step 2: Commit all changes and push to GitHub**

---
