#ifndef TOBELSOFT_THEME_H
#define TOBELSOFT_THEME_H

#include <windows.h>
#include <stdbool.h>
#include "ui_icons.h"

// ============================================================================
// INDUSTRIAL OBSIDIAN PRECISION COLOR MATRIX
// ============================================================================
#define COLOR_BG_VOID         RGB(10, 12, 18)    // #0a0c12 (Deep Obsidian Base)
#define COLOR_BG_SURFACE      RGB(14, 18, 26)    // #0e121a (Structural Sidebar Shell)
#define COLOR_BG_SURFACE_ALT  RGB(18, 23, 33)    // #121721 (Secondary Shell)
#define COLOR_BG_CARD         RGB(21, 27, 39)    // #151b27 (Elevated Bento Card)
#define COLOR_BG_CARD_HOVER   RGB(29, 37, 54)    // #1d2536 (Interactive Card Hover)
#define COLOR_BG_INPUT        RGB(12, 15, 22)    // #0c0f16 (Recessed Input/Tile)
#define COLOR_BG_KEYCAP       RGB(28, 36, 52)    // #1c2434 (3D Keycap Face)
#define COLOR_BG_KEYCAP_BASE  RGB(11, 14, 22)    // #0b0e16 (3D Keycap Shadow Base)

// High-Precision Neon Accents
#define COLOR_NEON_CYAN       RGB(0, 229, 255)   // #00e5ff (Electric HUD Cyan)
#define COLOR_NEON_CYAN_DIM   RGB(0, 42, 54)     // Cyan Tinted Glass
#define COLOR_NEON_GREEN      RGB(16, 185, 129)  // #10b981 (Tactical Emerald Pulse)
#define COLOR_NEON_GREEN_DIM  RGB(6, 44, 30)     // Green Tinted Glass
#define COLOR_NEON_PINK       RGB(239, 68, 68)   // #ef4444 (Cyber Strike Crimson)
#define COLOR_NEON_PINK_DIM   RGB(52, 15, 15)    // Red Tinted Glass
#define COLOR_NEON_AMBER      RGB(245, 158, 11)  // #f59e0b (Telemetry Amber)
#define COLOR_NEON_AMBER_DIM  RGB(48, 30, 4)     // Amber Tinted Glass
#define COLOR_NEON_INDIGO     RGB(99, 102, 241)  // #6366f1 (Brand Primary Indigo)
#define COLOR_NEON_INDIGO_DIM RGB(24, 27, 58)    // Indigo Tinted Glass
#define COLOR_NEON_PURPLE     RGB(168, 85, 247)  // #a855f7 (Action Hold Violet)
#define COLOR_NEON_PURPLE_DIM RGB(40, 18, 65)

// Typography Palette (High-Contrast & Extreme Readability)
#define COLOR_TEXT_PRIMARY    RGB(255, 255, 255) // Pure Crisp White
#define COLOR_TEXT_SECONDARY  RGB(226, 232, 240) // Slate 200 (High-Contrast Subtitles)
#define COLOR_TEXT_MUTED      RGB(148, 163, 184) // Slate 400 (Clear Labels/Telemetry)
#define COLOR_TEXT_DIM        RGB(100, 116, 139) // Slate 500 (Structure Guides)
#define COLOR_TEXT_DISABLED   RGB(100, 116, 139) // Slate 500 (Disabled Elements)

// Structural Grid & Borders
#define COLOR_BORDER_SUBTLE   RGB(30, 38, 56)    // Structural Dividers (#1e2638)
#define COLOR_BORDER_STRONG   RGB(48, 60, 88)    // High-Contrast Edge (#303c58)
#define COLOR_BORDER_GLOW     RGB(0, 229, 255)   // Focused Accent Border

// Backwards compatibility aliases
#define COLOR_BG_DARKEST      COLOR_BG_VOID
#define COLOR_BG_PANEL        COLOR_BG_SURFACE
#define COLOR_PRIMARY         COLOR_NEON_INDIGO
#define COLOR_PRIMARY_HOVER   RGB(79, 70, 229)
#define COLOR_PRIMARY_LIGHT   RGB(129, 140, 248)
#define COLOR_PRIMARY_BG      COLOR_NEON_INDIGO_DIM
#define COLOR_ACCENT          COLOR_NEON_CYAN
#define COLOR_ACCENT_LIGHT    COLOR_NEON_CYAN
#define COLOR_ACCENT_GLOW     RGB(125, 211, 252)
#define COLOR_ACCENT_BG       COLOR_NEON_CYAN_DIM
#define COLOR_SUCCESS         COLOR_NEON_GREEN
#define COLOR_SUCCESS_DARK    RGB(0, 160, 85)
#define COLOR_SUCCESS_BG      COLOR_NEON_GREEN_DIM
#define COLOR_ERROR           COLOR_NEON_PINK
#define COLOR_ERROR_DARK      RGB(180, 0, 60)
#define COLOR_ERROR_BG        COLOR_NEON_PINK_DIM
#define COLOR_WARNING         COLOR_NEON_AMBER
#define COLOR_WARNING_BG      COLOR_NEON_AMBER_DIM
#define COLOR_BORDER          COLOR_BORDER_SUBTLE
#define COLOR_BORDER_LIGHT    COLOR_BORDER_STRONG
#define COLOR_BORDER_FOCUS    COLOR_NEON_CYAN

