#include "input_hook.h"
#include "input_sender.h"
#include "../common/utils.h"
#include <stdio.h>
#include <string.h>

static HHOOK g_hKeyboardHook = NULL;
static HHOOK g_hMouseHook = NULL;
static HANDLE g_hHookThread = NULL;
static DWORD g_dwHookThreadId = 0;
static bool g_bRunning = false;
static bool g_bActive = false;

static FastTrigger g_fast_triggers[MAX_FAST_TRIGGERS];
static int g_fast_trigger_count = 0;
static CRITICAL_SECTION g_trigger_cs;

static MacroTriggerCallback g_trigger_callback = NULL;
static MasterToggleCallback g_master_callback = NULL;

// Interactive Capture Mode
static volatile bool g_bCapturing = false;
static volatile bool g_bAllowEscapeCapture = false;
static InputCaptureCallback g_capture_callback = NULL;

static uint8_t GetCurrentModifiers(void) {
    uint8_t mods = 0;
    if (GetAsyncKeyState(VK_CONTROL) & 0x8000) mods |= MODIFIER_CTRL;
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000)   mods |= MODIFIER_SHIFT;
    if (GetAsyncKeyState(VK_MENU) & 0x8000)    mods |= MODIFIER_ALT;
    if ((GetAsyncKeyState(VK_LWIN) & 0x8000) || (GetAsyncKeyState(VK_RWIN) & 0x8000)) mods |= MODIFIER_WIN;
    return mods;
}

bool ParseTriggerString(const char* trigger_str, FastTrigger* out_trigger) {
    if (!trigger_str || !out_trigger) return false;
    memset(out_trigger, 0, sizeof(FastTrigger));
    
    char clean[MAX_KEY_NAME_LEN];
    StrCopySafe(clean, trigger_str, sizeof(clean));
    StrToLower(clean);
    StrTrim(clean);
    
    if (!*clean) return false;
    
    // Check modifiers in string (e.g. "ctrl+shift+a")
    char* next_token = NULL;
    char temp[MAX_KEY_NAME_LEN];
    StrCopySafe(temp, clean, sizeof(temp));
    
    char* part = strtok_s(temp, "+", &next_token);
    char last_part[MAX_KEY_NAME_LEN] = {0};
    
    while (part) {
        StrTrim(part);
        if (strcmp(part, "ctrl") == 0 || strcmp(part, "lctrl") == 0 || strcmp(part, "rctrl") == 0) {
            out_trigger->modifiers_mask |= MODIFIER_CTRL;
        } else if (strcmp(part, "shift") == 0 || strcmp(part, "lshift") == 0 || strcmp(part, "rshift") == 0) {
            out_trigger->modifiers_mask |= MODIFIER_SHIFT;
        } else if (strcmp(part, "alt") == 0 || strcmp(part, "lalt") == 0 || strcmp(part, "ralt") == 0) {
            out_trigger->modifiers_mask |= MODIFIER_ALT;
        } else if (strcmp(part, "win") == 0 || strcmp(part, "lwin") == 0 || strcmp(part, "rwin") == 0) {
            out_trigger->modifiers_mask |= MODIFIER_WIN;
        }
        StrCopySafe(last_part, part, sizeof(last_part));
        part = strtok_s(NULL, "+", &next_token);
    }
    
    // Check if the base key is a mouse button
    if (strcmp(last_part, "mouse_left") == 0 || strcmp(last_part, "left") == 0) {
        out_trigger->mouse_btn = MOUSE_TRIGGER_LEFT;
    } else if (strcmp(last_part, "mouse_right") == 0 || strcmp(last_part, "right") == 0) {
        out_trigger->mouse_btn = MOUSE_TRIGGER_RIGHT;
    } else if (strcmp(last_part, "mouse_middle") == 0 || strcmp(last_part, "middle") == 0) {
        out_trigger->mouse_btn = MOUSE_TRIGGER_MIDDLE;
    } else if (strcmp(last_part, "mouse_x1") == 0 || strcmp(last_part, "x1") == 0) {
        out_trigger->mouse_btn = MOUSE_TRIGGER_X1;
    } else if (strcmp(last_part, "mouse_x2") == 0 || strcmp(last_part, "x2") == 0) {
        out_trigger->mouse_btn = MOUSE_TRIGGER_X2;
    } else {
        out_trigger->vk = KeyNameToVk(last_part);
        if (!out_trigger->vk) {
            return false;
        }
    }
    
    return true;
}

