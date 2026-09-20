# TOBELSOFT MACRO // INDUSTRIAL CYBER-BRUTALIST GAMING REDESIGN PLAN
**Spec Version:** 3.0.0-PRO  
**Architecture:** Pure C11 / Win32 Native GDI Double-Buffered  
**Target Visual Benchmark:** Razer Synapse 4, Wooting Wootility, SteelSeries Sonar, Linear App Dark Precision

---

## 1. DESIGN PHILOSOPHY & PALETTE SPECIFICATION

### 1.1 Visual Identity & Aesthetic Principles
- **Substrate:** Deep Obsidian Void (`#080a0f`) + Structural Shell Surface (`#0d111a`) + Elevated Bento Cards (`#141926`).
- **Accent Signals:**
  - **Electric Cyan (`#00f0ff`):** Primary interactive focus, HUD crosshairs, active triggers, key taps.
  - **Tactical Emerald (`#00ff88`):** Engine running armed state, live telemetry beacons, input suppression shields.
  - **Cyber Crimson / Pink (`#ff0055`):** Disarmed / error state, destructive delete actions, critical warnings.
  - **Telemetry Amber (`#ffaa00`):** Timing delays, rapid-fire cycles, rate limit warnings.
  - **Hyper Indigo (`#6366f1`):** Structural highlights, primary buttons, key hold actions.
- **Typography Scale:**
  - **Brand Header / Macro Titles:** `Segoe UI Bold` (14-17px) & `Consolas Bold` (16px).
  - **Tactical Telemetry & Monospace Readouts:** `Consolas Bold` (10-12px).
  - **Body / Documentation:** `Segoe UI Regular` (11-13px).

---

## 2. STRUCTURAL LAYOUT BLUEPRINT

### 2.1 Overall Window Hierarchy (1200 x 800)
```
+-------------------+-------------------------------------------------------------------------+
| [SIDEBAR (260px)] | [TOP KPI COMMAND CONSOLE (88px)]                                       |
| - Brand HUD Logo  | - Page Title & Sub-microsecond Pipeline Subtitle                        |
| - Navigation Tabs | - 4 Modern Telemetry KPI Badges (Latency, Engine, Profiles, Driver)     |
|   * Macro Hub     | - High-Visibility Master Power Toggle Button                            |
|   * Settings      +-------------------------------------------------------------------------+
| - Hardware Stats  | [ACTION TOOLBAR (44px)]                                                 |
| - DirectInput HUD | - [+ New Macro Pipeline]  [📥 Import]  [💾 Export]   [Active Count Pill]|
|                   +-------------------------------------------------------------------------+
|                   | [SCROLLABLE MACRO CARD DECK / BENTO GRID VIEWPORT]                      |
|                   |                                                                         |
|                   |  CARD 1: [⚡ RAPID CROUCH JUMP]                           [EDIT] [DEL]  |
|                   |  TRIGGER: [ C ] ──► ACTION: [⚡ C] ──► [⏳ 15ms] ──► [🔒 SPACE 50ms]    |
|                   |  [🔁 RAPID (45ms)] [🛡️ SUPPRESS ORIGINAL INPUT] [⚡ 0.40ns DISPATCH]     |
|                   |                                                                         |
|                   |  (Or Tactical Hero Empty State HUD when no macros configured)           |
+-------------------+-------------------------------------------------------------------------+
```

---

## 3. CORE COMPONENT SPECIFICATIONS

### 3.1 Sidebar (Left Panel - 260px)
- **Top Brand Emblem:** Chamfered cyber-badge with lightning bolt `⚡` and glowing cyan/indigo border.
- **Brand Typography:** `TOBELSOFT` in bold uppercase + monospace `[ENG: C11 NATIVE // v2.4 PRO]` pill badge.
- **Navigation Deck:**
  - `⚡  Macro Dispatch Hub` (with active left laser accent bar and right pill count badge `[ 3 ARMED ]`).
  - `⚙  Global Engine Settings` (with active laser bar and gear icon).
