#pragma once
#include "raylib.h"

// ============================================================
// THEME SUPPORT
// ============================================================
void SetActiveTheme(const char* name, bool instant = true);
void UpdateThemeTransition(float dt);
void InitColors(void);

// ============================================================
// COLOR HELPERS
// ============================================================
Color BlendColor(Color a, Color b, float t, float boost = 1.0f);

// ============================================================
// THEME MOOD VALUES
// ============================================================
extern float THEME_JITTER_BASELINE;
extern float THEME_SCANLINE_INTENSITY;
extern float THEME_GLOW_STRENGTH;
extern float THEME_DATA_RAIN_DENSITY;
extern float THEME_WARMTH;

// ============================================================
// THEME GRADIENT COLORS
// ============================================================
extern Color THEME_GRADIENT_TOP;
extern Color THEME_GRADIENT_BOTTOM;
extern Color THEME_GRADIENT_ACCENT;

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