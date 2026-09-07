#include "render_core.h"
#include "../game.h"
#include "./render_themes.h"

#include <cstdio>
#include <cmath>
#include <cstring>

// ============================================================
// GLOBALS DEFINITION
// ============================================================
Font g_fontVCR = {0};
float g_fontScale = 1.5f;
float g_uiScale = 1.0f;
float g_offsetX = 0.0f;
float g_offsetY = 0.0f;

extern GameState g_game;

// ============================================================
// SCALED PRIMITIVES
// ============================================================
float SX(float x) { return g_offsetX + x * g_uiScale; }
float SY(float y) { return g_offsetY + y * g_uiScale; }

void DrawScaledRect(float x, float y, float w, float h, Color color) {
    DrawRectangle((int)SX(x), (int)SY(y), (int)(w * g_uiScale), (int)(h * g_uiScale), color);
}

void DrawScaledRectLines(float x, float y, float w, float h, Color color) {
    DrawRectangleLines((int)SX(x), (int)SY(y), (int)(w * g_uiScale), (int)(h * g_uiScale), color);
}

void DrawScaledText(const char* text, float x, float y, float fontSize, Color color) {
    if (g_fontVCR.texture.id != 0) {
        DrawTextEx(g_fontVCR, text, {SX(x), SY(y)}, fontSize * g_uiScale * g_fontScale, 1.0f, color);
    } else {
        DrawText(text, (int)SX(x), (int)SY(y), (int)(fontSize * g_uiScale), color);
    }
}

float MeasureScaledTextWidth(const char* text, float fontSize) {
    if (g_fontVCR.texture.id != 0) {
        Vector2 sz = MeasureTextEx(g_fontVCR, text, fontSize * g_uiScale * g_fontScale, 1.0f);
        return sz.x / g_uiScale;
    }
    return (float)MeasureText(text, (int)fontSize);
}

void DrawScaledLine(float x1, float y1, float x2, float y2, Color color) {
    DrawLine((int)SX(x1), (int)SY(y1), (int)SX(x2), (int)SY(y2), color);
}

void DrawScaledCircle(float centerX, float centerY, float radius, Color color) {
    DrawCircle((int)(SX(centerX)), (int)(SY(centerY)), radius * g_uiScale, color);
}

void DrawScaledCircleLines(float centerX, float centerY, float radius, Color color) {
    DrawCircleLines((int)(SX(centerX)), (int)(SY(centerY)), radius * g_uiScale, color);
}

// ============================================================
// MOUSE HELPERS
// ============================================================
Vector2 GetRefMousePos(void) {
    Vector2 m = GetMousePosition();
    Vector2 ref;
    ref.x = (m.x - g_offsetX) / g_uiScale;
    ref.y = (m.y - g_offsetY) / g_uiScale;
    return ref;
}

bool RefRectHover(float x, float y, float w, float h, Vector2 refMouse) {
    return (refMouse.x >= x && refMouse.x <= x + w && 
            refMouse.y >= y && refMouse.y <= y + h);
}

// ============================================================
// ASSET MANAGEMENT
// ============================================================
void LoadAssets(void) {
    static const char* kExtraGlyphSample =
        " !\"#$%&'()*+,-./0123456789:;<=>?@"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
        "abcdefghijklmnopqrstuvwxyz{|}~"
        "°◈▶★✓✗║▓▒░⚙⚡⚠⏱◄►⟳⌂";

    int codepointCount = 0;
    int* codepoints = LoadCodepoints(kExtraGlyphSample, &codepointCount);

    g_fontVCR = LoadFontEx("assets/fonts/JetBrainsMono-Bold.ttf", 32, codepoints, codepointCount);

    UnloadCodepoints(codepoints);

    if (g_fontVCR.texture.id == 0) {
        printf("Warning: Could not load JetBrainsMono-Bold font. Using default font.\n");
    } else {
        SetTextureFilter(g_fontVCR.texture, TEXTURE_FILTER_BILINEAR);
        printf("JetBrainsMono-Bold font loaded successfully.\n");
    }
    // Colors are initialized in render_themes.cpp
}

void UnloadAssets(void) {
    if (g_fontVCR.texture.id != 0) {
        UnloadFont(g_fontVCR);
    }
}

void DrawGradientBackground(float x, float y, float w, float h, Color topColor, Color bottomColor) {
    for (int i = 0; i < (int)h; i += 2) {
        float t = (float)i / h;
        Color col = BlendColor(topColor, bottomColor, t);
        DrawScaledRect(x, y + i, w, 2, col);
    }
}