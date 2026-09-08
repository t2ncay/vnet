#include "render_effects.h"
#include "render_core.h"
#include "render_themes.h"
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <ctime>

// ============================================================
// JITTER STATE
// ============================================================
struct JitterState {
    float intensity;
    float timer;
    float decayRate;
    float noiseX[4];
    float noiseY[4];
    float noisePhase[4];
    float glitchIntensity;
    float glitchTimer;
    float glitchHold;
    float horizontalBias;
    float verticalBias;
    float crtWobble;
    float crtWobbleTimer;
};

static JitterState g_jitter = {0};

// ============================================================
// JITTER IMPLEMENTATION
// ============================================================
void InitJitter(void) {
    memset(&g_jitter, 0, sizeof(JitterState));
    g_jitter.decayRate = 1.5f;
    for (int i = 0; i < 4; i++) {
        g_jitter.noisePhase[i] = (float)(rand() % 1000) / 1000.0f * 6.28318f;
    }
}

void TriggerJitter(float intensity, float duration) {
    if (intensity < 0.0f) intensity = 0.0f;
    if (intensity > 1.0f) intensity = 1.0f;
    if (intensity <= g_jitter.intensity) return;
    
    g_jitter.intensity = intensity;
    g_jitter.timer = 0.0f;
    g_jitter.horizontalBias = (rand() % 100 - 50) / 100.0f;
    g_jitter.verticalBias = (rand() % 100 - 50) / 100.0f;
    
    if (intensity > 0.4f && (rand() % 100) < 40) {
        g_jitter.glitchIntensity = intensity * (0.5f + (rand() % 100) / 200.0f);
        g_jitter.glitchTimer = 0.0f;
        g_jitter.glitchHold = 0.05f + (rand() % 100) / 1000.0f;
    }
    
    if (duration > 0.0f) {
        g_jitter.decayRate = 1.0f / duration;
    } else {
        g_jitter.decayRate = 1.5f + intensity * 2.0f;
    }
}

void TriggerGlitch(float intensity) {
    g_jitter.glitchIntensity = intensity;
    g_jitter.glitchTimer = 0.0f;
    g_jitter.glitchHold = 0.03f + (rand() % 100) / 500.0f;
}

void UpdateJitter(float dt) {
    if (g_jitter.intensity > 0.0f) {
        g_jitter.intensity -= dt * g_jitter.decayRate;
        if (g_jitter.intensity < 0.0f) g_jitter.intensity = 0.0f;
        g_jitter.timer += dt;
    }
    
    for (int i = 0; i < 4; i++) {
        g_jitter.noisePhase[i] += dt * (0.5f + g_jitter.intensity * 3.0f);
    }
    
    float t = GetTime();
    for (int i = 0; i < 4; i++) {
        float freq = 2.0f + i * 3.0f + g_jitter.intensity * 10.0f;
        g_jitter.noiseX[i] = sinf(t * freq + g_jitter.noisePhase[i]);
        g_jitter.noiseY[i] = cosf(t * (freq * 0.7f + 1.3f) + g_jitter.noisePhase[i] * 0.6f);
    }
    
    if (g_jitter.glitchIntensity > 0.0f) {
        g_jitter.glitchTimer += dt;
        if (g_jitter.glitchTimer > g_jitter.glitchHold) {
            g_jitter.glitchIntensity -= dt * 8.0f;
            if (g_jitter.glitchIntensity < 0.0f) g_jitter.glitchIntensity = 0.0f;
        }
    }
    
    g_jitter.crtWobbleTimer += dt * 0.3f;
    g_jitter.crtWobble = sinf(g_jitter.crtWobbleTimer * 0.7f) * 0.15f + 
                         sinf(g_jitter.crtWobbleTimer * 1.3f + 2.0f) * 0.1f;
}

float GetJitterX(void) {
    float result = 0.0f;
    if (g_jitter.intensity > 0.01f) {
        float primary = g_jitter.noiseX[0] * 0.6f + 
                        g_jitter.noiseX[1] * 0.3f + 
                        g_jitter.noiseX[2] * 0.1f;
        float bias = 0.5f + g_jitter.horizontalBias * 0.5f;
        result += primary * g_jitter.intensity * 14.0f * bias;
    }
    if (g_jitter.glitchIntensity > 0.01f) {
        float glitch = sinf(g_jitter.glitchTimer * 120.0f) * 
                       (1.0f - g_jitter.glitchTimer / (g_jitter.glitchHold + 0.01f));
        result += glitch * g_jitter.glitchIntensity * 25.0f;
    }
    result += g_jitter.crtWobble * 0.3f;
    if (result > 30.0f) result = 30.0f;
    if (result < -30.0f) result = -30.0f;
    return result;
}

