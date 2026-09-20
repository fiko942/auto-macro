#ifndef TOBELSOFT_CONFIG_MANAGER_H
#define TOBELSOFT_CONFIG_MANAGER_H

#include "../common/types.h"
#include <stdbool.h>

// Initialize default configuration
void AppConfig_InitDefault(AppConfig* config);

// Load / Save configuration to JSON
bool AppConfig_Load(AppConfig* config, const char* filepath);
bool AppConfig_Save(const AppConfig* config, const char* filepath);

// Hotkey Binding operations in Config
bool AppConfig_AddBinding(AppConfig* config, const HotkeyBinding* binding);
bool AppConfig_UpdateBinding(AppConfig* config, const HotkeyBinding* binding);
bool AppConfig_DeleteBinding(AppConfig* config, const char* binding_id);
HotkeyBinding* AppConfig_GetBinding(AppConfig* config, const char* binding_id);
bool AppConfig_ToggleBinding(AppConfig* config, const char* binding_id, bool enabled);

// Master trigger keys operations
bool AppConfig_AddMasterTrigger(AppConfig* config, const char* key_name);
bool AppConfig_RemoveMasterTrigger(AppConfig* config, const char* key_name);
bool AppConfig_HasMasterTrigger(const AppConfig* config, const char* key_name);

// Import / Export single hotkey binding to JSON string or file
char* AppConfig_ExportBindingToJSON(const HotkeyBinding* binding);
bool AppConfig_ImportBindingFromJSON(AppConfig* config, const char* json_str);
bool AppConfig_ExportBindingToFile(const HotkeyBinding* binding, const char* filepath);
bool AppConfig_ImportBindingFromFile(AppConfig* config, const char* filepath);

#endif // TOBELSOFT_CONFIG_MANAGER_H
