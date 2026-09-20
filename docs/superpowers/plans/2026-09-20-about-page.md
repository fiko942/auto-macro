# Tobelsoft Macro // About Page & System Telemetry Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement a dedicated, industrial cyber-brutalist "About System" page in pure C11 / Win32 GDI with seamless 60 FPS sliding pill navigation, architecture telemetry bento grid, and interactive system utilities.

**Architecture:** 
- Add Tab 2 (`About System`) to the navigation matrix and state machine.
- Update `AnimState` navigation targets for 3 tab slots (`y = 114.0f`, `166.0f`, `218.0f`).
- Implement `DrawAboutPage` in `main_window.c` with 3 elevated bento cards:
  1. Top Command Console with 4 Architecture KPI Tiles (`v3.0.0 PRO`, `C11 Native x64`, `Windows Win32`, `Proprietary Pro`).
  2. Core Engine Manifesto & Hardware Emulation Badges.
  3. 4-Tile Technical Specifications Matrix (Developer, Compiler `-O3`, Memory Footprint, Config Path).
  4. System Actions Bar: `[📁 Open Config Folder]`, `[📋 Copy System Info]`, `[🌐 Engine Docs]`.
- Fix typo in Settings page header (`Hardware Architecture & Diagnostics Matrix`).

**Tech Stack:** Pure C11, Win32 GDI double-buffered, `ShellExecuteW` for folder opening.

---

## File Structure & Responsibilities

1. `c_src/ui/main_window.c`:
   - Add `RECT nav_about_rc` and `bool hovered_nav_about` to `MainWindowState`.
   - Update `DrawSidebar` to render Tab 2 (`ℹ About System`) at `y = 218`.
   - Update `WM_LBUTTONDOWN` and `WM_MOUSEMOVE` to handle Tab 2 selection and hover states.
   - Implement `DrawAboutPage` with industrial bento cards, technical specs, and utility buttons.
   - Fix `_Diagnostics` typo in Settings page.
2. `c_src/ui/animation.c` & `c_src/ui/animation.h`:
   - Ensure `Animation_SetNavTarget` handles `y = 218.0f` smoothly.
3. `build.bat`:
   - Compile and verify.

---

## Tasks

### Task 1: Navigation Matrix & State Machine Expansion

**Files:**
- Modify: `c_src/ui/main_window.c`

**Interfaces:**
- Updates:
  - `MainWindowState`: `nav_about_rc`, `btn_open_config_rc`, `btn_copy_diag_rc`, `hovered_nav_about`, `hovered_open_config`, `hovered_copy_diag`.
  - Tab routing: `current_tab == 0` (Macro Hub), `current_tab == 1` (Settings), `current_tab == 2` (About).

- [ ] **Step 1: Update Navigation Geometry in `DrawSidebar`**
  Add `g_main_state.nav_about_rc = (RECT){ 20, nav_y + 104, 20 + nav_w, nav_y + 104 + nav_h };`
  Draw `ℹ  About System` at `y = 218`.

- [ ] **Step 2: Update `WM_MOUSEMOVE` and `WM_SETCURSOR`**
  Add hit-testing for `nav_about_rc` and buttons on About page.

- [ ] **Step 3: Update `WM_LBUTTONDOWN`**
  Handle click on `nav_about_rc`: switch to `current_tab = 2`, set nav pill target `Animation_SetNavTarget(218.0f, 44.0f)`.

---

### Task 2: Implement About Page Bento UI (`DrawAboutPage`)

**Files:**
- Modify: `c_src/ui/main_window.c`

**Interfaces:**
- Produces: `static void DrawAboutPage(HDC hdc, int x, int y, int width, int height);`

- [ ] **Step 1: Top Command Console for About Page**
  Page title `ℹ About Tobelsoft Macro // System Telemetry` + 4 KPI tiles (`APP VERSION: v3.0 PRO`, `ARCHITECTURE: C11 x64`, `OPTIMIZATION: Clang -O3`, `SUBSYSTEM: Win32 GDI`).

- [ ] **Step 2: Manifesto & Architecture Card**
  Large glowing emblem badge `⚡`, bold typography `TOBELSOFT MACRO // CORE ENGINE MANIFESTO`, technical paragraph, and 5 tactical badges.

- [ ] **Step 3: 4-Tile Technical Specifications Matrix**
  Bento grid with tiles for Developer Studio, Memory Footprint, Timer Resolution, and Config File Path.

- [ ] **Step 4: Interactive Utility Action Buttons**
  - `📁 Open Config Directory`: Invokes `ShellExecuteW(NULL, L"explore", ...)` to open AppData folder.
  - `📋 Copy Diagnostics`: Formats diagnostic text and puts it into Windows Clipboard via `OpenClipboard` / `SetClipboardData`.

---

### Task 3: Build, Verify, and Compile

**Files:**
- Test via `build.bat`

- [ ] **Step 1: Build `TobelsoftMacro.exe` using `build.bat`**
- [ ] **Step 2: Verify zero warnings and zero compilation errors**
- [ ] **Step 3: Test navigation between Macro Hub, Settings, and About with 60 FPS sliding pill**
