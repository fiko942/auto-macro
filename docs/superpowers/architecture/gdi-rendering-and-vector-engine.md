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

### 3.4 In-App Resource Icon Rendering (`IDI_APP_ICON`)
For the main brand icon in the title bar, sidebar card, and modal dialogs:
- The embedded Windows resource `IDI_APP_ICON` is loaded on demand using `LoadImageW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_APP_ICON), IMAGE_ICON, size, size, LR_DEFAULTCOLOR)` with static caching.
- Drawn using `DrawIconEx(hdc, x, y, hIcon, size, size, 0, NULL, DI_NORMAL)`.
- If the resource cannot be loaded, it falls back seamlessly to the geometric `DrawVectorIcon(hdc, ICON_BOLT, ...)`.

---

## 4. Owner-Drawn Custom Controls

All UI controls are implemented via custom Win32 message interception:
1. **Sleek Toggle Switches:** Animated pill toggle with ease-out sliding knob and color transition (`#10B981` active / `#1E2330` inactive).
2. **Action Step Listbox:** Owner-drawn listbox (`LBS_OWNERDRAWVARIABLE`) responding to `WM_DRAWITEM` and `WM_MEASUREITEM`, rendering drag grips, step badges, action icons, and delete buttons.
3. **Cyberpunk Dropdown / Combobox:** Owner-drawn combobox with custom arrow rendering and dark menu dropdown palette.

---

## 5. Responsive Bento Card Signal Flow & Multi-Trigger Layout Budgeting

In `c_src/ui/main_window.c`, macro cards in the Macro Hub dynamically display multi-trigger combinations and action execution pipelines while guaranteeing zero visual collision with interactive switch and action buttons:
1. **Strict Spatial Boundaries:** Right-aligned interactive controls (Mechanical Switch at `card_rc.right - 190`, Edit Button at `card_rc.right - 128`, Delete Button at `card_rc.right - 64`) define a strict rightmost boundary of `max_flow_x = card_rc.right - 210`.
2. **Proportional Trigger Budgeting:** The trigger section is allocated a maximum width budget (~45% of available flow width). Up to 2 triggers are drawn inline as tactical keycaps (`[ mouse_left ] / [ ctrl+mouse_left ]`). If additional triggers exceed the budget or total trigger count is high, an overflow HUD badge (`[ +N keys ]`) is rendered seamlessly.
3. **Card Header Multi-Trigger Indicator:** Pipelines with multiple triggers display an elevated HUD badge in the card title row (`[ N TRIGGERS ]`) alongside `[ PIPE #01 ]`.
4. **Action Sequence Overflow Containment:** Action sequence nodes (`[ ⚡ 3 ] ➔ [ ⚡ 1 ] ➔ [ ⚡ Q ]`) dynamically evaluate remaining horizontal space before rendering each step. If remaining actions would encroach on `max_flow_x`, the loop terminates cleanly and displays a `[ +N more ]` badge, ensuring the signal flow remains pixel-perfect across all screen resolutions.

