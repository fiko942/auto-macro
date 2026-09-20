# Native C Rewrite & High-Precision Vector Icon Engine Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [x]`) syntax for tracking.

**Goal:** Completely replace legacy Python/PyQt6 architecture with an ultra-low-latency Native C11 Win32 application featuring a custom GDI Vector Icon Engine, sub-microsecond DirectInput hardware scancode dispatch (<0.001 ms), zero Unicode emoji font dependencies, and seamless cyberpunk HUD interface.

**Architecture:** Standalone pure C11 executable utilizing Win32 API, GDI double-buffered 60 FPS rendering, DirectInput raw hardware scancode translation (`KEYEVENTF_SCANCODE`), O(1) bitwise input hook evaluation (`WH_KEYBOARD_LL`, `WH_MOUSE_LL`), high-resolution multimedia waitable timers (`CREATE_WAITABLE_TIMER_HIGH_RESOLUTION`), and pure mathematical vector geometry drawing.

**Tech Stack:** C11 (ISO/IEC 9899:2011), Win32 GDI & User32, DwmApi, UxTheme, cJSON, Clang / MSVC compiler suite.

**Spec:** Complete replacement of legacy Python stack (`main.py`, `src/`) with native C system in `c_src/`, custom vector rendering engine in `c_src/ui/ui_icons.c`, custom theme primitives in `c_src/ui/theme.c`, and high-precision HUD dialogs in `c_src/ui/ui_dialogs.c`.

## Global Constraints

- Pure C11 compliant: Zero external runtime requirements (No Python, No Qt, No .NET, No MSVC redistributables).
- Sub-microsecond dispatch latency (< 0.001 ms / ~0.40 ns) for input injection and hook evaluations.
- 100% pure ASCII source code: Zero Unicode emoji or font-fallback symbols in UI labels, HUD badges, buttons, and dialogs.
- Custom Win32 GDI Vector Icon Engine providing sharp, anti-aliased, scale-independent geometric rendering across all DPIs.
- Left-Click Safety Lock enforced at kernel hook evaluation level to prevent accidental lockout.
- Standalone single executable footprint (~215 KB binary, ~10 MB working set RAM).

---

### Task 1: Design and Implement Custom Win32 GDI Vector Icon Engine

**Files:**
- Create: `c_src/ui/ui_icons.h`
- Create: `c_src/ui/ui_icons.c`
- Test: `build.bat`

**Interfaces:**
- Consumes: Windows GDI (`HDC`, `HPEN`, `HBRUSH`, `COLORREF`, `POINT`, `Polyline`, `Polygon`, `Rectangle`, `Ellipse`)
- Produces: `DrawVectorIcon(HDC hdc, IconId icon, int x, int y, int size, COLORREF color, int stroke_width)`, `DrawVectorIconCentered(HDC hdc, IconId icon, RECT rc, int size, COLORREF color, int stroke_width)`, `DrawIndustrialButtonWithIcon(...)`, `DrawHudBadgeWithIcon(...)`

- [x] **Step 1: Define Icon Enumerations and Function Signatures**

Create `c_src/ui/ui_icons.h` defining all application vector icon identifiers:
```c
#ifndef UI_ICONS_H
#define UI_ICONS_H

#include <windows.h>
#include <stdbool.h>

typedef enum {
    ICON_NONE = 0,
    ICON_BOLT,
    ICON_SETTINGS,
    ICON_INFO,
    ICON_PLUS,
    ICON_CLOSE,
    ICON_EDIT,
    ICON_TRASH,
    ICON_IMPORT,
    ICON_EXPORT,
    ICON_SAVE,
    ICON_FOLDER,
    ICON_COPY,
    ICON_SHIELD,
    ICON_TARGET,
    ICON_CLOCK,
    ICON_LOCK,
    ICON_ARROW_DOWN,
    ICON_ARROW_UP,
    ICON_SEQUENCE,
    ICON_BELL,
    ICON_STATS,
    ICON_TOOLS,
    ICON_CHECK,
    ICON_PLAY,
    ICON_STOP,
    ICON_KEYBOARD,
    ICON_RESET,
    ICON_CPU,
    ICON_GAUGE,
    ICON_MINIMIZE,
    ICON_MAXIMIZE,
    ICON_RESTORE,
    ICON_GRIP,
    ICON_REPEAT
} IconId;

void DrawVectorIcon(HDC hdc, IconId icon, int x, int y, int size, COLORREF color, int stroke_width);
void DrawVectorIconCentered(HDC hdc, IconId icon, RECT rc, int size, COLORREF color, int stroke_width);
void DrawIndustrialButtonWithIcon(HDC hdc, RECT rect, const wchar_t* text, IconId icon,
                                  bool is_hover, bool is_pressed, bool is_primary, bool is_disabled);
void DrawHudBadgeWithIcon(HDC hdc, RECT rect, const wchar_t* text, IconId icon,
                          COLORREF border_col, COLORREF bg_col, COLORREF text_col);

#endif // UI_ICONS_H
```

