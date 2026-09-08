#include "utils.h"

// Prevent GDI (wingdi.h) from defining Rectangle function
#ifdef _WIN32
#define NOGDI
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

#include <cstdlib>
#include <ctime>
#include <cctype>
#include <cstdio>
#include <chrono>      // for GetTimeSeconds

// ============================================================
// Get executable directory (Windows / Linux)
// ============================================================

std::string GetExecutableDirectory() {
    std::string path;
#ifdef _WIN32
    char buffer[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, buffer, MAX_PATH);
    if (len > 0 && len < MAX_PATH) {
        path = buffer;
        size_t lastSlash = path.find_last_of("\\/");
        if (lastSlash != std::string::npos) {
            path = path.substr(0, lastSlash + 1); // Keep trailing slash
        }
    }
#else
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer)-1);
    if (len != -1) {
        buffer[len] = '\0';
        path = buffer;
        size_t lastSlash = path.find_last_of("/");
        if (lastSlash != std::string::npos) {
            path = path.substr(0, lastSlash + 1);
        }
    }
#endif
    return path;
}

// ============================================================
// TIME UTILITIES - Using std::chrono (no Raylib dependency)
// ============================================================

float GetTimeSeconds(void) {
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<float> elapsed = now - start;
    return elapsed.count();
}

// ============================================================
// STRING UTILITIES (unchanged)
// ============================================================

char* CleanStr(char* str) {
    if (!str) return str;
    
    // Trim leading spaces
    char* start = str;
    while (*start && (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n')) {
        start++;
    }
    
    // Trim trailing spaces
    char* end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) {
        end--;
    }
    end[1] = '\0';
    
    // Remove quotes
    if (start[0] == '"' || start[0] == '\'') {
        size_t len = strlen(start);
        if (len > 0 && (start[len-1] == '"' || start[len-1] == '\'')) {
            start[len-1] = '\0';
            start++;
        }
    }
    
    // Shift if needed
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
    
    return str;
}

void TruncateStr(char* str, int maxLen) {
    if (!str) return;
    int len = (int)strlen(str);
    if (len > maxLen) {
        str[maxLen - 2] = '.';
        str[maxLen - 1] = '.';
        str[maxLen] = '\0';
    }
}

int ParsePackage(const char* str, char delimiter, char result[][256], int maxResults) {
    if (!str || maxResults <= 0) return 0;
    
    int count = 0;
    const char* start = str;
    const char* end = str;
    
    while (*end && count < maxResults) {
        if (*end == delimiter) {
            int len = (int)(end - start);
            if (len > 255) len = 255;
            strncpy(result[count], start, len);
            result[count][len] = '\0';
            count++;
            start = end + 1;
        }
        end++;
    }
    
    // Last token
    if (*start && count < maxResults) {
        int len = (int)(end - start);
        if (len > 255) len = 255;
        strncpy(result[count], start, len);
        result[count][len] = '\0';
        count++;
    }
    
    return count;
}

// ============================================================
// ADD MISSING UTILITIES (declared in utils.h)
// ============================================================

void StrTrim(char* str) {
    CleanStr(str); // reuse existing functionality
}

void StrToLower(char* str) {
    if (!str) return;
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

bool StrStartsWith(const char* str, const char* prefix) {
    if (!str || !prefix) return false;
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

bool StrEndsWith(const char* str, const char* suffix) {
    if (!str || !suffix) return false;
    size_t strLen = strlen(str);
    size_t sufLen = strlen(suffix);
    if (sufLen > strLen) return false;
    return strcmp(str + strLen - sufLen, suffix) == 0;
}

int RandomInt(int min, int max) {
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned)time(nullptr));
        seeded = true;
    }
    return min + rand() % (max - min + 1);
}

float RandomFloat(float min, float max) {
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned)time(nullptr));
        seeded = true;
    }
    return min + (float)rand() / (float)RAND_MAX * (max - min);
}

uint32_t ColorToHex(int r, int g, int b, int a) {
    return ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

void SafeStrCopy(char* dest, const char* src, size_t destSize) {
    if (!dest || destSize == 0) return;
    if (!src) { dest[0] = '\0'; return; }
    strncpy(dest, src, destSize - 1);
    dest[destSize - 1] = '\0';
}

void SafeStrCat(char* dest, const char* src, size_t destSize) {
    if (!dest || destSize == 0) return;
    if (!src) return;
    size_t curLen = strlen(dest);
    if (curLen >= destSize - 1) return;
    strncat(dest, src, destSize - curLen - 1);
    dest[destSize - 1] = '\0';
}

bool IsValidVNETUrl(const char* url) {
    if (!url) return false;
    // Simple check: ends with .vnet or is "vnet.dir"
    size_t len = strlen(url);
    if (len < 5) return false;
    if (strcmp(url, "vnet.dir") == 0) return true;
    return strcmp(url + len - 5, ".vnet") == 0;
}

void NormalizeVNETUrl(char* url) {
    if (!url) return;
    // Remove "vnet://" prefix if present
    if (strncmp(url, "vnet://", 7) == 0) {
        memmove(url, url + 7, strlen(url + 7) + 1);
    }
    // If no .vnet and not vnet.dir, append .vnet
    if (!StrEndsWith(url, ".vnet") && strcmp(url, "vnet.dir") != 0) {
        strcat(url, ".vnet");
    }
}