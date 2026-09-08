#include "render_core.h"
#include "../game.h"
#include "./render_themes.h"

#include <cstdio>
#include <cmath>
#include <cstring>
#include <unordered_set>

// ============================================================
// GLOBALS DEFINITION
// ============================================================
Font g_fontVCR = {0};
Font g_fontIcons = {0};   // monochrome fallback font for emoji/pictographs
float g_fontScale = 1.5f;
float g_uiScale = 1.0f;
float g_offsetX = 0.0f;
float g_offsetY = 0.0f;

extern GameState g_game;

// ============================================================
// GLYPH-FALLBACK BOOKKEEPING
// ============================================================
// raylib's LoadFontEx only bakes atlas glyphs for the exact codepoints you
// hand it; anything else falls back to '?' at draw time (see GetGlyphIndex
// in raylib's source - it recurses to the '?' glyph on a miss). Beyond that,
// a coding font like JetBrains Mono simply has no outlines at all for most
// pictographic emoji (folder, key, money bag, ...) - registering the
// codepoint doesn't conjure a glyph the font file doesn't contain. And true
// *color* emoji fonts (Noto Color Emoji, Apple Color Emoji) can't be drawn
// through raylib/stb_truetype at all, since those are bitmap-strike formats,
// not vector outlines raylib can rasterize and tint.
//
// The fix: keep g_fontVCR for text + box-drawing/symbols it actually has,
// and load a second, separate *monochrome* icon font (e.g. "Noto Emoji" -
// the plain black-outline variant, not "Noto Color Emoji") purely for
// pictographs. We track which codepoints we actually requested for each
// font and pick per-character at draw time.
static std::unordered_set<int> g_vcrGlyphSet;
static std::unordered_set<int> g_iconGlyphSet;

static void RegisterGlyphSet(const int* codepoints, int count, std::unordered_set<int>& out) {
    for (int i = 0; i < count; i++) out.insert(codepoints[i]);
}

static Font& FontForCodepoint(int cp) {
    if (g_vcrGlyphSet.count(cp)) return g_fontVCR;
    if (g_fontIcons.texture.id != 0 && g_iconGlyphSet.count(cp)) return g_fontIcons;
    return g_fontVCR; // unknown codepoint -> raylib's own '?' fallback, same as before
}

// Draws one codepoint and returns the pixel advance to the next pen
// position, replicating raylib's own DrawTextEx advance formula so runs
// that jump between the two fonts still land on a consistent baseline.
static float DrawGlyphAdvanced(Font font, int cp, float x, float y, float scaledSize, Color tint) {
    DrawTextCodepoint(font, cp, {x, y}, scaledSize, tint);
    float scaleFactor = scaledSize / (float)font.baseSize;
    int index = GetGlyphIndex(font, cp);
    float rawAdvance = (font.glyphs[index].advanceX == 0)
        ? (float)(font.recs[index].width + font.glyphs[index].offsetX)
        : (float)font.glyphs[index].advanceX;
    return rawAdvance * scaleFactor;
}

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
    if (g_fontVCR.texture.id == 0) {
        DrawText(text, (int)SX(x), (int)SY(y), (int)(fontSize * g_uiScale), color);
        return;
    }

    float scaledSize = fontSize * g_uiScale * g_fontScale;
    float spacing = 1.0f;
    float penX = SX(x);
    float penY = SY(y);

    const char* p = text;
    while (*p) {
        int cpSize = 0;
        int cp = GetCodepoint(p, &cpSize);
        if (cpSize == 0) { p++; continue; } // malformed byte, skip forward safely

        Font& font = FontForCodepoint(cp);
        penX += DrawGlyphAdvanced(font, cp, penX, penY, scaledSize, color) + spacing;
        p += cpSize;
    }
}

