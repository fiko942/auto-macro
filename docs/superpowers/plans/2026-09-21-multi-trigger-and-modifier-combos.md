# Multi-Trigger Support & Robust Modifier Combo Capture Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [x]`) syntax for tracking.

**Goal:** Fix multi-trigger management and modifier combo capture (`Ctrl+Left Click`, `Win+Left Click`, `Ctrl+Alt+Left Click`, keyboard combos) so macros seamlessly support multiple concurrent activation keys with live interactive HUD preview and zero OS interference.

**Architecture:** 
1. **Low-Level Hook Subsystem (`c_src/core/input_hook.c`)**: Introduce explicit atomic modifier tracking (`g_hook_modifiers`) synchronized with `WH_KEYBOARD_LL` and `WH_MOUSE_LL`, suppress bare Windows key (`VK_LWIN`/`VK_RWIN`) during capture to prevent Start Menu focus stealing, and support all modifier-mouse and modifier-key combos in `ParseTriggerString` and `BuildComboString`.
2. **Interactive Capture HUD Modal (`c_src/ui/ui_dialogs.c`)**: Real-time live HUD preview of active modifier chips (`[ CTRL ]`, `[ WIN ]`, `[ ALT ]`, `[ SHIFT ]`) as they are held down, 100% reliable mouse button capture (`mouse_left`, `mouse_right`, `mouse_middle`, `mouse_x1`, `mouse_x2`), and clean modal arming.
3. **Multi-Trigger Bento UI (`c_src/ui/ui_dialogs.c` & `c_src/ui/main_window.c`)**: Expand Card 1 trigger listbox with `WS_VSCROLL` (64px height) to render all assigned triggers (`Trigger #1`, `Trigger #2`, `Trigger #3`, `Trigger #4`) with crisp `ICON_TARGET` badges, and ensure the Main Dashboard Bento Cards display all triggers with visual `/` OR separators.

**Tech Stack:** C11, Win32 GDI & User32 Hooks (`WH_KEYBOARD_LL`, `WH_MOUSE_LL`), Clang / MSVC.

**Spec:** Complete overhaul of trigger capture and multi-trigger management in `c_src/core/input_hook.c`, `c_src/core/input_hook.h`, `c_src/ui/ui_dialogs.c`, `c_src/ui/main_window.c`, and automated test suite in `c_src/tests/test_benchmark.c`.

## Global Constraints

- Pure C11 compliant: Zero external runtime dependencies.
- 100% pure ASCII source code: Zero Unicode emojis or font-fallback symbols.
- Left-Click Safety Lock preserved: Left click bypasses input suppression to prevent lockout while allowing full trigger activation.
- Deterministic sub-microsecond hook matching (< 0.001 ms).

---

### Task 1: Overhaul Modifier Tracking & Combo Capture in Input Hook Engine

**Files:**
- Modify: `c_src/core/input_hook.h`
- Modify: `c_src/core/input_hook.c`
- Test: `c_src/tests/test_benchmark.c`

**Interfaces:**
- Consumes: Windows Hooks (`WH_KEYBOARD_LL`, `WH_MOUSE_LL`), `GetAsyncKeyState`, `KBDLLHOOKSTRUCT`, `MSLLHOOKSTRUCT`
- Produces: `GetCurrentModifiers()`, `BuildComboString()`, `ParseTriggerString()`, `InputHook_StartCapture()`, `InputHook_GetLiveModifiers()`

- [x] **Step 1: Update `input_hook.h` with Modifier Helpers and Live Callback API**

Add `InputHook_GetLiveModifiers()` declaration and enhance modifier mask constants:
```c
// Modifiers bitmask
#define MODIFIER_NONE  0x00
#define MODIFIER_CTRL  0x01
#define MODIFIER_SHIFT 0x02
#define MODIFIER_ALT   0x04
#define MODIFIER_WIN   0x08

uint8_t InputHook_GetLiveModifiers(void);
```

- [x] **Step 2: Implement Atomic Modifier State Tracking in `input_hook.c`**

