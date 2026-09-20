#include "config_manager.h"
#include "../common/cJSON.h"
#include "../common/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void AppConfig_InitDefault(AppConfig* config) {
    if (!config) return;
    memset(config, 0, sizeof(AppConfig));
    config->active = false;
    config->binding_count = 0;
    
    // Default master trigger is F1
    config->master_trigger_count = 1;
    StrCopySafe(config->master_triggers[0], "f1", MAX_KEY_NAME_LEN);
}

static cJSON* BindingToJSON(const HotkeyBinding* b) {
    cJSON* obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "id", b->id);
    cJSON_AddStringToObject(obj, "name", b->name);
    
    cJSON* triggers_arr = cJSON_AddArrayToObject(obj, "trigger_keys");
    for (int i = 0; i < b->trigger_count; i++) {
        cJSON_AddItemToArray(triggers_arr, cJSON_CreateString(b->trigger_keys[i]));
    }
    
    cJSON* actions_arr = cJSON_AddArrayToObject(obj, "actions");
    for (int i = 0; i < b->action_count; i++) {
        const KeyAction* a = &b->actions[i];
        cJSON* act_obj = cJSON_CreateObject();
        cJSON_AddStringToObject(act_obj, "action_type", ActionTypeToString(a->action_type));
        
        cJSON* keys_arr = cJSON_AddArrayToObject(act_obj, "keys");
        for (int k = 0; k < a->key_count; k++) {
            cJSON_AddItemToArray(keys_arr, cJSON_CreateString(a->keys[k]));
        }
        cJSON_AddNumberToObject(act_obj, "duration", a->duration);
        cJSON_AddItemToArray(actions_arr, act_obj);
    }
    
    cJSON_AddBoolToObject(obj, "enabled", b->enabled ? cJSON_True : cJSON_False);
    cJSON_AddBoolToObject(obj, "repeat", b->repeat ? cJSON_True : cJSON_False);
    cJSON_AddNumberToObject(obj, "repeat_delay", b->repeat_delay);
    cJSON_AddBoolToObject(obj, "block_input", b->block_input ? cJSON_True : cJSON_False);
    
    return obj;
}

static bool JSONToBinding(const cJSON* obj, HotkeyBinding* b) {
    if (!obj || !b) return false;
    memset(b, 0, sizeof(HotkeyBinding));
    
    cJSON* id_item = cJSON_GetObjectItem(obj, "id");
    if (id_item && cJSON_IsString(id_item)) {
        StrCopySafe(b->id, id_item->valuestring, sizeof(b->id));
    } else {
        GenerateUUID(b->id, sizeof(b->id));
    }
    
    cJSON* name_item = cJSON_GetObjectItem(obj, "name");
    if (name_item && cJSON_IsString(name_item)) {
        StrCopySafe(b->name, name_item->valuestring, sizeof(b->name));
    } else {
        StrCopySafe(b->name, "Hotkey", sizeof(b->name));
    }
    
    // Parse trigger_keys (or fallback single trigger_key)
    cJSON* triggers_arr = cJSON_GetObjectItem(obj, "trigger_keys");
    if (triggers_arr && cJSON_IsArray(triggers_arr)) {
        int count = cJSON_GetArraySize(triggers_arr);
        if (count > MAX_TRIGGERS_PER_BINDING) count = MAX_TRIGGERS_PER_BINDING;
        b->trigger_count = count;
        for (int i = 0; i < count; i++) {
            cJSON* item = cJSON_GetArrayItem(triggers_arr, i);
            if (item && cJSON_IsString(item)) {
                StrCopySafe(b->trigger_keys[i], item->valuestring, MAX_KEY_NAME_LEN);
                StrToLower(b->trigger_keys[i]);
            }
        }
    } else {
        cJSON* single_trig = cJSON_GetObjectItem(obj, "trigger_key");
        if (single_trig && cJSON_IsString(single_trig)) {
            b->trigger_count = 1;
            StrCopySafe(b->trigger_keys[0], single_trig->valuestring, MAX_KEY_NAME_LEN);
            StrToLower(b->trigger_keys[0]);
        }
    }
    
    // Parse actions
    cJSON* actions_arr = cJSON_GetObjectItem(obj, "actions");
    if (actions_arr && cJSON_IsArray(actions_arr)) {
        int count = cJSON_GetArraySize(actions_arr);
        if (count > MAX_ACTIONS_PER_BINDING) count = MAX_ACTIONS_PER_BINDING;
        b->action_count = count;
        for (int i = 0; i < count; i++) {
            cJSON* act_obj = cJSON_GetArrayItem(actions_arr, i);
            if (!act_obj) continue;
            
            KeyAction* a = &b->actions[i];
            cJSON* type_item = cJSON_GetObjectItem(act_obj, "action_type");
            if (type_item && cJSON_IsString(type_item)) {
                a->action_type = StringToActionType(type_item->valuestring);
            }
            
            cJSON* keys_arr = cJSON_GetObjectItem(act_obj, "keys");
            if (keys_arr && cJSON_IsArray(keys_arr)) {
                int k_count = cJSON_GetArraySize(keys_arr);
                if (k_count > MAX_KEYS_PER_ACTION) k_count = MAX_KEYS_PER_ACTION;
                a->key_count = k_count;
                for (int k = 0; k < k_count; k++) {
                    cJSON* k_item = cJSON_GetArrayItem(keys_arr, k);
                    if (k_item && cJSON_IsString(k_item)) {
                        StrCopySafe(a->keys[k], k_item->valuestring, MAX_KEY_NAME_LEN);
                        StrToLower(a->keys[k]);
                    }
                }
            }
            
            cJSON* dur_item = cJSON_GetObjectItem(act_obj, "duration");
            if (dur_item && cJSON_IsNumber(dur_item)) {
                a->duration = (int)dur_item->valuedouble;
            }
        }
    }
    
    cJSON* en_item = cJSON_GetObjectItem(obj, "enabled");
    b->enabled = en_item ? (cJSON_IsTrue(en_item) != 0) : true;
    
    cJSON* rep_item = cJSON_GetObjectItem(obj, "repeat");
    b->repeat = rep_item ? (cJSON_IsTrue(rep_item) != 0) : false;
    
    cJSON* delay_item = cJSON_GetObjectItem(obj, "repeat_delay");
    b->repeat_delay = delay_item ? (int)delay_item->valuedouble : 100;
    if (b->repeat_delay < 1) b->repeat_delay = 10;
    
    cJSON* blk_item = cJSON_GetObjectItem(obj, "block_input");
    b->block_input = blk_item ? (cJSON_IsTrue(blk_item) != 0) : false;
    
    return true;
}

