#include "ui_dialogs.h"
#include "theme.h"
#include "ui_icons.h"
#include "../resource.h"
#include "../core/input_hook.h"
#include "../common/utils.h"
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#pragma comment(lib, "comctl32.lib")

// ============================================================================
// 1. INPUT CAPTURE MODAL DIALOG (DIRECTINPUT HARDWARE HOOK HUD)
// ============================================================================
typedef struct {
    HWND hwnd;
    char captured[MAX_KEY_NAME_LEN];
    bool done;
    bool success;
} CaptureDialogState;

static CaptureDialogState g_capture_state;
static uint8_t g_last_live_mods = 0;

static void OnCaptureFinished(const char* key_name, const char* display_name) {
    (void)display_name;
    if (key_name && key_name[0]) {
        StrCopySafe(g_capture_state.captured, key_name, sizeof(g_capture_state.captured));
        g_capture_state.success = true;
    }
    g_capture_state.done = true;
    if (g_capture_state.hwnd && IsWindow(g_capture_state.hwnd)) {
        PostMessageW(g_capture_state.hwnd, WM_CLOSE, 0, 0);
    }
}

static LRESULT CALLBACK CaptureWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            SetTimer(hwnd, 1001, 30, NULL);
            return 0;
            
        case WM_TIMER: {
            if (wParam == 1001) {
                uint8_t current_mods = InputHook_GetLiveModifiers();
                if (current_mods != g_last_live_mods) {
                    g_last_live_mods = current_mods;
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            return 0;
        }

        case WM_LBUTTONDOWN: {
            int y = GET_Y_LPARAM(lParam);
            if (y < 34) {
                ReleaseCapture();
                SendMessageW(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
                return 0;
            }
            break;
        }

        case WM_DRAWITEM: {
            DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;
            if (dis->CtlType == ODT_BUTTON && dis->CtlID == 303) {
                bool is_selected = (dis->itemState & ODS_SELECTED);
                COLORREF bg_col = is_selected ? RGB(185, 28, 28) : RGB(21, 27, 39);
                COLORREF border_col = is_selected ? RGB(239, 68, 68) : COLOR_BORDER_SUBTLE;
                DrawRoundedRect(dis->hDC, &dis->rcItem, 4, bg_col, border_col, 1);
                
                COLORREF icon_col = is_selected ? RGB(255, 255, 255) : RGB(148, 163, 184);
                DrawVectorIconCentered(dis->hDC, ICON_CLOSE, &dis->rcItem, 10, icon_col);
                return TRUE;
            }
            break;
        }

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == 303) { // Close button
                InputHook_StopCapture();
                g_capture_state.success = false;
                g_capture_state.done = true;
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        }
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);
            
            // Double buffered background
            HBRUSH bg_br = CreateSolidBrush(COLOR_BG_PANEL);
            FillRect(hdc, &rc, bg_br);
            DeleteObject(bg_br);
            
            // Outer High-Tech Frame with Neon Cyan Border
            DrawRoundedRect(hdc, &rc, 8, COLOR_BG_PANEL, COLOR_NEON_CYAN, 2);
            
            // Top Titlebar Header (y: 0..34)
            RECT hdr_rc = { 1, 1, rc.right - 1, 34 };
            DrawGradientVertical(hdc, &hdr_rc, RGB(18, 24, 36), RGB(10, 13, 20));
            
            HPEN div_pen = CreatePen(PS_SOLID, 1, RGB(30, 42, 60));
            HGDIOBJ old_p = SelectObject(hdc, div_pen);
            MoveToEx(hdc, 0, 34, NULL);
            LineTo(hdc, rc.right, 34);
            SelectObject(hdc, old_p);
            DeleteObject(div_pen);
            
            // Titlebar Icon & Text
            static HICON s_hud_icon = NULL;
            static bool s_hud_icon_loaded = false;
            if (!s_hud_icon_loaded) {
                s_hud_icon = (HICON)LoadImageW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_APP_ICON), IMAGE_ICON, 14, 14, LR_DEFAULTCOLOR);
                s_hud_icon_loaded = true;
            }
            if (s_hud_icon) {
                DrawIconEx(hdc, 14, 9, s_hud_icon, 14, 14, 0, NULL, DI_NORMAL);
            } else {
                DrawVectorIcon(hdc, ICON_BOLT, 14, 9, 14, COLOR_NEON_CYAN);
            }
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, COLOR_NEON_CYAN);
            SelectObject(hdc, g_theme_fonts.font_mono_small);
            RECT title_rc = { 34, 8, rc.right - 40, 26 };
            DrawTextW(hdc, L"DIRECTINPUT CAPTURE HUD // LIVE HARDWARE HOOK", -1, &title_rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            
            // Center HUD Card
            RECT hud_card = { 16, 46, rc.right - 16, rc.bottom - 16 };
            DrawRoundedRect(hdc, &hud_card, 6, COLOR_BG_CARD, COLOR_BORDER_SUBTLE, 1);
            
            uint8_t live_mods = InputHook_GetLiveModifiers();
            if (live_mods != 0) {
                // Live held modifiers display
                int chip_x = 32;
                int chip_y = 58;
                
                SetTextColor(hdc, COLOR_TEXT_PRIMARY);
                SelectObject(hdc, g_theme_fonts.font_body_bold);
                RECT mod_hdr = { 32, 54, rc.right - 32, 72 };
                DrawTextW(hdc, L"HOLDING MODIFIERS DETECTED:", -1, &mod_hdr, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);
                
                chip_y = 78;
                
                // Draw active modifier chips
                struct { uint8_t mask; const wchar_t* name; COLORREF col; } mods_list[] = {
                    { MODIFIER_CTRL,  L"CTRL",  COLOR_NEON_CYAN },
                    { MODIFIER_WIN,   L"WIN",   COLOR_NEON_PURPLE },
                    { MODIFIER_ALT,   L"ALT",   COLOR_NEON_AMBER },
                    { MODIFIER_SHIFT, L"SHIFT", COLOR_NEON_INDIGO }
                };
                
                int total_w = 0;
                for (int i = 0; i < 4; i++) {
                    if (live_mods & mods_list[i].mask) {
                        total_w += 64 + 8; // chip width + gap
                    }
                }
                total_w += 140; // width for "+ [ PRESS KEY / CLICK ]"
                chip_x = (rc.right - total_w) / 2;
                if (chip_x < 24) chip_x = 24;
                
                for (int i = 0; i < 4; i++) {
                    if (live_mods & mods_list[i].mask) {
                        RECT out_rc;
                        DrawHudBadgeWithIcon(hdc, chip_x, chip_y, ICON_LOCK, mods_list[i].name, RGB(18, 24, 38), mods_list[i].col, mods_list[i].col, &out_rc);
                        chip_x = out_rc.right + 4;
                        
                        // '+' separator
                        SetTextColor(hdc, COLOR_TEXT_MUTED);
                        SelectObject(hdc, g_theme_fonts.font_mono_small);
                        RECT plus_rc = { chip_x, chip_y, chip_x + 12, chip_y + 24 };
                        DrawTextW(hdc, L"+", -1, &plus_rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                        chip_x += 16;
                    }
                }
                
                // Trailing target action prompt
                RECT out_prompt;
                DrawHudBadgeWithIcon(hdc, chip_x, chip_y, ICON_TARGET, L"Click or Press", RGB(16, 28, 24), COLOR_NEON_GREEN, COLOR_NEON_GREEN, &out_prompt);
            } else {
                // Default Prompt Header with Vector Reticle
                SetTextColor(hdc, COLOR_TEXT_PRIMARY);
                SelectObject(hdc, g_theme_fonts.font_title);
                SIZE pr_sz;
                GetTextExtentPoint32W(hdc, L"Press any Key or Mouse Button...", 32, &pr_sz);
                int total_pr_w = 20 + 10 + pr_sz.cx;
                int pr_start_x = (rc.right - total_pr_w) / 2;
                DrawVectorIcon(hdc, ICON_TARGET, pr_start_x, 62, 18, COLOR_NEON_CYAN);
                RECT pr_rc = { pr_start_x + 26, 60, pr_start_x + 26 + pr_sz.cx + 10, 86 };
                DrawTextW(hdc, L"Press any Key or Mouse Button...", -1, &pr_rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            }
            
            // Subtitle / Supported Inputs
            SetTextColor(hdc, COLOR_TEXT_MUTED);
            SelectObject(hdc, g_theme_fonts.font_body);
            RECT sub_rc = { 20, 114, rc.right - 20, 134 };
            DrawTextW(hdc, L"Keyboard keys, Mouse buttons (Left/Right/Middle/X1/X2), Combos (Win+, Ctrl+, Alt+)", -1, &sub_rc, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);
            
            // Technical Tag
            SetTextColor(hdc, COLOR_NEON_CYAN);
            SelectObject(hdc, g_theme_fonts.font_mono_small);
            RECT tag_rc = { 20, 138, rc.right - 20, 154 };
            DrawTextW(hdc, L"RAW HARDWARE SCANCODE DISPATCH // SUB-MICROSECOND PRECISION", -1, &tag_rc, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);
            
            // Cancel Hint
            SetTextColor(hdc, COLOR_TEXT_DISABLED);
            SelectObject(hdc, g_theme_fonts.font_small);
            RECT esc_rc = { 20, 160, rc.right - 20, 180 };
            DrawTextW(hdc, L"Press [ ESC ] to cancel capture", -1, &esc_rc, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                InputHook_StopCapture();
                g_capture_state.success = false;
                g_capture_state.done = true;
                DestroyWindow(hwnd);
                return 0;
            }
            break;
            
        case WM_CLOSE:
            KillTimer(hwnd, 1001);
            InputHook_StopCapture();
            DestroyWindow(hwnd);
            return 0;
            
        case WM_DESTROY:
            KillTimer(hwnd, 1001);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool ShowInputCaptureDialog(HWND parent_hwnd, const char* title, char* out_key_name, size_t max_len) {
    if (!out_key_name || max_len == 0) return false;
    (void)title;
    
    memset(&g_capture_state, 0, sizeof(CaptureDialogState));
    
    static bool class_registered = false;
    if (!class_registered) {
        WNDCLASSW wc = {0};
        wc.lpfnWndProc = CaptureWndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = L"TobelsoftCaptureDialogClass";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP_ICON));
        RegisterClassW(&wc);
        class_registered = true;
    }
    
    RECT parent_rc = {0, 0, 1000, 700};
    if (parent_hwnd && IsWindow(parent_hwnd)) {
        GetWindowRect(parent_hwnd, &parent_rc);
    }
    int w = 480, h = 210;
    int x = parent_rc.left + ((parent_rc.right - parent_rc.left) - w) / 2;
    int y = parent_rc.top + ((parent_rc.bottom - parent_rc.top) - h) / 2;
    if (x < 0) x = 100;
    if (y < 0) y = 100;
    
    HWND hwnd = CreateWindowExW(
        WS_EX_TOPMOST,
        L"TobelsoftCaptureDialogClass",
        L"DirectInput Hardware Capture",
        WS_POPUP | WS_CLIPCHILDREN,
        x, y, w, h,
        parent_hwnd, NULL, GetModuleHandle(NULL), NULL
    );
    
    if (!hwnd) return false;
    
    // Close button
    CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, w - 34, 4, 28, 24, hwnd, (HMENU)303, GetModuleHandle(NULL), NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    g_capture_state.hwnd = hwnd;
    if (parent_hwnd && IsWindow(parent_hwnd)) {
        EnableWindow(parent_hwnd, FALSE);
    }
    
    InputHook_StartCapture(OnCaptureFinished, false);
    
    MSG msg;
    while (IsWindow(hwnd) && GetMessageW(&msg, NULL, 0, 0)) {
        if (msg.message == WM_QUIT) {
            PostQuitMessage((int)msg.wParam);
            break;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    
    if (parent_hwnd && IsWindow(parent_hwnd)) {
        EnableWindow(parent_hwnd, TRUE);
        SetActiveWindow(parent_hwnd);
        SetForegroundWindow(parent_hwnd);
    }
    
    if (g_capture_state.success && g_capture_state.captured[0]) {
        StrCopySafe(out_key_name, g_capture_state.captured, max_len);
        return true;
    }
    return false;
}

// ============================================================================
// 2. ADD / EDIT HOTKEY MACRO MODAL DIALOG (AUDITED CYBER HUD)
// ============================================================================
typedef struct {
    HWND hwnd;
    HWND btn_close;
    HWND edit_name;
    
    // Multiple Triggers Section
    HWND list_triggers;
    HWND btn_add_trigger;
    HWND btn_remove_trigger;
    
    // Action Sequence Pipeline Section
    HWND list_actions;
    HWND btn_move_up;
    HWND btn_move_down;
    HWND btn_del_action;
    HWND btn_clear_actions;
    
    // Drag-to-Reorder State for list_actions
    bool is_dragging;
    int drag_from_idx;
    int drag_target_idx;
    POINT drag_start_pt;
    WNDPROC orig_list_actions_proc;

    // In-Place Action Step Editing State
    int editing_action_idx; // -1 when adding new step, >=0 when editing existing step
    
    // Action Composer Row Controls
    ActionType current_action_type;
    HWND btn_action_type_dropdown;
    HWND lbl_action_key;
    HWND edit_action_key;
    HWND btn_capture_action;
    HWND lbl_duration;
    HWND edit_duration;
    HWND lbl_ms;
    HWND btn_add_action;
    
    // Execution Configuration Section
    HWND chk_repeat;
    HWND lbl_repeat_delay;
    HWND edit_repeat_delay;
    HWND lbl_repeat_ms;
    HWND chk_block_input;
    
    // Dialog Footer Buttons
    HWND btn_save;
    HWND btn_cancel;
    
    HotkeyBinding binding;
    bool is_edit;
    bool saved;
} AddEditDialogState;

static AddEditDialogState g_add_edit_state;

typedef struct {
    ActionType type;
    IconId icon;
    const wchar_t* label;
    const wchar_t* tag;
    COLORREF tag_color;
} DropdownItemDef;

static const DropdownItemDef g_dropdown_items[6] = {
    { ACTION_KEY_PRESS,    ICON_BOLT,       L"Tap Key (Press)",   L"PRESS", COLOR_NEON_CYAN },
    { ACTION_KEY_HOLD,     ICON_LOCK,       L"Hold Key (ms)",     L"HOLD",  COLOR_NEON_PURPLE },
    { ACTION_DELAY,        ICON_CLOCK,      L"Delay (ms)",        L"DELAY", COLOR_NEON_AMBER },
    { ACTION_KEY_DOWN,     ICON_ARROW_DOWN, L"Key Down",          L"DOWN",  COLOR_NEON_GREEN },
    { ACTION_KEY_UP,       ICON_ARROW_UP,   L"Key Up",            L"UP",    COLOR_NEON_INDIGO },
    { ACTION_KEY_SEQUENCE, ICON_SEQUENCE,   L"Key Sequence",      L"SEQ",   COLOR_NEON_CYAN }
};

static HWND g_dropdown_popup_hwnd = NULL;
static int g_dropdown_hover_idx = -1;

static ActionType GetCurrentActionType(void) {
    return g_add_edit_state.current_action_type;
}

static void UpdateActionComposerState(void);

static LRESULT CALLBACK DropdownWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            SetCapture(hwnd);
            return 0;
            
        case WM_MOUSEMOVE: {
            int mx = GET_X_LPARAM(lParam);
            int my = GET_Y_LPARAM(lParam);
            RECT rc;
            GetClientRect(hwnd, &rc);
            int idx = -1;
            if (PtInRect(&rc, (POINT){mx, my})) {
                idx = (my - 4) / 32;
                if (idx < 0 || idx >= 6) idx = -1;
            }
            if (idx != g_dropdown_hover_idx) {
                g_dropdown_hover_idx = idx;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        
        case WM_LBUTTONDOWN: {
            int mx = GET_X_LPARAM(lParam);
            int my = GET_Y_LPARAM(lParam);
            RECT rc;
            GetClientRect(hwnd, &rc);
            if (PtInRect(&rc, (POINT){mx, my})) {
                int idx = (my - 4) / 32;
                if (idx >= 0 && idx < 6) {
                    g_add_edit_state.current_action_type = g_dropdown_items[idx].type;
                    UpdateActionComposerState();
                }
            }
            if (GetCapture() == hwnd) ReleaseCapture();
            DestroyWindow(hwnd);
            g_dropdown_popup_hwnd = NULL;
            if (g_add_edit_state.btn_action_type_dropdown && IsWindow(g_add_edit_state.btn_action_type_dropdown)) {
                InvalidateRect(g_add_edit_state.btn_action_type_dropdown, NULL, TRUE);
            }
            return 0;
        }

        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN: {
            if (GetCapture() == hwnd) ReleaseCapture();
            DestroyWindow(hwnd);
            g_dropdown_popup_hwnd = NULL;
            if (g_add_edit_state.btn_action_type_dropdown && IsWindow(g_add_edit_state.btn_action_type_dropdown)) {
                InvalidateRect(g_add_edit_state.btn_action_type_dropdown, NULL, TRUE);
            }
            return 0;
        }
        
        case WM_KEYDOWN: {
            if (wParam == VK_ESCAPE) {
                if (GetCapture() == hwnd) ReleaseCapture();
                DestroyWindow(hwnd);
                g_dropdown_popup_hwnd = NULL;
                if (g_add_edit_state.btn_action_type_dropdown && IsWindow(g_add_edit_state.btn_action_type_dropdown)) {
                    InvalidateRect(g_add_edit_state.btn_action_type_dropdown, NULL, TRUE);
                }
                return 0;
            } else if (wParam == VK_UP) {
                int cur = 0;
                for (int i = 0; i < 6; i++) {
                    if (g_dropdown_items[i].type == g_add_edit_state.current_action_type) {
                        cur = i;
                        break;
                    }
                }
                if (cur > 0) {
                    g_add_edit_state.current_action_type = g_dropdown_items[cur - 1].type;
                    UpdateActionComposerState();
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                return 0;
            } else if (wParam == VK_DOWN) {
                int cur = 0;
                for (int i = 0; i < 6; i++) {
                    if (g_dropdown_items[i].type == g_add_edit_state.current_action_type) {
                        cur = i;
                        break;
                    }
                }
                if (cur < 5) {
                    g_add_edit_state.current_action_type = g_dropdown_items[cur + 1].type;
                    UpdateActionComposerState();
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                return 0;
            } else if (wParam == VK_RETURN || wParam == VK_SPACE) {
                if (GetCapture() == hwnd) ReleaseCapture();
                DestroyWindow(hwnd);
                g_dropdown_popup_hwnd = NULL;
                if (g_add_edit_state.btn_action_type_dropdown && IsWindow(g_add_edit_state.btn_action_type_dropdown)) {
                    InvalidateRect(g_add_edit_state.btn_action_type_dropdown, NULL, TRUE);
                }
                return 0;
            }
            break;
        }
        
        case WM_KILLFOCUS: {
            if (GetCapture() == hwnd) ReleaseCapture();
            DestroyWindow(hwnd);
            g_dropdown_popup_hwnd = NULL;
            if (g_add_edit_state.btn_action_type_dropdown && IsWindow(g_add_edit_state.btn_action_type_dropdown)) {
                InvalidateRect(g_add_edit_state.btn_action_type_dropdown, NULL, TRUE);
            }
            return 0;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);
            int width = rc.right - rc.left;
            int height = rc.bottom - rc.top;
            
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBmp = CreateCompatibleBitmap(hdc, width, height);
            HGDIOBJ oldBmp = SelectObject(memDC, memBmp);
            
            // Fill background with deep card color
            HBRUSH bg_br = CreateSolidBrush(COLOR_BG_CARD);
            FillRect(memDC, &rc, bg_br);
            DeleteObject(bg_br);
            
            // Draw 1px Neon Cyan Border
            DrawRoundedRect(memDC, &rc, 6, COLOR_BG_CARD, COLOR_NEON_CYAN, 1);
            
            // Top sheen highlight
            HPEN sheen_pen = CreatePen(PS_SOLID, 1, RGB(45, 60, 90));
            HGDIOBJ old_p = SelectObject(memDC, sheen_pen);
            MoveToEx(memDC, 6, 1, NULL);
            LineTo(memDC, width - 6, 1);
            SelectObject(memDC, old_p);
            DeleteObject(sheen_pen);
            
            HBRUSH cyan_br = CreateSolidBrush(COLOR_NEON_CYAN);
            
            for (int i = 0; i < 6; i++) {
                RECT item_rc = { 4, 4 + i * 32, width - 4, 4 + (i + 1) * 32 };
                bool is_sel = (g_add_edit_state.current_action_type == g_dropdown_items[i].type);
                bool is_hov = (g_dropdown_hover_idx == i);
                
                if (is_hov || is_sel) {
                    COLORREF fill_top = is_sel ? RGB(28, 42, 68) : RGB(22, 32, 50);
                    COLORREF fill_bot = is_sel ? RGB(18, 28, 48) : RGB(14, 22, 36);
                    DrawRoundedRect(memDC, &item_rc, 4, fill_bot, is_hov ? COLOR_NEON_CYAN : COLOR_BORDER_STRONG, 1);
                    RECT g_rc = { item_rc.left + 1, item_rc.top + 1, item_rc.right - 1, item_rc.bottom - 1 };
                    DrawGradientVertical(memDC, &g_rc, fill_top, fill_bot);
                    
                    RECT bar_rc = { item_rc.left + 2, item_rc.top + 5, item_rc.left + 5, item_rc.bottom - 5 };
                    FillRect(memDC, &bar_rc, cyan_br);
                } else if (i < 5) {
                    HPEN div_pen = CreatePen(PS_SOLID, 1, RGB(22, 28, 40));
                    HGDIOBJ old_div = SelectObject(memDC, div_pen);
                    MoveToEx(memDC, item_rc.left + 6, item_rc.bottom - 1, NULL);
                    LineTo(memDC, item_rc.right - 6, item_rc.bottom - 1);
                    SelectObject(memDC, old_div);
                    DeleteObject(div_pen);
                }
                
                // Vector Icon
                DrawVectorIcon(memDC, g_dropdown_items[i].icon, item_rc.left + 8, item_rc.top + (32 - 14) / 2, 14, g_dropdown_items[i].tag_color);
                
                // Label
                SetBkMode(memDC, TRANSPARENT);
                SetTextColor(memDC, (is_hov || is_sel) ? COLOR_TEXT_PRIMARY : COLOR_TEXT_SECONDARY);
                SelectObject(memDC, is_sel ? g_theme_fonts.font_body_bold : g_theme_fonts.font_body);
                RECT lbl_rc = { item_rc.left + 28, item_rc.top, item_rc.right - 54, item_rc.bottom };
                DrawTextW(memDC, g_dropdown_items[i].label, -1, &lbl_rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                
                // Tag badge
                RECT badge_rc;
                DrawHudBadge(memDC, item_rc.right - 50, item_rc.top + 6, g_dropdown_items[i].tag, COLOR_BG_VOID, g_dropdown_items[i].tag_color, COLOR_BORDER_SUBTLE, &badge_rc);
            }
            
            DeleteObject(cyan_br);
            
            BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, oldBmp);
            DeleteObject(memBmp);
            DeleteDC(memDC);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_DESTROY:
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static void EnsureDropdownClassRegistered(HINSTANCE hInst) {
    static bool registered = false;
    if (registered) return;
    
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = DropdownWndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_HAND);
    wc.hbrBackground = NULL;
    wc.lpszClassName = L"TobelsoftCyberDropdownClass";
    RegisterClassExW(&wc);
    registered = true;
}

static void RefreshTriggerList(HWND hwnd_list) {
    if (!hwnd_list) return;
    SendMessageW(hwnd_list, LB_RESETCONTENT, 0, 0);
    int count = g_add_edit_state.binding.trigger_count;
    if (count < 0) count = 0;
    if (count > MAX_TRIGGERS_PER_BINDING) count = MAX_TRIGGERS_PER_BINDING;
    g_add_edit_state.binding.trigger_count = count;
    
    if (count == 0) {
        SendMessageW(hwnd_list, LB_ADDSTRING, 0, (LPARAM)L"  (No triggers assigned -- Click 'Add Trigger' below)");
        return;
    }
    
    for (int i = 0; i < count; i++) {
        wchar_t wtrig[MAX_KEY_NAME_LEN] = {0};
        MultiByteToWideChar(CP_UTF8, 0, g_add_edit_state.binding.trigger_keys[i], -1, wtrig, MAX_KEY_NAME_LEN);
        
        wchar_t line[128];
        swprintf_s(line, 128, L"  Trigger #%d:   %ls", i + 1, wtrig);
        SendMessageW(hwnd_list, LB_ADDSTRING, 0, (LPARAM)line);
    }
}

static void RefreshActionList(HWND hwnd_list) {
    if (!hwnd_list) return;
    SendMessageW(hwnd_list, LB_RESETCONTENT, 0, 0);
    int count = g_add_edit_state.binding.action_count;
    if (count < 0) count = 0;
    if (count > MAX_ACTIONS_PER_BINDING) count = MAX_ACTIONS_PER_BINDING;
    g_add_edit_state.binding.action_count = count;
    
    if (count == 0) {
        SendMessageW(hwnd_list, LB_ADDSTRING, 0, (LPARAM)L"  (Pipeline empty -- Configure step below and click 'Add Step')");
        return;
    }
    
    for (int i = 0; i < count; i++) {
        const KeyAction* a = &g_add_edit_state.binding.actions[i];
        wchar_t line[128];
        
        if (a->action_type == ACTION_DELAY) {
            swprintf_s(line, 128, L"  %02d.  [ DELAY ]            %d ms", i + 1, a->duration);
        } else if (a->action_type == ACTION_KEY_HOLD) {
            wchar_t wkey[MAX_KEY_NAME_LEN] = L"-";
            if (a->key_count > 0 && a->keys[0][0]) {
                MultiByteToWideChar(CP_UTF8, 0, a->keys[0], -1, wkey, MAX_KEY_NAME_LEN);
            }
            swprintf_s(line, 128, L"  %02d.  [ HOLD KEY ]         %ls   (%d ms)", i + 1, wkey, a->duration);
        } else if (a->action_type == ACTION_KEY_PRESS) {
            wchar_t wkey[MAX_KEY_NAME_LEN] = L"-";
            if (a->key_count > 0 && a->keys[0][0]) {
                MultiByteToWideChar(CP_UTF8, 0, a->keys[0], -1, wkey, MAX_KEY_NAME_LEN);
            }
            swprintf_s(line, 128, L"  %02d.  [ TAP KEY ]          %ls", i + 1, wkey);
        } else if (a->action_type == ACTION_KEY_DOWN) {
            wchar_t wkey[MAX_KEY_NAME_LEN] = L"-";
            if (a->key_count > 0 && a->keys[0][0]) {
                MultiByteToWideChar(CP_UTF8, 0, a->keys[0], -1, wkey, MAX_KEY_NAME_LEN);
            }
            swprintf_s(line, 128, L"  %02d.  [ KEY DOWN ]        %ls", i + 1, wkey);
        } else if (a->action_type == ACTION_KEY_UP) {
            wchar_t wkey[MAX_KEY_NAME_LEN] = L"-";
            if (a->key_count > 0 && a->keys[0][0]) {
                MultiByteToWideChar(CP_UTF8, 0, a->keys[0], -1, wkey, MAX_KEY_NAME_LEN);
            }
            swprintf_s(line, 128, L"  %02d.  [ KEY UP ]          %ls", i + 1, wkey);
        } else {
            wchar_t keys_str[64] = {0};
            for (int k = 0; k < a->key_count && k < MAX_KEYS_PER_ACTION; k++) {
                if (k > 0) wcscat_s(keys_str, 64, L", ");
                wchar_t wk[MAX_KEY_NAME_LEN];
                MultiByteToWideChar(CP_UTF8, 0, a->keys[k], -1, wk, MAX_KEY_NAME_LEN);
                wcscat_s(keys_str, 64, wk);
            }
            swprintf_s(line, 128, L"  %02d.  [ KEY SEQUENCE ]    %ls", i + 1, keys_str);
        }
        SendMessageW(hwnd_list, LB_ADDSTRING, 0, (LPARAM)line);
    }
}

// Stable 4-column Action Composer state updater (zero layout shifts)
static void UpdateActionComposerState(void) {
    if (!g_add_edit_state.btn_action_type_dropdown) return;
    ActionType act_type = GetCurrentActionType();
    
    if (act_type == ACTION_DELAY) {
        // Delay only: Disable key input & capture, enable duration input
        SetWindowTextW(g_add_edit_state.lbl_action_key, L"Target Key (N/A):");
        SetWindowTextW(g_add_edit_state.edit_action_key, L"[ Delay Only ]");
        EnableWindow(g_add_edit_state.edit_action_key, FALSE);
        EnableWindow(g_add_edit_state.btn_capture_action, FALSE);
        
        SetWindowTextW(g_add_edit_state.lbl_duration, L"Delay Time (ms):");
        EnableWindow(g_add_edit_state.edit_duration, TRUE);
        
        // Ensure duration has a valid default
        wchar_t dur_buf[32] = {0};
        GetWindowTextW(g_add_edit_state.edit_duration, dur_buf, 32);
        if (!dur_buf[0] || _wtoi(dur_buf) <= 0) {
            SetWindowTextW(g_add_edit_state.edit_duration, L"50");
        }
    } else if (act_type == ACTION_KEY_HOLD) {
        // Key Hold: Both Key and Duration active
        SetWindowTextW(g_add_edit_state.lbl_action_key, L"Target Key / Input:");
        EnableWindow(g_add_edit_state.edit_action_key, TRUE);
        EnableWindow(g_add_edit_state.btn_capture_action, TRUE);
        
        wchar_t key_buf[MAX_KEY_NAME_LEN] = {0};
        GetWindowTextW(g_add_edit_state.edit_action_key, key_buf, MAX_KEY_NAME_LEN);
        if (wcsstr(key_buf, L"[ Delay Only ]") != NULL) {
            SetWindowTextW(g_add_edit_state.edit_action_key, L"");
        }
        
        SetWindowTextW(g_add_edit_state.lbl_duration, L"Hold Time (ms):");
        EnableWindow(g_add_edit_state.edit_duration, TRUE);
        
        wchar_t dur_buf[32] = {0};
        GetWindowTextW(g_add_edit_state.edit_duration, dur_buf, 32);
        if (!dur_buf[0] || _wtoi(dur_buf) <= 0) {
            SetWindowTextW(g_add_edit_state.edit_duration, L"100");
        }
    } else {
        // Key Press / Down / Up / Sequence: Key active, Duration N/A
        if (act_type == ACTION_KEY_SEQUENCE) {
            SetWindowTextW(g_add_edit_state.lbl_action_key, L"Keys (e.g. A, B):");
        } else {
            SetWindowTextW(g_add_edit_state.lbl_action_key, L"Target Key / Input:");
        }
        EnableWindow(g_add_edit_state.edit_action_key, TRUE);
        EnableWindow(g_add_edit_state.btn_capture_action, TRUE);
        
        wchar_t key_buf[MAX_KEY_NAME_LEN] = {0};
        GetWindowTextW(g_add_edit_state.edit_action_key, key_buf, MAX_KEY_NAME_LEN);
        if (wcsstr(key_buf, L"[ Delay Only ]") != NULL) {
            SetWindowTextW(g_add_edit_state.edit_action_key, L"");
        }
        
        SetWindowTextW(g_add_edit_state.lbl_duration, L"Duration (N/A):");
        EnableWindow(g_add_edit_state.edit_duration, FALSE);
    }
    
    if (g_add_edit_state.btn_action_type_dropdown && IsWindow(g_add_edit_state.btn_action_type_dropdown)) {
        InvalidateRect(g_add_edit_state.btn_action_type_dropdown, NULL, TRUE);
    }
    InvalidateRect(g_add_edit_state.hwnd, NULL, FALSE);
}

static void ActionList_TriggerEdit(int item_idx) {
    if (item_idx < 0 || item_idx >= g_add_edit_state.binding.action_count) return;
    
    const KeyAction* a = &g_add_edit_state.binding.actions[item_idx];
    g_add_edit_state.editing_action_idx = item_idx;
    g_add_edit_state.current_action_type = a->action_type;
    
    if (a->action_type == ACTION_DELAY) {
        wchar_t wdur[32] = {0};
        swprintf_s(wdur, 32, L"%d", a->duration > 0 ? a->duration : 50);
        SetWindowTextW(g_add_edit_state.edit_duration, wdur);
    } else if (a->action_type == ACTION_KEY_HOLD) {
        wchar_t wkey[MAX_KEY_NAME_LEN] = {0};
        if (a->key_count > 0 && a->keys[0][0]) {
            MultiByteToWideChar(CP_UTF8, 0, a->keys[0], -1, wkey, MAX_KEY_NAME_LEN);
        }
        SetWindowTextW(g_add_edit_state.edit_action_key, wkey);
        
        wchar_t wdur[32] = {0};
        swprintf_s(wdur, 32, L"%d", a->duration > 0 ? a->duration : 100);
        SetWindowTextW(g_add_edit_state.edit_duration, wdur);
    } else if (a->action_type == ACTION_KEY_SEQUENCE) {
        wchar_t wkeys[MAX_KEY_NAME_LEN * 4] = {0};
        for (int k = 0; k < a->key_count && k < MAX_KEYS_PER_ACTION; k++) {
            if (k > 0) wcscat_s(wkeys, sizeof(wkeys)/sizeof(wchar_t), L", ");
            wchar_t wk[MAX_KEY_NAME_LEN] = {0};
            MultiByteToWideChar(CP_UTF8, 0, a->keys[k], -1, wk, MAX_KEY_NAME_LEN);
            wcscat_s(wkeys, sizeof(wkeys)/sizeof(wchar_t), wk);
        }
        SetWindowTextW(g_add_edit_state.edit_action_key, wkeys);
    } else {
        wchar_t wkey[MAX_KEY_NAME_LEN] = {0};
        if (a->key_count > 0 && a->keys[0][0]) {
            MultiByteToWideChar(CP_UTF8, 0, a->keys[0], -1, wkey, MAX_KEY_NAME_LEN);
        }
        SetWindowTextW(g_add_edit_state.edit_action_key, wkey);
    }
    
    UpdateActionComposerState();
    
    if (g_add_edit_state.btn_action_type_dropdown && IsWindow(g_add_edit_state.btn_action_type_dropdown)) {
        InvalidateRect(g_add_edit_state.btn_action_type_dropdown, NULL, TRUE);
    }
    if (g_add_edit_state.btn_add_action && IsWindow(g_add_edit_state.btn_add_action)) {
        InvalidateRect(g_add_edit_state.btn_add_action, NULL, TRUE);
    }
    if (g_add_edit_state.list_actions && IsWindow(g_add_edit_state.list_actions)) {
        SendMessageW(g_add_edit_state.list_actions, LB_SETCURSEL, item_idx, 0);
        InvalidateRect(g_add_edit_state.list_actions, NULL, FALSE);
    }
    if (g_add_edit_state.hwnd && IsWindow(g_add_edit_state.hwnd)) {
        InvalidateRect(g_add_edit_state.hwnd, NULL, FALSE);
    }
    
    // Focus the relevant input field
    if (a->action_type == ACTION_DELAY) {
        SetFocus(g_add_edit_state.edit_duration);
        SendMessageW(g_add_edit_state.edit_duration, EM_SETSEL, 0, -1);
    } else {
        SetFocus(g_add_edit_state.edit_action_key);
        SendMessageW(g_add_edit_state.edit_action_key, EM_SETSEL, 0, -1);
    }
}

static LRESULT CALLBACK ActionListSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_LBUTTONDOWN: {
            int mx = GET_X_LPARAM(lParam);
            int my = GET_Y_LPARAM(lParam);
            DWORD item_res = (DWORD)SendMessageW(hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(mx, my));
            if (HIWORD(item_res) == 0) {
                int item_idx = LOWORD(item_res);
                if (item_idx >= 0 && item_idx < g_add_edit_state.binding.action_count) {
                    g_add_edit_state.drag_from_idx = item_idx;
                    g_add_edit_state.drag_target_idx = item_idx;
                    g_add_edit_state.drag_start_pt = (POINT){ mx, my };
                    g_add_edit_state.is_dragging = false;
                    SetCapture(hwnd);
                    SendMessageW(hwnd, LB_SETCURSEL, item_idx, 0);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            break;
        }
        
        case WM_MOUSEMOVE: {
            if (GetCapture() == hwnd && g_add_edit_state.drag_from_idx >= 0) {
                int mx = GET_X_LPARAM(lParam);
                int my = GET_Y_LPARAM(lParam);
                int dx = mx - g_add_edit_state.drag_start_pt.x;
                int dy = my - g_add_edit_state.drag_start_pt.y;
                if (!g_add_edit_state.is_dragging) {
                    if (abs(dx) > 3 || abs(dy) > 3) {
                        g_add_edit_state.is_dragging = true;
                    }
                }
                
                if (g_add_edit_state.is_dragging) {
                    DWORD item_res = (DWORD)SendMessageW(hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(mx, my));
                    int tgt = g_add_edit_state.drag_from_idx;
                    if (HIWORD(item_res) == 0) {
                        tgt = LOWORD(item_res);
                    } else {
                        tgt = (my < 0) ? 0 : (g_add_edit_state.binding.action_count - 1);
                    }
                    if (tgt < 0) tgt = 0;
                    if (tgt >= g_add_edit_state.binding.action_count) tgt = g_add_edit_state.binding.action_count - 1;
                    
                    if (tgt != g_add_edit_state.drag_target_idx) {
                        g_add_edit_state.drag_target_idx = tgt;
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                    SetCursor(LoadCursor(NULL, IDC_SIZENS));
                    return 0;
                }
            }
            break;
        }
        
        case WM_LBUTTONUP: {
            if (GetCapture() == hwnd) {
                ReleaseCapture();
                if (g_add_edit_state.is_dragging && g_add_edit_state.drag_from_idx >= 0) {
                    int from = g_add_edit_state.drag_from_idx;
                    int to = g_add_edit_state.drag_target_idx;
                    if (to >= 0 && to < g_add_edit_state.binding.action_count && from != to) {
                        KeyAction moving = g_add_edit_state.binding.actions[from];
                        if (from < to) {
                            for (int i = from; i < to; i++) {
                                g_add_edit_state.binding.actions[i] = g_add_edit_state.binding.actions[i + 1];
                            }
                        } else {
                            for (int i = from; i > to; i--) {
                                g_add_edit_state.binding.actions[i] = g_add_edit_state.binding.actions[i - 1];
                            }
                        }
                        g_add_edit_state.binding.actions[to] = moving;
                        
                        if (g_add_edit_state.editing_action_idx == from) {
                            g_add_edit_state.editing_action_idx = to;
                        } else if (from < to && g_add_edit_state.editing_action_idx > from && g_add_edit_state.editing_action_idx <= to) {
                            g_add_edit_state.editing_action_idx--;
                        } else if (from > to && g_add_edit_state.editing_action_idx >= to && g_add_edit_state.editing_action_idx < from) {
                            g_add_edit_state.editing_action_idx++;
                        }
                        
                        RefreshActionList(hwnd);
                        SendMessageW(hwnd, LB_SETCURSEL, to, 0);
                        if (g_add_edit_state.btn_add_action && IsWindow(g_add_edit_state.btn_add_action)) {
                            InvalidateRect(g_add_edit_state.btn_add_action, NULL, TRUE);
                        }
                    }
                }
                g_add_edit_state.is_dragging = false;
                g_add_edit_state.drag_from_idx = -1;
                g_add_edit_state.drag_target_idx = -1;
                InvalidateRect(hwnd, NULL, FALSE);
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                return 0;
            }
            break;
        }
        
        case WM_LBUTTONDBLCLK: {
            int mx = GET_X_LPARAM(lParam);
            int my = GET_Y_LPARAM(lParam);
            DWORD item_res = (DWORD)SendMessageW(hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(mx, my));
            if (HIWORD(item_res) == 0) {
                int item_idx = LOWORD(item_res);
                if (item_idx >= 0 && item_idx < g_add_edit_state.binding.action_count) {
                    ActionList_TriggerEdit(item_idx);
                    return 0;
                }
            }
            break;
        }
        
        case WM_KEYDOWN: {
            if (wParam == VK_ESCAPE) {
                if (g_add_edit_state.editing_action_idx >= 0) {
                    g_add_edit_state.editing_action_idx = -1;
                    UpdateActionComposerState();
                    if (g_add_edit_state.btn_add_action && IsWindow(g_add_edit_state.btn_add_action)) {
                        InvalidateRect(g_add_edit_state.btn_add_action, NULL, TRUE);
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                    if (g_add_edit_state.hwnd && IsWindow(g_add_edit_state.hwnd)) {
                        InvalidateRect(g_add_edit_state.hwnd, NULL, FALSE);
                    }
                    return 0;
                }
            }
            break;
        }
    }
    
    if (g_add_edit_state.orig_list_actions_proc) {
        return CallWindowProcW(g_add_edit_state.orig_list_actions_proc, hwnd, msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK AddEditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            return 0;
            
        case WM_MEASUREITEM: {
            MEASUREITEMSTRUCT* mis = (MEASUREITEMSTRUCT*)lParam;
            if (mis->CtlType == ODT_LISTBOX) {
                if (mis->CtlID == 103) {
                    mis->itemHeight = 24;
                } else {
                    mis->itemHeight = 28;
                }
                return TRUE;
            }
            break;
        }

        case WM_LBUTTONDOWN: {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            if (y < 38 && x < 600) {
                ReleaseCapture();
                SendMessageW(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
                return 0;
            }
            break;
        }
            
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            HWND hwndStatic = (HWND)lParam;
            
            // Check if this is a disabled edit control
            if (hwndStatic == g_add_edit_state.edit_action_key || hwndStatic == g_add_edit_state.edit_duration || hwndStatic == g_add_edit_state.edit_repeat_delay) {
                if (!IsWindowEnabled(hwndStatic)) {
                    SetTextColor(hdcStatic, COLOR_TEXT_DISABLED);
                    SetBkColor(hdcStatic, RGB(14, 18, 26));
                    static HBRUSH s_disabled_br = NULL;
                    if (!s_disabled_br) s_disabled_br = CreateSolidBrush(RGB(14, 18, 26));
                    return (LRESULT)s_disabled_br;
                }
            }
            
            SetTextColor(hdcStatic, COLOR_TEXT_SECONDARY);
            SetBkMode(hdcStatic, TRANSPARENT);
            static HBRUSH s_card_bg = NULL;
            if (!s_card_bg) s_card_bg = CreateSolidBrush(COLOR_BG_CARD);
            return (LRESULT)s_card_bg;
        }
        
        case WM_CTLCOLORBTN: {
            static HBRUSH s_btn_bg = NULL;
            if (!s_btn_bg) s_btn_bg = CreateSolidBrush(COLOR_BG_CARD);
            return (LRESULT)s_btn_bg;
        }
        
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX: {
            HDC hdcEdit = (HDC)wParam;
            SetTextColor(hdcEdit, COLOR_TEXT_PRIMARY);
            SetBkColor(hdcEdit, COLOR_BG_INPUT);
            static HBRUSH s_input_bg = NULL;
            if (!s_input_bg) s_input_bg = CreateSolidBrush(COLOR_BG_INPUT);
            return (LRESULT)s_input_bg;
        }
        
        case WM_DRAWITEM: {
            DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;
            
            // ================================================================
            // CUSTOM CYBER DROPDOWN BUTTON (ID 201)
            // ================================================================
            if (dis->CtlID == 201) {
                bool is_open = (g_dropdown_popup_hwnd != NULL);
                bool is_selected = (dis->itemState & ODS_SELECTED);
                bool is_focused = (dis->itemState & ODS_FOCUS);
                
                COLORREF bg_col = (is_open || is_selected) ? RGB(26, 38, 58) : COLOR_BG_INPUT;
                COLORREF border_col = (is_open || is_focused) ? COLOR_NEON_CYAN : COLOR_BORDER_STRONG;
                
                DrawRoundedRect(dis->hDC, &dis->rcItem, 4, bg_col, border_col, (is_open || is_focused) ? 2 : 1);
                
                RECT grad_rc = { dis->rcItem.left + 1, dis->rcItem.top + 1, dis->rcItem.right - 1, dis->rcItem.bottom - 1 };
                DrawGradientVertical(dis->hDC, &grad_rc, (is_open || is_selected) ? RGB(32, 46, 72) : RGB(18, 24, 36), bg_col);
                
                const DropdownItemDef* cur_item = &g_dropdown_items[0];
                for (int i = 0; i < 6; i++) {
                    if (g_dropdown_items[i].type == g_add_edit_state.current_action_type) {
                        cur_item = &g_dropdown_items[i];
                        break;
                    }
                }
                
                // Draw Vector Icon
                DrawVectorIcon(dis->hDC, cur_item->icon, dis->rcItem.left + 8, dis->rcItem.top + (dis->rcItem.bottom - dis->rcItem.top - 14) / 2, 14, cur_item->tag_color);
                
                SetBkMode(dis->hDC, TRANSPARENT);
                SetTextColor(dis->hDC, COLOR_TEXT_PRIMARY);
                SelectObject(dis->hDC, g_theme_fonts.font_small_bold);
                RECT text_rc = { dis->rcItem.left + 28, dis->rcItem.top, dis->rcItem.right - 22, dis->rcItem.bottom };
                DrawTextW(dis->hDC, cur_item->label, -1, &text_rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                
                int cx = dis->rcItem.right - 12;
                int cy = dis->rcItem.top + (dis->rcItem.bottom - dis->rcItem.top) / 2;
                HPEN arr_pen = CreatePen(PS_SOLID, 2, (is_open || is_focused) ? COLOR_NEON_CYAN : COLOR_TEXT_MUTED);
                HGDIOBJ old_p = SelectObject(dis->hDC, arr_pen);
                if (is_open) {
                    MoveToEx(dis->hDC, cx - 4, cy + 2, NULL);
                    LineTo(dis->hDC, cx, cy - 2);
                    LineTo(dis->hDC, cx + 4, cy + 2);
                } else {
                    MoveToEx(dis->hDC, cx - 4, cy - 2, NULL);
                    LineTo(dis->hDC, cx, cy + 2);
                    LineTo(dis->hDC, cx + 4, cy - 2);
                }
                SelectObject(dis->hDC, old_p);
                DeleteObject(arr_pen);
                return TRUE;
            }

            // ================================================================
            // CUSTOM CYBER LISTBOX ROWS (ODT_LISTBOX)
            // ================================================================
            if (dis->CtlType == ODT_LISTBOX) {
                bool is_selected = (dis->itemState & ODS_SELECTED);
                bool is_action_list = (dis->hwndItem == g_add_edit_state.list_actions);
                bool is_editing = is_action_list && (dis->itemID != (UINT)-1) && ((int)dis->itemID == g_add_edit_state.editing_action_idx);
                bool is_drag_src = is_action_list && g_add_edit_state.is_dragging && ((int)dis->itemID == g_add_edit_state.drag_from_idx);
                bool is_drag_tgt = is_action_list && g_add_edit_state.is_dragging && ((int)dis->itemID == g_add_edit_state.drag_target_idx);
                
                COLORREF bg_col;
                if (is_editing) {
                    bg_col = RGB(10, 36, 32);
                } else if (is_drag_src) {
                    bg_col = RGB(14, 20, 30);
                } else if (is_selected) {
                    bg_col = RGB(20, 32, 52);
                } else {
                    bg_col = COLOR_BG_INPUT;
                }
                
                HBRUSH bg_br = CreateSolidBrush(bg_col);
                FillRect(dis->hDC, &dis->rcItem, bg_br);
                DeleteObject(bg_br);
                
                if (is_editing) {
                    HPEN edit_pen = CreatePen(PS_SOLID, 1, COLOR_NEON_GREEN);
                    HGDIOBJ old_p = SelectObject(dis->hDC, edit_pen);
                    HGDIOBJ old_b = SelectObject(dis->hDC, GetStockObject(NULL_BRUSH));
                    Rectangle(dis->hDC, dis->rcItem.left, dis->rcItem.top, dis->rcItem.right, dis->rcItem.bottom);
                    SelectObject(dis->hDC, old_b);
                    SelectObject(dis->hDC, old_p);
                    DeleteObject(edit_pen);
                    
                    RECT ind_rc = { dis->rcItem.left, dis->rcItem.top, dis->rcItem.left + 4, dis->rcItem.bottom };
                    HBRUSH ind_br = CreateSolidBrush(COLOR_NEON_GREEN);
                    FillRect(dis->hDC, &ind_rc, ind_br);
                    DeleteObject(ind_br);
                } else if (is_selected) {
                    HPEN sel_pen = CreatePen(PS_SOLID, 1, COLOR_NEON_CYAN);
                    HGDIOBJ old_p = SelectObject(dis->hDC, sel_pen);
                    HGDIOBJ old_b = SelectObject(dis->hDC, GetStockObject(NULL_BRUSH));
                    Rectangle(dis->hDC, dis->rcItem.left, dis->rcItem.top, dis->rcItem.right, dis->rcItem.bottom);
                    SelectObject(dis->hDC, old_b);
                    SelectObject(dis->hDC, old_p);
                    DeleteObject(sel_pen);
                    
                    RECT ind_rc = { dis->rcItem.left, dis->rcItem.top, dis->rcItem.left + 3, dis->rcItem.bottom };
                    HBRUSH ind_br = CreateSolidBrush(COLOR_NEON_CYAN);
                    FillRect(dis->hDC, &ind_rc, ind_br);
                    DeleteObject(ind_br);
                } else {
                    HPEN div_pen = CreatePen(PS_SOLID, 1, RGB(22, 28, 40));
                    HGDIOBJ old_p = SelectObject(dis->hDC, div_pen);
                    MoveToEx(dis->hDC, dis->rcItem.left + 4, dis->rcItem.bottom - 1, NULL);
                    LineTo(dis->hDC, dis->rcItem.right - 4, dis->rcItem.bottom - 1);
                    SelectObject(dis->hDC, old_p);
                    DeleteObject(div_pen);
                }
                
                // Visual insertion indicator line when dragging
                if (is_drag_tgt && !is_drag_src) {
                    int line_y = (g_add_edit_state.drag_from_idx < g_add_edit_state.drag_target_idx) ? (dis->rcItem.bottom - 2) : dis->rcItem.top;
                    HPEN insert_pen = CreatePen(PS_SOLID, 3, COLOR_NEON_CYAN);
                    HGDIOBJ old_p = SelectObject(dis->hDC, insert_pen);
                    MoveToEx(dis->hDC, dis->rcItem.left + 2, line_y, NULL);
                    LineTo(dis->hDC, dis->rcItem.right - 2, line_y);
                    SelectObject(dis->hDC, old_p);
                    DeleteObject(insert_pen);
                }
                
                if (dis->itemID != (UINT)-1) {
                    wchar_t text[256] = {0};
                    SendMessageW(dis->hwndItem, LB_GETTEXT, dis->itemID, (LPARAM)text);
                    
                    SetBkMode(dis->hDC, TRANSPARENT);
                    if (dis->hwndItem == g_add_edit_state.list_actions) {
                        if (wcsstr(text, L"Pipeline empty") != NULL) {
                            SetTextColor(dis->hDC, COLOR_TEXT_MUTED);
                            SelectObject(dis->hDC, g_theme_fonts.font_body);
                            RECT trc = dis->rcItem;
                            trc.left += 10;
                            DrawTextW(dis->hDC, text, -1, &trc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                        } else if (dis->itemID < (UINT)g_add_edit_state.binding.action_count) {
                            const KeyAction* a = &g_add_edit_state.binding.actions[dis->itemID];
                            
                            // Drag Grip Handle Icon
                            DrawVectorIcon(dis->hDC, ICON_GRIP, dis->rcItem.left + 5, dis->rcItem.top + (28 - 12) / 2, 12, is_selected ? COLOR_NEON_CYAN : RGB(60, 75, 100));
                            
                            // Step Index: "01.", "02."
                            wchar_t idx_buf[16];
                            swprintf_s(idx_buf, 16, L"%02d.", dis->itemID + 1);
                            SetTextColor(dis->hDC, is_editing ? COLOR_NEON_GREEN : (is_selected ? COLOR_NEON_CYAN : COLOR_TEXT_MUTED));
                            SelectObject(dis->hDC, g_theme_fonts.font_mono_small);
                            RECT idx_rc = { dis->rcItem.left + 16, dis->rcItem.top, dis->rcItem.left + 42, dis->rcItem.bottom };
                            DrawTextW(dis->hDC, idx_buf, -1, &idx_rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                            
                            // Action Type Badge with Vector Icon
                            const wchar_t* tag = L"TAP KEY";
                            COLORREF tag_bg = COLOR_NEON_CYAN_DIM;
                            COLORREF tag_fg = COLOR_NEON_CYAN;
                            COLORREF tag_bd = RGB(0, 180, 200);
                            
                            if (a->action_type == ACTION_DELAY) {
                                tag = L"DELAY";
                                tag_bg = COLOR_NEON_AMBER_DIM;
                                tag_fg = COLOR_NEON_AMBER;
                                tag_bd = RGB(200, 120, 0);
                            } else if (a->action_type == ACTION_KEY_HOLD) {
                                tag = L"HOLD KEY";
                                tag_bg = COLOR_NEON_PURPLE_DIM;
                                tag_fg = COLOR_NEON_PURPLE;
                                tag_bd = RGB(168, 85, 247);
                            } else if (a->action_type == ACTION_KEY_DOWN) {
                                tag = L"KEY DOWN";
                                tag_bg = COLOR_NEON_GREEN_DIM;
                                tag_fg = COLOR_NEON_GREEN;
                                tag_bd = RGB(0, 180, 80);
                            } else if (a->action_type == ACTION_KEY_UP) {
                                tag = L"KEY UP";
                                tag_bg = COLOR_NEON_INDIGO_DIM;
                                tag_fg = COLOR_NEON_INDIGO;
                                tag_bd = RGB(99, 102, 241);
                            } else if (a->action_type == ACTION_KEY_SEQUENCE) {
                                tag = L"KEY SEQ";
                                tag_bg = COLOR_NEON_CYAN_DIM;
                                tag_fg = COLOR_NEON_CYAN;
                                tag_bd = RGB(0, 180, 200);
                            }
                            
                            RECT badge_rc;
                            DrawHudBadgeWithIcon(dis->hDC, dis->rcItem.left + 44, dis->rcItem.top + 4, GetActionTypeIcon(a->action_type), tag, tag_bg, tag_fg, tag_bd, &badge_rc);
                            
                            int max_right = is_editing ? (dis->rcItem.right - 92) : (dis->rcItem.right - 8);
                            int val_x = badge_rc.right + 12;
                            if (a->action_type == ACTION_DELAY) {
                                wchar_t val_buf[32];
                                swprintf_s(val_buf, 32, L"%d ms", a->duration);
                                SetTextColor(dis->hDC, COLOR_NEON_AMBER);
                                SelectObject(dis->hDC, g_theme_fonts.font_mono_data);
                                RECT val_rc = { val_x, dis->rcItem.top, max_right, dis->rcItem.bottom };
                                DrawTextW(dis->hDC, val_buf, -1, &val_rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                            } else if (a->action_type == ACTION_KEY_HOLD) {
                                wchar_t wkey[MAX_KEY_NAME_LEN] = L"-";
                                if (a->key_count > 0 && a->keys[0][0]) {
                                    MultiByteToWideChar(CP_UTF8, 0, a->keys[0], -1, wkey, MAX_KEY_NAME_LEN);
                                }
                                wchar_t val_buf[64];
                                swprintf_s(val_buf, 64, L"%ls  (%d ms)", wkey, a->duration);
                                SetTextColor(dis->hDC, is_selected ? COLOR_TEXT_PRIMARY : COLOR_TEXT_SECONDARY);
                                SelectObject(dis->hDC, g_theme_fonts.font_mono_data);
                                RECT val_rc = { val_x, dis->rcItem.top, max_right, dis->rcItem.bottom };
                                DrawTextW(dis->hDC, val_buf, -1, &val_rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                            } else if (a->action_type == ACTION_KEY_SEQUENCE) {
                                wchar_t keys_str[64] = {0};
                                for (int k = 0; k < a->key_count && k < MAX_KEYS_PER_ACTION; k++) {
                                    if (k > 0) wcscat_s(keys_str, 64, L", ");
                                    wchar_t wk[MAX_KEY_NAME_LEN];
                                    MultiByteToWideChar(CP_UTF8, 0, a->keys[k], -1, wk, MAX_KEY_NAME_LEN);
                                    wcscat_s(keys_str, 64, wk);
                                }
                                SetTextColor(dis->hDC, is_selected ? COLOR_TEXT_PRIMARY : COLOR_TEXT_SECONDARY);
                                SelectObject(dis->hDC, g_theme_fonts.font_mono_data);
                                RECT val_rc = { val_x, dis->rcItem.top, max_right, dis->rcItem.bottom };
                                DrawTextW(dis->hDC, keys_str, -1, &val_rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                            } else {
                                wchar_t wkey[MAX_KEY_NAME_LEN] = L"-";
                                if (a->key_count > 0 && a->keys[0][0]) {
                                    MultiByteToWideChar(CP_UTF8, 0, a->keys[0], -1, wkey, MAX_KEY_NAME_LEN);
                                }
                                SetTextColor(dis->hDC, is_selected ? COLOR_TEXT_PRIMARY : COLOR_TEXT_SECONDARY);
                                SelectObject(dis->hDC, g_theme_fonts.font_mono_data);
                                RECT val_rc = { val_x, dis->rcItem.top, max_right, dis->rcItem.bottom };
                                DrawTextW(dis->hDC, wkey, -1, &val_rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                            }
                            
                            // If this item is currently in active edit mode, draw the EDITING neon badge with Vector Edit Icon
                            if (is_editing) {
                                RECT edit_badge_rc;
                                DrawHudBadgeWithIcon(dis->hDC, dis->rcItem.right - 92, dis->rcItem.top + 4, ICON_EDIT, L"EDITING", COLOR_NEON_GREEN_DIM, COLOR_NEON_GREEN, RGB(16, 185, 129), &edit_badge_rc);
                            }
                        }
                    } else if (dis->hwndItem == g_add_edit_state.list_triggers) {
                        if (wcsstr(text, L"No triggers") != NULL) {
                            SetTextColor(dis->hDC, COLOR_TEXT_MUTED);
                            SelectObject(dis->hDC, g_theme_fonts.font_body);
                            RECT trc = dis->rcItem;
                            trc.left += 10;
                            DrawTextW(dis->hDC, text, -1, &trc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                        } else if (dis->itemID < (UINT)g_add_edit_state.binding.trigger_count) {
                            DrawVectorIcon(dis->hDC, ICON_TARGET, dis->rcItem.left + 8, dis->rcItem.top + (24 - 12) / 2, 12, is_selected ? COLOR_NEON_CYAN : COLOR_TEXT_MUTED);
                            
                            wchar_t trig_tag[32];
                            swprintf_s(trig_tag, 32, L"Trigger #%d:", dis->itemID + 1);
                            
                            SetTextColor(dis->hDC, is_selected ? COLOR_NEON_CYAN : COLOR_TEXT_MUTED);
                            SelectObject(dis->hDC, g_theme_fonts.font_mono_small);
                            RECT tag_rc = { dis->rcItem.left + 24, dis->rcItem.top, dis->rcItem.left + 95, dis->rcItem.bottom };
                            DrawTextW(dis->hDC, trig_tag, -1, &tag_rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                            
                            wchar_t wtrig[MAX_KEY_NAME_LEN] = {0};
                            MultiByteToWideChar(CP_UTF8, 0, g_add_edit_state.binding.trigger_keys[dis->itemID], -1, wtrig, MAX_KEY_NAME_LEN);
                            
                            SetTextColor(dis->hDC, is_selected ? COLOR_TEXT_PRIMARY : COLOR_NEON_CYAN);
                            SelectObject(dis->hDC, g_theme_fonts.font_mono_data);
                            RECT val_rc = { dis->rcItem.left + 100, dis->rcItem.top, dis->rcItem.right - 8, dis->rcItem.bottom };
                            DrawTextW(dis->hDC, wtrig, -1, &val_rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                        }
                    }
                }
                return TRUE;
            }

            if (dis->CtlType == ODT_BUTTON) {
                // ============================================================
                // TITLEBAR CLOSE BUTTON (ID 303)
                // ============================================================
                if (dis->CtlID == 303) {
                    bool is_selected = (dis->itemState & ODS_SELECTED);
                    COLORREF bg_col = is_selected ? RGB(185, 28, 28) : RGB(21, 27, 39);
                    COLORREF border_col = is_selected ? RGB(239, 68, 68) : COLOR_BORDER_SUBTLE;
                    DrawRoundedRect(dis->hDC, &dis->rcItem, 4, bg_col, border_col, 1);
                    
                    COLORREF icon_col = is_selected ? RGB(255, 255, 255) : RGB(148, 163, 184);
                    DrawVectorIconCentered(dis->hDC, ICON_CLOSE, &dis->rcItem, 10, icon_col);
                    return TRUE;
                }

                // ============================================================
                // CYBER CHECKBOXES (ID 401: Repeat, ID 402: Block Input)
                // ============================================================
                if (dis->CtlID == 401 || dis->CtlID == 402) {
                    bool is_checked = false;
                    const wchar_t* label = L"";
                    COLORREF accent = COLOR_NEON_CYAN;
                    
                    if (dis->CtlID == 401) {
                        is_checked = g_add_edit_state.binding.repeat;
                        label = L"Repeat while trigger key is held";
                        accent = COLOR_NEON_CYAN;
                    } else if (dis->CtlID == 402) {
                        is_checked = g_add_edit_state.binding.block_input;
                        label = L"Block Original Input (Suppress trigger key from reaching game/apps)";
                        accent = COLOR_NEON_GREEN;
                    }
                    
                    bool is_selected = (dis->itemState & ODS_SELECTED);
                    bool is_focused = (dis->itemState & ODS_FOCUS);
                    
                    HBRUSH bg_br = CreateSolidBrush(COLOR_BG_CARD);
                    FillRect(dis->hDC, &dis->rcItem, bg_br);
                    DeleteObject(bg_br);
                    
                    int box_size = 18;
                    int box_x = dis->rcItem.left + 2;
                    int box_y = dis->rcItem.top + (dis->rcItem.bottom - dis->rcItem.top - box_size) / 2;
                    RECT box_rc = { box_x, box_y, box_x + box_size, box_y + box_size };
                    
                    if (is_checked) {
                        COLORREF fill_top = (dis->CtlID == 401) ? RGB(79, 70, 229) : RGB(16, 185, 129);
                        COLORREF fill_bot = (dis->CtlID == 401) ? RGB(49, 46, 129) : RGB(6, 78, 59);
                        DrawRoundedRect(dis->hDC, &box_rc, 4, fill_bot, accent, (is_selected || is_focused) ? 2 : 1);
                        RECT grad_box = { box_rc.left + 1, box_rc.top + 1, box_rc.right - 1, box_rc.bottom - 1 };
                        DrawGradientVertical(dis->hDC, &grad_box, fill_top, fill_bot);
                        
                        // Top sheen
                        HPEN sheen_pen = CreatePen(PS_SOLID, 1, (dis->CtlID == 401) ? RGB(165, 180, 252) : RGB(110, 231, 183));
                        HGDIOBJ old_p = SelectObject(dis->hDC, sheen_pen);
                        MoveToEx(dis->hDC, box_rc.left + 2, box_rc.top + 1, NULL);
                        LineTo(dis->hDC, box_rc.right - 2, box_rc.top + 1);
                        SelectObject(dis->hDC, old_p);
                        DeleteObject(sheen_pen);
                        
                        // Crisp white vector checkmark
                        HPEN check_pen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
                        old_p = SelectObject(dis->hDC, check_pen);
                        MoveToEx(dis->hDC, box_x + 4, box_y + 9, NULL);
                        LineTo(dis->hDC, box_x + 7, box_y + 13);
                        LineTo(dis->hDC, box_x + 14, box_y + 4);
                        SelectObject(dis->hDC, old_p);
                        DeleteObject(check_pen);
                    } else {
                        COLORREF border = (is_selected || is_focused) ? accent : COLOR_BORDER_STRONG;
                        DrawRoundedRect(dis->hDC, &box_rc, 4, COLOR_BG_INPUT, border, (is_selected || is_focused) ? 2 : 1);
                    }
                    
                    SetBkMode(dis->hDC, TRANSPARENT);
                    SetTextColor(dis->hDC, is_checked ? COLOR_TEXT_PRIMARY : COLOR_TEXT_SECONDARY);
                    HGDIOBJ old_font = SelectObject(dis->hDC, is_checked ? g_theme_fonts.font_body_bold : g_theme_fonts.font_body);
                    RECT text_rc = { box_x + box_size + 10, dis->rcItem.top, dis->rcItem.right, dis->rcItem.bottom };
                    DrawTextW(dis->hDC, label, -1, &text_rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                    SelectObject(dis->hDC, old_font);
                    return TRUE;
                }
                
                // ============================================================
                // HERO CTA: SAVE MACRO CONFIGURATION (ID 301)
                // ============================================================
                if (dis->CtlID == 301) {
                    bool is_selected = (dis->itemState & ODS_SELECTED);
                    bool is_focused = (dis->itemState & ODS_FOCUS);
                    DrawIndustrialButtonWithIcon(dis->hDC, &dis->rcItem, ICON_SAVE, L"SAVE CONFIGURATION", is_selected || is_focused, true, false, false);
                    return TRUE;
                }

                // ============================================================
                // CANCEL BUTTON (ID 302)
                // ============================================================
                if (dis->CtlID == 302) {
                    bool is_selected = (dis->itemState & ODS_SELECTED);
                    bool is_focused = (dis->itemState & ODS_FOCUS);
                    DrawIndustrialButtonWithIcon(dis->hDC, &dis->rcItem, ICON_CLOSE, L"Cancel", is_selected || is_focused, false, false, false);
                    return TRUE;
                }

                // ============================================================
                // REGULAR CYBER ACTION BUTTONS (ID 101, 102, 202, 203, 204, 205, 206, 207)
                // ============================================================
                bool is_selected = (dis->itemState & ODS_SELECTED);
                bool is_focused = (dis->itemState & ODS_FOCUS);
                bool is_hover = is_selected || is_focused;
                
                if (dis->CtlID == 101) {
                    DrawIndustrialButtonWithIcon(dis->hDC, &dis->rcItem, ICON_PLUS, L"Add Trigger", is_hover, true, false, false);
                } else if (dis->CtlID == 102) {
                    DrawIndustrialButtonWithIcon(dis->hDC, &dis->rcItem, ICON_TRASH, L"Remove", is_hover, false, is_selected, false);
                } else if (dis->CtlID == 202) {
                    DrawIndustrialButtonWithIcon(dis->hDC, &dis->rcItem, ICON_TARGET, NULL, is_hover, false, false, false);
                } else if (dis->CtlID == 203) {
                    if (g_add_edit_state.editing_action_idx >= 0) {
                        wchar_t u_buf[64];
                        swprintf_s(u_buf, 64, L"Update Step #%02d", g_add_edit_state.editing_action_idx + 1);
                        DrawIndustrialButtonWithIcon(dis->hDC, &dis->rcItem, ICON_CHECK, u_buf, is_hover, true, false, true);
                    } else {
                        DrawIndustrialButtonWithIcon(dis->hDC, &dis->rcItem, ICON_PLUS, L"Add Step", is_hover, true, false, false);
                    }
                } else if (dis->CtlID == 204) {
                    DrawIndustrialButtonWithIcon(dis->hDC, &dis->rcItem, ICON_ARROW_UP, L"Move Up", is_hover, false, false, false);
                } else if (dis->CtlID == 205) {
                    DrawIndustrialButtonWithIcon(dis->hDC, &dis->rcItem, ICON_ARROW_DOWN, L"Move Down", is_hover, false, false, false);
                } else if (dis->CtlID == 206) {
                    DrawIndustrialButtonWithIcon(dis->hDC, &dis->rcItem, ICON_TRASH, L"Delete Step", is_hover, false, is_selected, false);
                } else if (dis->CtlID == 207) {
                    DrawIndustrialButtonWithIcon(dis->hDC, &dis->rcItem, ICON_RESET, L"Clear All", is_hover, false, false, false);
                }
                return TRUE;
            }
            break;
        }
        
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);
            
            // Action Type Custom Cyber Dropdown Trigger (ID 201)
            if (id == 201 && code == BN_CLICKED) {
                if (g_dropdown_popup_hwnd && IsWindow(g_dropdown_popup_hwnd)) {
                    DestroyWindow(g_dropdown_popup_hwnd);
                    g_dropdown_popup_hwnd = NULL;
                } else {
                    EnsureDropdownClassRegistered(GetModuleHandle(NULL));
                    RECT btn_rc;
                    GetWindowRect(g_add_edit_state.btn_action_type_dropdown, &btn_rc);
                    int pop_w = btn_rc.right - btn_rc.left;
                    if (pop_w < 205) pop_w = 205;
                    int pop_h = 4 + 6 * 32 + 4;
                    g_dropdown_hover_idx = -1;
                    g_dropdown_popup_hwnd = CreateWindowExW(
                        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
                        L"TobelsoftCyberDropdownClass",
                        L"",
                        WS_POPUP | WS_VISIBLE,
                        btn_rc.left, btn_rc.bottom + 2, pop_w, pop_h,
                        hwnd, NULL, GetModuleHandle(NULL), NULL
                    );
                }
                InvalidateRect(g_add_edit_state.btn_action_type_dropdown, NULL, TRUE);
                return 0;
            }
            
            // + Add Trigger Button
            if (id == 101 && code == BN_CLICKED) {
                if (g_add_edit_state.binding.trigger_count >= MAX_TRIGGERS_PER_BINDING) {
                    MessageBoxW(hwnd, L"Maximum triggers reached (32 keys max).", L"Limit Exceeded", MB_OK | MB_ICONWARNING);
                    return 0;
                }
                char captured[MAX_KEY_NAME_LEN] = {0};
                if (ShowInputCaptureDialog(hwnd, "Capture Trigger Key", captured, sizeof(captured))) {
                    bool exists = false;
                    for (int t = 0; t < g_add_edit_state.binding.trigger_count; t++) {
                        if (_stricmp(g_add_edit_state.binding.trigger_keys[t], captured) == 0) {
                            exists = true;
                            break;
                        }
                    }
                    if (!exists) {
                        int new_idx = g_add_edit_state.binding.trigger_count++;
                        StrCopySafe(g_add_edit_state.binding.trigger_keys[new_idx], captured, MAX_KEY_NAME_LEN);
                        RefreshTriggerList(g_add_edit_state.list_triggers);
                        SendMessageW(g_add_edit_state.list_triggers, LB_SETCURSEL, new_idx, 0);
                    }
                }
                return 0;
            }
            
            // Remove Trigger Button
            if (id == 102 && code == BN_CLICKED) {
                int sel = (int)SendMessageW(g_add_edit_state.list_triggers, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR && sel >= 0 && sel < g_add_edit_state.binding.trigger_count) {
                    for (int i = sel; i < g_add_edit_state.binding.trigger_count - 1; i++) {
                        StrCopySafe(g_add_edit_state.binding.trigger_keys[i], g_add_edit_state.binding.trigger_keys[i + 1], MAX_KEY_NAME_LEN);
                    }
                    g_add_edit_state.binding.trigger_count--;
                    RefreshTriggerList(g_add_edit_state.list_triggers);
                    if (g_add_edit_state.binding.trigger_count > 0) {
                        int new_sel = sel < g_add_edit_state.binding.trigger_count ? sel : (g_add_edit_state.binding.trigger_count - 1);
                        SendMessageW(g_add_edit_state.list_triggers, LB_SETCURSEL, new_sel, 0);
                    }
                }
                return 0;
            }
            
            // Action Key Capture Button
            if (id == 202 && code == BN_CLICKED) {
                char captured[MAX_KEY_NAME_LEN] = {0};
                if (ShowInputCaptureDialog(hwnd, "Capture Target Key", captured, sizeof(captured))) {
                    wchar_t wact[MAX_KEY_NAME_LEN];
                    MultiByteToWideChar(CP_UTF8, 0, captured, -1, wact, MAX_KEY_NAME_LEN);
                    SetWindowTextW(g_add_edit_state.edit_action_key, wact);
                    SetFocus(g_add_edit_state.edit_action_key);
                }
                return 0;
            }
            
            // + Add Action Step Button
            if (id == 203 && code == BN_CLICKED) {
                ActionType act_type = GetCurrentActionType();
                KeyAction a;
                memset(&a, 0, sizeof(KeyAction));
                a.action_type = act_type;
                
                if (act_type == ACTION_DELAY) {
                    wchar_t wdur_buf[32] = {0};
                    GetWindowTextW(g_add_edit_state.edit_duration, wdur_buf, 32);
                    int duration = _wtoi(wdur_buf);
                    if (duration <= 0) duration = 50;
                    a.duration = duration;
                } else if (act_type == ACTION_KEY_HOLD) {
                    wchar_t wkey_buf[MAX_KEY_NAME_LEN] = {0};
                    GetWindowTextW(g_add_edit_state.edit_action_key, wkey_buf, MAX_KEY_NAME_LEN);
                    char key_buf[MAX_KEY_NAME_LEN] = {0};
                    WideCharToMultiByte(CP_UTF8, 0, wkey_buf, -1, key_buf, MAX_KEY_NAME_LEN, NULL, NULL);
                    StrTrim(key_buf);
                    if (!*key_buf || strcmp(key_buf, "[ Delay Only ]") == 0) {
                        MessageBoxW(hwnd, L"Please enter or capture a target key to hold.", L"Target Key Required", MB_OK | MB_ICONWARNING);
                        SetFocus(g_add_edit_state.edit_action_key);
                        return 0;
                    }
                    
                    wchar_t wdur_buf[32] = {0};
                    GetWindowTextW(g_add_edit_state.edit_duration, wdur_buf, 32);
                    int duration = _wtoi(wdur_buf);
                    if (duration <= 0) duration = 100;
                    
                    a.key_count = 1;
                    StrCopySafe(a.keys[0], key_buf, MAX_KEY_NAME_LEN);
                    a.duration = duration;
                } else if (act_type == ACTION_KEY_SEQUENCE) {
                    wchar_t wkey_buf[MAX_KEY_NAME_LEN * 4] = {0};
                    GetWindowTextW(g_add_edit_state.edit_action_key, wkey_buf, sizeof(wkey_buf)/sizeof(wchar_t));
                    char key_buf[MAX_KEY_NAME_LEN * 4] = {0};
                    WideCharToMultiByte(CP_UTF8, 0, wkey_buf, -1, key_buf, sizeof(key_buf), NULL, NULL);
                    StrTrim(key_buf);
                    if (!*key_buf) {
                        MessageBoxW(hwnd, L"Please specify one or more keys (comma-separated, e.g. 'A, B, C').", L"Target Keys Required", MB_OK | MB_ICONWARNING);
                        SetFocus(g_add_edit_state.edit_action_key);
                        return 0;
                    }
                    
                    // Parse comma-separated sequence
                    char* next_tok = NULL;
                    char* tok = strtok_s(key_buf, ",", &next_tok);
                    int k_idx = 0;
                    while (tok && k_idx < MAX_KEYS_PER_ACTION) {
                        StrTrim(tok);
                        if (*tok) {
                            StrCopySafe(a.keys[k_idx++], tok, MAX_KEY_NAME_LEN);
                        }
                        tok = strtok_s(NULL, ",", &next_tok);
                    }
                    a.key_count = k_idx > 0 ? k_idx : 1;
                } else {
                    wchar_t wkey_buf[MAX_KEY_NAME_LEN] = {0};
                    GetWindowTextW(g_add_edit_state.edit_action_key, wkey_buf, MAX_KEY_NAME_LEN);
                    char key_buf[MAX_KEY_NAME_LEN] = {0};
                    WideCharToMultiByte(CP_UTF8, 0, wkey_buf, -1, key_buf, MAX_KEY_NAME_LEN, NULL, NULL);
                    StrTrim(key_buf);
                    if (!*key_buf || strcmp(key_buf, "[ Delay Only ]") == 0) {
                        MessageBoxW(hwnd, L"Please enter or capture a target key for this action.", L"Target Key Required", MB_OK | MB_ICONWARNING);
                        SetFocus(g_add_edit_state.edit_action_key);
                        return 0;
                    }
                    a.key_count = 1;
                    StrCopySafe(a.keys[0], key_buf, MAX_KEY_NAME_LEN);
                }
                
                if (g_add_edit_state.editing_action_idx >= 0 && g_add_edit_state.editing_action_idx < g_add_edit_state.binding.action_count) {
                    int updated_idx = g_add_edit_state.editing_action_idx;
                    g_add_edit_state.binding.actions[updated_idx] = a;
                    g_add_edit_state.editing_action_idx = -1;
                    RefreshActionList(g_add_edit_state.list_actions);
                    SendMessageW(g_add_edit_state.list_actions, LB_SETCURSEL, updated_idx, 0);
                } else {
                    if (g_add_edit_state.binding.action_count >= MAX_ACTIONS_PER_BINDING) {
                        MessageBoxW(hwnd, L"Maximum action steps reached for this macro (16 steps max).", L"Limit Exceeded", MB_OK | MB_ICONWARNING);
                        return 0;
                    }
                    g_add_edit_state.binding.actions[g_add_edit_state.binding.action_count++] = a;
                    RefreshActionList(g_add_edit_state.list_actions);
                    SendMessageW(g_add_edit_state.list_actions, LB_SETCURSEL, g_add_edit_state.binding.action_count - 1, 0);
                }
                
                UpdateActionComposerState();
                if (g_add_edit_state.btn_add_action && IsWindow(g_add_edit_state.btn_add_action)) {
                    InvalidateRect(g_add_edit_state.btn_add_action, NULL, TRUE);
                }
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            
            // Move Up Action Button
            if (id == 204 && code == BN_CLICKED) {
                int sel = (int)SendMessageW(g_add_edit_state.list_actions, LB_GETCURSEL, 0, 0);
                if (sel > 0 && sel < g_add_edit_state.binding.action_count) {
                    KeyAction tmp = g_add_edit_state.binding.actions[sel];
                    g_add_edit_state.binding.actions[sel] = g_add_edit_state.binding.actions[sel - 1];
                    g_add_edit_state.binding.actions[sel - 1] = tmp;
                    
                    if (g_add_edit_state.editing_action_idx == sel) {
                        g_add_edit_state.editing_action_idx = sel - 1;
                    } else if (g_add_edit_state.editing_action_idx == sel - 1) {
                        g_add_edit_state.editing_action_idx = sel;
                    }
                    
                    RefreshActionList(g_add_edit_state.list_actions);
                    SendMessageW(g_add_edit_state.list_actions, LB_SETCURSEL, sel - 1, 0);
                    if (g_add_edit_state.btn_add_action && IsWindow(g_add_edit_state.btn_add_action)) {
                        InvalidateRect(g_add_edit_state.btn_add_action, NULL, TRUE);
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                return 0;
            }
            
            // Move Down Action Button
            if (id == 205 && code == BN_CLICKED) {
                int sel = (int)SendMessageW(g_add_edit_state.list_actions, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR && sel >= 0 && sel < g_add_edit_state.binding.action_count - 1) {
                    KeyAction tmp = g_add_edit_state.binding.actions[sel];
                    g_add_edit_state.binding.actions[sel] = g_add_edit_state.binding.actions[sel + 1];
                    g_add_edit_state.binding.actions[sel + 1] = tmp;
                    
                    if (g_add_edit_state.editing_action_idx == sel) {
                        g_add_edit_state.editing_action_idx = sel + 1;
                    } else if (g_add_edit_state.editing_action_idx == sel + 1) {
                        g_add_edit_state.editing_action_idx = sel;
                    }
                    
                    RefreshActionList(g_add_edit_state.list_actions);
                    SendMessageW(g_add_edit_state.list_actions, LB_SETCURSEL, sel + 1, 0);
                    if (g_add_edit_state.btn_add_action && IsWindow(g_add_edit_state.btn_add_action)) {
                        InvalidateRect(g_add_edit_state.btn_add_action, NULL, TRUE);
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                return 0;
            }
            
            // Delete Selected Action Button
            if (id == 206 && code == BN_CLICKED) {
                int sel = (int)SendMessageW(g_add_edit_state.list_actions, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR && sel >= 0 && sel < g_add_edit_state.binding.action_count) {
                    for (int i = sel; i < g_add_edit_state.binding.action_count - 1; i++) {
                        g_add_edit_state.binding.actions[i] = g_add_edit_state.binding.actions[i + 1];
                    }
                    g_add_edit_state.binding.action_count--;
                    
                    if (g_add_edit_state.editing_action_idx == sel) {
                        g_add_edit_state.editing_action_idx = -1;
                        UpdateActionComposerState();
                    } else if (g_add_edit_state.editing_action_idx > sel) {
                        g_add_edit_state.editing_action_idx--;
                    }
                    
                    RefreshActionList(g_add_edit_state.list_actions);
                    if (sel < g_add_edit_state.binding.action_count) {
                        SendMessageW(g_add_edit_state.list_actions, LB_SETCURSEL, sel, 0);
                    } else if (g_add_edit_state.binding.action_count > 0) {
                        SendMessageW(g_add_edit_state.list_actions, LB_SETCURSEL, g_add_edit_state.binding.action_count - 1, 0);
                    }
                    if (g_add_edit_state.btn_add_action && IsWindow(g_add_edit_state.btn_add_action)) {
                        InvalidateRect(g_add_edit_state.btn_add_action, NULL, TRUE);
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                return 0;
            }
            
            // Clear All Actions Button
            if (id == 207 && code == BN_CLICKED) {
                g_add_edit_state.binding.action_count = 0;
                g_add_edit_state.editing_action_idx = -1;
                UpdateActionComposerState();
                RefreshActionList(g_add_edit_state.list_actions);
                if (g_add_edit_state.btn_add_action && IsWindow(g_add_edit_state.btn_add_action)) {
                    InvalidateRect(g_add_edit_state.btn_add_action, NULL, TRUE);
                }
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            
            // Save Button
            if (id == 301 && code == BN_CLICKED) {
                wchar_t wname_buf[MAX_NAME_LEN] = {0};
                GetWindowTextW(g_add_edit_state.edit_name, wname_buf, MAX_NAME_LEN);
                char name_buf[MAX_NAME_LEN] = {0};
                WideCharToMultiByte(CP_UTF8, 0, wname_buf, -1, name_buf, MAX_NAME_LEN, NULL, NULL);
                StrTrim(name_buf);
                if (!*name_buf) {
                    MessageBoxW(hwnd, L"Please enter a valid macro name.", L"Macro Name Required", MB_OK | MB_ICONWARNING);
                    SetFocus(g_add_edit_state.edit_name);
                    return 0;
                }
                
                if (g_add_edit_state.binding.trigger_count == 0) {
                    MessageBoxW(hwnd, L"Please add at least one trigger key (+ Add Trigger).", L"Trigger Key Required", MB_OK | MB_ICONWARNING);
                    return 0;
                }
                
                if (g_add_edit_state.binding.action_count == 0) {
                    MessageBoxW(hwnd, L"Please configure at least one action step in the pipeline (+ Add Step).", L"Action Sequence Empty", MB_OK | MB_ICONWARNING);
                    return 0;
                }
                
                StrCopySafe(g_add_edit_state.binding.name, name_buf, sizeof(g_add_edit_state.binding.name));
                
                wchar_t wrep_delay_buf[32] = {0};
                GetWindowTextW(g_add_edit_state.edit_repeat_delay, wrep_delay_buf, 32);
                int rep_delay = _wtoi(wrep_delay_buf);
                g_add_edit_state.binding.repeat_delay = rep_delay > 0 ? rep_delay : 100;
                g_add_edit_state.binding.enabled = true;
                
                g_add_edit_state.saved = true;
                DestroyWindow(hwnd);
                return 0;
            }
            
            // Cancel / Close Buttons
            if ((id == 302 || id == 303) && code == BN_CLICKED) {
                DestroyWindow(hwnd);
                return 0;
            }

            // Cyber Checkbox: Repeat Toggle (401)
            if (id == 401 && code == BN_CLICKED) {
                g_add_edit_state.binding.repeat = !g_add_edit_state.binding.repeat;
                InvalidateRect(g_add_edit_state.chk_repeat, NULL, TRUE);
                EnableWindow(g_add_edit_state.edit_repeat_delay, g_add_edit_state.binding.repeat ? TRUE : FALSE);
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }

            // Cyber Checkbox: Block Input Toggle (402)
            if (id == 402 && code == BN_CLICKED) {
                g_add_edit_state.binding.block_input = !g_add_edit_state.binding.block_input;
                InvalidateRect(g_add_edit_state.chk_block_input, NULL, TRUE);
                return 0;
            }
            return 0;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);
            
            // Double buffered rich dark panel background
            HBRUSH bg_brush = CreateSolidBrush(COLOR_BG_PANEL);
            FillRect(hdc, &rc, bg_brush);
            DeleteObject(bg_brush);
            
            // Top Titlebar Header (0..38)
            RECT hdr_rc = { 0, 0, rc.right, 38 };
            DrawGradientVertical(hdc, &hdr_rc, RGB(18, 24, 36), RGB(10, 13, 20));
            
            HPEN div_pen = CreatePen(PS_SOLID, 1, RGB(30, 42, 60));
            HGDIOBJ old_p = SelectObject(hdc, div_pen);
            MoveToEx(hdc, 0, 38, NULL);
            LineTo(hdc, rc.right, 38);
            SelectObject(hdc, old_p);
            DeleteObject(div_pen);
            
            // Titlebar Icon & Header Text
            static HICON s_dlg_icon = NULL;
            static bool s_dlg_icon_loaded = false;
            if (!s_dlg_icon_loaded) {
                s_dlg_icon = (HICON)LoadImageW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_APP_ICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
                s_dlg_icon_loaded = true;
            }
            if (s_dlg_icon) {
                DrawIconEx(hdc, 16, 11, s_dlg_icon, 16, 16, 0, NULL, DI_NORMAL);
            } else {
                DrawVectorIcon(hdc, ICON_BOLT, 16, 11, 16, COLOR_NEON_CYAN);
            }
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, COLOR_TEXT_PRIMARY);
            SelectObject(hdc, g_theme_fonts.font_header);
            RECT title_rc = { 38, 8, 420, 32 };
            DrawTextW(hdc, g_add_edit_state.is_edit ? L"EDIT HOTKEY MACRO CONFIGURATION" : L"ADD NEW HOTKEY MACRO CONFIGURATION", -1, &title_rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            
            // Technical Tag
            SetTextColor(hdc, COLOR_TEXT_MUTED);
            SelectObject(hdc, g_theme_fonts.font_mono_small);
            RECT tag_rc = { 390, 10, rc.right - 45, 30 };
            DrawTextW(hdc, L"NATIVE C11 DIRECTINPUT PIPELINE", -1, &tag_rc, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            
            // Card 1: Macro Identity & Trigger Keys Bento Box (y: 46..286)
            RECT box1 = { 16, 46, rc.right - 16, 286 };
            DrawRoundedRect(hdc, &box1, 8, COLOR_BG_CARD, COLOR_BORDER_SUBTLE, 1);
            DrawVectorIcon(hdc, ICON_KEYBOARD, 28, 52, 14, COLOR_NEON_CYAN);
            SetTextColor(hdc, COLOR_NEON_CYAN);
            SelectObject(hdc, g_theme_fonts.font_mono_small);
            RECT b1_title = { 46, 51, rc.right - 28, 68 };
            DrawTextW(hdc, L"01 // MACRO IDENTITY & TRIGGER ACTIVATION (UP TO 32 TRIGGERS)", -1, &b1_title, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
            
            // Card 2: Action Sequence Pipeline & Composer Bento Box (y: 294..524)
            RECT box2 = { 16, 294, rc.right - 16, 524 };
            DrawRoundedRect(hdc, &box2, 8, COLOR_BG_CARD, COLOR_BORDER_SUBTLE, 1);
            DrawVectorIcon(hdc, ICON_SEQUENCE, 28, 300, 14, COLOR_NEON_CYAN);
            SetTextColor(hdc, COLOR_NEON_CYAN);
            SelectObject(hdc, g_theme_fonts.font_mono_small);
            RECT b2_title = { 46, 299, rc.right - 28, 316 };
            DrawTextW(hdc, L"02 // ACTION EXECUTION PIPELINE (ORDER OF EXECUTION)", -1, &b2_title, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
            
            // Divider line inside Card 2
            HPEN card2_div = CreatePen(PS_SOLID, 1, COLOR_BORDER_SUBTLE);
            old_p = SelectObject(hdc, card2_div);
            MoveToEx(hdc, 28, 440, NULL);
            LineTo(hdc, rc.right - 28, 440);
            SelectObject(hdc, old_p);
            DeleteObject(card2_div);
            
            // Action Composer Sub-header
            DrawVectorIcon(hdc, (g_add_edit_state.editing_action_idx >= 0) ? ICON_EDIT : ICON_TOOLS, 28, 447, 13, (g_add_edit_state.editing_action_idx >= 0) ? COLOR_NEON_GREEN : COLOR_TEXT_MUTED);
            SetTextColor(hdc, (g_add_edit_state.editing_action_idx >= 0) ? COLOR_NEON_GREEN : COLOR_TEXT_MUTED);
            SelectObject(hdc, g_theme_fonts.font_mono_small);
            RECT b2_sub = { 46, 446, rc.right - 28, 462 };
            if (g_add_edit_state.editing_action_idx >= 0) {
                wchar_t sub_buf[80];
                swprintf_s(sub_buf, 80, L"ACTION COMPOSER // EDITING STEP #%02d (ESC TO CANCEL):", g_add_edit_state.editing_action_idx + 1);
                DrawTextW(hdc, sub_buf, -1, &b2_sub, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
            } else {
                DrawTextW(hdc, L"ACTION COMPOSER // CONFIGURE NEXT STEP IN PIPELINE:", -1, &b2_sub, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
            }
            
            // Card 3: Execution Configuration & Safety Lock Bento Box (y: 532..684)
            RECT box3 = { 16, 532, rc.right - 16, 684 };
            DrawRoundedRect(hdc, &box3, 8, COLOR_BG_CARD, COLOR_BORDER_SUBTLE, 1);
            DrawVectorIcon(hdc, ICON_SETTINGS, 28, 538, 14, COLOR_NEON_CYAN);
            SetTextColor(hdc, COLOR_NEON_CYAN);
            SelectObject(hdc, g_theme_fonts.font_mono_small);
            RECT b3_title = { 46, 537, rc.right - 28, 554 };
            DrawTextW(hdc, L"03 // EXECUTION SETTINGS & SAFETY PROTOCOL", -1, &b3_title, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
            
            // Recessed Safety Lock Glass Tile inside Card 3 (y: 616..674)
            RECT safety_tile = { 28, 616, rc.right - 28, 674 };
            DrawRoundedRect(hdc, &safety_tile, 6, COLOR_BG_INPUT, RGB(16, 185, 129), 1);
            
            DrawVectorIcon(hdc, ICON_SHIELD, 38, 624, 14, COLOR_NEON_GREEN);
            SetTextColor(hdc, COLOR_NEON_GREEN);
            SelectObject(hdc, g_theme_fonts.font_mono_small);
            RECT st_rc = { 58, 622, rc.right - 38, 638 };
            DrawTextW(hdc, L"HARDWARE SAFETY LOCK ACTIVE", -1, &st_rc, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
            
            SetTextColor(hdc, COLOR_TEXT_SECONDARY);
            SelectObject(hdc, g_theme_fonts.font_small);
            RECT sd_rc = { 38, 640, rc.right - 38, 670 };
            DrawTextW(hdc, L"Left Click (mouse_left) bypasses input suppression filters unconditionally to ensure normal OS cursor navigation and prevent accidental desktop lockouts.", -1, &sd_rc, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);
            
            // Footer bottom divider line (y: 692)
            HPEN footer_div = CreatePen(PS_SOLID, 1, RGB(30, 42, 60));
            old_p = SelectObject(hdc, footer_div);
            MoveToEx(hdc, 0, 692, NULL);
            LineTo(hdc, rc.right, 692);
            SelectObject(hdc, old_p);
            DeleteObject(footer_div);
            
            // Left Status Indicator in Footer
            HBRUSH dot_br = CreateSolidBrush(COLOR_NEON_GREEN);
            RECT dot_rc = { 28, 715, 36, 723 };
            FillRect(hdc, &dot_rc, dot_br);
            DeleteObject(dot_br);
            
            SetTextColor(hdc, COLOR_TEXT_MUTED);
            SelectObject(hdc, g_theme_fonts.font_mono_small);
            RECT foot_stat_rc = { 42, 709, 230, 729 };
            DrawTextW(hdc, L"READY // HARDWARE DISPATCH", -1, &foot_stat_rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            
            // Window Outer Border (1px high-precision border around entire frameless window)
            HPEN win_border = CreatePen(PS_SOLID, 1, COLOR_BORDER_STRONG);
            old_p = SelectObject(hdc, win_border);
            HGDIOBJ old_b = SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Rectangle(hdc, 0, 0, rc.right, rc.bottom);
            SelectObject(hdc, old_b);
            SelectObject(hdc, old_p);
            DeleteObject(win_border);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
            
        case WM_DESTROY:
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool ShowAddEditHotkeyDialog(HWND parent_hwnd, HotkeyBinding* in_out_binding, bool is_edit) {
    if (!in_out_binding) return false;
    
    memset(&g_add_edit_state, 0, sizeof(AddEditDialogState));
    if (is_edit) {
        g_add_edit_state.binding = *in_out_binding;
        if (g_add_edit_state.binding.action_count < 0) g_add_edit_state.binding.action_count = 0;
        if (g_add_edit_state.binding.action_count > MAX_ACTIONS_PER_BINDING) g_add_edit_state.binding.action_count = MAX_ACTIONS_PER_BINDING;
        if (g_add_edit_state.binding.trigger_count < 0) g_add_edit_state.binding.trigger_count = 0;
        if (g_add_edit_state.binding.trigger_count > MAX_TRIGGERS_PER_BINDING) g_add_edit_state.binding.trigger_count = MAX_TRIGGERS_PER_BINDING;
    } else {
        memset(&g_add_edit_state.binding, 0, sizeof(HotkeyBinding));
        GenerateUUID(g_add_edit_state.binding.id, sizeof(g_add_edit_state.binding.id));
        StrCopySafe(g_add_edit_state.binding.name, "New Macro", sizeof(g_add_edit_state.binding.name));
        g_add_edit_state.binding.repeat_delay = 100;
        g_add_edit_state.binding.enabled = true;
    }
    g_add_edit_state.is_edit = is_edit;
    g_add_edit_state.saved = false;
    
    static bool class_registered = false;
    if (!class_registered) {
        WNDCLASSW wc = {0};
        wc.lpfnWndProc = AddEditWndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = L"TobelsoftAddEditDialogClass";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP_ICON));
        RegisterClassW(&wc);
        class_registered = true;
    }
    
    RECT parent_rc = {0, 0, 1000, 800};
    if (parent_hwnd && IsWindow(parent_hwnd)) {
        GetWindowRect(parent_hwnd, &parent_rc);
    }
    int w = 650, h = 760;
    int x = parent_rc.left + ((parent_rc.right - parent_rc.left) - w) / 2;
    int y = parent_rc.top + ((parent_rc.bottom - parent_rc.top) - h) / 2;
    if (x < 0) x = 50;
    if (y < 0) y = 30;
    
    HWND hwnd = CreateWindowExW(
        WS_EX_TOPMOST,
        L"TobelsoftAddEditDialogClass",
        is_edit ? L"Edit Hotkey Macro" : L"Add New Hotkey Macro",
        WS_POPUP | WS_CLIPCHILDREN,
        x, y, w, h,
        parent_hwnd, NULL, GetModuleHandle(NULL), NULL
    );
    
    if (!hwnd) return false;
    
    g_add_edit_state.hwnd = hwnd;
    HINSTANCE hInst = GetModuleHandle(NULL);
    HFONT hFont = g_theme_fonts.font_body;
    HFONT hFontBold = g_theme_fonts.font_body_bold;
    HFONT hFontSmall = g_theme_fonts.font_small;
    HFONT hFontSmallBold = g_theme_fonts.font_small_bold;
    
    // Top Titlebar Close Button
    g_add_edit_state.btn_close = CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 610, 6, 30, 26, hwnd, (HMENU)303, hInst, NULL);
    
    // ========================================================================
    // CARD 1 CONTROLS: Macro Name & Multiple Trigger Keys (y: 46..286)
    // ========================================================================
    HWND lbl_name = CreateWindowW(L"STATIC", L"Macro Name:", WS_CHILD | WS_VISIBLE, 28, 72, 120, 16, hwnd, NULL, hInst, NULL);
    SendMessageW(lbl_name, WM_SETFONT, (WPARAM)hFontSmallBold, TRUE);
    
    wchar_t wname_init[MAX_NAME_LEN] = {0};
    MultiByteToWideChar(CP_UTF8, 0, g_add_edit_state.binding.name, -1, wname_init, MAX_NAME_LEN);
    g_add_edit_state.edit_name = CreateWindowW(L"EDIT", wname_init, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 28, 90, 310, 26, hwnd, NULL, hInst, NULL);
    SendMessageW(g_add_edit_state.edit_name, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    HWND lbl_trig = CreateWindowW(L"STATIC", L"Trigger Activation Keys (ANY of these triggers macro -- 6 visible per scroll, up to 32 triggers):", WS_CHILD | WS_VISIBLE, 28, 120, 580, 16, hwnd, NULL, hInst, NULL);
    SendMessageW(lbl_trig, WM_SETFONT, (WPARAM)hFontSmallBold, TRUE);
    
    // Height 144px fits exactly 6 full items (6 x 24px = 144px) + WS_VSCROLL for unlimited triggers
    g_add_edit_state.list_triggers = CreateWindowW(
        L"LISTBOX", NULL, 
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | LBS_NOTIFY, 
        28, 138, 446, 144, 
        hwnd, (HMENU)103, hInst, NULL
    );
    SendMessageW(g_add_edit_state.list_triggers, WM_SETFONT, (WPARAM)g_theme_fonts.font_mono_small, TRUE);
    RefreshTriggerList(g_add_edit_state.list_triggers);
    
    g_add_edit_state.btn_add_trigger = CreateWindowW(L"BUTTON", L"+ Add Trigger", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 486, 138, 126, 28, hwnd, (HMENU)101, hInst, NULL);
    SendMessageW(g_add_edit_state.btn_add_trigger, WM_SETFONT, (WPARAM)hFontBold, TRUE);
    
    g_add_edit_state.btn_remove_trigger = CreateWindowW(L"BUTTON", L"Remove", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 486, 172, 126, 28, hwnd, (HMENU)102, hInst, NULL);
    SendMessageW(g_add_edit_state.btn_remove_trigger, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // ========================================================================
    // CARD 2 CONTROLS: Action Sequence Pipeline & Action Composer (y: 294..524)
    // ========================================================================
    g_add_edit_state.list_actions = CreateWindowW(
        L"LISTBOX", NULL, 
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | LBS_NOTIFY, 
        28, 320, 446, 114, 
        hwnd, (HMENU)104, hInst, NULL
    );
    SendMessageW(g_add_edit_state.list_actions, WM_SETFONT, (WPARAM)g_theme_fonts.font_mono_data, TRUE);
    RefreshActionList(g_add_edit_state.list_actions);
    
    // Subclass list_actions for Drag-to-Reorder and Double-Click-to-Edit
    g_add_edit_state.orig_list_actions_proc = (WNDPROC)SetWindowLongPtrW(g_add_edit_state.list_actions, GWLP_WNDPROC, (LONG_PTR)ActionListSubclassProc);
    g_add_edit_state.editing_action_idx = -1;
    g_add_edit_state.drag_from_idx = -1;
    g_add_edit_state.drag_target_idx = -1;
    g_add_edit_state.is_dragging = false;
    
    g_add_edit_state.btn_move_up = CreateWindowW(L"BUTTON", L"Move Up", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 486, 320, 126, 25, hwnd, (HMENU)204, hInst, NULL);
    SendMessageW(g_add_edit_state.btn_move_up, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    g_add_edit_state.btn_move_down = CreateWindowW(L"BUTTON", L"Move Down", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 486, 349, 126, 25, hwnd, (HMENU)205, hInst, NULL);
    SendMessageW(g_add_edit_state.btn_move_down, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    g_add_edit_state.btn_del_action = CreateWindowW(L"BUTTON", L"Delete Step", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 486, 378, 126, 25, hwnd, (HMENU)206, hInst, NULL);
    SendMessageW(g_add_edit_state.btn_del_action, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    g_add_edit_state.btn_clear_actions = CreateWindowW(L"BUTTON", L"Clear All", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 486, 407, 126, 25, hwnd, (HMENU)207, hInst, NULL);
    SendMessageW(g_add_edit_state.btn_clear_actions, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // Action Composer Row Controls (Stable 4-Column Architecture)
    // Column 1: Action Type Custom Cyber Dropdown
    HWND lbl_act_type = CreateWindowW(L"STATIC", L"Action Type:", WS_CHILD | WS_VISIBLE, 28, 464, 144, 16, hwnd, NULL, hInst, NULL);
    SendMessageW(lbl_act_type, WM_SETFONT, (WPARAM)hFontSmallBold, TRUE);
    
    g_add_edit_state.current_action_type = ACTION_KEY_PRESS;
    g_add_edit_state.btn_action_type_dropdown = CreateWindowW(
        L"BUTTON", L"", 
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 
        28, 482, 144, 28, 
        hwnd, (HMENU)201, hInst, NULL
    );
    SendMessageW(g_add_edit_state.btn_action_type_dropdown, WM_SETFONT, (WPARAM)hFontBold, TRUE);
    
    // Column 2: Target Key / Input + Capture
    g_add_edit_state.lbl_action_key = CreateWindowW(L"STATIC", L"Target Key / Input:", WS_CHILD | WS_VISIBLE, 178, 464, 130, 16, hwnd, NULL, hInst, NULL);
    SendMessageW(g_add_edit_state.lbl_action_key, WM_SETFONT, (WPARAM)hFontSmallBold, TRUE);
    
    g_add_edit_state.edit_action_key = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 178, 482, 122, 28, hwnd, NULL, hInst, NULL);
    SendMessageW(g_add_edit_state.edit_action_key, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    g_add_edit_state.btn_capture_action = CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 304, 482, 36, 28, hwnd, (HMENU)202, hInst, NULL);
    SendMessageW(g_add_edit_state.btn_capture_action, WM_SETFONT, (WPARAM)hFontBold, TRUE);
    
    // Column 3: Duration / Delay (ms)
    g_add_edit_state.lbl_duration = CreateWindowW(L"STATIC", L"Duration / Delay:", WS_CHILD | WS_VISIBLE, 348, 464, 100, 16, hwnd, NULL, hInst, NULL);
    SendMessageW(g_add_edit_state.lbl_duration, WM_SETFONT, (WPARAM)hFontSmallBold, TRUE);
    
    g_add_edit_state.edit_duration = CreateWindowW(L"EDIT", L"50", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER, 348, 482, 58, 28, hwnd, NULL, hInst, NULL);
    SendMessageW(g_add_edit_state.edit_duration, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    g_add_edit_state.lbl_ms = CreateWindowW(L"STATIC", L"ms", WS_CHILD | WS_VISIBLE, 410, 486, 22, 20, hwnd, NULL, hInst, NULL);
    SendMessageW(g_add_edit_state.lbl_ms, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
    
    // Column 4: Add Step Button
    g_add_edit_state.btn_add_action = CreateWindowW(L"BUTTON", L"Add Step", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 440, 482, 172, 28, hwnd, (HMENU)203, hInst, NULL);
    SendMessageW(g_add_edit_state.btn_add_action, WM_SETFONT, (WPARAM)hFontBold, TRUE);
    
    UpdateActionComposerState();
    
    // ========================================================================
    // CARD 3 CONTROLS: Execution Settings & Input Block (y: 532..684)
    // ========================================================================
    g_add_edit_state.chk_repeat = CreateWindowW(
        L"BUTTON", 
        L"Repeat while trigger key is held", 
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 
        28, 558, 260, 24, 
        hwnd, (HMENU)401, hInst, NULL
    );
    SendMessageW(g_add_edit_state.chk_repeat, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    g_add_edit_state.lbl_repeat_delay = CreateWindowW(
        L"STATIC", 
        L"Interval Delay:", 
        WS_CHILD | WS_VISIBLE, 
        295, 560, 85, 20, 
        hwnd, NULL, hInst, NULL
    );
    SendMessageW(g_add_edit_state.lbl_repeat_delay, WM_SETFONT, (WPARAM)hFontSmallBold, TRUE);
    
    wchar_t rep_delay_str[32];
    swprintf_s(rep_delay_str, 32, L"%d", g_add_edit_state.binding.repeat_delay > 0 ? g_add_edit_state.binding.repeat_delay : 100);
    g_add_edit_state.edit_repeat_delay = CreateWindowW(
        L"EDIT", 
        rep_delay_str, 
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER, 
        382, 558, 55, 24, 
        hwnd, NULL, hInst, NULL
    );
    SendMessageW(g_add_edit_state.edit_repeat_delay, WM_SETFONT, (WPARAM)hFont, TRUE);
    EnableWindow(g_add_edit_state.edit_repeat_delay, g_add_edit_state.binding.repeat ? TRUE : FALSE);
    
    g_add_edit_state.lbl_repeat_ms = CreateWindowW(
        L"STATIC", 
        L"ms", 
        WS_CHILD | WS_VISIBLE, 
        441, 560, 25, 20, 
        hwnd, NULL, hInst, NULL
    );
    SendMessageW(g_add_edit_state.lbl_repeat_ms, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
    
    g_add_edit_state.chk_block_input = CreateWindowW(
        L"BUTTON", 
        L"Block Original Input (Suppress trigger key from reaching game/apps)", 
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 
        28, 588, 580, 24, 
        hwnd, (HMENU)402, hInst, NULL
    );
    SendMessageW(g_add_edit_state.chk_block_input, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // ========================================================================
    // FOOTER CONTROLS: Save & Cancel Buttons (y: 700)
    // ========================================================================
    g_add_edit_state.btn_cancel = CreateWindowW(
        L"BUTTON", L"Cancel", 
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 
        286, 700, 104, 38, 
        hwnd, (HMENU)302, hInst, NULL
    );
    SendMessageW(g_add_edit_state.btn_cancel, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    g_add_edit_state.btn_save = CreateWindowW(
        L"BUTTON", L"SAVE CONFIGURATION", 
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 
        402, 700, 210, 38, 
        hwnd, (HMENU)301, hInst, NULL
    );
    SendMessageW(g_add_edit_state.btn_save, WM_SETFONT, (WPARAM)hFontBold, TRUE);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    if (parent_hwnd && IsWindow(parent_hwnd)) {
        EnableWindow(parent_hwnd, FALSE);
    }
    
    MSG msg;
    while (IsWindow(hwnd) && GetMessageW(&msg, NULL, 0, 0)) {
        if (msg.message == WM_QUIT) {
            PostQuitMessage((int)msg.wParam);
            break;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    
    if (parent_hwnd && IsWindow(parent_hwnd)) {
        EnableWindow(parent_hwnd, TRUE);
        SetActiveWindow(parent_hwnd);
        SetForegroundWindow(parent_hwnd);
    }
    
    if (g_add_edit_state.saved) {
        *in_out_binding = g_add_edit_state.binding;
        return true;
    }
    return false;
}