float GetJitterY(void) {
    float result = 0.0f;
    if (g_jitter.intensity > 0.01f) {
        float primary = g_jitter.noiseY[0] * 0.5f + 
                        g_jitter.noiseY[1] * 0.3f + 
                        g_jitter.noiseY[2] * 0.2f;
        float bias = 0.5f + g_jitter.verticalBias * 0.5f;
        result += primary * g_jitter.intensity * 12.0f * bias;
    }
    if (g_jitter.glitchIntensity > 0.01f) {
        float glitch = cosf(g_jitter.glitchTimer * 90.0f) * 
                       (1.0f - g_jitter.glitchTimer / (g_jitter.glitchHold + 0.01f));
        result += glitch * g_jitter.glitchIntensity * 18.0f;
    }
    result += g_jitter.crtWobble * 0.25f;
    if (result > 25.0f) result = 25.0f;
    if (result < -25.0f) result = -25.0f;
    return result;
}

bool IsJittering(void) {
    return g_jitter.intensity > 0.01f || g_jitter.glitchIntensity > 0.01f;
}

float GetJitterIntensity(void) {
    return g_jitter.intensity + g_jitter.glitchIntensity * 0.5f;
}

// ============================================================
// ANIMATION HELPERS
// ============================================================
float PulseWave(float speed, float phase) {
    return sinf((float)GetTime() * speed + phase) * 0.5f + 0.5f;
}

Color ColorPulse(Color base, float speed, float minAlpha, float maxAlpha) {
    float p = PulseWave(speed);
    float a = minAlpha + (maxAlpha - minAlpha) * p;
    return Fade(base, a);
}

void DrawGlowRect(float x, float y, float w, float h, Color color, float glowSize, int layers) {
    if (layers < 1) layers = 1;
    for (int i = layers; i >= 1; i--) {
        float t = (float)i / (float)layers;
        float pad = glowSize * t;
        float alpha = (1.0f - t) * (1.0f - t) * 0.55f;
        DrawScaledRectLines(x - pad, y - pad, w + pad * 2.0f, h + pad * 2.0f, Fade(color, alpha));
    }
    DrawScaledRectLines(x, y, w, h, color);
}

void DrawScanlineOverlay(float x, float y, float w, float h, Color tint, float speed, float spacing) {
    float t = (float)GetTime();
    float intensity = THEME_SCANLINE_INTENSITY;  // Use theme mood value
    for (float i = 0; i < h; i += spacing / intensity) {
        float scanY = y + i + fmodf(t * speed * intensity, spacing / intensity);
        DrawScaledRect(x, scanY, w, 1.0f, Fade(tint, intensity * 0.8f));
    }
}

void DrawDataStream(float x, float y, float w, float h, Color color, int columns, float speed) {
    if (columns < 1) columns = 1;
    float density = THEME_DATA_RAIN_DENSITY;  // Use theme mood value
    int actualColumns = (int)(columns * density);
    if (actualColumns < 1) actualColumns = 1;

    float t = (float)GetTime();
    float colW = w / (float)actualColumns;
    for (int i = 0; i < actualColumns; i++) {
        unsigned int seed = (unsigned int)(i * 7919u + 13u);
        float colSpeed = speed * (0.6f + (float)(seed % 100) / 130.0f);
        float offset = (float)(seed % 1000) / 1000.0f * h;
        float dropY = fmodf(t * colSpeed + offset, h + 40.0f) - 40.0f;

        float cx = x + i * colW + colW * 0.5f;
        int glyphCount = 5;
        for (int g = 0; g < glyphCount; g++) {
            float gy = y + dropY - g * 14.0f;
            if (gy < y || gy > y + h) continue;
            float fade = 1.0f - (float)g / (float)glyphCount;
            DrawScaledRect(cx - 2.0f, gy, 3.0f, 8.0f, Fade(color, fade * 0.8f));
        }
    }
}

// ============================================================
// WINDOW FRAME / GLOW TEXT HELPERS
// ============================================================

