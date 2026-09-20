#ifndef TOBELSOFT_INPUT_HOOK_H
#define TOBELSOFT_INPUT_HOOK_H

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>
#include "../common/types.h"

// Modifiers bitmask
#define MODIFIER_NONE  0x00
#define MODIFIER_CTRL  (1 << 0)
#define MODIFIER_SHIFT (1 << 1)
#define MODIFIER_ALT   (1 << 2)
#define MODIFIER_WIN   (1 << 3)

uint8_t InputHook_GetLiveModifiers(void);

// Mouse button types for triggers
typedef enum {
    MOUSE_TRIGGER_NONE = 0,
    MOUSE_TRIGGER_LEFT,
    MOUSE_TRIGGER_RIGHT,
    MOUSE_TRIGGER_MIDDLE,
    MOUSE_TRIGGER_X1,
    MOUSE_TRIGGER_X2
} MouseTriggerBtn;

// Fast Trigger Representation for O(1) matching
typedef struct {
    int binding_index;          // Index in AppConfig or -1 for master toggle
    WORD vk;                    // Virtual key code or 0 if mouse
    MouseTriggerBtn mouse_btn;  // Mouse trigger button
    uint8_t modifiers_mask;     // Bitmask of MODIFIER_*
    bool block_input;           // Suppress original OS event
    bool is_master_toggle;      // Is this a master toggle key
} FastTrigger;

#define MAX_FAST_TRIGGERS 2048

// Callback signatures
typedef void (*MacroTriggerCallback)(int binding_index, bool is_down);
typedef void (*MasterToggleCallback)(bool new_active_state);
typedef void (*InputCaptureCallback)(const char* captured_key_name, const char* display_name);

// Input Hook lifecycle
bool InputHook_Start(void);
void InputHook_Stop(void);
bool InputHook_IsRunning(void);

// Update active triggers from configuration
void InputHook_UpdateTriggers(const AppConfig* config);

// Set / Get Master Active state
void InputHook_SetActive(bool active);
bool InputHook_IsActive(void);

// Register event callbacks
void InputHook_SetTriggerCallback(MacroTriggerCallback callback);
void InputHook_SetMasterToggleCallback(MasterToggleCallback callback);

// Interactive Capture Mode for UI
void InputHook_StartCapture(InputCaptureCallback callback, bool allow_escape);
void InputHook_StopCapture(void);
bool InputHook_IsCapturing(void);

// Helper to convert trigger string to FastTrigger struct
bool ParseTriggerString(const char* trigger_str, FastTrigger* out_trigger);
void BuildComboString(uint8_t mods, const char* base_key, char* out_combo, size_t max_len);

#endif // TOBELSOFT_INPUT_HOOK_H
