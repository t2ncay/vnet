#pragma once
#include "raylib.h"

// ============================================================
// SCALED PRIMITIVES
// ============================================================
float SX(float x);
float SY(float y);
void DrawScaledRect(float x, float y, float w, float h, Color color);
void DrawScaledRectLines(float x, float y, float w, float h, Color color);
void DrawScaledText(const char* text, float x, float y, float fontSize, Color color);
void DrawScaledLine(float x1, float y1, float x2, float y2, Color color);
void DrawScaledCircle(float centerX, float centerY, float radius, Color color);
void DrawScaledCircleLines(float centerX, float centerY, float radius, Color color);
float MeasureScaledTextWidth(const char* text, float fontSize);

// ============================================================
// MOUSE / HOVER HELPERS
// ============================================================
Vector2 GetRefMousePos(void);
bool RefRectHover(float x, float y, float w, float h, Vector2 refMouse);

// ============================================================
// ASSET MANAGEMENT
// ============================================================
void LoadAssets(void);
void UnloadAssets(void);

// ============================================================
// GLOBALS
// ============================================================
extern Font g_fontVCR;
extern float g_fontScale;
extern float g_uiScale;
extern float g_offsetX;
extern float g_offsetY;