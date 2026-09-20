# Tobelsoft Macro // Full 60 FPS Smooth UI Animation Subsystem Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement a high-performance, 60 FPS hardware-synchronized UI animation engine in pure native C (Win32 GDI double-buffered), featuring a sliding navigation pill, smooth switch sliders, tab page crossfades, card hover springs, pulsing neon beacons, and signal flow laser telemetry.

**Architecture:** 
- Embedded Animation Subsystem with a 60 FPS Win32 timer (`SetTimer(hwnd, ID_ANIM_TIMER, 16, NULL)`).
- Delta-time based smooth exponential decay / spring interpolation: `current += (target - current) * (1.0f - expf(-speed * dt))`.
- Animated State Structs for sliding navigation pill, per-binding switch positions, hover glow intensities, and live signal flow pulse phases.
- Double-buffered GDI rendering ensuring 100% flicker-free 60 FPS frame delivery with < 0.1% CPU consumption.

**Tech Stack:** Pure C11, Win32 API (`windows.h`, `windowsx.h`), GDI (`gdi32.dll`, `msimg32.dll`, `dwmapi.dll`), Clang/MSVC compiler.

---

## Global Constraints
- Pure C11 / Win32 native GDI double-buffered rendering.
- Zero external runtime dependencies (no Qt, no Python, no heavy animation frameworks).
- 60 FPS frame rate target with smart settle detection to prevent unnecessary CPU cycles.
- Sub-pixel smooth color morphing (`lerp_color`) and floating-point geometry interpolation.

---

## File Structure & Responsibilities

1. `c_src/ui/animation.h` & `c_src/ui/animation.c`:
   - Animation engine types, timing utilities (`QueryPerformanceCounter`), interpolation helpers (`lerp_float`, `lerp_color`, `ease_out_cubic`), and animation state trackers.
2. `c_src/ui/theme.h` & `c_src/ui/theme.c`:
   - Upgrade drawing primitives with continuous floating-point animation inputs (e.g. `DrawAnimatedSwitch`, `DrawSlidingNavPill`, `DrawPulsingSignalConnector`, `DrawAnimatedCard`).
3. `c_src/ui/main_window.c`:
   - Hook up the 60 FPS animation timer, integrate the sliding nav pill, card hover springs, smooth switch clicks, and tab transition crossfades.
4. `build.bat`:
   - Include `c_src/ui/animation.c` in build compilation.

---

## Tasks

### Task 1: Animation Subsystem Core (`animation.h` & `animation.c`)

**Files:**
- Create: `c_src/ui/animation.h`
- Create: `c_src/ui/animation.c`

**Interfaces:**
- Produces:
  - `float LerpFloat(float current, float target, float speed, float dt);`
  - `COLORREF LerpColor(COLORREF c1, COLORREF c2, float t);`
  - `void Animation_Init(void);`
  - `void Animation_Update(float dt);`
  - `typedef struct { float nav_pill_y; float nav_pill_target_y; float nav_pill_h; float nav_pill_target_h; float nav_pill_glow; float switch_pos[MAX_BINDINGS]; float switch_target[MAX_BINDINGS]; float card_hover[MAX_BINDINGS]; float pulse_phase; float page_alpha; float master_toggle_hover; } AnimState;`

- [ ] **Step 1: Write `c_src/ui/animation.h`**
  Define math helpers, interpolation curves, timing functions, and global `AnimState`.

- [ ] **Step 2: Write `c_src/ui/animation.c`**
  Implement high-resolution delta-time timer, exponential decay lerping, color blending, and animation state updates.

---

### Task 2: Animated Drawing Primitives (`theme.h` & `theme.c`)

**Files:**
- Modify: `c_src/ui/theme.h`
- Modify: `c_src/ui/theme.c`

**Interfaces:**
- Produces:
  - `void DrawSlidingNavPill(HDC hdc, int x, float y, int width, float height, float glow_alpha);`
  - `void DrawAnimatedSwitch(HDC hdc, const RECT* rect, float pos);`
  - `void DrawPulsingSignalFlowConnector(HDC hdc, int x, int y_center, int width, COLORREF base_col, float pulse_phase, bool active);`
  - `void DrawAnimatedIndustrialButton(HDC hdc, const RECT* rect, const wchar_t* text, float hover_t, bool is_primary, bool is_danger, bool is_active_toggle, float pulse_phase);`

- [ ] **Step 1: Implement `DrawSlidingNavPill`**
  Draws the smooth sliding sidebar indicator with soft ambient glow and active vertical laser bar.

- [ ] **Step 2: Implement `DrawAnimatedSwitch`**
  Draws the mechanical sliding switch knob at continuous position `pos` (0.0 to 1.0) with dynamic track color blending from dark slate to vivid tactical emerald.

- [ ] **Step 3: Implement `DrawPulsingSignalFlowConnector`**
  Draws the high-tech signal pipeline with a moving laser beam pulse across the connector line when macros are armed.

- [ ] **Step 4: Implement `DrawAnimatedIndustrialButton`**
  Draws buttons with smooth hover brightness interpolation and pulsing glow for the active master power button.

---

### Task 3: MainWindow Integration & 60 FPS Event Loop (`main_window.c`)

**Files:**
- Modify: `c_src/ui/main_window.c`

**Interfaces:**
- Consumes: `animation.h`, `theme.h`
- Handles:
  - `WM_CREATE`: Start 60 FPS timer (`SetTimer(hwnd, ID_ANIM_TIMER, 16, NULL)`), initialize `AnimState`.
  - `WM_TIMER`: Calculate delta time via `QueryPerformanceCounter`, update `AnimState`, check dirty state, `InvalidateRect`.
  - `WM_LBUTTONDOWN`: Smoothly set navigation targets and switch targets.
  - `WM_MOUSEMOVE`: Smoothly set hover targets.
  - `WM_PAINT`: Render with smooth animated pill, smooth switch knob positions, card hover glows, and live signal pulse.

- [ ] **Step 1: Add Animation Timer & State Management**
  Hook up `WM_TIMER` at 60 FPS with sub-millisecond precision.

- [ ] **Step 2: Integrate Sliding Navigation Pill in Sidebar**
  Replace static nav items with continuous sliding pill rendering.

- [ ] **Step 3: Integrate Animated Switches & Card Hover Glow in Macro Hub**
  Animate switch knob sliding on click, card border glow on hover, and live signal pulse along pipeline connector arrows.

- [ ] **Step 4: Integrate Animated Master Toggle Button**
  Animate power button hover and pulsing emerald ring.

---

### Task 4: Build, Benchmark, and Verification

**Files:**
- Modify: `build.bat`

- [ ] **Step 1: Update `build.bat` to include `c_src/ui/animation.c`**
- [ ] **Step 2: Compile `TobelsoftMacro.exe` and test for zero errors and zero warnings**
- [ ] **Step 3: Verify smooth 60 FPS animations, sub-pixel sliding nav pill, and 0.40ns engine dispatch**
