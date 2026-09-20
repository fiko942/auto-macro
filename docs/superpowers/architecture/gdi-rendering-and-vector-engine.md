# Win32 GDI Rendering & Vector Icon Engine Architecture

**Version:** 2.0 Native C  
**Date:** 2026-09-21  

---

## 1. Zero-Flicker GDI Double Buffering

Standard Win32 painting triggers window redraws directly on the screen DC, leading to noticeable tearing and flickering during resizing and rapid updates.

Tobelsoft Macro enforces double buffering on all window classes:
1. `WM_ERASEBKGND` returns `1` (bypasses default background clearing).
2. `WM_PAINT` creates a memory DC (`CreateCompatibleDC`), allocates a memory bitmap matching client dimensions, renders all background bento cards, vector icons, text, and buttons into the memory DC, and executes a single atomic `BitBlt` copy to the screen DC.

```c
PAINTSTRUCT ps;
HDC hdc_screen = BeginPaint(hwnd, &ps);
RECT rc;
GetClientRect(hwnd, &rc);

HDC hdc_mem = CreateCompatibleDC(hdc_screen);
HBITMAP hbm_mem = CreateCompatibleBitmap(hdc_screen, rc.right, rc.bottom);
HBITMAP hbm_old = (HBITMAP)SelectObject(hdc_mem, hbm_mem);

// Render Cyber UI, Bento Grids, and Vector Icons into hdc_mem
DrawCyberDashboard(hdc_mem, rc);

// Atomic Blit to Screen
BitBlt(hdc_screen, 0, 0, rc.right, rc.bottom, hdc_mem, 0, 0, SRCCOPY);

SelectObject(hdc_mem, hbm_old);
DeleteObject(hbm_mem);
DeleteDC(hdc_mem);
EndPaint(hwnd, &ps);
```

---

## 2. Cyberpunk Design Tokens & Color Matrix

All color tokens in `c_src/ui/theme.h` are defined as native Win32 `COLORREF` values:

```c
#define COLOR_BG_MAIN        RGB(8, 9, 12)       // #08090C Deep void
#define COLOR_BG_CARD        RGB(15, 17, 23)     // #0F1117 Card surface
#define COLOR_BG_CARD_HOVER  RGB(22, 26, 36)     // #161A24 Hover card
#define COLOR_BORDER_DEFAULT RGB(30, 35, 48)     // #1E2330 Subtle border
#define COLOR_BORDER_ACTIVE  RGB(99, 102, 241)   // #6366F1 Cyber indigo
#define COLOR_ACCENT_PRIMARY RGB(99, 102, 241)   // #6366F1 Primary accent
#define COLOR_ACCENT_SUCCESS RGB(16, 185, 129)   // #10B981 Neon emerald
#define COLOR_ACCENT_DANGER  RGB(239, 68, 68)    // #EF4444 Laser red
#define COLOR_ACCENT_WARNING RGB(245, 158, 11)   // #F59E0B Amber warning
#define COLOR_TEXT_PRIMARY   RGB(248, 250, 252)  // #F8FAFC Pure bright text
#define COLOR_TEXT_MUTED     RGB(148, 163, 184)  // #94A3B8 Secondary text
#define COLOR_TEXT_DIM       RGB(71, 85, 105)    // #475569 Disabled / dim text
```

---

## 3. Geometric Vector Icon Algorithms

Rather than loading SVG files or font textures at runtime, the Vector Icon Engine computes coordinates mathematically on-the-fly:

### 3.1 Lightning Bolt (`ICON_BOLT`)
Constructed from a 7-point polygon with normalized vertex coordinates:
- `P0 = (0.58, 0.00)` (Top point)
- `P1 = (0.15, 0.52)` (Mid left outer)
- `P2 = (0.48, 0.52)` (Mid left inner)
- `P3 = (0.38, 1.00)` (Bottom spike)
- `P4 = (0.85, 0.44)` (Mid right outer)
- `P5 = (0.52, 0.44)` (Mid right inner)
- `P6 = (0.58, 0.00)` (Return to start)

### 3.2 Target Reticle (`ICON_TARGET`)
- Outer circle: Radius $R = \text{size} / 2 - 1$.
- Inner circle: Radius $R/2$.
- Center dot: Solid ellipse $2 \times 2$ px.
- 4 Crosshair ticks: Stems extending inward from outer perimeter.

### 3.3 Repeat Cycle (`ICON_REPEAT`)
- Upper path: Arc / rounded line moving left-to-right with arrowhead at $(x + \text{size} - 2, y + \text{size}/3)$.
- Lower path: Arc / rounded line moving right-to-left with arrowhead at $(x + 2, y + 2\text{size}/3)$.

---

## 4. Owner-Drawn Custom Controls

All UI controls are implemented via custom Win32 message interception:
1. **Sleek Toggle Switches:** Animated pill toggle with ease-out sliding knob and color transition (`#10B981` active / `#1E2330` inactive).
2. **Action Step Listbox:** Owner-drawn listbox (`LBS_OWNERDRAWVARIABLE`) responding to `WM_DRAWITEM` and `WM_MEASUREITEM`, rendering drag grips, step badges, action icons, and delete buttons.
3. **Cyberpunk Dropdown / Combobox:** Owner-drawn combobox with custom arrow rendering and dark menu dropdown palette.