bool AppConfig_Load(AppConfig* config, const char* filepath) {
    if (!config || !filepath) return false;
    AppConfig_InitDefault(config);
    
    FILE* f = NULL;
    fopen_s(&f, filepath, "rb");
    if (!f) return false;
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (size <= 0) {
        fclose(f);
        return false;
    }
    
    char* buf = (char*)malloc(size + 1);
    if (!buf) {
        fclose(f);
        return false;
    }
    
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);
    
    cJSON* root = cJSON_Parse(buf);
    free(buf);
    if (!root) return false;
    
    // Parse bindings
    cJSON* bindings_arr = cJSON_GetObjectItem(root, "bindings");
    if (bindings_arr && cJSON_IsArray(bindings_arr)) {
        int count = cJSON_GetArraySize(bindings_arr);
        if (count > MAX_BINDINGS) count = MAX_BINDINGS;
        config->binding_count = count;
        for (int i = 0; i < count; i++) {
            cJSON* item = cJSON_GetArrayItem(bindings_arr, i);
            JSONToBinding(item, &config->bindings[i]);
        }
    }
    
    // Parse master_trigger_keys
    cJSON* masters_arr = cJSON_GetObjectItem(root, "master_trigger_keys");
    if (masters_arr && cJSON_IsArray(masters_arr)) {
        int count = cJSON_GetArraySize(masters_arr);
        if (count > MAX_MASTER_TRIGGERS) count = MAX_MASTER_TRIGGERS;
        config->master_trigger_count = count;
        for (int i = 0; i < count; i++) {
            cJSON* item = cJSON_GetArrayItem(masters_arr, i);
            if (item && cJSON_IsString(item)) {
                StrCopySafe(config->master_triggers[i], item->valuestring, MAX_KEY_NAME_LEN);
                StrToLower(config->master_triggers[i]);
            }
        }
    }
    
    cJSON_Delete(root);
    return true;
}

bool AppConfig_Save(const AppConfig* config, const char* filepath) {
    if (!config || !filepath) return false;
    
    cJSON* root = cJSON_CreateObject();
    
    cJSON* bindings_arr = cJSON_AddArrayToObject(root, "bindings");
    for (int i = 0; i < config->binding_count; i++) {
        cJSON_AddItemToArray(bindings_arr, BindingToJSON(&config->bindings[i]));
    }
    
    cJSON* masters_arr = cJSON_AddArrayToObject(root, "master_trigger_keys");
    for (int i = 0; i < config->master_trigger_count; i++) {
        cJSON_AddItemToArray(masters_arr, cJSON_CreateString(config->master_triggers[i]));
    }
    
    char* json_text = cJSON_Print(root);
    cJSON_Delete(root);
    if (!json_text) return false;
    
    FILE* f = NULL;
    fopen_s(&f, filepath, "wb");
    if (!f) {
        free(json_text);
        return false;
    }
    
    fwrite(json_text, 1, strlen(json_text), f);
    fclose(f);
    free(json_text);
    return true;
}

