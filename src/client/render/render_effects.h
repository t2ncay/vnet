#pragma once
#include "raylib.h"

// ============================================================
// WINDOW FRAME HELPERS
// ============================================================
struct WindowFrameOpts {
    bool showTitleBar = true;
    bool showCornerReticles = true;
    bool showShadow = true;
    float cornerSize = 25.0f;
    Color borderColor = {0, 220, 240, 100};
    Color titleColor = COLOR_BLOOD;
};

void DrawWindowFrame(float x, float y, float w, float h, 
                     const char* title, 
                     const WindowFrameOpts& opts = WindowFrameOpts());

// ============================================================
// JITTER / SHAKE SYSTEM
// ============================================================
void InitJitter(void);
void TriggerJitter(float intensity, float duration = 0.0f);
void TriggerGlitch(float intensity);
void UpdateJitter(float dt);
float GetJitterX(void);
float GetJitterY(void);
bool IsJittering(void);
float GetJitterIntensity(void);

// ============================================================
// ANIMATION / GLOW HELPERS
// ============================================================
float PulseWave(float speed, float phase = 0.0f);
Color ColorPulse(Color base, float speed, float minAlpha = 0.35f, float maxAlpha = 1.0f);
void DrawGlowRect(float x, float y, float w, float h, Color color, float glowSize = 10.0f, int layers = 4);
void DrawScanlineOverlay(float x, float y, float w, float h, Color tint, float speed = 90.0f, float spacing = 5.0f);
void DrawDataStream(float x, float y, float w, float h, Color color, int columns = 12, float speed = 220.0f);