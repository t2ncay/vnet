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
    
    // ============================================================
    // LOADING STATE - NEW!
    // ============================================================
    bool isLoading;              // true when connecting
    float loadingProgress;       // 0.0 - 1.0
    float loadingTimer;
    char loadingStatus[64];      // Current status text
    bool loadingComplete;        // true when connected

    // ============================================================
    // COMPLETION / OUTRO TRANSITION - NEW!
    // Drives the smoothed handoff to the desktop inside
    // DrawLoadingScreen once loadingComplete goes true, instead of
    // the screen just getting hard-cut away by the caller.
    // ============================================================
    float completionTimer;       // seconds elapsed since loadingComplete became true
};

void InitLoginScreen(LoginScreen& login);
void UpdateLoginScreen(LoginScreen& login, float dt);
void DrawLoginScreen(const LoginScreen& login);
bool HandleLoginInput(LoginScreen& login);
void StartLoading(LoginScreen& login);  // NEW!

// Getter for IP
const char* GetLoginIP(const LoginScreen& login);
const char* GetLoginHandle(const LoginScreen& login);