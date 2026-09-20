/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <float.h>

#include "cJSON.h"

#define internal_malloc malloc
#define internal_free free
#define internal_realloc realloc

static const char *global_error = NULL;

const char *cJSON_GetErrorPtr(void) {
    return global_error;
}

void cJSON_free(void *object) {
    free(object);
}

static cJSON *cJSON_New_Item(void) {
    cJSON* node = (cJSON*)malloc(sizeof(cJSON));
    if (node) {
        memset(node, 0, sizeof(cJSON));
    }
    return node;
}

void cJSON_Delete(cJSON *item) {
    cJSON *next = NULL;
    while (item != NULL) {
        next = item->next;
        if (!(item->type & cJSON_IsReference) && (item->child != NULL)) {
            cJSON_Delete(item->child);
        }
        if (!(item->type & cJSON_IsReference) && (item->valuestring != NULL)) {
            free(item->valuestring);
        }
        if (!(item->type & cJSON_StringIsConst) && (item->string != NULL)) {
            free(item->string);
        }
        free(item);
        item = next;
    }
}

static const char *skip(const char *in) {
    while (in && *in && ((unsigned char)*in <= 32)) in++;
    return in;
}

static const char *parse_number(cJSON *item, const char *num) {
    double n = 0;
    char *endptr = NULL;
    n = strtod(num, &endptr);
    if (endptr == num) return NULL;
    item->valuedouble = n;
    item->valueint = (int)n;
    item->type = cJSON_Number;
    return endptr;
}

static const char *parse_string(cJSON *item, const char *str) {
    const char *ptr = str + 1;
    char *ptr2;
    char *out;
    int len = 0;
    if (*str != '\"') { global_error = str; return NULL; }
    while (*ptr != '\"' && *ptr && ++len) {
        if (*ptr++ == '\\') ptr++;
    }
    out = (char*)malloc(len + 1);
    if (!out) return NULL;
    ptr = str + 1;
    ptr2 = out;
    while (*ptr != '\"' && *ptr) {
        if (*ptr != '\\') *ptr2++ = *ptr++;
        else {
            ptr++;
            switch (*ptr) {
                case 'b': *ptr2++ = '\b'; break;
                case 'f': *ptr2++ = '\f'; break;
                case 'n': *ptr2++ = '\n'; break;
                case 'r': *ptr2++ = '\r'; break;
                case 't': *ptr2++ = '\t'; break;
                case '\"': *ptr2++ = '\"'; break;
                case '\\': *ptr2++ = '\\'; break;
                case '/': *ptr2++ = '/'; break;
                default: *ptr2++ = *ptr; break;
            }
            ptr++;
        }
    }
    *ptr2 = 0;
    if (*ptr == '\"') ptr++;
    item->valuestring = out;
    item->type = cJSON_String;
    return ptr;
}

static const char *parse_value(cJSON *item, const char *value);
static const char *parse_array(cJSON *item, const char *value);
static const char *parse_object(cJSON *item, const char *value);

static const char *parse_value(cJSON *item, const char *value) {
    if (!value) return NULL;
    value = skip(value);
    if (!*value) return NULL;
    if (!strncmp(value, "null", 4)) { item->type = cJSON_NULLType; return value + 4; }
    if (!strncmp(value, "false", 5)) { item->type = cJSON_FalseType; return value + 5; }
    if (!strncmp(value, "true", 4)) { item->type = cJSON_TrueType; item->valueint = 1; return value + 4; }
    if (*value == '\"') { return parse_string(item, value); }
    if (*value == '-' || (*value >= '0' && *value <= '9')) { return parse_number(item, value); }
    if (*value == '[') { return parse_array(item, value); }
    if (*value == '{') { return parse_object(item, value); }
    global_error = value;
    return NULL;
}

static const char *parse_array(cJSON *item, const char *value) {
    cJSON *child;
    if (*value != '[') { global_error = value; return NULL; }
    item->type = cJSON_Array;
    value = skip(value + 1);
    if (*value == ']') return value + 1;
    item->child = child = cJSON_New_Item();
    if (!item->child) return NULL;
    value = skip(parse_value(child, skip(value)));
    if (!value) return NULL;
    while (*value == ',') {
        cJSON *new_item = cJSON_New_Item();
        if (!new_item) return NULL;
        child->next = new_item;
        new_item->prev = child;
        child = new_item;
        value = skip(parse_value(child, skip(value + 1)));
        if (!value) return NULL;
    }
    if (*value == ']') return value + 1;
    global_error = value;
    return NULL;
}

