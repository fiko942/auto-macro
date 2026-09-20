# Tobelsoft Macro: System Architecture & Design Overview

**Version:** 2.0 Native C  
**Date:** 2026-09-21  

---

## 1. Directory & File Organization

The entire native codebase is organized into clean functional modules under `c_src/`:

```
c_src/
├── main.c                     # WinMain entry point, single-instance mutex, DPI setup, message loop
├── core/
│   ├── types.h                # Core structs: MacroItem, ActionStep, AppConfig, HotkeyTrigger
│   ├── direct_input.h / .c    # DirectInput hardware scancode definitions and key translation
│   ├── input_hook.h / .c      # WH_KEYBOARD_LL & WH_MOUSE_LL low-level hooks & trigger filtering
│   ├── input_sender.h / .c    # SendInput hardware scancode injection & high-precision timers
│   ├── macro_engine.h / .c    # Async execution thread management, loop control, safety stops
│   └── config_manager.h / .c  # cJSON preset parser, serialization, and %APPDATA% persistence
├── ui/
│   ├── theme.h / .c           # Cyberpunk dark theme color tokens, typography, custom control widgets
│   ├── ui_icons.h / .c        # Pure Win32 GDI geometric vector icon rendering engine
│   ├── main_window.h / .c     # Top-level window, titlebar hit-testing, bento cards, navigation
│   └── ui_dialogs.h / .c      # DirectInput Capture HUD modal, Action Step Editor, Cyber dropdowns
└── tests/
    └── test_benchmark.c       # Microsecond latency benchmark & memory audit suite
```

---

## 2. Process Thread Model

Tobelsoft Macro utilizes a multi-threaded architecture with strict separation between UI rendering and input execution:

```
+-------------------------------------------------------------------------+
|                              PROCESS THREADS                            |
+-------------------------------------------------------------------------+
|                                                                         |
|  [ Thread 1: Main UI & Hook Thread ]                                    |
|    - Win32 Message Pump (GetMessageW / DispatchMessageW)                |
|    - Low-Level Keyboard/Mouse Hooks (WH_KEYBOARD_LL / WH_MOUSE_LL)      |
|    - Double-Buffered 60 FPS Paint Cycle (WM_PAINT / WM_TIMER)           |
|    - Modal Dialog Lifecycle Management                                  |
|                                                                         |
|  [ Thread 2..N: Macro Worker Execution Threads ]                        |
|    - Spawned on-demand or managed via lightweight worker pool           |
|    - Executes sequence of ActionSteps (KEY_PRESS, HOLD, DELAY, etc.)    |
|    - Direct Hardware Scancode Dispatch via SendInput                    |
|    - Synchronized high-precision spinlocks (QueryPerformanceCounter)    |
|    - Monitored for emergency termination flags                          |
+-------------------------------------------------------------------------+
```

---

## 3. Data Structures & Memory Layout

### 3.1 `ActionStep`
Represents a single atomic macro action:
```c
typedef enum {
    ACTION_KEY_PRESS = 0,
    ACTION_KEY_DOWN,
    ACTION_KEY_UP,
    ACTION_KEY_HOLD,
    ACTION_MOUSE_CLICK,
    ACTION_MOUSE_DOWN,
    ACTION_MOUSE_UP,
    ACTION_DELAY
} ActionType;

typedef struct {
    ActionType type;
    DWORD vk_code;          // Virtual Key code or Mouse Button ID
    DWORD scan_code;        // DirectInput hardware scan code
    DWORD duration_ms;      // Hold or delay duration in milliseconds
    DWORD mouse_button;     // 0=None, 1=Left, 2=Right, 3=Middle, 4=X1, 5=X2
    POINT mouse_pos;        // Target coordinates (if applicable)
} ActionStep;
```

### 3.2 `HotkeyTrigger` & `MacroItem`
Represents independent activation triggers and a complete automated macro profile:
```c
#define MAX_TRIGGERS_PER_MACRO 8

typedef struct {
    TriggerType type;       // TRIGGER_TYPE_KEYBOARD or TRIGGER_TYPE_MOUSE
    DWORD vk_code;          // Virtual Key code or 0 for mouse
    DWORD mouse_button;     // 1=Left, 2=Right, 3=Middle, 4=X1, 5=X2
    uint8_t modifiers;      // Bitmask: MODIFIER_CTRL | MODIFIER_SHIFT | MODIFIER_ALT | MODIFIER_WIN
    char raw_combo[64];     // Normalized representation (e.g. "ctrl+mouse_left")
} HotkeyTrigger;

typedef struct {
    char id[64];
    wchar_t name[128];
    bool is_enabled;
    bool is_executing;
    
    // Multi-Trigger Configuration
    HotkeyTrigger triggers[MAX_TRIGGERS_PER_MACRO];
    int trigger_count;
    
    bool suppress_original_input;
    bool left_click_safety_lock;
    int repeat_count;       // 0 = infinite while held
    DWORD repeat_delay_ms;
    ActionStep* steps;
    int step_count;
    int step_capacity;
    HANDLE h_worker_thread;
    volatile bool cancel_requested;
} MacroItem;
```

---

## 4. Lifecycle & Application Initialization Sequence

1. **WinMain Execution:**
   - Single-Instance Check via `CreateMutexW(NULL, TRUE, L"TobelsoftMacro_SingleInstance_Mutex")`.
   - Set High-DPI Awareness (`SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)`).
   - Initialize Multimedia Timer Period (`timeBeginPeriod(1)`).
2. **Configuration Loading:**
   - Locate `%APPDATA%\TobelsoftMacro\presets.json`. If missing, generate default gaming & productivity presets.
3. **Window Creation & Hook Installation:**
   - Register custom `TobelsoftMacro_WindowClass` with `CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS`.
   - Create window with `WS_POPUP | WS_VISIBLE | WS_MINIMIZEBOX | WS_MAXIMIZEBOX`.
   - Install `WH_KEYBOARD_LL` and `WH_MOUSE_LL` hooks.
4. **Message Pump & 60 FPS Render Loop:**
   - Execute standard Win32 message loop while driving UI animation via `SetTimer(hwnd, IDT_ANIMATION, 16, NULL)`.
5. **Clean Termination:**
   - Unhook all low-level hooks (`UnhookWindowsHookEx`).
   - Stop and join all active macro worker threads.
   - Flush active configuration to disk.
   - Release multimedia timer period (`timeEndPeriod(1)`).