- **Bottom Telemetry Box:**
  - Tactical corner brackets.
  - Glowing status beacon (`● RUNNING` in neon green / `○ STANDBY` in neon red).
  - Telemetry readouts: `DISPATCH: <0.001 ms (0.40ns)`, `KERNEL: DIRECTINPUT SCANCODE`, `RAM: ~8.4 MB`.

### 3.2 Top Command Console & KPI Metrics Bar
- Top-level status banner containing 4 data tiles:
  1. **DISPATCH LATENCY:** `< 0.001 ms` (with live green beacon)
  2. **ENGINE STATE:** `ARMED // RUNNING` (Green) or `STANDBY // IDLE` (Red)
  3. **ACTIVE PIPELINES:** `N / M ARMED`
  4. **INJECTION DRIVER:** `DIRECTINPUT HW`
- **Master Power Button:** Large tactile toggle with LED indicator ring, glass top sheen, and smooth hover animation.

### 3.3 Macro Deck: Visual Signal Flow Pipeline Cards
Each card visually diagrams the execution sequence:
- **Header:** Status dot (`● ON` / `○ OFF`), Macro Name (`Segoe UI Bold 14px`), Action Counter.
- **Signal Flow Row:**
  - `TRIGGER KEYCAPS` (3D mechanical keycaps with laser etched font and bottom shadow).
  - High-tech neon directional connector arrows: `──►`.
  - `ACTION SEQUENCE NODES`:
    - `[⚡ TAP KEY]` (Electric Cyan)
    - `[⏳ DELAY N ms]` (Telemetry Amber)
    - `[🔒 HOLD KEY N ms]` (Hyper Indigo)
    - `[⬇ KEY DOWN]` / `[⬆ KEY UP]` (Slate Blue)
- **Bottom Feature Badges:**
  - `🔁 Rapid (N ms)` in amber glass.
  - `🛡️ Suppress Original Input` in emerald glass.
  - `⚡ DirectInput Hardware Level` in subtle slate glass.
- **Right Action Buttons:**
  - Tactile Switch (On/Off).
  - `[ ✏ Edit ]` with blue hover glow.
  - `[ 🗑 ]` with cyber red hover glow.

### 3.4 Tactical Hero Empty State HUD
- Rendered when 0 macros are configured:
  - High-tech corner bracketed HUD frame.
  - Glowing central lightning emblem.
  - Headline: `NO MACRO PIPELINES CONFIGURED`.
  - Declassified technical explanation of DirectInput hardware dispatch.
  - High-contrast primary action CTA: `+ INITIALIZE FIRST MACRO PIPELINE`.

### 3.5 Global Settings Bento Grid Page
- **Card 1: Global Trigger Matrix (Master Toggle)**
  - Interactive 3D Keycaps for master toggle triggers.
  - Selected keycap ring, hover highlight, double-click to rebind hint.
  - `+ Add Trigger Key` and `Remove Trigger Key` buttons.
- **Card 2: 4-Tile Hardware Architecture & Telemetry Grid**
  - Modern tiles with neon corner accents:
    1. Input Injection: `DirectInput ScanCode` (Hardware driver level)
    2. Dispatch Latency: `< 0.001 ms` (0.40ns O(1) table lookup)
    3. Timer Precision: `High-Res Waitable` (Zero CPU spinlock)
    4. Memory Footprint: `~8.4 MB` (Zero GC overhead)
- **Card 3: Engine Architecture & Persistence Matrix**
  - Storage location, JSON schema compliance, auto-save status.

---

## 4. IMPLEMENTATION CHECKLIST

- [x] Plan architecture & design specs documented in `plan.md`.
- [ ] Upgrade `c_src/ui/theme.h` and `c_src/ui/theme.c` with all required drawing primitives, KPI tiles, and signal flow connectors.
- [ ] Overhaul `c_src/ui/main_window.c` with new Sidebar, Top KPI Command Console, Signal Flow Pipeline Cards, and Tactical Hero HUD.
- [ ] Refine Settings Bento Grid & Hardware Diagnostics.
- [ ] Polish modal dialogs in `c_src/ui/ui_dialogs.c`.
- [ ] Compile with `build.bat` and run benchmark verification.