Add `g_hook_modifiers` and update it on every key event in `LowLevelKeyboardProc`:
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

uint8_t InputHook_GetLiveModifiers(void) {
    uint8_t mods = g_hook_modifiers;
    if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) || (GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000)) mods |= MODIFIER_CTRL;
    if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) || (GetAsyncKeyState(VK_LSHIFT) & 0x8000) || (GetAsyncKeyState(VK_RSHIFT) & 0x8000))   mods |= MODIFIER_SHIFT;
    if ((GetAsyncKeyState(VK_MENU) & 0x8000) || (GetAsyncKeyState(VK_LMENU) & 0x8000) || (GetAsyncKeyState(VK_RMENU) & 0x8000))    mods |= MODIFIER_ALT;
    if ((GetAsyncKeyState(VK_LWIN) & 0x8000) || (GetAsyncKeyState(VK_RWIN) & 0x8000)) mods |= MODIFIER_WIN;
    return mods;
}
```

- [x] **Step 3: Enhance `ParseTriggerString` to Support All Modifier and Mouse Variants**

Update `ParseTriggerString` to support `ctrl`, `shift`, `alt`, `win`, `lctrl`, `rctrl`, `lwin`, `rwin`, `mouse_left`, `left`, `mouse_right`, `right`, `mouse_middle`, `middle`, `mouse_x1`, `x1`, `mouse_x2`, `x2`, plus key aliases.

- [x] **Step 4: Update `LowLevelKeyboardProc` & `LowLevelMouseProc` for Capture HUD**

In `LowLevelKeyboardProc`:
1. When `g_bCapturing` is true:
   - Suppress `VK_LWIN` and `VK_RWIN` from opening the Start menu.
   - Allow modifier key presses to update `g_hook_modifiers` and notify capture window.
   - When non-modifier key is pressed, compose full combo string (e.g., `win+left`, `ctrl+alt+x`) and fire capture callback.

In `LowLevelMouseProc`:
1. When `g_bCapturing` is true and `is_down` is true:
   - Read `InputHook_GetLiveModifiers()`.
   - Build combo string (e.g. `ctrl+mouse_left`, `win+mouse_left`, `ctrl+alt+mouse_left`).
   - Fire capture callback and block captured click.

---

### Task 2: Enhance Interactive Capture HUD with Live Modifier Display

**Files:**
- Modify: `c_src/ui/ui_dialogs.c`
- Test: `build.bat`

**Interfaces:**
- Consumes: `InputHook_GetLiveModifiers()`, `ui_icons.h`, `theme.h`
- Produces: Live pulsing Capture HUD showing active modifier chips (`[ CTRL ] + [ ALT ] + [ ... ]`) in real-time

- [x] **Step 1: Add Animation & Live Modifier Timer to `CaptureWndProc`**

In `CaptureWndProc`:
- On `WM_CREATE`, set a 30 ms timer (`SetTimer(hwnd, 1, 30, NULL)`) to poll `InputHook_GetLiveModifiers()` and refresh HUD if modifiers change.
- On `WM_TIMER`, check if current live modifier mask changed; if so, trigger `InvalidateRect(hwnd, NULL, FALSE)`.

- [x] **Step 2: Render Live Modifier Chips in Capture HUD `WM_PAINT`**

If any modifier is held (`mods != 0`), draw active glowing neon chips in the center of the HUD:
- `[ CTRL ]` in `COLOR_NEON_CYAN`
- `[ SHIFT ]` in `COLOR_NEON_INDIGO`
- `[ ALT ]` in `COLOR_NEON_AMBER`
- `[ WIN ]` in `COLOR_NEON_PURPLE`
- Followed by `+ [ Press Key or Click Mouse ]`

---

### Task 3: Redesign Card 1 Multi-Trigger UI in Macro Add/Edit Dialog

**Files:**
- Modify: `c_src/ui/ui_dialogs.c`
- Test: `build.bat`

**Interfaces:**
- Consumes: `g_add_edit_state.binding.trigger_keys`, `g_add_edit_state.binding.trigger_count`
- Produces: Spacious multi-trigger listbox (height 62px with `WS_VSCROLL`), custom owner-draw items with `ICON_TARGET` reticle and trigger numbering, and distinct `+ Add Trigger` and `Remove Trigger` actions

- [x] **Step 1: Increase `list_triggers` Dimensions and Enable Scrollbar**

Update `CreateWindowW` for `g_add_edit_state.list_triggers`:
- Set style: `WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | LBS_NOTIFY`
- Dimensions: `x: 28, y: 132, w: 320, h: 62`
- Move `btn_add_trigger` to `x: 360, y: 132, w: 120, h: 28`
- Move `btn_remove_trigger` to `x: 490, y: 132, w: 120, h: 28`

- [x] **Step 2: Adjust Vertical Layout for Cards 1, 2, and 3**

Ensure Card 1 spans `y: 46..206` (height 160px), Card 2 spans `y: 214..444`, and Card 3 spans `y: 452..614` with dialog height `670px`.

- [x] **Step 3: Refactor `RefreshTriggerList` and `WM_DRAWITEM` for `list_triggers`**

In `RefreshTriggerList`:
- Populate each trigger line.
In `WM_DRAWITEM`:
- Render rounded badge with `ICON_TARGET`, trigger index (`Trigger #1`, `Trigger #2`, ...), and the full key string (e.g. `ctrl+mouse_left`, `win+mouse_left`).
- Highlight selected trigger row with neon border and active background.

