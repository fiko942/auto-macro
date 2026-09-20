#include "input_sender.h"
#include "../common/types.h"
#include "../common/utils.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    const char* name;
    WORD vk;
} KeyMapEntry;

static const KeyMapEntry g_key_map[] = {
    // Letters
    {"a", 'A'}, {"b", 'B'}, {"c", 'C'}, {"d", 'D'}, {"e", 'E'},
    {"f", 'F'}, {"g", 'G'}, {"h", 'H'}, {"i", 'I'}, {"j", 'J'},
    {"k", 'K'}, {"l", 'L'}, {"m", 'M'}, {"n", 'N'}, {"o", 'O'},
    {"p", 'P'}, {"q", 'Q'}, {"r", 'R'}, {"s", 'S'}, {"t", 'T'},
    {"u", 'U'}, {"v", 'V'}, {"w", 'W'}, {"x", 'X'}, {"y", 'Y'}, {"z", 'Z'},
    
    // Numbers
    {"0", '0'}, {"1", '1'}, {"2", '2'}, {"3", '3'}, {"4", '4'},
    {"5", '5'}, {"6", '6'}, {"7", '7'}, {"8", '8'}, {"9", '9'},
    
    // Function keys
    {"f1", VK_F1}, {"f2", VK_F2}, {"f3", VK_F3}, {"f4", VK_F4},
    {"f5", VK_F5}, {"f6", VK_F6}, {"f7", VK_F7}, {"f8", VK_F8},
    {"f9", VK_F9}, {"f10", VK_F10}, {"f11", VK_F11}, {"f12", VK_F12},
    {"f13", VK_F13}, {"f14", VK_F14}, {"f15", VK_F15}, {"f16", VK_F16},
    {"f17", VK_F17}, {"f18", VK_F18}, {"f19", VK_F19}, {"f20", VK_F20},
    {"f21", VK_F21}, {"f22", VK_F22}, {"f23", VK_F23}, {"f24", VK_F24},
    
    // Numpad
    {"num0", VK_NUMPAD0}, {"num1", VK_NUMPAD1}, {"num2", VK_NUMPAD2},
    {"num3", VK_NUMPAD3}, {"num4", VK_NUMPAD4}, {"num5", VK_NUMPAD5},
    {"num6", VK_NUMPAD6}, {"num7", VK_NUMPAD7}, {"num8", VK_NUMPAD8},
    {"num9", VK_NUMPAD9}, {"nummultiply", VK_MULTIPLY}, {"numadd", VK_ADD},
    {"numseparator", VK_SEPARATOR}, {"numsubtract", VK_SUBTRACT},
    {"numdecimal", VK_DECIMAL}, {"numdivide", VK_DIVIDE},
    
    // Control / Modifier Keys
    {"ctrl", VK_CONTROL}, {"lctrl", VK_LCONTROL}, {"rctrl", VK_RCONTROL},
    {"shift", VK_SHIFT}, {"lshift", VK_LSHIFT}, {"rshift", VK_RSHIFT},
    {"alt", VK_MENU}, {"lalt", VK_LMENU}, {"ralt", VK_RMENU},
    {"win", VK_LWIN}, {"lwin", VK_LWIN}, {"rwin", VK_RWIN},
    
    // Special / Navigation Keys
    {"space", VK_SPACE}, {"enter", VK_RETURN}, {"return", VK_RETURN},
    {"tab", VK_TAB}, {"backspace", VK_BACK}, {"esc", VK_ESCAPE},
    {"escape", VK_ESCAPE}, {"capslock", VK_CAPITAL}, {"numlock", VK_NUMLOCK},
    {"scrolllock", VK_SCROLL}, {"printscreen", VK_SNAPSHOT}, {"pause", VK_PAUSE},
    {"insert", VK_INSERT}, {"delete", VK_DELETE}, {"home", VK_HOME},
    {"end", VK_END}, {"pageup", VK_PRIOR}, {"pagedown", VK_NEXT},
    {"up", VK_UP}, {"down", VK_DOWN}, {"left", VK_LEFT}, {"right", VK_RIGHT},
    
    // Symbols
    {"minus", VK_OEM_MINUS}, {"plus", VK_OEM_PLUS}, {"equal", VK_OEM_PLUS},
    {"comma", VK_OEM_COMMA}, {"period", VK_OEM_PERIOD}, {"slash", VK_OEM_2},
    {"backslash", VK_OEM_5}, {"semicolon", VK_OEM_1}, {"apostrophe", VK_OEM_7},
    {"bracketleft", VK_OEM_4}, {"bracketright", VK_OEM_6}, {"grave", VK_OEM_3},
};