bool AppConfig_AddBinding(AppConfig* config, const HotkeyBinding* binding) {
    if (!config || !binding) return false;
    
    // Check if ID already exists, if so update it
    for (int i = 0; i < config->binding_count; i++) {
        if (strcmp(config->bindings[i].id, binding->id) == 0) {
            config->bindings[i] = *binding;
            return true;
        }
    }
    
    if (config->binding_count >= MAX_BINDINGS) return false;
    config->bindings[config->binding_count++] = *binding;
    return true;
}

bool AppConfig_UpdateBinding(AppConfig* config, const HotkeyBinding* binding) {
    return AppConfig_AddBinding(config, binding);
}

bool AppConfig_DeleteBinding(AppConfig* config, const char* binding_id) {
    if (!config || !binding_id) return false;
    for (int i = 0; i < config->binding_count; i++) {
        if (strcmp(config->bindings[i].id, binding_id) == 0) {
            // Shift remaining items left
            for (int j = i; j < config->binding_count - 1; j++) {
                config->bindings[j] = config->bindings[j + 1];
            }
            config->binding_count--;
            return true;
        }
    }
    return false;
}

HotkeyBinding* AppConfig_GetBinding(AppConfig* config, const char* binding_id) {
    if (!config || !binding_id) return NULL;
    for (int i = 0; i < config->binding_count; i++) {
        if (strcmp(config->bindings[i].id, binding_id) == 0) {
            return &config->bindings[i];
        }
    }
    return NULL;
}

bool AppConfig_ToggleBinding(AppConfig* config, const char* binding_id, bool enabled) {
    HotkeyBinding* b = AppConfig_GetBinding(config, binding_id);
    if (!b) return false;
    b->enabled = enabled;
    return true;
}

bool AppConfig_AddMasterTrigger(AppConfig* config, const char* key_name) {
    if (!config || !key_name || !*key_name) return false;
    if (AppConfig_HasMasterTrigger(config, key_name)) return true;
    if (config->master_trigger_count >= MAX_MASTER_TRIGGERS) return false;
    
    StrCopySafe(config->master_triggers[config->master_trigger_count++], key_name, MAX_KEY_NAME_LEN);
    StrToLower(config->master_triggers[config->master_trigger_count - 1]);
    return true;
}

bool AppConfig_RemoveMasterTrigger(AppConfig* config, const char* key_name) {
    if (!config || !key_name) return false;
    for (int i = 0; i < config->master_trigger_count; i++) {
        if (StrEqualsIgnoreCase(config->master_triggers[i], key_name)) {
            for (int j = i; j < config->master_trigger_count - 1; j++) {
                StrCopySafe(config->master_triggers[j], config->master_triggers[j + 1], MAX_KEY_NAME_LEN);
            }
            config->master_trigger_count--;
            return true;
        }
    }
    return false;
}

bool AppConfig_HasMasterTrigger(const AppConfig* config, const char* key_name) {
    if (!config || !key_name) return false;
    for (int i = 0; i < config->master_trigger_count; i++) {
        if (StrEqualsIgnoreCase(config->master_triggers[i], key_name)) {
            return true;
        }
    }
    return false;
}

char* AppConfig_ExportBindingToJSON(const HotkeyBinding* binding) {
    if (!binding) return NULL;
    cJSON* obj = BindingToJSON(binding);
    char* str = cJSON_Print(obj);
    cJSON_Delete(obj);
    return str;
}

bool AppConfig_ImportBindingFromJSON(AppConfig* config, const char* json_str) {
    if (!config || !json_str) return false;
    cJSON* obj = cJSON_Parse(json_str);
    if (!obj) return false;
    
    HotkeyBinding binding;
    bool ok = JSONToBinding(obj, &binding);
    cJSON_Delete(obj);
    if (!ok) return false;
    
    return AppConfig_AddBinding(config, &binding);
}

bool AppConfig_ExportBindingToFile(const HotkeyBinding* binding, const char* filepath) {
    if (!binding || !filepath) return false;
    char* json_str = AppConfig_ExportBindingToJSON(binding);
    if (!json_str) return false;
    
    FILE* f = NULL;
    fopen_s(&f, filepath, "wb");
    if (!f) {
        free(json_str);
        return false;
    }
    fwrite(json_str, 1, strlen(json_str), f);
    fclose(f);
    free(json_str);
    return true;
}

bool AppConfig_ImportBindingFromFile(AppConfig* config, const char* filepath) {
    if (!config || !filepath) return false;
    FILE* f = NULL;
    fopen_s(&f, filepath, "rb");
    if (!f) return false;
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0) { fclose(f); return false; }
    
    char* buf = (char*)malloc(size + 1);
    if (!buf) { fclose(f); return false; }
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);
    
    bool ok = AppConfig_ImportBindingFromJSON(config, buf);
    free(buf);
    return ok;
}
