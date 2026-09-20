#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <mmsystem.h>
#include <shlobj.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")

static LARGE_INTEGER g_perf_frequency = {0};
static HANDLE g_hWaitableTimer = NULL;

void StrToLower(char* str) {
    if (!str) return;
    for (int i = 0; str[i]; i++) {
        str[i] = (char)tolower((unsigned char)str[i]);
    }
}

void StrTrim(char* str) {
    if (!str) return;
    // Trim leading
    char* start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
    // Trim trailing
    size_t len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[--len] = '\0';
    }
}

void StrCopySafe(char* dest, const char* src, size_t max_len) {
    if (!dest || max_len == 0) return;
    if (!src) {
        dest[0] = '\0';
        return;
    }
    strncpy_s(dest, max_len, src, _TRUNCATE);
    dest[max_len - 1] = '\0';
}

bool StrEqualsIgnoreCase(const char* a, const char* b) {
    if (!a || !b) return a == b;
    return _stricmp(a, b) == 0;
}

void GenerateUUID(char* out_uuid, size_t max_len) {
    if (!out_uuid || max_len < 37) return;
    GUID guid;
    if (SUCCEEDED(CoCreateGuid(&guid))) {
        snprintf(out_uuid, max_len,
            "%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            guid.Data1, guid.Data2, guid.Data3,
            guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
            guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
    } else {
        // Fallback pseudo UUID
        snprintf(out_uuid, max_len, "%08x-%04x-%04x-%04x-%012llx",
            rand(), rand() & 0xFFFF, (rand() & 0x0FFF) | 0x4000,
            (rand() & 0x3FFF) | 0x8000, ((uint64_t)rand() << 32) | rand());
    }
}

bool EnsureDirectoryExists(const char* dir_path) {
    if (!dir_path || !*dir_path) return false;
    char tmp[MAX_PATH];
    StrCopySafe(tmp, dir_path, sizeof(tmp));
    
    for (char* p = tmp + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            *p = '\0';
            CreateDirectoryA(tmp, NULL);
            *p = '\\';
        }
    }
    return CreateDirectoryA(tmp, NULL) || GetLastError() == ERROR_ALREADY_EXISTS;
}

void GetAppDataDirectory(char* out_path, size_t max_len) {
    if (!out_path || max_len == 0) return;
    char exe_dir[MAX_PATH];
    GetExecutableDirPath(exe_dir, sizeof(exe_dir));
    snprintf(out_path, max_len, "%s\\appdata", exe_dir);
    EnsureDirectoryExists(out_path);
}

void GetAppDataConfigPath(char* out_path, size_t max_len) {
    if (!out_path || max_len == 0) return;
    char local_appdata[MAX_PATH];
    GetAppDataDirectory(local_appdata, sizeof(local_appdata));
    snprintf(out_path, max_len, "%s\\tobelsoft_macro_data.json", local_appdata);
}

void GetExecutableDirPath(char* out_path, size_t max_len) {
    if (!out_path || max_len == 0) return;
    GetModuleFileNameA(NULL, out_path, (DWORD)max_len);
    char* last_slash = strrchr(out_path, '\\');
    if (last_slash) {
        *last_slash = '\0';
    }
}

void InitHighResolutionTimer(void) {
    QueryPerformanceFrequency(&g_perf_frequency);
    timeBeginPeriod(1);
    
    // Attempt to create high resolution waitable timer (supported on modern Windows 10/11)
    g_hWaitableTimer = CreateWaitableTimerExW(NULL, NULL, 
        CREATE_WAITABLE_TIMER_HIGH_RESOLUTION | CREATE_WAITABLE_TIMER_MANUAL_RESET, 
        TIMER_ALL_ACCESS);
    if (!g_hWaitableTimer) {
        g_hWaitableTimer = CreateWaitableTimerW(NULL, TRUE, NULL);
    }
}

void CleanupHighResolutionTimer(void) {
    if (g_hWaitableTimer) {
        CloseHandle(g_hWaitableTimer);
        g_hWaitableTimer = NULL;
    }
    timeEndPeriod(1);
}

double GetTimeSeconds(void) {
    if (g_perf_frequency.QuadPart == 0) {
        QueryPerformanceFrequency(&g_perf_frequency);
    }
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)g_perf_frequency.QuadPart;
}

uint64_t GetTimeMicroseconds(void) {
    if (g_perf_frequency.QuadPart == 0) {
        QueryPerformanceFrequency(&g_perf_frequency);
    }
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (uint64_t)((counter.QuadPart * 1000000ULL) / g_perf_frequency.QuadPart);
}

void HighResSleep(DWORD milliseconds) {
    if (milliseconds == 0) return;
    
    if (g_hWaitableTimer) {
        LARGE_INTEGER due_time;
        // Negative value means relative time in 100-nanosecond intervals
        due_time.QuadPart = -((LONGLONG)milliseconds * 10000LL);
        SetWaitableTimer(g_hWaitableTimer, &due_time, 0, NULL, NULL, FALSE);
        WaitForSingleObject(g_hWaitableTimer, INFINITE);
    } else {
        // Fallback sleep with 1ms resolution
        Sleep(milliseconds);
    }
}