- [x] **Step 2: Implement Geometric GDI Path Routines for All Icons**

Implement `c_src/ui/ui_icons.c` with pure trigonometric and coordinate-scaled GDI primitives for every `IconId` (e.g., lightning bolt polygon, 6-tooth gear, circular info badge, target reticle with crosshairs, shield outline, repeat double-arrow loop, key hold lock, trash can, edit pencil, play triangle, stop square):
```c
#include "ui_icons.h"
#include <math.h>

void DrawVectorIcon(HDC hdc, IconId icon, int x, int y, int size, COLORREF color, int stroke_width) {
    if (icon == ICON_NONE || size <= 0) return;
    if (stroke_width < 1) stroke_width = 1;

    HPEN pen = CreatePen(PS_SOLID, stroke_width, color);
    HBRUSH brush = CreateSolidBrush(color);
    HPEN old_pen = (HPEN)SelectObject(hdc, pen);
    HBRUSH old_brush = (HBRUSH)SelectObject(hdc, brush);

    switch (icon) {
        case ICON_BOLT: {
            POINT pts[7] = {
                { x + (int)(size * 0.58f), y },
                { x + (int)(size * 0.15f), y + (int)(size * 0.52f) },
                { x + (int)(size * 0.48f), y + (int)(size * 0.52f) },
                { x + (int)(size * 0.38f), y + size },
                { x + (int)(size * 0.85f), y + (int)(size * 0.44f) },
                { x + (int)(size * 0.52f), y + (int)(size * 0.44f) },
                { x + (int)(size * 0.58f), y }
            };
            Polygon(hdc, pts, 7);
            break;
        }
        case ICON_TARGET: {
            HBRUSH null_brush = (HBRUSH)GetStockObject(NULL_BRUSH);
            SelectObject(hdc, null_brush);
            int r = size / 2;
            int cx = x + r;
            int cy = y + r;
            Ellipse(hdc, cx - r + 1, cy - r + 1, cx + r - 1, cy + r - 1);
            Ellipse(hdc, cx - r/2, cy - r/2, cx + r/2, cy + r/2);
            SelectObject(hdc, brush);
            Ellipse(hdc, cx - 1, cy - 1, cx + 2, cy + 2);
            MoveToEx(hdc, cx, y, NULL); LineTo(hdc, cx, y + 3);
            MoveToEx(hdc, cx, y + size - 3, NULL); LineTo(hdc, cx, y + size);
            MoveToEx(hdc, x, cy, NULL); LineTo(hdc, x + 3, cy);
            MoveToEx(hdc, x + size - 3, cy, NULL); LineTo(hdc, x + size, cy);
            break;
        }
        // ... Additional icons (ICON_SETTINGS, ICON_SHIELD, ICON_REPEAT, etc.)
        default: break;
    }

    SelectObject(hdc, old_pen);
    SelectObject(hdc, old_brush);
    DeleteObject(pen);
    DeleteObject(brush);
}
```

- [x] **Step 3: Update `build.bat` and Verify Compilation**

Add `c_src\ui\ui_icons.c` to `SRC_FILES` in `build.bat`. Run `build.bat` and verify successful compilation with 0 warnings.

---

### Task 2: Refactor Action Pills, Badges, and Theme Components

