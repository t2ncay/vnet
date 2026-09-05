#include "utils.h"
#ifndef HEADLESS_SERVER
    #include "raylib.h"
#endif
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <cstdio>

// ============================================================
// TIME UTILITIES - FIXED
// ============================================================

float GetTimeSeconds(void) {
#ifndef HEADLESS_SERVER
    return (float)GetTime();
#else
    // For server, use clock() or chrono
    static clock_t start = clock();
    return (float)(clock() - start) / CLOCKS_PER_SEC;
#endif
}

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

// Truncate string to max length
void TruncateStr(char* str, int maxLen) {
    if (!str) return;
    int len = strlen(str);
    if (len > maxLen) {
        str[maxLen - 2] = '.';
        str[maxLen - 1] = '.';
        str[maxLen] = '\0';
    }
}

// Parse package - splits string by delimiter
int ParsePackage(const char* str, char delimiter, char result[][256], int maxResults) {
    if (!str || maxResults <= 0) return 0;
    
    int count = 0;
    const char* start = str;
    const char* end = str;
    
    while (*end && count < maxResults) {
        if (*end == delimiter) {
            int len = end - start;
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
        int len = end - start;
        if (len > 255) len = 255;
        strncpy(result[count], start, len);
        result[count][len] = '\0';
        count++;
    }
    
    return count;
}