float DrawWindowFrame(float x, float y, float w, float h, const WindowFrameOpts& opts) {
    // ---- Background ----
    DrawScaledRect(x, y, w, h, COLOR_PANEL);
    DrawScaledRectLines(x, y, w, h, Fade(opts.accentColor, 0.4f));

    // ---- Title bar ----
    float titleH = 26.0f;
    DrawScaledRect(x, y, w, titleH, Fade(opts.accentColor, 0.15f));
    DrawScaledLine(x, y + titleH, x + w, y + titleH, Fade(opts.accentColor, 0.3f));

    // ---- Title text ----
    float titleFontSize = 11.0f;
    float tw = MeasureScaledTextWidth(opts.title, titleFontSize);
    DrawScaledText(opts.title, x + 12.0f, y + (titleH - titleFontSize) / 2.0f + 2.0f,
                   titleFontSize, opts.accentColor);

    // ---- Small corner accents ----
    Color cornerCol = Fade(opts.accentColor, 0.2f);
    float cSize = 6.0f;
    // Top-left
    DrawScaledLine(x + 4.0f, y + 4.0f, x + 4.0f + cSize, y + 4.0f, cornerCol);
    DrawScaledLine(x + 4.0f, y + 4.0f, x + 4.0f, y + 4.0f + cSize, cornerCol);
    // Top-right
    DrawScaledLine(x + w - 4.0f - cSize, y + 4.0f, x + w - 4.0f, y + 4.0f, cornerCol);
    DrawScaledLine(x + w - 4.0f, y + 4.0f, x + w - 4.0f, y + 4.0f + cSize, cornerCol);
    // Bottom-left
    DrawScaledLine(x + 4.0f, y + h - 4.0f - cSize, x + 4.0f, y + h - 4.0f, cornerCol);
    DrawScaledLine(x + 4.0f, y + h - 4.0f, x + 4.0f + cSize, y + h - 4.0f, cornerCol);
    // Bottom-right
    DrawScaledLine(x + w - 4.0f, y + h - 4.0f - cSize, x + w - 4.0f, y + h - 4.0f, cornerCol);
    DrawScaledLine(x + w - 4.0f - cSize, y + h - 4.0f, x + w - 4.0f, y + h - 4.0f, cornerCol);

    // ---- Return content start Y (below title bar + small padding) ----
    return y + titleH + 4.0f;
}

void DrawGlowText(const char* text, float x, float y, float fontSize, Color color, float intensity) {
    if (intensity <= 0.0f || text == nullptr || text[0] == '\0') {
        DrawScaledText(text, x, y, fontSize, color);
        return;
    }
    // Draw multiple layers with decreasing alpha
    int layers = 4;
    for (int i = layers; i >= 1; i--) {
        float alpha = (float)i / (float)layers * intensity * 0.4f;
        float offset = (float)(layers - i + 1) * 1.0f;
        DrawScaledText(text, x - offset, y - offset, fontSize, Fade(color, alpha));
        DrawScaledText(text, x + offset, y + offset, fontSize, Fade(color, alpha));
        DrawScaledText(text, x - offset, y + offset, fontSize, Fade(color, alpha));
        DrawScaledText(text, x + offset, y - offset, fontSize, Fade(color, alpha));
    }
    // Main text on top
    DrawScaledText(text, x, y, fontSize, color);
}

void DrawChromaticText(const char* text, float x, float y, float fontSize, Color baseColor, float offset) {
    if (text == nullptr || text[0] == '\0') return;

    // Red channel offset (positive X)
    Color red = baseColor;
    red.r = 255;
    red.g = 0;
    red.b = 0;
    DrawScaledText(text, x + offset, y, fontSize, Fade(red, 0.3f));

    // Blue channel offset (negative X)
    Color blue = baseColor;
    blue.r = 0;
    blue.g = 0;
    blue.b = 255;
    DrawScaledText(text, x - offset, y, fontSize, Fade(blue, 0.3f));

    // Green channel (main) – we use the base color's green component
    Color green = baseColor;
    green.r = 0;
    green.g = 255;
    green.b = 0;
    DrawScaledText(text, x, y, fontSize, Fade(green, 0.3f));

    // Finally draw the full base color text on top (slightly opaque)
    DrawScaledText(text, x, y, fontSize, Fade(baseColor, 0.9f));
}