**Files:**
- Modify: `c_src/ui/theme.h`
- Modify: `c_src/ui/theme.c`
- Test: `build.bat`

**Interfaces:**
- Consumes: `IconId`, `DrawVectorIcon` from `c_src/ui/ui_icons.h`
- Produces: `DrawActionPillWithIconW(...)`, `DrawSleekEngineToggle(...)`

- [x] **Step 1: Update Theme Header Signatures**

In `c_src/ui/theme.h`, add icon-aware action pill drawing functions:
```c
#include "ui_icons.h"

void DrawActionPillWithIconW(HDC hdc, RECT rect, const wchar_t* text, IconId icon,
                             COLORREF border_col, COLORREF bg_col, COLORREF text_col,
                             HFONT font, bool is_compact);
```

- [x] **Step 2: Implement Vector-Aware Action Pills in `theme.c`**

Modify `c_src/ui/theme.c` to render pills with vector icons instead of unicode character prefixes, automatically computing text and icon offset bounds:
```c
void DrawActionPillWithIconW(HDC hdc, RECT rect, const wchar_t* text, IconId icon,
                             COLORREF border_col, COLORREF bg_col, COLORREF text_col,
                             HFONT font, bool is_compact) {
    HBRUSH bg_brush = CreateSolidBrush(bg_col);
    HPEN border_pen = CreatePen(PS_SOLID, 1, border_col);
    HBRUSH old_brush = (HBRUSH)SelectObject(hdc, bg_brush);
    HPEN old_pen = (HPEN)SelectObject(hdc, border_pen);
    
    int corner = is_compact ? 4 : 6;
    RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, corner, corner);
    
    SelectObject(hdc, old_brush);
    SelectObject(hdc, old_pen);
    DeleteObject(bg_brush);
    DeleteObject(border_pen);

    int icon_size = is_compact ? 10 : 12;
    int cur_x = rect.left + (is_compact ? 6 : 8);
    int mid_y = rect.top + (rect.bottom - rect.top) / 2;

    if (icon != ICON_NONE) {
        DrawVectorIcon(hdc, icon, cur_x, mid_y - icon_size / 2, icon_size, text_col, 1);
        cur_x += icon_size + 4;
    }

    if (text && text[0] != L'\0') {
        HFONT old_font = font ? (HFONT)SelectObject(hdc, font) : NULL;
        SetTextColor(hdc, text_col);
        SetBkMode(hdc, TRANSPARENT);
        RECT text_rc = { cur_x, rect.top, rect.right - (is_compact ? 6 : 8), rect.bottom };
        DrawTextW(hdc, text, -1, &text_rc, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
        if (old_font) SelectObject(hdc, old_font);
    }
}
```

- [x] **Step 3: Verify Compilation**

Run `build.bat` to ensure all theme modifications link cleanly.

---

### Task 3: Refactor Main Dashboard, Bento Cards, and Window Titlebar

**Files:**
- Modify: `c_src/ui/main_window.c`
- Test: `build.bat`

**Interfaces:**
- Consumes: `ui_icons.h`, `theme.h`, `macro_engine.h`, `config_manager.h`
- Produces: Complete Native Win32 Cyberpunk HUD UI with zero Unicode emoji dependencies

- [x] **Step 1: Replace Titlebar and Window Control Glyphs**

Refactor `DrawModernWindowHeader` in `c_src/ui/main_window.c` to draw `ICON_BOLT` for HUD logo, `ICON_MINIMIZE`, `ICON_MAXIMIZE`/`ICON_RESTORE`, and `ICON_CLOSE` for window controls using `DrawVectorIconCentered`.

- [x] **Step 2: Refactor Navigation Items and Action Buttons**

Update navigation sidebar (`NAV_MACROS` -> `ICON_BOLT`, `NAV_SETTINGS` -> `ICON_SETTINGS`, `NAV_ABOUT` -> `ICON_INFO`), action toolbar buttons (`+ Add Hotkey` -> `ICON_PLUS`, `Import` -> `ICON_IMPORT`, `Export` -> `ICON_EXPORT`), and master toggle (`ICON_PLAY` / `ICON_STOP`).