void BuildComboString(uint8_t mods, const char* base_key, char* out_combo, size_t max_len) {
    if (!out_combo || max_len == 0) return;
    out_combo[0] = '\0';
    
    if (mods & MODIFIER_CTRL)  strcat_s(out_combo, max_len, "ctrl+");
    if (mods & MODIFIER_SHIFT) strcat_s(out_combo, max_len, "shift+");
    if (mods & MODIFIER_ALT)   strcat_s(out_combo, max_len, "alt+");
    if (mods & MODIFIER_WIN)   strcat_s(out_combo, max_len, "win+");
    
    if (base_key && *base_key) {
        strcat_s(out_combo, max_len, base_key);
    }
}

static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pkb = (KBDLLHOOKSTRUCT*)lParam;
        
        // Skip synthetic inputs injected by SendInput (prevent recursion & self-triggering)
        if (pkb->flags & LLKHF_INJECTED) {
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        }
        
        bool is_down = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
        bool is_up = (wParam == WM_KEYUP || wParam == WM_SYSKEYUP);
        
        WORD vk = (WORD)pkb->vkCode;
        uint8_t mods = GetCurrentModifiers();
        
        // Handle Interactive Capture Mode
        if (g_bCapturing && is_down) {
            if (vk == VK_ESCAPE && !g_bAllowEscapeCapture) {
                // Cancel capture on unhandled escape
                InputHook_StopCapture();
                return 1;
            }
            
            // Ignore bare modifier presses during capture until base key is hit
            if (vk != VK_CONTROL && vk != VK_LCONTROL && vk != VK_RCONTROL &&
                vk != VK_SHIFT && vk != VK_LSHIFT && vk != VK_RSHIFT &&
                vk != VK_MENU && vk != VK_LMENU && vk != VK_RMENU &&
                vk != VK_LWIN && vk != VK_RWIN) {
                
                char combo[64];
                const char* base_name = VkToKeyName(vk);
                BuildComboString(mods, base_name, combo, sizeof(combo));
                
                InputCaptureCallback cb = g_capture_callback;
                InputHook_StopCapture();
                if (cb) {
                    cb(combo, combo);
                }
                return 1; // Block the test key
            }
        }
        
        if (is_down || is_up) {
            EnterCriticalSection(&g_trigger_cs);
            
            // 1. Check Master Toggle Triggers (Highest Priority)
            if (is_down) {
                for (int i = 0; i < g_fast_trigger_count; i++) {
                    const FastTrigger* ft = &g_fast_triggers[i];
                    if (ft->is_master_toggle && ft->vk == vk) {
                        // Match modifiers
                        if (ft->modifiers_mask == 0 || (ft->modifiers_mask == (mods & ft->modifiers_mask))) {
                            g_bActive = !g_bActive;
                            LeaveCriticalSection(&g_trigger_cs);
                            if (g_master_callback) {
                                g_master_callback(g_bActive);
                            }
                            return 1; // Block master toggle key
                        }
                    }
                }
            }
            
            // If macro system is inactive, pass through immediately
            if (!g_bActive) {
                LeaveCriticalSection(&g_trigger_cs);
                return CallNextHookEx(NULL, nCode, wParam, lParam);
            }
            
            // 2. Check Macro Hotkey Bindings
            for (int i = 0; i < g_fast_trigger_count; i++) {
                const FastTrigger* ft = &g_fast_triggers[i];
                if (!ft->is_master_toggle && ft->vk == vk) {
                    // Check modifiers
                    if (ft->modifiers_mask == (mods & (MODIFIER_CTRL | MODIFIER_SHIFT | MODIFIER_ALT | MODIFIER_WIN))) {
                        int binding_idx = ft->binding_index;
                        bool block = ft->block_input;
                        LeaveCriticalSection(&g_trigger_cs);
                        
                        if (g_trigger_callback) {
                            g_trigger_callback(binding_idx, is_down);
                        }
                        
                        if (block) {
                            return 1; // Suppress original input
                        }
                        return CallNextHookEx(NULL, nCode, wParam, lParam);
                    }
                }
            }
            
            LeaveCriticalSection(&g_trigger_cs);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        MSLLHOOKSTRUCT* pms = (MSLLHOOKSTRUCT*)lParam;
        
        // Skip synthetic mouse inputs injected by SendInput
        if (pms->flags & LLMHF_INJECTED) {
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        }
        
        MouseTriggerBtn btn = MOUSE_TRIGGER_NONE;
        bool is_down = false;
        
        switch (wParam) {
            case WM_LBUTTONDOWN: btn = MOUSE_TRIGGER_LEFT; is_down = true; break;
            case WM_LBUTTONUP:   btn = MOUSE_TRIGGER_LEFT; is_down = false; break;
            case WM_RBUTTONDOWN: btn = MOUSE_TRIGGER_RIGHT; is_down = true; break;
            case WM_RBUTTONUP:   btn = MOUSE_TRIGGER_RIGHT; is_down = false; break;
            case WM_MBUTTONDOWN: btn = MOUSE_TRIGGER_MIDDLE; is_down = true; break;
            case WM_MBUTTONUP:   btn = MOUSE_TRIGGER_MIDDLE; is_down = false; break;
            case WM_XBUTTONDOWN:
                btn = (HIWORD(pms->mouseData) == XBUTTON1) ? MOUSE_TRIGGER_X1 : MOUSE_TRIGGER_X2;
                is_down = true;
                break;
            case WM_XBUTTONUP:
                btn = (HIWORD(pms->mouseData) == XBUTTON1) ? MOUSE_TRIGGER_X1 : MOUSE_TRIGGER_X2;
                is_down = false;
                break;
            default: break;
        }
        
        if (btn != MOUSE_TRIGGER_NONE) {
            uint8_t mods = GetCurrentModifiers();
            
            // Handle Interactive Capture Mode for Mouse
            if (g_bCapturing && is_down) {
                const char* base_name = "mouse_left";
                if (btn == MOUSE_TRIGGER_RIGHT) base_name = "mouse_right";
                else if (btn == MOUSE_TRIGGER_MIDDLE) base_name = "mouse_middle";
                else if (btn == MOUSE_TRIGGER_X1) base_name = "mouse_x1";
                else if (btn == MOUSE_TRIGGER_X2) base_name = "mouse_x2";
                
                char combo[64];
                BuildComboString(mods, base_name, combo, sizeof(combo));
                
                InputCaptureCallback cb = g_capture_callback;
                InputHook_StopCapture();
                if (cb) {
                    cb(combo, combo);
                }
                return 1; // Block captured mouse click during modal capture dialog
            }
            
            EnterCriticalSection(&g_trigger_cs);
            
            // Check Master Toggle (if mouse button is set as master)
            if (is_down) {
                for (int i = 0; i < g_fast_trigger_count; i++) {
                    const FastTrigger* ft = &g_fast_triggers[i];
                    if (ft->is_master_toggle && ft->mouse_btn == btn) {
                        if (ft->modifiers_mask == (mods & ft->modifiers_mask)) {
                            g_bActive = !g_bActive;
                            LeaveCriticalSection(&g_trigger_cs);
                            if (g_master_callback) {
                                g_master_callback(g_bActive);
                            }
                            // Safety Lock: Left Click is NEVER blocked even if set as master toggle
                            if (btn == MOUSE_TRIGGER_LEFT) {
                                return CallNextHookEx(NULL, nCode, wParam, lParam);
                            }
                            return 1;
                        }
                    }
                }
            }
            
            if (!g_bActive) {
                LeaveCriticalSection(&g_trigger_cs);
                return CallNextHookEx(NULL, nCode, wParam, lParam);
            }
            
            // Check Macro Hotkey Bindings
            for (int i = 0; i < g_fast_trigger_count; i++) {
                const FastTrigger* ft = &g_fast_triggers[i];
                if (!ft->is_master_toggle && ft->mouse_btn == btn) {
                    if (ft->modifiers_mask == (mods & (MODIFIER_CTRL | MODIFIER_SHIFT | MODIFIER_ALT | MODIFIER_WIN))) {
                        int binding_idx = ft->binding_index;
                        bool block = ft->block_input;
                        
                        // Safety Lock: Left Mouse Button must NEVER be blocked/swallowed
                        if (btn == MOUSE_TRIGGER_LEFT) {
                            block = false;
                        }
                        
                        LeaveCriticalSection(&g_trigger_cs);
                        
                        if (g_trigger_callback) {
                            g_trigger_callback(binding_idx, is_down);
                        }
                        
                        if (block) {
                            return 1;
                        }
                        return CallNextHookEx(NULL, nCode, wParam, lParam);
                    }
                }
            }
            
            LeaveCriticalSection(&g_trigger_cs);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

static DWORD WINAPI HookThreadProc(LPVOID lpParam) {
    (void)lpParam;
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    
    HINSTANCE hInstance = GetModuleHandle(NULL);
    g_hKeyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);
    g_hMouseHook = SetWindowsHookExW(WH_MOUSE_LL, LowLevelMouseProc, hInstance, 0);
    
    MSG msg;
    while (g_bRunning && GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    if (g_hKeyboardHook) {
        UnhookWindowsHookEx(g_hKeyboardHook);
        g_hKeyboardHook = NULL;
    }
    if (g_hMouseHook) {
        UnhookWindowsHookEx(g_hMouseHook);
        g_hMouseHook = NULL;
    }
    return 0;
}

bool InputHook_Start(void) {
    if (g_bRunning) return true;
    InitializeCriticalSection(&g_trigger_cs);
    
    g_bRunning = true;
    g_hHookThread = CreateThread(NULL, 0, HookThreadProc, NULL, 0, &g_dwHookThreadId);
    return g_hHookThread != NULL;
}

void InputHook_Stop(void) {
    if (!g_bRunning) return;
    g_bRunning = false;
    
    if (g_dwHookThreadId) {
        PostThreadMessage(g_dwHookThreadId, WM_QUIT, 0, 0);
    }
    
    if (g_hHookThread) {
        WaitForSingleObject(g_hHookThread, 2000);
        CloseHandle(g_hHookThread);
        g_hHookThread = NULL;
    }
    DeleteCriticalSection(&g_trigger_cs);
}

bool InputHook_IsRunning(void) {
    return g_bRunning;
}

void InputHook_UpdateTriggers(const AppConfig* config) {
    if (!config) return;
    
    EnterCriticalSection(&g_trigger_cs);
    g_fast_trigger_count = 0;
    
    // 1. Add Master Toggle Triggers
    for (int m = 0; m < config->master_trigger_count; m++) {
        FastTrigger ft;
        if (ParseTriggerString(config->master_triggers[m], &ft)) {
            ft.is_master_toggle = true;
            ft.binding_index = -1;
            // Left click is never blocked
            ft.block_input = (ft.mouse_btn != MOUSE_TRIGGER_LEFT);
            if (g_fast_trigger_count < MAX_FAST_TRIGGERS) {
                g_fast_triggers[g_fast_trigger_count++] = ft;
            }
        }
    }
    
    // 2. Add Hotkey Bindings
    for (int b = 0; b < config->binding_count; b++) {
        const HotkeyBinding* binding = &config->bindings[b];
        if (!binding->enabled) continue;
        
        for (int t = 0; t < binding->trigger_count; t++) {
            FastTrigger ft;
            if (ParseTriggerString(binding->trigger_keys[t], &ft)) {
                ft.is_master_toggle = false;
                ft.binding_index = b;
                // Safety Lock: Force block_input false if trigger is mouse_left
                ft.block_input = (ft.mouse_btn == MOUSE_TRIGGER_LEFT) ? false : binding->block_input;
                if (g_fast_trigger_count < MAX_FAST_TRIGGERS) {
                    g_fast_triggers[g_fast_trigger_count++] = ft;
                }
            }
        }
    }
    
    g_bActive = config->active;
    LeaveCriticalSection(&g_trigger_cs);
}

void InputHook_SetActive(bool active) {
    EnterCriticalSection(&g_trigger_cs);
    g_bActive = active;
    LeaveCriticalSection(&g_trigger_cs);
}

bool InputHook_IsActive(void) {
    return g_bActive;
}

void InputHook_SetTriggerCallback(MacroTriggerCallback callback) {
    g_trigger_callback = callback;
}

void InputHook_SetMasterToggleCallback(MasterToggleCallback callback) {
    g_master_callback = callback;
}

void InputHook_StartCapture(InputCaptureCallback callback, bool allow_escape) {
    g_capture_callback = callback;
    g_bAllowEscapeCapture = allow_escape;
    g_bCapturing = true;
}

void InputHook_StopCapture(void) {
    g_bCapturing = false;
    g_capture_callback = NULL;
}

bool InputHook_IsCapturing(void) {
    return g_bCapturing;
}
