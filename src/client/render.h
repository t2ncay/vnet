#pragma once
#include "raylib.h"

#include <string> 

// Color palette
extern Color COLOR_BLACK;
extern Color COLOR_PANEL;
extern Color COLOR_CLI_BG;
extern Color COLOR_URLBAR;
extern Color COLOR_BORDER;
extern Color COLOR_BLOOD;
extern Color COLOR_CYAN;
extern Color COLOR_AMBER;
extern Color COLOR_TOXIC;
extern Color COLOR_GHOST;

// Font
extern Font g_fontVCR;
extern float g_fontScale;

// UI Scale and letterbox offsets
extern float g_uiScale;
extern float g_offsetX;
extern float g_offsetY;

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