static const char *parse_object(cJSON *item, const char *value) {
    cJSON *child;
    if (*value != '{') { global_error = value; return NULL; }
    item->type = cJSON_Object;
    value = skip(value + 1);
    if (*value == '}') return value + 1;
    item->child = child = cJSON_New_Item();
    if (!item->child) return NULL;
    value = skip(parse_string(child, skip(value)));
    if (!value) return NULL;
    child->string = child->valuestring;
    child->valuestring = NULL;
    if (*value != ':') { global_error = value; return NULL; }
    value = skip(parse_value(child, skip(value + 1)));
    if (!value) return NULL;
    while (*value == ',') {
        cJSON *new_item = cJSON_New_Item();
        if (!new_item) return NULL;
        child->next = new_item;
        new_item->prev = child;
        child = new_item;
        value = skip(parse_string(child, skip(value + 1)));
        if (!value) return NULL;
        child->string = child->valuestring;
        child->valuestring = NULL;
        if (*value != ':') { global_error = value; return NULL; }
        value = skip(parse_value(child, skip(value + 1)));
        if (!value) return NULL;
    }
    if (*value == '}') return value + 1;
    global_error = value;
    return NULL;
}

cJSON *cJSON_Parse(const char *value) {
    cJSON *c = cJSON_New_Item();
    if (!c) return NULL;
    global_error = NULL;
    if (!parse_value(c, skip(value))) {
        cJSON_Delete(c);
        return NULL;
    }
    return c;
}

int cJSON_GetArraySize(const cJSON *array) {
    cJSON *c = array ? array->child : NULL;
    int i = 0;
    while (c) { i++; c = c->next; }
    return i;
}

cJSON *cJSON_GetArrayItem(const cJSON *array, int index) {
    cJSON *c = array ? array->child : NULL;
    while (c && index > 0) { index--; c = c->next; }
    return c;
}

cJSON *cJSON_GetObjectItem(const cJSON * const object, const char * const string) {
    cJSON *c = object ? object->child : NULL;
    while (c && _stricmp(c->string, string)) { c = c->next; }
    return c;
}

cJSON *cJSON_GetObjectItemCaseSensitive(const cJSON * const object, const char * const string) {
    cJSON *c = object ? object->child : NULL;
    while (c && strcmp(c->string, string)) { c = c->next; }
    return c;
}

cJSON_bool cJSON_HasObjectItem(const cJSON *object, const char *string) {
    return cJSON_GetObjectItem(object, string) ? 1 : 0;
}

char *cJSON_GetStringValue(const cJSON * const item) {
    if (!item) return NULL;
    return item->valuestring;
}

double cJSON_GetNumberValue(const cJSON * const item) {
    if (!item) return 0;
    return item->valuedouble;
}

cJSON_bool cJSON_IsTrue(const cJSON * const item) {
    return item && (item->type == cJSON_TrueType);
}

cJSON_bool cJSON_IsFalse(const cJSON * const item) {
    return item && (item->type == cJSON_FalseType);
}

cJSON_bool cJSON_IsBool(const cJSON * const item) {
    return item && ((item->type == cJSON_TrueType) || (item->type == cJSON_FalseType));
}

cJSON_bool cJSON_IsNumber(const cJSON * const item) {
    return item && (item->type == cJSON_Number);
}

cJSON_bool cJSON_IsString(const cJSON * const item) {
    return item && (item->type == cJSON_String);
}

cJSON_bool cJSON_IsArray(const cJSON * const item) {
    return item && (item->type == cJSON_Array);
}

cJSON_bool cJSON_IsObject(const cJSON * const item) {
    return item && (item->type == cJSON_Object);
}

