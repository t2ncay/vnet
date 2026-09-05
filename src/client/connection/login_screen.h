#pragma once
#include "raylib.h"
#include <string>

// ============================================================
// VEKTRA OS LOGIN SCREEN
// ============================================================

struct LoginScreen {
    bool isActive;
    bool ipBoxFocused;
    char ipInputBuffer[64];
    char handleBuffer[32];
    float animTime;
    
    // UI state
    bool showCursor;
    float cursorBlinkTimer;
    int selectedField; // 0 = IP, 1 = Handle
    
    // Glitch effects
    float glitchTimer;
    float scanlineOffset;
};

void InitLoginScreen(LoginScreen& login);
void UpdateLoginScreen(LoginScreen& login, float dt);
void DrawLoginScreen(const LoginScreen& login);
bool HandleLoginInput(LoginScreen& login);

// Getter for IP
const char* GetLoginIP(const LoginScreen& login);
const char* GetLoginHandle(const LoginScreen& login);