// ============================================================================
// TYPOGRAPHY HIERARCHY
// ============================================================================
typedef struct {
    HFONT font_brand_hud;    // 16px Bold Consolas (HUD Brand)
    HFONT font_title;        // 17px Bold Segoe UI (Page Title)
    HFONT font_header;       // 14px Bold Segoe UI (Card Header)
    HFONT font_body;         // 13px Regular Segoe UI
    HFONT font_body_bold;    // 13px Bold Segoe UI
    HFONT font_mono_data;    // 12px Bold Consolas (Telemetry & KPI)
    HFONT font_mono_keycap;  // 12px Bold Consolas (3D Mechanical Keycaps)
    HFONT font_mono_small;   // 10px Bold Consolas (Micro Badges & Technical Telemetry)
    HFONT font_small;        // 11px Regular Segoe UI
    HFONT font_small_bold;   // 11px Bold Segoe UI
    HFONT font_hero_icon;    // 32px Bold Segoe UI
    HFONT font_keycap;       // Compatibility alias
    HFONT font_brand;        // Compatibility alias
} ThemeFonts;

extern ThemeFonts g_theme_fonts;

// Lifecycle
void Theme_Init(void);
void Theme_Cleanup(void);

// ============================================================================
// PRECISION DRAWING PRIMITIVES
// ============================================================================
void DrawRoundedRect(HDC hdc, const RECT* rect, int radius, COLORREF fill_color, COLORREF border_color, int border_width);
void DrawHudCornerBox(HDC hdc, const RECT* rect, COLORREF fill, COLORREF border, int corner_len);
void DrawGradientVertical(HDC hdc, const RECT* rect, COLORREF top_color, COLORREF bottom_color);
void DrawGradientHorizontal(HDC hdc, const RECT* rect, COLORREF left_color, COLORREF right_color);

// Keycaps and Badges
void DrawTactileKeycap(HDC hdc, int x, int y, const wchar_t* text, bool is_selected, bool is_hovered, RECT* out_rect);
void Draw3DKeycapW(HDC hdc, int x, int y, const wchar_t* text, bool is_selected, bool is_hovered, RECT* out_rect);
void DrawKeycapChipW(HDC hdc, int x, int y, const wchar_t* text, RECT* out_rect);
void DrawKeycapChipA(HDC hdc, int x, int y, const char* text, RECT* out_rect);

void DrawHudBadge(HDC hdc, int x, int y, const wchar_t* text, COLORREF bg_col, COLORREF text_col, COLORREF border_col, RECT* out_rect);
void DrawPillBadgeW(HDC hdc, int x, int y, const wchar_t* text, COLORREF bg_col, COLORREF text_col, COLORREF border_col, RECT* out_rect);
void DrawActionPillWithIconW(HDC hdc, int x, int y, IconId icon, const wchar_t* text, COLORREF text_color, RECT* out_rect);
void DrawActionPillW(HDC hdc, int x, int y, const wchar_t* text, COLORREF text_color, RECT* out_rect);
void DrawActionPillA(HDC hdc, int x, int y, const char* text, COLORREF text_color, RECT* out_rect);

// Industrial Buttons & Switches
void DrawIndustrialButton(HDC hdc, const RECT* rect, const wchar_t* text, bool is_hovered, bool is_primary, bool is_danger, bool is_active_toggle);
void DrawSleekEngineToggle(HDC hdc, const RECT* rect, bool is_active, bool is_hovered, float pulse_phase);
void DrawGlowingButtonW(HDC hdc, const RECT* rect, const wchar_t* text, bool is_hovered, bool is_primary, bool is_active_toggle);
void DrawGlowingButtonA(HDC hdc, const RECT* rect, const char* text, bool is_hovered, bool is_primary, bool is_active_toggle);

void DrawStatusBeacon(HDC hdc, int x, int y, bool is_active, const wchar_t* label);
void DrawSwitch(HDC hdc, const RECT* rect, bool is_on);
void DrawSignalFlowConnector(HDC hdc, int x, int y_center, int width, COLORREF color);
void DrawKpiTile(HDC hdc, const RECT* rect, const wchar_t* label, const wchar_t* value, const wchar_t* subtext, COLORREF accent_color, bool has_beacon, bool beacon_active);

// ============================================================================
// ANIMATED DRAWING PRIMITIVES (60 FPS FLUID RENDERING)
// ============================================================================
void DrawSlidingNavPill(HDC hdc, int x, float y, int width, float height, float pulse_phase);
void DrawAnimatedSwitch(HDC hdc, const RECT* rect, float pos);
void DrawPulsingSignalFlowConnector(HDC hdc, int x, int y_center, int width, COLORREF base_col, float pulse_phase, bool active);
void DrawAnimatedCard(HDC hdc, const RECT* rect, float hover_t, bool is_enabled);

void EnableDarkTitleBar(HWND hwnd);

#endif // TOBELSOFT_THEME_H
