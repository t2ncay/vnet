#pragma once
#include "raylib.h"

// ============================================================
// THEME SUPPORT
// ============================================================
void SetActiveTheme(const char* name);
void InitColors(void);

// ============================================================
// COLOR PALETTE
// ============================================================
// Base surfaces
extern Color COLOR_BLACK;
extern Color COLOR_PANEL;
extern Color COLOR_CLI_BG;
extern Color COLOR_URLBAR;
extern Color COLOR_BORDER;

// Core accents
extern Color COLOR_BLOOD;
extern Color COLOR_CYAN;
extern Color COLOR_AMBER;
extern Color COLOR_TOXIC;
extern Color COLOR_GHOST;

// Extended accents
extern Color COLOR_VOID;
extern Color COLOR_SIGNAL;
extern Color COLOR_NEON_PURPLE;
extern Color COLOR_NEON_PINK;
extern Color COLOR_STATIC;

// Semantic aliases
extern Color COLOR_SUCCESS;
extern Color COLOR_WARNING;
extern Color COLOR_ERROR;
extern Color COLOR_INFO;