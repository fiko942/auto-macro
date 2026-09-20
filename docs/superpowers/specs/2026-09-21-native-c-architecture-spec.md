# Tobelsoft Macro: Native C11 Architecture Specification

**Status:** Completed & Approved  
**Date:** 2026-09-21  
**Target:** Win32 (Windows 10 / 11 64-bit & 32-bit)  
**Compiler:** Clang 17+ / MSVC 19+ (`-O3 -Wall -Wextra -mwindows`)  
**Executable:** `TobelsoftMacro.exe` (~215 KB binary footprint)  

---

## 1. Executive Summary

Tobelsoft Macro is an ultra-low-latency, zero-dependency native Windows automation application engineered in pure C11. Replacing the legacy Python/PyQt6 prototype, this architecture delivers sub-microsecond hardware input dispatch (<0.001 ms), deterministic low-level keyboard/mouse hooks, 60 FPS double-buffered GDI rendering, and an embedded geometric vector icon engine with zero Unicode emoji/font dependencies.

---

## 2. Core Architecture & Subsystems

```
+-------------------------------------------------------------------------+
|                        TobelsoftMacro.exe (C11)                         |
+-------------------------------------------------------------------------+
|                                                                         |
|  +---------------------------+        +------------------------------+  |
|  |       Win32 GDI GUI       |        |   DirectInput Macro Engine   |  |
|  | - Custom Frameless Window |        | - WH_KEYBOARD_LL / MOUSE_LL  |  |
|  | - Double-Buffered Canvas  |        | - Hardware Scancode Dispatch |  |
|  | - 60 FPS Render Loop      |        | - QueryPerformanceCounter    |  |
|  | - High-DPI Awareness      |        | - O(1) Bitwise Hook Filter   |  |
|  +-------------+-------------+        +--------------+---------------+  |
|                |                                     |                  |
|  +-------------v-------------+        +--------------v---------------+  |
|  |   GDI Vector Icon Engine  |        |    Async Execution Pool      |  |
|  | - Pure Math Geometry      |        | - Lock-free Trigger Match    |  |
|  | - 35+ Vector Icon Enums   |        | - Multimedia Waitable Timers |  |
|  | - 0 OS Glyph Dependencies |        | - Emergency Kill-Switch      |  |
|  +-------------+-------------+        +--------------+---------------+  |
|                |                                     |                  |
|  +-------------v-------------------------------------v---------------+  |
|  |                Config & Preset Manager (cJSON)                    |  |
|  | - %APPDATA%\TobelsoftMacro\presets.json                           |  |
|  | - Fast In-Memory Serialization / Atomic Disk Writing              |  |
|  +-------------------------------------------------------------------+  |
+-------------------------------------------------------------------------+
```

---

## 3. Key Design Principles

1. **Deterministic Latency (< 0.001 ms)**:
   - Zero garbage collection pauses (no Python runtime, no GC sweeps).
   - Direct `SendInput` with `KEYEVENTF_SCANCODE` hardware translation.
   - Microsecond delay timing using `timeBeginPeriod(1)` and `QueryPerformanceCounter`.

2. **Zero External Runtime Dependencies**:
   - Compiles down to a single standalone static executable (~215 KB).
   - Links only against Windows core system DLLs (`user32.dll`, `gdi32.dll`, `winmm.dll`, `shell32.dll`, `ole32.dll`, `dwmapi.dll`, `uxtheme.dll`).
   - Zero requirement for Python, PyQt6, VC++ Redistributable packages, or external font packs.

3. **100% Vector Graphic UI**:
   - Zero fallback to default Windows Unicode emojis or colored font glyphs.
   - All HUD icons, buttons, action pills, grips, and titlebar controls are drawn geometrically via Windows GDI primitives (`Polygon`, `Polyline`, `Ellipse`, `RoundRect`).
   - Crisp rendering across all DPI scaling levels (100%, 125%, 150%, 200%).

4. **Safety and Isolation**:
   - Hardcoded Left-Click Safety Lock to prevent irreversible OS lockouts during high-frequency mouse triggering.
   - Emergency Escape Kill Switch (`VK_ESCAPE`) monitored at the lowest hook priority.

---

## 4. Subsystem Specifications

### 4.1 Input Hook Subsystem (`c_src/core/input_hook.c`, `c_src/core/input_hook.h`)
- **Hook Types:** `WH_KEYBOARD_LL` (Keyboard) and `WH_MOUSE_LL` (Mouse).
- **Modifier State Tracking:** Atomic bitmask `g_hook_modifiers` tracking `MODIFIER_CTRL`, `MODIFIER_SHIFT`, `MODIFIER_ALT`, and `MODIFIER_WIN` with live query API `InputHook_GetLiveModifiers()`.
- **Multi-Trigger Matching:** O(1) bitwise multi-trigger lookup (up to 8 triggers per macro) matching key/button + exact modifier masks.
- **Capture HUD & Isolation:** Suppresses `VK_LWIN`/`VK_RWIN` during capture modal to prevent Start Menu interference and captures full modifier combos (`ctrl+mouse_left`, `win+left`, etc.).
- **Pass-through / Block:** Selective suppression controlled by `suppress_input` flag on matching macros, with Left-Click Safety Lock bypassing mouse suppression when active.

### 4.2 Input Dispatcher (`c_src/core/input_sender.c`, `c_src/core/input_sender.h`)
- **API:** Win32 `SendInput` with `INPUT_KEYBOARD` / `INPUT_MOUSE`.
- **Mode:** Hardware Scancodes via `MapVirtualKeyExW(vk, MAPVK_VK_TO_VSC_EX, ...)` with `KEYEVENTF_SCANCODE`.
- **Extended Keys:** Automatic handling of `KEYEVENTF_EXTENDEDKEY` for arrow keys, Ins, Del, Home, End, PageUp, PageDown.

### 4.3 Macro Execution Engine (`c_src/core/macro_engine.c`, `c_src/core/macro_engine.h`)
- **Execution Model:** Background worker thread per triggered macro or reusable thread pool.
- **Timing:** Hybrid spinlock + `CreateWaitableTimerExW` with `CREATE_WAITABLE_TIMER_HIGH_RESOLUTION` for accurate millisecond and sub-millisecond sleeps.
- **Cycle Control:** Configurable repeat count (0 = infinite while held / toggle mode).

### 4.4 Preset & Configuration Manager (`c_src/core/config_manager.c`, `c_src/core/config_manager.h`)
- **Storage Path:** `%APPDATA%\TobelsoftMacro\presets.json`
- **Parser:** Inlined lightweight `cJSON` parser.
- **Atomicity:** Safe write to `.tmp` file followed by atomic `MoveFileExW` with `MOVEFILE_REPLACE_EXISTING`.

### 4.5 Win32 Cyberpunk UI & GDI Vector Engine (`c_src/ui/`)
- **Window:** Custom framed Win32 window with borderless dark theme (`#08090C` background).
- **Double Buffering:** 100% flicker-free rendering via off-screen memory DC (`CreateCompatibleDC` + `CreateCompatibleBitmap` + `BitBlt`).
- **Vector Icons:** `c_src/ui/ui_icons.c` rendering 35+ icon types.
