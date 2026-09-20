#ifndef TOBELSOFT_UI_DIALOGS_H
#define TOBELSOFT_UI_DIALOGS_H

#include <windows.h>
#include <stdbool.h>
#include "../common/types.h"

// Show Real-time Input Capture Modal Dialog (captures keys/mouse)
bool ShowInputCaptureDialog(HWND parent_hwnd, const char* title, char* out_key_name, size_t max_len);

// Show Add / Edit Hotkey Configuration Dialog
bool ShowAddEditHotkeyDialog(HWND parent_hwnd, HotkeyBinding* in_out_binding, bool is_edit);

#endif // TOBELSOFT_UI_DIALOGS_H
