#include "types.h"
#include <string.h>

const char* ActionTypeToString(ActionType type) {
    switch (type) {
        case ACTION_KEY_PRESS: return "key_press";
        case ACTION_KEY_DOWN: return "key_down";
        case ACTION_KEY_UP: return "key_up";
        case ACTION_KEY_HOLD: return "key_hold";
        case ACTION_KEY_SEQUENCE: return "key_sequence";
        case ACTION_DELAY: return "delay";
        default: return "key_press";
    }
}

ActionType StringToActionType(const char* str) {
    if (!str) return ACTION_KEY_PRESS;
    if (strcmp(str, "key_press") == 0) return ACTION_KEY_PRESS;
    if (strcmp(str, "key_down") == 0) return ACTION_KEY_DOWN;
    if (strcmp(str, "key_up") == 0) return ACTION_KEY_UP;
    if (strcmp(str, "key_hold") == 0) return ACTION_KEY_HOLD;
    if (strcmp(str, "key_sequence") == 0) return ACTION_KEY_SEQUENCE;
    if (strcmp(str, "delay") == 0) return ACTION_DELAY;
    return ACTION_KEY_PRESS;
}
