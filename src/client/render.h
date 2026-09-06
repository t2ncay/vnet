#pragma once
#include "raylib.h"
#include "connection/login_screen.h"

#include <string> 

struct LoginScreen;
void DrawLoginScreen(const LoginScreen& login);

// ============================================================
// COLOR PALETTE
// ============================================================
// Base surfaces
extern Color COLOR_BLACK;
extern Color COLOR_PANEL;
extern Color COLOR_CLI_BG;
extern Color COLOR_URLBAR;
extern Color COLOR_BORDER;

// Core accents (theme-driven, see kThemes in render.cpp)
extern Color COLOR_BLOOD;
extern Color COLOR_CYAN;
extern Color COLOR_AMBER;
extern Color COLOR_TOXIC;
extern Color COLOR_GHOST;

// Extended accents — derived each SetActiveTheme() call so every
// theme automatically gets a matching "hacker" accent set instead
// of hardcoding a second 15-row table.
extern Color COLOR_VOID;          // near-black, sits behind COLOR_BLACK for deep vignette layers
extern Color COLOR_SIGNAL;        // hot cyan/toxic blend — "live data" accent (progress, active links)
extern Color COLOR_NEON_PURPLE;   // cyan/blood blend — secondary accent for gradients & glows
extern Color COLOR_NEON_PINK;     // blood/amber blend — tertiary accent for gradients & glows
extern Color COLOR_STATIC;        // low-alpha ghost — CRT static / noise texture color

// Semantic aliases (use these in new code instead of raw accent names —
// keeps intent readable: a red badge because it's an ERROR, not because it's "blood")
extern Color COLOR_SUCCESS;   // = COLOR_TOXIC
extern Color COLOR_WARNING;   // = COLOR_AMBER
extern Color COLOR_ERROR;     // = COLOR_BLOOD
extern Color COLOR_INFO;      // = COLOR_CYAN

// Font
extern Font g_fontVCR;
extern float g_fontScale;

// UI Scale and letterbox offsets
extern float g_uiScale;
extern float g_offsetX;
extern float g_offsetY;

extern struct LoginScreen g_loginScreen;

void InitColors(void);
void DrawUI(void);
void DrawTerminal(void);
void DrawFeedPanel(void);
void DrawStatusBar(void);
void DrawVPulse(float x, float y, float size, float time);
void DrawHellroomUI(float jx, float jy, Vector2 refMouse, bool clicked);

void DrawMarkupPage(float contentX, float contentY, float contentW, float contentH);

// Scaled drawing helpers
void DrawScaledRect(float x, float y, float w, float h, Color color);
void DrawScaledRectLines(float x, float y, float w, float h, Color color);
void DrawScaledText(const char* text, float x, float y, float fontSize, Color color);
void DrawScaledLine(float x1, float y1, float x2, float y2, Color color);
void DrawScaledCircle(float centerX, float centerY, float radius, Color color);
void DrawScaledCircleLines(float centerX, float centerY, float radius, Color color);

// ============================================================
// ANIMATION / GLOW HELPERS (network-hacking visual language)
// ============================================================
// Small, reusable building blocks so every panel/badge/overlay in the
// game can share the same pulse-and-glow vocabulary instead of each
// screen re-deriving its own sine wave (see raid_ui.cpp for the pattern
// this generalizes).

// Returns a 0..1 sine pulse. Use to drive alpha, scale, or offsets.
float PulseWave(float speed, float phase = 0.0f);

// Returns `base` with alpha oscillating between minAlpha and maxAlpha.
Color ColorPulse(Color base, float speed, float minAlpha = 0.35f, float maxAlpha = 1.0f);

// Soft glow made of stacked, fading outlines around a rect — the
// "this node is alive" look used on badges, buttons, and alerts.
void DrawGlowRect(float x, float y, float w, float h, Color color, float glowSize = 10.0f, int layers = 4);

// Thin scanline sweep over a region (extracted from the raid overlay
// so any panel can get the same CRT texture in one call).
void DrawScanlineOverlay(float x, float y, float w, float h, Color tint, float speed = 90.0f, float spacing = 5.0f);

// Vertical "falling glyph" data-rain effect for backgrounds / transitions.
// Cheap and seeded per-column so it stays stable frame to frame.
void DrawDataStream(float x, float y, float w, float h, Color color, int columns = 12, float speed = 220.0f);


// Asset management
void LoadAssets(void);
void UnloadAssets(void);

// Theme support
void SetActiveTheme(const char* name);

Vector2 GetRefMousePos(void);
bool RefRectHover(float x, float y, float w, float h, Vector2 refMouse);

void DrawConnectionOverlay(void);

void TriggerJitter(float intensity, float duration = 0.0f);
float GetJitterX(void);
float GetJitterY(void);
bool IsJittering(void);
void InitJitter(void);
float GetJitterIntensity(void);
void TriggerGlitch(float intensity);

float SX(float x);
float SY(float y);
bool StartsWith(const std::string& str, const char* prefix);
float MeasureScaledTextWidth(const char* text, float fontSize);