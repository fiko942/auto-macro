#include "macro_engine.h"
#include "input_hook.h"
#include "input_sender.h"
#include "../storage/config_manager.h"
#include "../common/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AppConfig g_config;
static CRITICAL_SECTION g_engine_cs;
static bool g_bInitialized = false;

static EngineStatusChangedCallback g_status_cb = NULL;
static EngineBindingTriggeredCallback g_triggered_cb = NULL;

// Repeat thread state
static HANDLE g_hRepeatThread = NULL;
static volatile int g_repeat_binding_idx = -1;
static volatile bool g_bStopRepeat = false;
static HANDLE g_hRepeatStopEvent = NULL;

static void OnHookMasterToggle(bool new_active) {
    EnterCriticalSection(&g_engine_cs);
    g_config.active = new_active;
    LeaveCriticalSection(&g_engine_cs);
    
    if (g_status_cb) {
        g_status_cb(new_active);
    }
}

static DWORD WINAPI RepeatThreadProc(LPVOID lpParam) {
    int b_idx = (int)(intptr_t)lpParam;
    if (b_idx < 0 || b_idx >= g_config.binding_count) return 0;
    
    HotkeyBinding binding;
    EnterCriticalSection(&g_engine_cs);
    binding = g_config.bindings[b_idx];
    LeaveCriticalSection(&g_engine_cs);
    
    DWORD delay = (DWORD)binding.repeat_delay;
    if (delay < 1) delay = 10;
    
    while (!g_bStopRepeat && MacroEngine_IsActive()) {
        MacroEngine_ExecuteActions(binding.actions, binding.action_count);
        
        // Wait for delay or stop event
        if (WaitForSingleObject(g_hRepeatStopEvent, delay) != WAIT_TIMEOUT) {
            break;
        }
    }
    
    return 0;
}

void MacroEngine_ExecuteActions(const KeyAction* actions, int action_count) {
    if (!actions || action_count == 0) return;
    
    for (int i = 0; i < action_count; i++) {
        const KeyAction* a = &actions[i];
        switch (a->action_type) {
            case ACTION_KEY_PRESS:
                for (int k = 0; k < a->key_count; k++) {
                    InputSender_KeyPress(a->keys[k], 10);
                    HighResSleep(1);
                }
                break;
            case ACTION_KEY_SEQUENCE:
                for (int k = 0; k < a->key_count; k++) {
                    InputSender_KeyPress(a->keys[k], 10);
                    HighResSleep(1);
                }
                break;
            case ACTION_KEY_DOWN:
                for (int k = 0; k < a->key_count; k++) {
                    InputSender_KeyDown(a->keys[k]);
                }
                break;
            case ACTION_KEY_UP:
                for (int k = 0; k < a->key_count; k++) {
                    InputSender_KeyUp(a->keys[k]);
                }
                break;
            case ACTION_KEY_HOLD:
                if (a->key_count > 0) {
                    InputSender_KeyDown(a->keys[0]);
                    DWORD hold_ms = (DWORD)(a->duration > 0 ? a->duration : 100);
                    HighResSleep(hold_ms);
                    InputSender_KeyUp(a->keys[0]);
                }
                break;
            case ACTION_DELAY:
                if (a->duration > 0) {
                    HighResSleep((DWORD)a->duration);
                }
                break;
            default:
                break;
        }
    }
}

static void StopCurrentRepeat(void) {
    if (g_hRepeatThread) {
        g_bStopRepeat = true;
        if (g_hRepeatStopEvent) {
            SetEvent(g_hRepeatStopEvent);
        }
        WaitForSingleObject(g_hRepeatThread, 500);
        CloseHandle(g_hRepeatThread);
        g_hRepeatThread = NULL;
    }
    g_repeat_binding_idx = -1;
}

typedef struct {
    KeyAction actions[MAX_ACTIONS_PER_BINDING];
    int action_count;
} AsyncActionContext;

static DWORD WINAPI AsyncActionWorkerProc(LPVOID lpParam) {
    AsyncActionContext* ctx = (AsyncActionContext*)lpParam;
    if (ctx) {
        MacroEngine_ExecuteActions(ctx->actions, ctx->action_count);
        free(ctx);
    }
    return 0;
}

