#ifndef TOBELSOFT_MACRO_ENGINE_H
#define TOBELSOFT_MACRO_ENGINE_H

#include "../common/types.h"
#include <stdbool.h>

// UI Notification callbacks
typedef void (*EngineStatusChangedCallback)(bool is_active);
typedef void (*EngineBindingTriggeredCallback)(const char* binding_id);

// Macro Engine Lifecycle
bool MacroEngine_Init(void);
void MacroEngine_Shutdown(void);

// Configuration & State
void MacroEngine_SetConfig(const AppConfig* config);
const AppConfig* MacroEngine_GetConfig(void);
bool MacroEngine_Start(void);
void MacroEngine_Stop(void);
void MacroEngine_Toggle(void);
bool MacroEngine_IsActive(void);

// Execute actions directly (synchronous or on worker thread)
void MacroEngine_ExecuteActions(const KeyAction* actions, int action_count);
void MacroEngine_TriggerBindingByIndex(int binding_index, bool is_down);

// Callback registration
void MacroEngine_SetStatusChangedCallback(EngineStatusChangedCallback callback);
void MacroEngine_SetBindingTriggeredCallback(EngineBindingTriggeredCallback callback);

#endif // TOBELSOFT_MACRO_ENGINE_H