cJSON *cJSON_CreateNull(void) { cJSON *item = cJSON_New_Item(); if (item) item->type = cJSON_NULLType; return item; }
cJSON *cJSON_CreateTrue(void) { cJSON *item = cJSON_New_Item(); if (item) { item->type = cJSON_TrueType; item->valueint = 1; } return item; }
cJSON *cJSON_CreateFalse(void) { cJSON *item = cJSON_New_Item(); if (item) item->type = cJSON_FalseType; return item; }
cJSON *cJSON_CreateBool(cJSON_bool b) { return b ? cJSON_CreateTrue() : cJSON_CreateFalse(); }
cJSON *cJSON_CreateNumber(double num) {
    cJSON *item = cJSON_New_Item();
    if (item) { item->type = cJSON_Number; item->valuedouble = num; item->valueint = (int)num; }
    return item;
}
cJSON *cJSON_CreateString(const char *string) {
    cJSON *item = cJSON_New_Item();
    if (item) { item->type = cJSON_String; item->valuestring = _strdup(string ? string : ""); }
    return item;
}
cJSON *cJSON_CreateArray(void) { cJSON *item = cJSON_New_Item(); if (item) item->type = cJSON_Array; return item; }
cJSON *cJSON_CreateObject(void) { cJSON *item = cJSON_New_Item(); if (item) item->type = cJSON_Object; return item; }

cJSON_bool cJSON_AddItemToArray(cJSON *array, cJSON *item) {
    if (!item || !array) return 0;
    cJSON *child = array->child;
    if (!child) { array->child = item; }
    else {
        while (child->next) child = child->next;
        child->next = item;
        item->prev = child;
    }
    return 1;
}

cJSON_bool cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item) {
    if (!item || !object || !string) return 0;
    if (item->string) free(item->string);
    item->string = _strdup(string);
    return cJSON_AddItemToArray(object, item);
}

cJSON* cJSON_AddNullToObject(cJSON * const object, const char * const name) {
    cJSON *item = cJSON_CreateNull();
    cJSON_AddItemToObject(object, name, item);
    return item;
}

cJSON* cJSON_AddTrueToObject(cJSON * const object, const char * const name) {
    cJSON *item = cJSON_CreateTrue();
    cJSON_AddItemToObject(object, name, item);
    return item;
}

cJSON* cJSON_AddFalseToObject(cJSON * const object, const char * const name) {
    cJSON *item = cJSON_CreateFalse();
    cJSON_AddItemToObject(object, name, item);
    return item;
}

cJSON* cJSON_AddBoolToObject(cJSON * const object, const char * const name, const cJSON_bool boolean) {
    cJSON *item = cJSON_CreateBool(boolean);
    cJSON_AddItemToObject(object, name, item);
    return item;
}

cJSON* cJSON_AddNumberToObject(cJSON * const object, const char * const name, const double number) {
    cJSON *item = cJSON_CreateNumber(number);
    cJSON_AddItemToObject(object, name, item);
    return item;
}

cJSON* cJSON_AddStringToObject(cJSON * const object, const char * const name, const char * const string) {
    cJSON *item = cJSON_CreateString(string);
    cJSON_AddItemToObject(object, name, item);
    return item;
}

cJSON* cJSON_AddObjectToObject(cJSON * const object, const char * const name) {
    cJSON *item = cJSON_CreateObject();
    cJSON_AddItemToObject(object, name, item);
    return item;
}

cJSON* cJSON_AddArrayToObject(cJSON * const object, const char * const name) {
    cJSON *item = cJSON_CreateArray();
    cJSON_AddItemToObject(object, name, item);
    return item;
}

typedef struct {
    char *buffer;
    size_t length;
    size_t offset;
} printbuffer;

static char* ensure(printbuffer *p, size_t needed) {
    if (!p || !p->buffer) return NULL;
    if (p->offset + needed >= p->length) {
        size_t new_len = (p->length + needed) * 2;
        char *new_buf = (char*)realloc(p->buffer, new_len);
        if (!new_buf) return NULL;
        p->buffer = new_buf;
        p->length = new_len;
    }
    return p->buffer + p->offset;
}

static void print_value(const cJSON *item, printbuffer *p, int depth, int fmt);

static void print_number(const cJSON *item, printbuffer *p) {
    char str[64];
    if (floor(item->valuedouble) == item->valuedouble) {
        snprintf(str, sizeof(str), "%d", item->valueint);
    } else {
        snprintf(str, sizeof(str), "%f", item->valuedouble);
    }
    size_t len = strlen(str);
    char *ptr = ensure(p, len + 1);
    if (ptr) { strcpy(ptr, str); p->offset += len; }
}

