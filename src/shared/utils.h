#pragma once
#include <cstdint>
#include <cstring>

// String utilities
void StrTrim(char* str);
void StrToLower(char* str);
bool StrStartsWith(const char* str, const char* prefix);
bool StrEndsWith(const char* str, const char* suffix);

// Random utilities
int RandomInt(int min, int max);
float RandomFloat(float min, float max);

// Color utilities (for future)
uint32_t ColorToHex(int r, int g, int b, int a);

// Time utilities
float GetTimeSeconds(void);

// Memory utilities
void SafeStrCopy(char* dest, const char* src, size_t destSize);
void SafeStrCat(char* dest, const char* src, size_t destSize);

// VNET specific utilities
bool IsValidVNETUrl(const char* url);
void NormalizeVNETUrl(char* url);