static const size_t g_key_map_count = sizeof(g_key_map) / sizeof(g_key_map[0]);

// Set of extended virtual key codes
static const WORD g_extended_vks[] = {
    VK_PRIOR, VK_NEXT, VK_END, VK_HOME,
    VK_LEFT, VK_UP, VK_RIGHT, VK_DOWN,
    VK_INSERT, VK_DELETE, VK_NUMLOCK, VK_RCONTROL,
    VK_RMENU, VK_DIVIDE
};
static const size_t g_extended_vks_count = sizeof(g_extended_vks) / sizeof(g_extended_vks[0]);

void InputSender_Init(void) {
    InitHighResolutionTimer();
}

void InputSender_Cleanup(void) {
    CleanupHighResolutionTimer();
}

WORD KeyNameToVk(const char* key_name) {
    if (!key_name || !*key_name) return 0;
    
    char clean[MAX_KEY_NAME_LEN];
    StrCopySafe(clean, key_name, sizeof(clean));
    StrToLower(clean);
    StrTrim(clean);
    
    // If it contains '+', extract the last key (e.g. "ctrl+shift+a" -> "a")
    char* last_plus = strrchr(clean, '+');
    const char* search = last_plus ? (last_plus + 1) : clean;
    
    for (size_t i = 0; i < g_key_map_count; i++) {
        if (strcmp(search, g_key_map[i].name) == 0) {
            return g_key_map[i].vk;
        }
    }
    
    // Check single character
    if (strlen(search) == 1) {
        SHORT vk_scan = VkKeyScanA(search[0]);
        if (vk_scan != -1) {
            return (WORD)(vk_scan & 0xFF);
        }
        return (WORD)toupper((unsigned char)search[0]);
    }
    
    return 0;
}

const char* VkToKeyName(WORD vk) {
    // Check exact matches
    for (size_t i = 0; i < g_key_map_count; i++) {
        if (g_key_map[i].vk == vk) {
            return g_key_map[i].name;
        }
    }
    
    static char buf[16];
    if (vk >= 'A' && vk <= 'Z') {
        buf[0] = (char)tolower(vk);
        buf[1] = '\0';
        return buf;
    }
    if (vk >= '0' && vk <= '9') {
        buf[0] = (char)vk;
        buf[1] = '\0';
        return buf;
    }
    
    snprintf(buf, sizeof(buf), "vk_0x%02x", vk);
    return buf;
}

WORD VkToDirectInputScanCode(WORD vk, bool* out_is_extended) {
    if (out_is_extended) {
        *out_is_extended = false;
        for (size_t i = 0; i < g_extended_vks_count; i++) {
            if (g_extended_vks[i] == vk) {
                *out_is_extended = true;
                break;
            }
        }
    }
    return (WORD)MapVirtualKeyW(vk, MAPVK_VK_TO_VSC);
}

bool InputSender_SendBatch(const INPUT* pInputs, UINT count) {
    if (!pInputs || count == 0) return false;
    UINT sent = SendInput(count, (LPINPUT)pInputs, sizeof(INPUT));
    return sent == count;
}

static bool MakeKeyInput(WORD vk, bool is_key_up, INPUT* out_input) {
    if (!out_input) return false;
    memset(out_input, 0, sizeof(INPUT));
    
    bool is_extended = false;
    WORD scan = VkToDirectInputScanCode(vk, &is_extended);
    DWORD flags = KEYEVENTF_SCANCODE;
    if (is_key_up) flags |= KEYEVENTF_KEYUP;
    if (is_extended) flags |= KEYEVENTF_EXTENDEDKEY;
    
    out_input->type = INPUT_KEYBOARD;
    out_input->ki.wVk = 0;
    out_input->ki.wScan = scan;
    out_input->ki.dwFlags = flags;
    out_input->ki.time = 0;
    out_input->ki.dwExtraInfo = 0;
    return true;
}

bool InputSender_KeyDown(const char* key_name) {
    if (!key_name) return false;
    if (strncmp(key_name, "mouse_", 6) == 0) {
        return InputSender_MouseDown(key_name + 6);
    }
    WORD vk = KeyNameToVk(key_name);
    if (!vk) return false;
    
    INPUT inp;
    MakeKeyInput(vk, false, &inp);
    return InputSender_SendBatch(&inp, 1);
}

bool InputSender_KeyUp(const char* key_name) {
    if (!key_name) return false;
    if (strncmp(key_name, "mouse_", 6) == 0) {
        return InputSender_MouseUp(key_name + 6);
    }
    WORD vk = KeyNameToVk(key_name);
    if (!vk) return false;
    
    INPUT inp;
    MakeKeyInput(vk, true, &inp);
    return InputSender_SendBatch(&inp, 1);
}