float MeasureScaledTextWidth(const char* text, float fontSize) {
    if (g_fontVCR.texture.id == 0) {
        return (float)MeasureText(text, (int)fontSize);
    }

    float scaledSize = fontSize * g_uiScale * g_fontScale;
    float spacing = 1.0f;
    float width = 0.0f;

    const char* p = text;
    while (*p) {
        int cpSize = 0;
        int cp = GetCodepoint(p, &cpSize);
        if (cpSize == 0) { p++; continue; }

        Font& font = FontForCodepoint(cp);
        float scaleFactor = scaledSize / (float)font.baseSize;
        int index = GetGlyphIndex(font, cp);
        float rawAdvance = (font.glyphs[index].advanceX == 0)
            ? (float)(font.recs[index].width + font.glyphs[index].offsetX)
            : (float)font.glyphs[index].advanceX;
        width += rawAdvance * scaleFactor + spacing;
        p += cpSize;
    }
    return width / g_uiScale; // back to reference space, matching old behavior
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
    // ---- Primary text font: monospace, handles ASCII + box-drawing/symbols ----
    static const char* kTextGlyphSample =
        " !\"#$%&'()*+,-./0123456789:;<=>?@"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
        "abcdefghijklmnopqrstuvwxyz{|}~"
        "\xC2\xB0"                                  // °
        "\xE2\x97\x88\xE2\x96\xB6\xE2\x98\x85\xE2\x9C\x93\xE2\x9C\x97"   // ◈▶★✓✗
        "\xE2\x95\x91\xE2\x96\x93\xE2\x96\x92\xE2\x96\x91"               // ║▓▒░
        "\xE2\x9A\x99\xE2\x9A\xA1\xE2\x9A\xA0\xE2\x8F\xB1"               // ⚙⚡⚠⏱
        "\xE2\x97\x84\xE2\x96\xBA\xE2\x9F\xB3\xE2\x8C\x82"               // ◄►⟳⌂
        "\xE2\x95\x94\xE2\x95\x90\xE2\x95\x97\xE2\x96\x88\xE2\x95\x9D\xE2\x95\x9A"  // ╔═╗█╝╚
        "\xE2\x96\xBE\xE2\x96\xB8\xE2\x80\xA2";                          // ▾▸•

    int textCpCount = 0;
    int* textCodepoints = LoadCodepoints(kTextGlyphSample, &textCpCount);
    g_fontVCR = LoadFontEx("assets/fonts/JetBrainsMono-Bold.ttf", 32, textCodepoints, textCpCount);
    RegisterGlyphSet(textCodepoints, textCpCount, g_vcrGlyphSet);
    UnloadCodepoints(textCodepoints);

    if (g_fontVCR.texture.id == 0) {
        printf("Warning: Could not load JetBrainsMono-Bold font. Using default font.\n");
    } else {
        SetTextureFilter(g_fontVCR.texture, TEXTURE_FILTER_BILINEAR);
        printf("JetBrainsMono-Bold font loaded successfully.\n");
    }

    // ---- Fallback icon font: every pictographic "emoji" used anywhere in the UI ----
    // Needs a MONOCHROME vector emoji font on disk, e.g. Noto Emoji
    // (SIL OFL, github.com/googlefonts/noto-emoji) - NOT Noto Color Emoji,
    // which raylib cannot rasterize at all.
    // If you add a new emoji anywhere in the app later, it has to be added
    // to this string too, or it'll silently fall back to '?' again.
    static const char* kIconGlyphSample =
        "\xF0\x9F\x93\x81\xF0\x9F\x93\x82\xF0\x9F\x93\x84"               // 📁📂📄
        "\xF0\x9F\x91\xA4\xF0\x9F\x91\xA5"                               // 👤👥
        "\xF0\x9F\x92\xB0\xF0\x9F\x94\x92\xF0\x9F\x94\x93\xF0\x9F\x94\x91"  // 💰🔒🔓🔑
        "\xF0\x9F\x94\x8D\xE2\x9C\x95"                                   // 🔍✕
        "\xF0\x9F\x93\x8A\xF0\x9F\x92\xBE\xF0\x9F\x93\x85\xF0\x9F\x97\x82" // 📊💾📅🗂
        "\xF0\x9F\x8E\xAF\xF0\x9F\x93\xA1\xF0\x9F\x94\x97"               // 🎯📡🔗
        "\xF0\x9F\x8E\xA8\xF0\x9F\x94\x8A\xF0\x9F\x96\xA5\xF0\x9F\x93\xA6"; // 🎨🔊🖥📦

    int iconCpCount = 0;
    int* iconCodepoints = LoadCodepoints(kIconGlyphSample, &iconCpCount);
    g_fontIcons = LoadFontEx("assets/fonts/NotoEmoji-Regular.ttf", 32, iconCodepoints, iconCpCount);
    RegisterGlyphSet(iconCodepoints, iconCpCount, g_iconGlyphSet);
    UnloadCodepoints(iconCodepoints);

    if (g_fontIcons.texture.id == 0) {
        printf("Warning: Could not load icon font (assets/fonts/NotoEmoji-Regular.ttf missing?). Icons will show as '?'.\n");
    } else {
        SetTextureFilter(g_fontIcons.texture, TEXTURE_FILTER_BILINEAR);
        printf("Icon font loaded successfully.\n");
    }
    // Colors are initialized in render_themes.cpp
}

void UnloadAssets(void) {
    if (g_fontVCR.texture.id != 0) {
        UnloadFont(g_fontVCR);
    }
    if (g_fontIcons.texture.id != 0) {
        UnloadFont(g_fontIcons);
    }
}

void DrawGradientBackground(float x, float y, float w, float h, Color topColor, Color bottomColor) {
    for (int i = 0; i < (int)h; i += 2) {
        float t = (float)i / h;
        Color col = BlendColor(topColor, bottomColor, t);
        DrawScaledRect(x, y + i, w, 2, col);
    }
}