---

### Task 4: Verify Multi-Trigger Signal Flow on Main Dashboard Bento Cards

**Files:**
- Modify: `c_src/ui/main_window.c`
- Test: `build.bat`

**Interfaces:**
- Consumes: `HotkeyBinding.trigger_keys`, `HotkeyBinding.trigger_count`
- Produces: Visual trigger flow chips separated by `/` on Bento cards

- [x] **Step 1: Verify `DrawMacroBentoCard` Trigger Flow Loop**

Ensure `b->trigger_count` up to `MAX_TRIGGERS_PER_BINDING` is rendered with keycap chips and formatted cleanly.

---

### Task 5: Automated Benchmarking and End-to-End Verification

**Files:**
- Modify: `c_src/tests/test_benchmark.c`
- Execute: `build.bat`
- Test: `test_benchmark.exe` and `TobelsoftMacro.exe`

- [x] **Step 1: Add Unit Tests for Modifier-Mouse Combos in `test_benchmark.c`**

Add tests for:
- `ParseTriggerString("ctrl+mouse_left", &ft)` -> `modifiers_mask == MODIFIER_CTRL`, `mouse_btn == MOUSE_TRIGGER_LEFT`
- `ParseTriggerString("win+mouse_left", &ft)` -> `modifiers_mask == MODIFIER_WIN`, `mouse_btn == MOUSE_TRIGGER_LEFT`
- `ParseTriggerString("ctrl+alt+mouse_left", &ft)` -> `modifiers_mask == (MODIFIER_CTRL | MODIFIER_ALT)`, `mouse_btn == MOUSE_TRIGGER_LEFT`
- `ParseTriggerString("shift+right", &ft)` -> `modifiers_mask == MODIFIER_SHIFT`, `mouse_btn == MOUSE_TRIGGER_RIGHT`
- Multi-trigger binding registration and fast trigger lookup.

- [x] **Step 2: Compile and Run Benchmark Suite**

Compile and execute `test_benchmark.exe`. Verify all tests pass.

- [x] **Step 3: Compile and Test `TobelsoftMacro.exe`**

Run `build.bat` to produce the final executable. Verify zero warnings and clean execution.

---

## Plan Review & Verification Checklist
1. All modifier combinations (`Ctrl`, `Shift`, `Alt`, `Win` + Mouse / Keys) supported.
2. Multiple triggers per macro properly displayed and managed in the UI.
3. Live modifier feedback displayed in the Capture HUD modal.
4. Left click safety lock preserved.
5. 100% pure ASCII source code verified.