bool InputSender_KeyPress(const char* key_name, DWORD press_duration_ms) {
    if (!key_name) return false;
    if (strncmp(key_name, "mouse_", 6) == 0) {
        return InputSender_MouseClick(key_name + 6);
    }
    WORD vk = KeyNameToVk(key_name);
    if (!vk) return false;
    
    if (press_duration_ms == 0) {
        // Atomic batch press (down + up together)
        INPUT inps[2];
        MakeKeyInput(vk, false, &inps[0]);
        MakeKeyInput(vk, true, &inps[1]);
        return InputSender_SendBatch(inps, 2);
    } else {
        INPUT down, up;
        MakeKeyInput(vk, false, &down);
        MakeKeyInput(vk, true, &up);
        InputSender_SendBatch(&down, 1);
        HighResSleep(press_duration_ms);
        return InputSender_SendBatch(&up, 1);
    }
}

bool InputSender_MouseDown(const char* mouse_button) {
    if (!mouse_button) return false;
    DWORD flags = 0;
    DWORD data = 0;
    
    if (StrEqualsIgnoreCase(mouse_button, "left")) flags = MOUSEEVENTF_LEFTDOWN;
    else if (StrEqualsIgnoreCase(mouse_button, "right")) flags = MOUSEEVENTF_RIGHTDOWN;
    else if (StrEqualsIgnoreCase(mouse_button, "middle")) flags = MOUSEEVENTF_MIDDLEDOWN;
    else if (StrEqualsIgnoreCase(mouse_button, "x1")) { flags = MOUSEEVENTF_XDOWN; data = XBUTTON1; }
    else if (StrEqualsIgnoreCase(mouse_button, "x2")) { flags = MOUSEEVENTF_XDOWN; data = XBUTTON2; }
    else return false;
    
    INPUT inp = {0};
    inp.type = INPUT_MOUSE;
    inp.mi.dwFlags = flags;
    inp.mi.mouseData = data;
    return InputSender_SendBatch(&inp, 1);
}

bool InputSender_MouseUp(const char* mouse_button) {
    if (!mouse_button) return false;
    DWORD flags = 0;
    DWORD data = 0;
    
    if (StrEqualsIgnoreCase(mouse_button, "left")) flags = MOUSEEVENTF_LEFTUP;
    else if (StrEqualsIgnoreCase(mouse_button, "right")) flags = MOUSEEVENTF_RIGHTUP;
    else if (StrEqualsIgnoreCase(mouse_button, "middle")) flags = MOUSEEVENTF_MIDDLEUP;
    else if (StrEqualsIgnoreCase(mouse_button, "x1")) { flags = MOUSEEVENTF_XUP; data = XBUTTON1; }
    else if (StrEqualsIgnoreCase(mouse_button, "x2")) { flags = MOUSEEVENTF_XUP; data = XBUTTON2; }
    else return false;
    
    INPUT inp = {0};
    inp.type = INPUT_MOUSE;
    inp.mi.dwFlags = flags;
    inp.mi.mouseData = data;
    return InputSender_SendBatch(&inp, 1);
}

bool InputSender_MouseClick(const char* mouse_button) {
    if (!mouse_button) return false;
    DWORD down_flag = 0, up_flag = 0, data = 0;
    
    if (StrEqualsIgnoreCase(mouse_button, "left")) { down_flag = MOUSEEVENTF_LEFTDOWN; up_flag = MOUSEEVENTF_LEFTUP; }
    else if (StrEqualsIgnoreCase(mouse_button, "right")) { down_flag = MOUSEEVENTF_RIGHTDOWN; up_flag = MOUSEEVENTF_RIGHTUP; }
    else if (StrEqualsIgnoreCase(mouse_button, "middle")) { down_flag = MOUSEEVENTF_MIDDLEDOWN; up_flag = MOUSEEVENTF_MIDDLEUP; }
    else if (StrEqualsIgnoreCase(mouse_button, "x1")) { down_flag = MOUSEEVENTF_XDOWN; up_flag = MOUSEEVENTF_XUP; data = XBUTTON1; }
    else if (StrEqualsIgnoreCase(mouse_button, "x2")) { down_flag = MOUSEEVENTF_XDOWN; up_flag = MOUSEEVENTF_XUP; data = XBUTTON2; }
    else return false;
    
    INPUT inps[2] = {0};
    inps[0].type = INPUT_MOUSE;
    inps[0].mi.dwFlags = down_flag;
    inps[0].mi.mouseData = data;
    
    inps[1].type = INPUT_MOUSE;
    inps[1].mi.dwFlags = up_flag;
    inps[1].mi.mouseData = data;
    
    return InputSender_SendBatch(inps, 2);
}
