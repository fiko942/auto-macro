#include "theme.h"
#include "animation.h"
#include <dwmapi.h>
#include <uxtheme.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "msimg32.lib")

ThemeFonts g_theme_fonts = {0};

void Theme_Init(void) {
    // 1. Page & Section Titles: Crisp, bold 18-20px Segoe UI
    g_theme_fonts.font_title = CreateFontW(-20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    // 2. Brand HUD & Sub-headers: Crisp 16px Bold Segoe UI
    g_theme_fonts.font_brand_hud = CreateFontW(-16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        
    g_theme_fonts.font_header = CreateFontW(-16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    // 3. Body Text: Standard comfortable 14px Segoe UI (Medium / Bold)
    g_theme_fonts.font_body = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        
    g_theme_fonts.font_body_bold = CreateFontW(-14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    // 4. Secondary & Helper Labels: Crisp 13px Segoe UI
    g_theme_fonts.font_small = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    g_theme_fonts.font_small_bold = CreateFontW(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        
    // 5. Monospace Telemetry, Code, KPIs: 14px Bold Consolas
    g_theme_fonts.font_mono_data = CreateFontW(-14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Consolas");

    // 6. Tactile 3D Keycaps: 13px Bold Consolas for maximum key symbol clarity
    g_theme_fonts.font_mono_keycap = CreateFontW(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Consolas");

    // 7. Micro HUD Badges & Technical Indicators: 12px Bold Consolas
    g_theme_fonts.font_mono_small = CreateFontW(-12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Consolas");

    // 8. Large Hero Icons: 32px Bold Segoe UI
    g_theme_fonts.font_hero_icon = CreateFontW(-32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    g_theme_fonts.font_keycap = g_theme_fonts.font_mono_keycap;
    g_theme_fonts.font_brand = g_theme_fonts.font_title;
}

void Theme_Cleanup(void) {
    if (g_theme_fonts.font_brand_hud) DeleteObject(g_theme_fonts.font_brand_hud);
    if (g_theme_fonts.font_title) DeleteObject(g_theme_fonts.font_title);
    if (g_theme_fonts.font_header) DeleteObject(g_theme_fonts.font_header);
    if (g_theme_fonts.font_body) DeleteObject(g_theme_fonts.font_body);
    if (g_theme_fonts.font_body_bold) DeleteObject(g_theme_fonts.font_body_bold);
    if (g_theme_fonts.font_mono_data) DeleteObject(g_theme_fonts.font_mono_data);
    if (g_theme_fonts.font_mono_keycap) DeleteObject(g_theme_fonts.font_mono_keycap);
    if (g_theme_fonts.font_mono_small) DeleteObject(g_theme_fonts.font_mono_small);
    if (g_theme_fonts.font_small) DeleteObject(g_theme_fonts.font_small);
    if (g_theme_fonts.font_small_bold) DeleteObject(g_theme_fonts.font_small_bold);
    if (g_theme_fonts.font_hero_icon) DeleteObject(g_theme_fonts.font_hero_icon);
    memset(&g_theme_fonts, 0, sizeof(ThemeFonts));
}

void EnableDarkTitleBar(HWND hwnd) {
    if (!hwnd) return;
    BOOL value = TRUE;
    // DWMWA_USE_IMMERSIVE_DARK_MODE (20 on Win 11/10 20H1+, 19 on older Win 10 builds)
    if (FAILED(DwmSetWindowAttribute(hwnd, 20, &value, sizeof(value)))) {
        DwmSetWindowAttribute(hwnd, 19, &value, sizeof(value));
    }
    COLORREF caption_col = RGB(10, 12, 18);
    DwmSetWindowAttribute(hwnd, 35, &caption_col, sizeof(caption_col));
    COLORREF text_col = RGB(248, 250, 252);
    DwmSetWindowAttribute(hwnd, 36, &text_col, sizeof(text_col));
    COLORREF border_col = RGB(30, 39, 56);
    DwmSetWindowAttribute(hwnd, 34, &border_col, sizeof(border_col));
    
    // Force DWM to recompute and repaint the non-client frame
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

void DrawGradientVertical(HDC hdc, const RECT* rect, COLORREF top_color, COLORREF bottom_color) {
    if (!rect) return;
    TRIVERTEX vertex[2];
    vertex[0].x = rect->left;
    vertex[0].y = rect->top;
    vertex[0].Red = (COLOR16)(GetRValue(top_color) << 8);
    vertex[0].Green = (COLOR16)(GetGValue(top_color) << 8);
    vertex[0].Blue = (COLOR16)(GetBValue(top_color) << 8);
    vertex[0].Alpha = 0x0000;

    vertex[1].x = rect->right;
    vertex[1].y = rect->bottom;
    vertex[1].Red = (COLOR16)(GetRValue(bottom_color) << 8);
    vertex[1].Green = (COLOR16)(GetGValue(bottom_color) << 8);
    vertex[1].Blue = (COLOR16)(GetBValue(bottom_color) << 8);
    vertex[1].Alpha = 0x0000;

    GRADIENT_RECT gRect = { 0, 1 };
    GradientFill(hdc, vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
}

void DrawGradientHorizontal(HDC hdc, const RECT* rect, COLORREF left_color, COLORREF right_color) {
    if (!rect) return;
    TRIVERTEX vertex[2];
    vertex[0].x = rect->left;
    vertex[0].y = rect->top;
    vertex[0].Red = (COLOR16)(GetRValue(left_color) << 8);
    vertex[0].Green = (COLOR16)(GetGValue(left_color) << 8);
    vertex[0].Blue = (COLOR16)(GetBValue(left_color) << 8);
    vertex[0].Alpha = 0x0000;

    vertex[1].x = rect->right;
    vertex[1].y = rect->bottom;
    vertex[1].Red = (COLOR16)(GetRValue(right_color) << 8);
    vertex[1].Green = (COLOR16)(GetGValue(right_color) << 8);
    vertex[1].Blue = (COLOR16)(GetBValue(right_color) << 8);
    vertex[1].Alpha = 0x0000;

    GRADIENT_RECT gRect = { 0, 1 };
    GradientFill(hdc, vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
}

void DrawRoundedRect(HDC hdc, const RECT* rect, int radius, COLORREF fill_color, COLORREF border_color, int border_width) {
    if (!rect) return;
    HBRUSH brush = CreateSolidBrush(fill_color);
    HPEN pen = (border_width > 0) ? CreatePen(PS_SOLID, border_width, border_color) : CreatePen(PS_NULL, 0, 0);
    
    HGDIOBJ old_brush = SelectObject(hdc, brush);
    HGDIOBJ old_pen = SelectObject(hdc, pen);
    
    RoundRect(hdc, rect->left, rect->top, rect->right, rect->bottom, radius, radius);
    
    SelectObject(hdc, old_brush);
    SelectObject(hdc, old_pen);
    DeleteObject(brush);
    DeleteObject(pen);
}

void DrawHudCornerBox(HDC hdc, const RECT* rect, COLORREF fill, COLORREF border, int corner_len) {
    if (!rect) return;
    
    // Fill interior
    HBRUSH bg_br = CreateSolidBrush(fill);
    FillRect(hdc, rect, bg_br);
    DeleteObject(bg_br);
    
    // Subtle border
    HPEN border_pen = CreatePen(PS_SOLID, 1, COLOR_BORDER_SUBTLE);
    HGDIOBJ old_pen = SelectObject(hdc, border_pen);
    SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
    Rectangle(hdc, rect->left, rect->top, rect->right, rect->bottom);
    DeleteObject(border_pen);
    
    // 4 High-tech corner brackets
    if (corner_len > 0) {
        HPEN corner_pen = CreatePen(PS_SOLID, 2, border);
        SelectObject(hdc, corner_pen);
        
        // Top-Left
        MoveToEx(hdc, rect->left, rect->top + corner_len, NULL);
        LineTo(hdc, rect->left, rect->top);
        LineTo(hdc, rect->left + corner_len, rect->top);
        
        // Top-Right
        MoveToEx(hdc, rect->right - corner_len, rect->top, NULL);
        LineTo(hdc, rect->right - 1, rect->top);
        LineTo(hdc, rect->right - 1, rect->top + corner_len);
        
        // Bottom-Left
        MoveToEx(hdc, rect->left, rect->bottom - corner_len, NULL);
        LineTo(hdc, rect->left, rect->bottom - 1);
        LineTo(hdc, rect->left + corner_len, rect->bottom - 1);
        
        // Bottom-Right
        MoveToEx(hdc, rect->right - corner_len, rect->bottom - 1, NULL);
        LineTo(hdc, rect->right - 1, rect->bottom - 1);
        LineTo(hdc, rect->right - 1, rect->bottom - corner_len);
        
        SelectObject(hdc, old_pen);
        DeleteObject(corner_pen);
    } else {
        SelectObject(hdc, old_pen);
    }
}

void DrawTactileKeycap(HDC hdc, int x, int y, const wchar_t* text, bool is_selected, bool is_hovered, RECT* out_rect) {
    if (!text) return;
    
    HGDIOBJ old_font = SelectObject(hdc, g_theme_fonts.font_mono_keycap);
    SIZE sz;
    GetTextExtentPoint32W(hdc, text, (int)wcslen(text), &sz);
    
    int pad_x = 10;
    int pad_y = 4;
    int width = sz.cx + pad_x * 2;
    int height = sz.cy + pad_y * 2 + 2;
    if (width < 32) width = 32;
    
    RECT rc = { x, y, x + width, y + height };
    if (out_rect) *out_rect = rc;
    
    // Bottom 3D base shadow
    RECT shadow_rc = { x, y + 2, x + width, y + height };
    DrawRoundedRect(hdc, &shadow_rc, 6, COLOR_BG_KEYCAP_BASE, RGB(18, 22, 34), 1);
    
    // Top elevated tactile face
    RECT face_rc = { x, y, x + width, y + height - 2 };
    COLORREF face_fill = is_selected ? COLOR_NEON_CYAN_DIM : (is_hovered ? COLOR_NEON_INDIGO_DIM : COLOR_BG_KEYCAP);
    COLORREF border_col = is_selected ? COLOR_NEON_CYAN : (is_hovered ? COLOR_NEON_INDIGO : COLOR_BORDER_STRONG);
    int bwidth = (is_selected || is_hovered) ? 2 : 1;
    
    DrawRoundedRect(hdc, &face_rc, 5, face_fill, border_col, bwidth);
    
    // Top highlight sheen rim
    HPEN rim_pen = CreatePen(PS_SOLID, 1, is_selected ? RGB(125, 211, 252) : (is_hovered ? RGB(165, 180, 252) : RGB(50, 60, 90)));
    HGDIOBJ old_pen = SelectObject(hdc, rim_pen);
    MoveToEx(hdc, face_rc.left + 3, face_rc.top + 1, NULL);
    LineTo(hdc, face_rc.right - 3, face_rc.top + 1);
    SelectObject(hdc, old_pen);
    DeleteObject(rim_pen);
    
    // Laser-etched keycap text
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, is_selected ? COLOR_NEON_CYAN : (is_hovered ? COLOR_TEXT_PRIMARY : COLOR_TEXT_SECONDARY));
    
    RECT tr = face_rc;
    DrawTextW(hdc, text, -1, &tr, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(hdc, old_font);
}

void Draw3DKeycapW(HDC hdc, int x, int y, const wchar_t* text, bool is_selected, bool is_hovered, RECT* out_rect) {
    DrawTactileKeycap(hdc, x, y, text, is_selected, is_hovered, out_rect);
}

void DrawKeycapChipW(HDC hdc, int x, int y, const wchar_t* text, RECT* out_rect) {
    DrawTactileKeycap(hdc, x, y, text, false, false, out_rect);
}

void DrawKeycapChipA(HDC hdc, int x, int y, const char* text, RECT* out_rect) {
    if (!text) return;
    wchar_t wbuf[128];
    MultiByteToWideChar(CP_UTF8, 0, text, -1, wbuf, 128);
    DrawTactileKeycap(hdc, x, y, wbuf, false, false, out_rect);
}

void DrawHudBadge(HDC hdc, int x, int y, const wchar_t* text, COLORREF bg_col, COLORREF text_col, COLORREF border_col, RECT* out_rect) {
    DrawHudBadgeWithIcon(hdc, x, y, ICON_NONE, text, bg_col, text_col, border_col, out_rect);
}

void DrawPillBadgeW(HDC hdc, int x, int y, const wchar_t* text, COLORREF bg_col, COLORREF text_col, COLORREF border_col, RECT* out_rect) {
    DrawHudBadge(hdc, x, y, text, bg_col, text_col, border_col, out_rect);
}

void DrawActionPillWithIconW(HDC hdc, int x, int y, IconId icon, const wchar_t* text, COLORREF text_color, RECT* out_rect) {
    COLORREF bg = COLOR_BG_INPUT;
    COLORREF border = COLOR_BORDER_SUBTLE;
    
    if (text_color == COLOR_NEON_CYAN || text_color == COLOR_ACCENT_LIGHT) {
        bg = COLOR_NEON_CYAN_DIM;
        border = RGB(0, 180, 200);
    } else if (text_color == COLOR_NEON_AMBER || text_color == COLOR_WARNING) {
        bg = COLOR_NEON_AMBER_DIM;
        border = RGB(200, 120, 0);
    } else if (text_color == COLOR_NEON_GREEN || text_color == COLOR_SUCCESS) {
        bg = COLOR_NEON_GREEN_DIM;
        border = RGB(0, 180, 80);
    } else if (text_color == COLOR_NEON_PURPLE || text_color == COLOR_PRIMARY_LIGHT) {
        bg = COLOR_NEON_PURPLE_DIM;
        border = RGB(140, 60, 200);
    } else if (text_color == COLOR_NEON_PINK || text_color == COLOR_ERROR) {
        bg = COLOR_NEON_PINK_DIM;
        border = RGB(200, 0, 70);
    }
    
    DrawHudBadgeWithIcon(hdc, x, y, icon, text, bg, text_color, border, out_rect);
}

void DrawActionPillW(HDC hdc, int x, int y, const wchar_t* text, COLORREF text_color, RECT* out_rect) {
    DrawActionPillWithIconW(hdc, x, y, ICON_NONE, text, text_color, out_rect);
}

void DrawActionPillA(HDC hdc, int x, int y, const char* text, COLORREF text_color, RECT* out_rect) {
    if (!text) return;
    wchar_t wbuf[128];
    MultiByteToWideChar(CP_UTF8, 0, text, -1, wbuf, 128);
    DrawActionPillW(hdc, x, y, wbuf, text_color, out_rect);
}

void DrawIndustrialButton(HDC hdc, const RECT* rect, const wchar_t* text, bool is_hovered, bool is_primary, bool is_danger, bool is_active_toggle) {
    DrawIndustrialButtonWithIcon(hdc, rect, ICON_NONE, text, is_hovered, is_primary, is_danger, is_active_toggle);
}

void DrawSleekEngineToggle(HDC hdc, const RECT* rect, bool is_active, bool is_hovered, float pulse_phase) {
    if (!rect) return;
    
    COLORREF fill_top, fill_bottom, border_col, text_col, led_col;
    const wchar_t* btn_text;
    
    if (is_active) {
        // Emerald active state
        fill_top = is_hovered ? RGB(16, 72, 45) : RGB(10, 48, 30);
        fill_bottom = is_hovered ? RGB(8, 40, 24) : RGB(5, 28, 16);
        
        float pulse = 0.6f + 0.4f * sinf(pulse_phase * 2.5f);
        border_col = LerpColor(RGB(16, 185, 129), RGB(52, 211, 153), pulse);
        led_col = LerpColor(RGB(16, 185, 129), RGB(110, 231, 183), pulse);
        text_col = RGB(240, 253, 244);
        btn_text = is_hovered ? L"STOP ENGINE" : L"ARMED (ONLINE)";
    } else {
        // Amber standby state
        fill_top = is_hovered ? RGB(56, 34, 10) : RGB(36, 22, 6);
        fill_bottom = is_hovered ? RGB(32, 18, 5) : RGB(20, 12, 3);
        
        border_col = is_hovered ? RGB(251, 191, 36) : RGB(180, 100, 10);
        led_col = RGB(245, 158, 11);
        text_col = RGB(254, 243, 199);
        btn_text = L"START ENGINE";
    }
    
    int radius = 8;
    DrawRoundedRect(hdc, rect, radius, fill_bottom, border_col, is_hovered ? 2 : 1);
    
    RECT grad_rc = { rect->left + 1, rect->top + 1, rect->right - 1, rect->bottom - 1 };
    DrawGradientVertical(hdc, &grad_rc, fill_top, fill_bottom);
    
    // Top highlight rim
    HPEN rim_pen = CreatePen(PS_SOLID, 1, is_active ? RGB(110, 231, 183) : RGB(252, 211, 77));
    HGDIOBJ old_pen = SelectObject(hdc, rim_pen);
    MoveToEx(hdc, rect->left + radius, rect->top + 1, NULL);
    LineTo(hdc, rect->right - radius, rect->top + 1);
    SelectObject(hdc, old_pen);
    DeleteObject(rim_pen);
    
    // Glowing LED Dot on the left
    int led_size = 8;
    int led_x = rect->left + 14;
    int led_y = rect->top + (rect->bottom - rect->top - led_size) / 2;
    RECT led_rc = { led_x, led_y, led_x + led_size, led_y + led_size };
    DrawRoundedRect(hdc, &led_rc, led_size, led_col, led_col, 0);
    
    // Text Label
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, text_col);
    HGDIOBJ old_font = SelectObject(hdc, g_theme_fonts.font_body_bold);
    
    RECT tr = { led_x + led_size + 8, rect->top, rect->right - 10, rect->bottom };
    DrawTextW(hdc, btn_text, -1, &tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(hdc, old_font);
}

void DrawGlowingButtonW(HDC hdc, const RECT* rect, const wchar_t* text, bool is_hovered, bool is_primary, bool is_active_toggle) {
    DrawIndustrialButton(hdc, rect, text, is_hovered, is_primary, false, is_active_toggle);
}

void DrawGlowingButtonA(HDC hdc, const RECT* rect, const char* text, bool is_hovered, bool is_primary, bool is_active_toggle) {
    if (!text) {
        DrawGlowingButtonW(hdc, rect, NULL, is_hovered, is_primary, is_active_toggle);
        return;
    }
    wchar_t wbuf[256];
    MultiByteToWideChar(CP_UTF8, 0, text, -1, wbuf, 256);
    DrawGlowingButtonW(hdc, rect, wbuf, is_hovered, is_primary, is_active_toggle);
}

void DrawStatusBeacon(HDC hdc, int x, int y, bool is_active, const wchar_t* label) {
    COLORREF col = is_active ? COLOR_NEON_GREEN : COLOR_NEON_PINK;
    COLORREF dim_col = is_active ? RGB(6, 44, 30) : RGB(52, 15, 15);
    
    RECT beacon_rc = { x, y, x + 8, y + 8 };
    DrawRoundedRect(hdc, &beacon_rc, 4, col, dim_col, 0);
    
    if (label) {
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, col);
        HGDIOBJ old_font = SelectObject(hdc, g_theme_fonts.font_mono_small);
        
        RECT text_rc = { x + 14, y - 2, x + 200, y + 12 };
        DrawTextW(hdc, label, -1, &text_rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(hdc, old_font);
    }
}

void DrawSwitch(HDC hdc, const RECT* rect, bool is_on) {
    DrawAnimatedSwitch(hdc, rect, is_on ? 1.0f : 0.0f);
}

void DrawSignalFlowConnector(HDC hdc, int x, int y_center, int width, COLORREF color) {
    DrawPulsingSignalFlowConnector(hdc, x, y_center, width, color, 0.0f, false);
}

void DrawKpiTile(HDC hdc, const RECT* rect, const wchar_t* label, const wchar_t* value, const wchar_t* subtext, COLORREF accent_color, bool has_beacon, bool beacon_active) {
    if (!rect) return;
    
    // Elevated bento tile background with subtle gradient
    COLORREF bg_top = COLOR_BG_CARD;
    COLORREF bg_bottom = COLOR_BG_INPUT;
    DrawRoundedRect(hdc, rect, 8, bg_bottom, COLOR_BORDER_SUBTLE, 1);
    
    RECT grad_rc = { rect->left + 1, rect->top + 1, rect->right - 1, rect->bottom - 1 };
    DrawGradientVertical(hdc, &grad_rc, bg_top, bg_bottom);
    
    // Top accent indicator line
    RECT top_bar = { rect->left + 8, rect->top, rect->right - 8, rect->top + 2 };
    DrawRoundedRect(hdc, &top_bar, 1, accent_color, accent_color, 0);
    
    // Text Labels
    SetBkMode(hdc, TRANSPARENT);
    
    // Label Header
    SetTextColor(hdc, COLOR_TEXT_MUTED);
    HGDIOBJ old_font = SelectObject(hdc, g_theme_fonts.font_mono_small);
    
    if (has_beacon) {
        DrawStatusBeacon(hdc, rect->left + 12, rect->top + 10, beacon_active, NULL);
        RECT lbl_rc = { rect->left + 26, rect->top + 8, rect->right - 10, rect->top + 22 };
        DrawTextW(hdc, label, -1, &lbl_rc, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
    } else {
        RECT lbl_rc = { rect->left + 12, rect->top + 8, rect->right - 10, rect->top + 22 };
        DrawTextW(hdc, label, -1, &lbl_rc, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
    }
    
    // Primary Value
    SetTextColor(hdc, COLOR_TEXT_PRIMARY);
    SelectObject(hdc, g_theme_fonts.font_header);
    RECT val_rc = { rect->left + 12, rect->top + 22, rect->right - 10, rect->top + 42 };
    DrawTextW(hdc, value, -1, &val_rc, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
    
    // Subtext
    if (subtext) {
        SetTextColor(hdc, COLOR_TEXT_MUTED);
        SelectObject(hdc, g_theme_fonts.font_small);
        RECT sub_rc = { rect->left + 12, rect->top + 40, rect->right - 10, rect->top + 54 };
        DrawTextW(hdc, subtext, -1, &sub_rc, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
    }
    SelectObject(hdc, old_font);
}

// ============================================================================
// 60 FPS FLUID ANIMATION PRIMITIVES
// ============================================================================
void DrawSlidingNavPill(HDC hdc, int x, float y, int width, float height, float pulse_phase) {
    int iy = (int)roundf(y);
    int ih = (int)roundf(height);
    
    RECT pill_rc = { x, iy, x + width, iy + ih };
    
    // Smooth elevated active card background
    COLORREF fill_top = RGB(26, 36, 58);
    COLORREF fill_bottom = RGB(16, 22, 36);
    DrawRoundedRect(hdc, &pill_rc, 8, fill_bottom, COLOR_NEON_CYAN, 1);
    
    RECT grad_rc = { pill_rc.left + 1, pill_rc.top + 1, pill_rc.right - 1, pill_rc.bottom - 1 };
    DrawGradientVertical(hdc, &grad_rc, fill_top, fill_bottom);
    
    // Top highlight sheen rim
    HPEN sheen_pen = CreatePen(PS_SOLID, 1, RGB(45, 68, 105));
    HGDIOBJ old_pen = SelectObject(hdc, sheen_pen);
    MoveToEx(hdc, pill_rc.left + 8, pill_rc.top + 1, NULL);
    LineTo(hdc, pill_rc.right - 8, pill_rc.top + 1);
    SelectObject(hdc, old_pen);
    DeleteObject(sheen_pen);
    
    // Distinct glowing laser indicator bar on the left edge
    float pulse = 0.75f + 0.25f * sinf(pulse_phase * 2.0f);
    COLORREF laser_col = LerpColor(RGB(0, 190, 255), COLOR_NEON_CYAN, pulse);
    
    RECT bar_rc = { pill_rc.left + 2, pill_rc.top + 8, pill_rc.left + 6, pill_rc.bottom - 8 };
    DrawRoundedRect(hdc, &bar_rc, 2, laser_col, laser_col, 0);
}

void DrawAnimatedSwitch(HDC hdc, const RECT* rect, float pos) {
    if (!rect) return;
    if (pos < 0.0f) pos = 0.0f;
    if (pos > 1.0f) pos = 1.0f;
    
    int h = rect->bottom - rect->top;
    
    // Smooth color morphing of switch track
    // OFF state: sleek industrial dark slate
    // ON state: vibrant tactical neon emerald
    COLORREF off_bg = RGB(22, 28, 42);
    COLORREF on_bg = RGB(16, 185, 129);
    COLORREF track_bg = LerpColor(off_bg, on_bg, pos);
    
    COLORREF off_border = COLOR_BORDER_STRONG;
    COLORREF on_border = RGB(52, 211, 153);
    COLORREF track_border = LerpColor(off_border, on_border, pos);
    
    // Outer rounded track
    DrawRoundedRect(hdc, rect, h, track_bg, track_border, 1);
    
    // Inner circular knob sliding smoothly with 3D drop shadow
    int knob_margin = 3;
    int knob_size = h - (knob_margin * 2);
    float min_x = (float)(rect->left + knob_margin);
    float max_x = (float)(rect->right - knob_size - knob_margin);
    float knob_xf = min_x + (max_x - min_x) * pos;
    int knob_x = (int)roundf(knob_xf);
    int knob_y = rect->top + knob_margin;
    
    // Knob shadow
    RECT knob_shadow = { knob_x, knob_y + 1, knob_x + knob_size, knob_y + knob_size + 1 };
    DrawRoundedRect(hdc, &knob_shadow, knob_size, RGB(10, 14, 22), RGB(10, 14, 22), 0);
    
    // Knob face
    RECT knob_rc = { knob_x, knob_y, knob_x + knob_size, knob_y + knob_size };
    COLORREF knob_border = LerpColor(RGB(148, 163, 184), RGB(167, 243, 208), pos);
    DrawRoundedRect(hdc, &knob_rc, knob_size, COLOR_TEXT_PRIMARY, knob_border, 1);
}

void DrawPulsingSignalFlowConnector(HDC hdc, int x, int y_center, int width, COLORREF base_col, float pulse_phase, bool active) {
    COLORREF line_col = base_col;
    if (active) {
        float pulse = 0.5f + 0.5f * sinf(pulse_phase * 2.0f);
        line_col = LerpColor(base_col, COLOR_NEON_CYAN, pulse);
    }
    
    HPEN line_pen = CreatePen(PS_SOLID, 2, line_col);
    HGDIOBJ old_pen = SelectObject(hdc, line_pen);
    
    // Main horizontal flow line
    MoveToEx(hdc, x, y_center, NULL);
    LineTo(hdc, x + width - 6, y_center);
    
    // Arrowhead chevron
    MoveToEx(hdc, x + width - 8, y_center - 4, NULL);
    LineTo(hdc, x + width - 2, y_center);
    LineTo(hdc, x + width - 8, y_center + 4);
    
    SelectObject(hdc, old_pen);
    DeleteObject(line_pen);
    
    // If active, draw moving laser packet along the pipeline
    if (active) {
        float phase_norm = fmodf(pulse_phase, 2.0f * 3.14159f) / (2.0f * 3.14159f);
        int dot_x = x + (int)((width - 8) * phase_norm);
        RECT dot_rc = { dot_x - 2, y_center - 2, dot_x + 2, y_center + 2 };
        DrawRoundedRect(hdc, &dot_rc, 2, COLOR_NEON_CYAN, COLOR_NEON_CYAN, 0);
    }
}

void DrawAnimatedCard(HDC hdc, const RECT* rect, float hover_t, bool is_enabled) {
    if (!rect) return;
    if (hover_t < 0.0f) hover_t = 0.0f;
    if (hover_t > 1.0f) hover_t = 1.0f;
    
    COLORREF bg_rest = COLOR_BG_CARD;
    COLORREF bg_hover = COLOR_BG_CARD_HOVER;
    COLORREF fill_color = LerpColor(bg_rest, bg_hover, hover_t);
    
    COLORREF border_rest = COLOR_BORDER_SUBTLE;
    COLORREF border_hover = COLOR_NEON_INDIGO;
    COLORREF border_color = LerpColor(border_rest, border_hover, hover_t);
    
    int border_width = (hover_t > 0.3f) ? 2 : 1;
    DrawRoundedRect(hdc, rect, 8, fill_color, border_color, border_width);
    
    // Top Glass Highlight Rim
    HPEN glass_pen = CreatePen(PS_SOLID, 1, LerpColor(RGB(24, 32, 48), RGB(60, 75, 110), hover_t));
    HGDIOBJ old_pen = SelectObject(hdc, glass_pen);
    MoveToEx(hdc, rect->left + 10, rect->top + 1, NULL);
    LineTo(hdc, rect->right - 10, rect->top + 1);
    SelectObject(hdc, old_pen);
    DeleteObject(glass_pen);
    
    // Left Laser Status Stripe
    COLORREF stripe_off = COLOR_BORDER_SUBTLE;
    COLORREF stripe_on = COLOR_NEON_GREEN;
    COLORREF stripe_col = is_enabled ? stripe_on : stripe_off;
    
    RECT stripe_rc = { rect->left, rect->top + 10, rect->left + 4, rect->bottom - 10 };
    DrawRoundedRect(hdc, &stripe_rc, 2, stripe_col, stripe_col, 0);
}