void MacroEngine_TriggerBindingByIndex(int binding_index, bool is_down) {
    EnterCriticalSection(&g_engine_cs);
    if (!g_config.active || binding_index < 0 || binding_index >= g_config.binding_count) {
        LeaveCriticalSection(&g_engine_cs);
        return;
    }
    
    HotkeyBinding binding = g_config.bindings[binding_index];
    LeaveCriticalSection(&g_engine_cs);
    
    if (!binding.enabled) return;
    
    if (is_down) {
        if (g_triggered_cb) {
            g_triggered_cb(binding.id);
        }
        
        if (binding.repeat) {
            StopCurrentRepeat();
            g_bStopRepeat = false;
            ResetEvent(g_hRepeatStopEvent);
            g_repeat_binding_idx = binding_index;
            g_hRepeatThread = CreateThread(NULL, 0, RepeatThreadProc, (LPVOID)(intptr_t)binding_index, 0, NULL);
        } else {
            // Offload one-shot action execution to worker thread pool to prevent stalling the hook thread
            AsyncActionContext* ctx = (AsyncActionContext*)malloc(sizeof(AsyncActionContext));
            if (ctx) {
                ctx->action_count = binding.action_count;
                memcpy(ctx->actions, binding.actions, sizeof(KeyAction) * binding.action_count);
                if (!QueueUserWorkItem(AsyncActionWorkerProc, ctx, WT_EXECUTEDEFAULT)) {
                    // Fallback if threadpool queue fails
                    AsyncActionWorkerProc(ctx);
                }
            }
        }
    } else {
        // Key up
        if (binding.repeat && g_repeat_binding_idx == binding_index) {
            StopCurrentRepeat();
        }
    }
}

static void OnHookMacroTrigger(int binding_index, bool is_down) {
    MacroEngine_TriggerBindingByIndex(binding_index, is_down);
}

bool MacroEngine_Init(void) {
    if (g_bInitialized) return true;
    
    InitializeCriticalSection(&g_engine_cs);
    InputSender_Init();
    
    g_hRepeatStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    AppConfig_InitDefault(&g_config);
    
    InputHook_SetMasterToggleCallback(OnHookMasterToggle);
    InputHook_SetTriggerCallback(OnHookMacroTrigger);
    
    if (!InputHook_Start()) {
        DeleteCriticalSection(&g_engine_cs);
        return false;
    }
    
    g_bInitialized = true;
    return true;
}

void MacroEngine_Shutdown(void) {
    if (!g_bInitialized) return;
    
    StopCurrentRepeat();
    if (g_hRepeatStopEvent) {
        CloseHandle(g_hRepeatStopEvent);
        g_hRepeatStopEvent = NULL;
    }
    
    InputHook_Stop();
    InputSender_Cleanup();
    DeleteCriticalSection(&g_engine_cs);
    g_bInitialized = false;
}

void MacroEngine_SetConfig(const AppConfig* config) {
    if (!config) return;
    EnterCriticalSection(&g_engine_cs);
    g_config = *config;
    InputHook_UpdateTriggers(&g_config);
    LeaveCriticalSection(&g_engine_cs);
}

const AppConfig* MacroEngine_GetConfig(void) {
    return &g_config;
}

bool MacroEngine_Start(void) {
    EnterCriticalSection(&g_engine_cs);
    g_config.active = true;
    InputHook_SetActive(true);
    LeaveCriticalSection(&g_engine_cs);
    
    if (g_status_cb) {
        g_status_cb(true);
    }
    return true;
}

void MacroEngine_Stop(void) {
    StopCurrentRepeat();
    EnterCriticalSection(&g_engine_cs);
    g_config.active = false;
    InputHook_SetActive(false);
    LeaveCriticalSection(&g_engine_cs);
    
    if (g_status_cb) {
        g_status_cb(false);
    }
}

void MacroEngine_Toggle(void) {
    if (MacroEngine_IsActive()) {
        MacroEngine_Stop();
    } else {
        MacroEngine_Start();
    }
}

bool MacroEngine_IsActive(void) {
    return g_config.active && InputHook_IsActive();
}

void MacroEngine_SetStatusChangedCallback(EngineStatusChangedCallback callback) {
    g_status_cb = callback;
}

void MacroEngine_SetBindingTriggeredCallback(EngineBindingTriggeredCallback callback) {
    g_triggered_cb = callback;
}
