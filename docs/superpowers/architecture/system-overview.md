# Tobelsoft Macro: System Architecture & Design Overview

**Version:** 2.0 Native C  
**Date:** 2026-09-21  

---

## 1. Directory & File Organization

The entire native codebase is organized into clean functional modules under `c_src/`:

```
c_src/
├── main.c                     # WinMain entry point, single-instance mutex, DPI setup, message loop
├── app.manifest               # High-DPI Per-Monitor V2 and Common Controls v6 manifest
├── common/
│   ├── types.h / .c           # Core structs: HotkeyBinding, KeyAction, AppConfig, ActionType
│   ├── utils.h / .c           # Safe string utils, UUID generation, high-resolution timers
│   └── cJSON.h / .c           # Ultra-lightweight JSON parser and serializer
├── core/
│   ├── input_hook.h / .c      # WH_KEYBOARD_LL & WH_MOUSE_LL low-level hooks & modifier tracking
│   ├── input_sender.h / .c    # SendInput hardware scancode injection & key resolution
│   └── macro_engine.h / .c    # Async execution thread management, loop control, safety stops
├── storage/
│   └── config_manager.h / .c  # JSON preset parser, serialization, and %APPDATA% persistence
├── ui/
│   ├── animation.h / .c       # Micro-animation tweening and easing engine
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

### 3.1 `ActionType` & `KeyAction` (`c_src/common/types.h`)
Represents an individual action step inside a macro sequence:
```c
#define MAX_KEYS_PER_ACTION 4
#define MAX_KEY_NAME_LEN 32

typedef enum {
    ACTION_KEY_PRESS = 0,     // Tap key down and up
    ACTION_KEY_DOWN,          // Hold key down
    ACTION_KEY_UP,            // Release key up
    ACTION_KEY_HOLD,          // Hold key for specific duration in ms
    ACTION_KEY_SEQUENCE,      // Sequence of keys in order
    ACTION_DELAY              // Wait / delay in ms
} ActionType;

typedef struct {
    ActionType action_type;
    char keys[MAX_KEYS_PER_ACTION][MAX_KEY_NAME_LEN];
    int key_count;
    int duration;             // In milliseconds
} KeyAction;
```

### 3.2 `HotkeyBinding` & `AppConfig` (`c_src/common/types.h`)
Represents a complete macro binding profile and the application root state:
```c
#define MAX_BINDINGS 64
#define MAX_ACTIONS_PER_BINDING 16
#define MAX_TRIGGERS_PER_BINDING 32
#define MAX_MASTER_TRIGGERS 8
#define MAX_NAME_LEN 64
#define MAX_ID_LEN 40

typedef struct {
    char id[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    char trigger_keys[MAX_TRIGGERS_PER_BINDING][MAX_KEY_NAME_LEN];
    int trigger_count;
    KeyAction actions[MAX_ACTIONS_PER_BINDING];
    int action_count;
    bool enabled;
    bool repeat;
    int repeat_delay;         // In milliseconds
    bool block_input;
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
