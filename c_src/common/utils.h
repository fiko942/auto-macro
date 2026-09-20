#ifndef TOBELSOFT_UTILS_H
#define TOBELSOFT_UTILS_H

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>

// String manipulation
void StrToLower(char* str);
void StrTrim(char* str);
void StrCopySafe(char* dest, const char* src, size_t max_len);
bool StrEqualsIgnoreCase(const char* a, const char* b);

// UUID Generation
void GenerateUUID(char* out_uuid, size_t max_len);

// Path & Directory Helpers
bool EnsureDirectoryExists(const char* dir_path);
void GetAppDataDirectory(char* out_path, size_t max_len);
void GetAppDataConfigPath(char* out_path, size_t max_len);
void GetExecutableDirPath(char* out_path, size_t max_len);

// High-Resolution Timing
void InitHighResolutionTimer(void);
void CleanupHighResolutionTimer(void);
double GetTimeSeconds(void);
uint64_t GetTimeMicroseconds(void);
void HighResSleep(DWORD milliseconds);

#endif // TOBELSOFT_UTILS_H
