#ifndef TOBELSOFT_TYPES_H
#define TOBELSOFT_TYPES_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_BINDINGS 64
#define MAX_ACTIONS_PER_BINDING 16
#define MAX_TRIGGERS_PER_BINDING 4
#define MAX_KEYS_PER_ACTION 4
#define MAX_KEY_NAME_LEN 32
#define MAX_NAME_LEN 64
#define MAX_ID_LEN 40
#define MAX_MASTER_TRIGGERS 8

typedef enum {
    ACTION_KEY_PRESS = 0,     // Tekan dan lepas (tap)
    ACTION_KEY_DOWN,          // Hanya tekan (down)
    ACTION_KEY_UP,            // Hanya lepas (up)
    ACTION_KEY_HOLD,          // Tahan selama durasi ms
    ACTION_KEY_SEQUENCE,      // Urutan tombol berturut-turut
    ACTION_DELAY              // Jeda / tunggu ms
} ActionType;

typedef struct {
    ActionType action_type;
    char keys[MAX_KEYS_PER_ACTION][MAX_KEY_NAME_LEN];
    int key_count;
    int duration; // in milliseconds
} KeyAction;

typedef struct {
    char id[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    char trigger_keys[MAX_TRIGGERS_PER_BINDING][MAX_KEY_NAME_LEN];
    int trigger_count;
    KeyAction actions[MAX_ACTIONS_PER_BINDING];
    int action_count;
    bool enabled;
    bool repeat;
    int repeat_delay; // in milliseconds
    bool block_input;
} HotkeyBinding;

typedef struct {
    HotkeyBinding bindings[MAX_BINDINGS];
    int binding_count;
    char master_triggers[MAX_MASTER_TRIGGERS][MAX_KEY_NAME_LEN];
    int master_trigger_count;
    bool active;
} AppConfig;

// Convert ActionType to string and back
const char* ActionTypeToString(ActionType type);
ActionType StringToActionType(const char* str);

#endif // TOBELSOFT_TYPES_H