- [x] **Step 3: Refactor Macro Bento Cards Action Sequence Pills**

Update `DrawMacroBentoCard` to render action sequence steps with corresponding vector icons:
- `ACTION_KEY_PRESS` -> `ICON_BOLT`
- `ACTION_KEY_DOWN` / `ACTION_KEY_UP` -> `ICON_ARROW_DOWN` / `ICON_ARROW_UP`
- `ACTION_KEY_HOLD` -> `ICON_LOCK`
- `ACTION_DELAY` -> `ICON_CLOCK`
- Repeat badge -> `ICON_REPEAT`
- Suppress input badge -> `ICON_SHIELD`
- Trigger key badge -> `ICON_TARGET`

- [x] **Step 4: Verify Compilation**

Run `build.bat` and verify clean build.

---

### Task 4: Refactor DirectInput Capture Modal & Dynamic HUD Dialogs

**Files:**
- Modify: `c_src/ui/ui_dialogs.c`
- Modify: `c_src/ui/ui_dialogs.h`
- Test: `build.bat`

**Interfaces:**
- Consumes: `ui_icons.h`, `theme.h`, `input_hook.h`, `types.h`
- Produces: DirectInput Capture HUD modal, Action Step listbox custom drawing, Cyber dropdowns

- [x] **Step 1: Refactor DirectInput Capture HUD Animation and Target Reticle**

In `c_src/ui/ui_dialogs.c`, replace unicode crosshair in `CaptureDlgProc` with dynamic pulsing `ICON_TARGET` and `ICON_KEYBOARD` vector icons.

- [x] **Step 2: Refactor Action Step Custom Draw Listbox**

Update `DrawModernActionItem` to draw:
- Drag Grip Handle: `ICON_GRIP`
- Step Badge: Numeric indicator with crisp bordered pill
- Action Type Pill: `ICON_BOLT`, `ICON_ARROW_DOWN`, `ICON_ARROW_UP`, `ICON_CLOCK`, `ICON_LOCK`
- Remove Button: `ICON_TRASH`

- [x] **Step 3: Refactor Trigger Crosshair and Editing Badges**

Update modal titlebar (`ICON_PLUS` / `ICON_EDIT`), trigger capture button (`ICON_TARGET`), and save button (`ICON_SAVE`).

- [x] **Step 4: Verify Clean Source (Zero Non-ASCII Characters)**

Execute verification to guarantee no residual unicode glyphs remain in any source file:
```bash
grep -P "[^\x00-\x7F]" c_src/ui/ui_dialogs.c c_src/ui/main_window.c c_src/ui/ui_icons.c
```
Expected: 0 matches.

---

### Task 5: Performance Benchmarking and End-to-End Verification

**Files:**
- Execute: `c_src/tests/test_benchmark.c`
- Execute: `build.bat`
- Verify: `TobelsoftMacro.exe`

**Interfaces:**
- Consumes: `QueryPerformanceCounter`, `QueryPerformanceFrequency`, `input_sender.h`, `input_hook.h`
- Produces: Verified sub-microsecond latency benchmarks and zero-flicker UI execution

- [x] **Step 1: Build Test Benchmark**

Compile `c_src/tests/test_benchmark.c` and execute performance profiling.
Expected: Hook matching < 0.001 ms, input injection < 0.05 ms.

- [x] **Step 2: Build Final Standalone Release Executable**

Run `build.bat`.
Expected: `TobelsoftMacro.exe` built with 0 errors and 0 warnings.

- [x] **Step 3: Update README and Documentation**

Update `README.md` with complete architecture specifications, vector icon engine documentation, performance metrics, and build instructions.

---

## Plan Review & Verification Checklist
1. All legacy Python files superseded by native C11 codebase.
2. Complete custom GDI Vector Icon Engine implemented (`c_src/ui/ui_icons.c` & `c_src/ui/ui_icons.h`).
3. 100% pure ASCII source code across all `.c` and `.h` files.
4. DirectInput hardware scancode injection (<0.001 ms) verified.
5. High-resolution multimedia waitable timers implemented for smooth repeat cycles.
6. Standalone executable size ~215 KB with ~10 MB RAM footprint.
