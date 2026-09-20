#ifndef TOBELSOFT_INPUT_SENDER_H
#define TOBELSOFT_INPUT_SENDER_H

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>

// Initialize input sender subsystem
void InputSender_Init(void);
void InputSender_Cleanup(void);

// Convert key string (e.g. "a", "ctrl", "f1", "space", "mouse_left") to Virtual Key and hardware scan code
WORD KeyNameToVk(const char* key_name);
WORD VkToDirectInputScanCode(WORD vk, bool* out_is_extended);
const char* VkToKeyName(WORD vk);

// Direct Input execution functions (Atomic / Zero-lag)
bool InputSender_KeyDown(const char* key_name);
bool InputSender_KeyUp(const char* key_name);
bool InputSender_KeyPress(const char* key_name, DWORD press_duration_ms);
bool InputSender_MouseClick(const char* mouse_button);
bool InputSender_MouseDown(const char* mouse_button);
bool InputSender_MouseUp(const char* mouse_button);

// High performance batch SendInput execution
bool InputSender_SendBatch(const INPUT* pInputs, UINT count);

#endif // TOBELSOFT_INPUT_SENDER_H