static void print_string_ptr(const char *str, printbuffer *p) {
    const char *ptr = str;
    ensure(p, 2);
    p->buffer[p->offset++] = '\"';
    while (*ptr) {
        if (*ptr == '\"' || *ptr == '\\') {
            ensure(p, 2);
            p->buffer[p->offset++] = '\\';
            p->buffer[p->offset++] = *ptr++;
        } else if (*ptr == '\n') {
            ensure(p, 2);
            p->buffer[p->offset++] = '\\';
            p->buffer[p->offset++] = 'n';
            ptr++;
        } else if (*ptr == '\t') {
            ensure(p, 2);
            p->buffer[p->offset++] = '\\';
            p->buffer[p->offset++] = 't';
            ptr++;
        } else {
            ensure(p, 1);
            p->buffer[p->offset++] = *ptr++;
        }
    }
    ensure(p, 2);
    p->buffer[p->offset++] = '\"';
    p->buffer[p->offset] = '\0';
}

static void print_array(const cJSON *item, printbuffer *p, int depth, int fmt) {
    ensure(p, 2);
    p->buffer[p->offset++] = '[';
    cJSON *child = item->child;
    while (child) {
        if (fmt) {
            ensure(p, 1 + (depth + 1) * 2);
            p->buffer[p->offset++] = '\n';
            for (int i = 0; i < (depth + 1) * 2; i++) p->buffer[p->offset++] = ' ';
        }
        print_value(child, p, depth + 1, fmt);
        if (child->next) {
            ensure(p, 2);
            p->buffer[p->offset++] = ',';
        }
        child = child->next;
    }
    if (fmt && item->child) {
        ensure(p, 1 + depth * 2);
        p->buffer[p->offset++] = '\n';
        for (int i = 0; i < depth * 2; i++) p->buffer[p->offset++] = ' ';
    }
    ensure(p, 2);
    p->buffer[p->offset++] = ']';
    p->buffer[p->offset] = '\0';
}

static void print_object(const cJSON *item, printbuffer *p, int depth, int fmt) {
    ensure(p, 2);
    p->buffer[p->offset++] = '{';
    cJSON *child = item->child;
    while (child) {
        if (fmt) {
            ensure(p, 1 + (depth + 1) * 2);
            p->buffer[p->offset++] = '\n';
            for (int i = 0; i < (depth + 1) * 2; i++) p->buffer[p->offset++] = ' ';
        }
        print_string_ptr(child->string, p);
        ensure(p, 2);
        p->buffer[p->offset++] = ':';
        if (fmt) p->buffer[p->offset++] = ' ';
        print_value(child, p, depth + 1, fmt);
        if (child->next) {
            ensure(p, 2);
            p->buffer[p->offset++] = ',';
        }
        child = child->next;
    }
    if (fmt && item->child) {
        ensure(p, 1 + depth * 2);
        p->buffer[p->offset++] = '\n';
        for (int i = 0; i < depth * 2; i++) p->buffer[p->offset++] = ' ';
    }
    ensure(p, 2);
    p->buffer[p->offset++] = '}';
    p->buffer[p->offset] = '\0';
}

static void print_value(const cJSON *item, printbuffer *p, int depth, int fmt) {
    if (!item) return;
    switch (item->type) {
        case cJSON_NULLType: { char* s = ensure(p, 5); if (s) { strcpy(s, "null"); p->offset += 4; } break; }
        case cJSON_FalseType: { char* s = ensure(p, 6); if (s) { strcpy(s, "false"); p->offset += 5; } break; }
        case cJSON_TrueType: { char* s = ensure(p, 5); if (s) { strcpy(s, "true"); p->offset += 4; } break; }
        case cJSON_Number: print_number(item, p); break;
        case cJSON_String: print_string_ptr(item->valuestring, p); break;
        case cJSON_Array: print_array(item, p, depth, fmt); break;
        case cJSON_Object: print_object(item, p, depth, fmt); break;
        default: break;
    }
}

char *cJSON_Print(const cJSON *item) {
    printbuffer p;
    p.length = 256;
    p.buffer = (char*)malloc(p.length);
    p.offset = 0;
    if (!p.buffer) return NULL;
    print_value(item, &p, 0, 1);
    return p.buffer;
}

char *cJSON_PrintUnformatted(const cJSON *item) {
    printbuffer p;
    p.length = 256;
    p.buffer = (char*)malloc(p.length);
    p.offset = 0;
    if (!p.buffer) return NULL;
    print_value(item, &p, 0, 0);
    return p.buffer;
}
