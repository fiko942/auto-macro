#ifndef TOBELSOFT_UI_ICONS_H
#define TOBELSOFT_UI_ICONS_H

#include <windows.h>
#include <stdbool.h>
#include "../common/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// VECTOR ICON IDENTIFIERS
// Dedicated vector graphics engine for Tobelsoft Macro Native C HUD
// ============================================================================
typedef enum {
    ICON_NONE = 0,
    ICON_BOLT,              // Lightning bolt (Speed, Macro Hub, DirectInput Trigger)
    ICON_SETTINGS,          // Precision gear/cog (Settings Matrix, Engine preferences)
    ICON_INFO,              // Info circle (About System, Specifications)
    ICON_PLUS,              // Plus sign (Add Hotkey, Add Trigger, Add Step, Add Key)
    ICON_CLOSE,             // Cross / X (Close window, Cancel dialog, Dismiss)
    ICON_EDIT,              // Pencil / Edit tool (Edit Macro, Edit Action Step)
    ICON_TRASH,             // Wastebasket / Delete (Remove Hotkey, Remove Action Step, Clear)
    ICON_IMPORT,            // Inward arrow into tray (Import Profile)
    ICON_EXPORT,            // Outward arrow from tray (Export Profile)
    ICON_SAVE,              // Floppy disk / chip save (Save Configuration, Save Settings)
    ICON_FOLDER,            // Directory folder (Open AppData Profile Directory)
    ICON_COPY,              // Dual clipboard sheets (Copy Telemetry / Diagnostics)
    ICON_SHIELD,            // Security shield (Hardware Safety Lock, Bypasses)
    ICON_TARGET,            // Precision crosshair / reticle (DirectInput Capture HUD, Capture trigger)
    ICON_CLOCK,             // Precision clock / delay timer (Delay Action, High-Res Timers)
    ICON_LOCK,              // Secure padlock (Key Hold Action, Atomic Lock)
    ICON_ARROW_DOWN,        // Downward chevron / arrow (Key Down Action, Move Step Down)
    ICON_ARROW_UP,          // Upward chevron / arrow (Key Up Action, Move Step Up)
    ICON_SEQUENCE,          // Stepping blocks sequence (Key Sequence Action, Execution Pipeline)
    ICON_BELL,              // Tactical notification chime bell (Audio Chime Setting)
    ICON_STATS,             // Bar graph / analytics chart (Technical Specifications)
    ICON_TOOLS,             // Mechanical wrench / utility tool (System Utilities)
    ICON_CHECK,             // Verification checkmark (Update Step, Success Status)
    ICON_PLAY,              // Right-pointing play triangle (Start Macro Engine)
    ICON_STOP,              // Solid stop square (Stop Macro Engine)
    ICON_KEYBOARD,          // Physical keyboard grid (Key Trigger, Scancode Binding)
    ICON_RESET,             // Circular reset arrow (Reset to Defaults)
    ICON_CPU,               // Microchip processor with pins (Kernel Architecture, Hardware Scancodes)
    ICON_GAUGE,             // High-speed tachometer / speedometer (0.40ns Latency, Telemetry)
    ICON_MINIMIZE,          // Horizontal bar (Titlebar Minimize)
    ICON_MAXIMIZE,          // Square frame (Titlebar Maximize)
    ICON_RESTORE,           // Overlapping square frames (Titlebar Restore)
    ICON_GRIP,              // Vertical 6-dot drag reordering grip handle
    ICON_REPEAT             // Loop / repeat arrows (Repeat while held)
} IconId;

// ============================================================================
// CORE VECTOR ICON DRAWING API (GDI+ SUBPIXEL ANTI-ALIASED ENGINE)
// ============================================================================

/**
 * Initialize and shutdown GDI+ vector graphics subsystem.
 */
void UiIcons_Init(void);
void UiIcons_Cleanup(void);

/**
 * Draw a vector icon at specific top-left coordinates (x, y) with bounding size (pixels).
 */
void DrawVectorIcon(HDC hdc, IconId icon, int x, int y, int size, COLORREF color);

/**
 * Draw a vector icon centered within a given bounding RECT.
 */
void DrawVectorIconCentered(HDC hdc, IconId icon, const RECT* rect, int size, COLORREF color);

/**
 * Helper to map an ActionType enum directly to its corresponding vector IconId.
 */
IconId GetActionTypeIcon(ActionType type);

// ============================================================================
// INTEGRATED THEME COMPONENT DRAWING WITH VECTOR ICONS
// ============================================================================

/**
 * Render a sleek industrial button with an optional vector icon aligned beside the text
 * (or centered if text is NULL / empty).
 */
void DrawIndustrialButtonWithIcon(HDC hdc, const RECT* rect, IconId icon, const wchar_t* text, 
                                 bool is_hovered, bool is_primary, bool is_danger, bool is_active_toggle);

/**
 * Render a HUD pill badge with a crisp vector icon followed by label text.
 */
void DrawHudBadgeWithIcon(HDC hdc, int x, int y, IconId icon, const wchar_t* text, 
                          COLORREF bg_col, COLORREF text_col, COLORREF border_col, RECT* out_rect);

#ifdef __cplusplus
}
#endif

#endif // TOBELSOFT_UI_ICONS_H
