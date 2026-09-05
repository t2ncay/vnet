#include "render.h"
#include "vnet.h"
#include "game.h"
#include "vnet_client.h"
#include "vnet_protocol.h"
#include "desktop.h"

#include <cstdio>
#include <cmath>
#include <cstdarg>
#include <cstring>
#include <string>
#include <vector>

// ============================================================
// ACTIVE THEME COLORS
// ============================================================
Color COLOR_BLACK   = {2, 2, 4, 255};
Color COLOR_PANEL   = {10, 12, 16, 255};
Color COLOR_CLI_BG  = {6, 8, 12, 245};
Color COLOR_URLBAR  = {16, 20, 26, 255};
Color COLOR_BORDER  = {40, 50, 60, 255};
Color COLOR_BLOOD   = {220, 20, 40, 255};
Color COLOR_CYAN    = {0, 220, 240, 255};
Color COLOR_AMBER   = {255, 150, 0, 255};
Color COLOR_TOXIC   = {40, 240, 100, 255};
Color COLOR_GHOST   = {160, 170, 185, 255};

Font g_fontVCR = {0};

float g_uiScale = 1.0f;
float g_offsetX = 0.0f;
float g_offsetY = 0.0f;

extern Player g_player;
extern char g_cliLogs[250][256];
extern int g_cliLogCount;
extern char g_feedLogs[100][256];
extern int g_feedLogCount;
extern GameState g_game;

// ============================================================
// ENHANCED JITTER/SHAKE SYSTEM (Weighted CRT Interference)
// ============================================================

// Jitter state structure
struct JitterState {
    // Primary shake
    float intensity;           // 0.0 - 1.0
    float timer;
    float decayRate;
    
    // Multi-dimensional noise
    float noiseX[4];
    float noiseY[4];
    float noisePhase[4];
    
    // Glitch components
    float glitchIntensity;     // 0.0 - 1.0 (spikes)
    float glitchTimer;
    float glitchHold;
    
    // Horizontal/vertical bias
    float horizontalBias;
    float verticalBias;
    
    // CRT wobble
    float crtWobble;
    float crtWobbleTimer;
};

static JitterState g_jitter = {0};

void InitJitter(void) {
    memset(&g_jitter, 0, sizeof(JitterState));
    g_jitter.decayRate = 1.5f;
    
    // Initialize noise phases
    for (int i = 0; i < 4; i++) {
        g_jitter.noisePhase[i] = (float)(rand() % 1000) / 1000.0f * 6.28318f;
    }
}

void TriggerJitter(float intensity, float duration) {
    if (intensity < 0.0f) intensity = 0.0f;
    if (intensity > 1.0f) intensity = 1.0f;
    
    // If existing jitter is stronger, keep it
    if (intensity <= g_jitter.intensity) return;
    
    g_jitter.intensity = intensity;
    g_jitter.timer = 0.0f;
    
    // Random bias for this jitter event
    g_jitter.horizontalBias = (rand() % 100 - 50) / 100.0f;
    g_jitter.verticalBias = (rand() % 100 - 50) / 100.0f;
    
    // Sometimes trigger glitch spikes
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
    // Decay primary intensity
    if (g_jitter.intensity > 0.0f) {
        g_jitter.intensity -= dt * g_jitter.decayRate;
        if (g_jitter.intensity < 0.0f) g_jitter.intensity = 0.0f;
        g_jitter.timer += dt;
    }
    
    // Update noise phases
    for (int i = 0; i < 4; i++) {
        g_jitter.noisePhase[i] += dt * (0.5f + g_jitter.intensity * 3.0f);
    }
    
    // Update noise values using multiple frequencies
    float t = GetTime();
    for (int i = 0; i < 4; i++) {
        float freq = 2.0f + i * 3.0f + g_jitter.intensity * 10.0f;
        g_jitter.noiseX[i] = sinf(t * freq + g_jitter.noisePhase[i]);
        g_jitter.noiseY[i] = cosf(t * (freq * 0.7f + 1.3f) + g_jitter.noisePhase[i] * 0.6f);
    }
    
    // Update glitch
    if (g_jitter.glitchIntensity > 0.0f) {
        g_jitter.glitchTimer += dt;
        if (g_jitter.glitchTimer > g_jitter.glitchHold) {
            g_jitter.glitchIntensity -= dt * 8.0f;
            if (g_jitter.glitchIntensity < 0.0f) g_jitter.glitchIntensity = 0.0f;
        }
    }
    
    // CRT wobble (always present at low intensity)
    g_jitter.crtWobbleTimer += dt * 0.3f;
    g_jitter.crtWobble = sinf(g_jitter.crtWobbleTimer * 0.7f) * 0.15f + 
                         sinf(g_jitter.crtWobbleTimer * 1.3f + 2.0f) * 0.1f;
}

float GetJitterX(void) {
    float result = 0.0f;
    
    // 1. Primary shake (multi-frequency)
    if (g_jitter.intensity > 0.01f) {
        float primary = g_jitter.noiseX[0] * 0.6f + 
                        g_jitter.noiseX[1] * 0.3f + 
                        g_jitter.noiseX[2] * 0.1f;
        
        // Biased based on horizontal bias
        float bias = 0.5f + g_jitter.horizontalBias * 0.5f;
        result += primary * g_jitter.intensity * 14.0f * bias;
    }
    
    // 2. Glitch spikes (jagged, sudden movements)
    if (g_jitter.glitchIntensity > 0.01f) {
        float glitch = sinf(g_jitter.glitchTimer * 120.0f) * 
                       (1.0f - g_jitter.glitchTimer / (g_jitter.glitchHold + 0.01f));
        result += glitch * g_jitter.glitchIntensity * 25.0f;
    }
    
    // 3. CRT wobble (subtle, always present)
    result += g_jitter.crtWobble * 0.3f;
    
    // Clamp
    if (result > 30.0f) result = 30.0f;
    if (result < -30.0f) result = -30.0f;
    
    return result;
}

float GetJitterY(void) {
    float result = 0.0f;
    
    // 1. Primary shake (multi-frequency)
    if (g_jitter.intensity > 0.01f) {
        float primary = g_jitter.noiseY[0] * 0.5f + 
                        g_jitter.noiseY[1] * 0.3f + 
                        g_jitter.noiseY[2] * 0.2f;
        
        // Biased based on vertical bias
        float bias = 0.5f + g_jitter.verticalBias * 0.5f;
        result += primary * g_jitter.intensity * 12.0f * bias;
    }
    
    // 2. Glitch spikes (sometimes vertical only)
    if (g_jitter.glitchIntensity > 0.01f) {
        float glitch = cosf(g_jitter.glitchTimer * 90.0f) * 
                       (1.0f - g_jitter.glitchTimer / (g_jitter.glitchHold + 0.01f));
        result += glitch * g_jitter.glitchIntensity * 18.0f;
    }
    
    // 3. CRT wobble
    result += g_jitter.crtWobble * 0.25f;
    
    // Clamp
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
// THEME TABLE (mirrors get_theme()/set_active_theme() in the Vyne script)
// ============================================================
struct ThemeDef {
    const char* name;
    Color bg, panel, cliBg, urlbar, border, blood, cyan, amber, toxic, ghost;
};

static const ThemeDef kThemes[] = {
    {"classic",   {2,2,4,255},     {10,12,16,255},  {6,8,12,245},    {16,20,26,255},  {40,50,60,255},   {220,20,40,255},  {0,220,240,255},  {255,150,0,255},  {40,240,100,255}, {160,170,185,255}},
    {"tokyo",     {16,18,28,255},  {22,25,38,230},  {14,16,26,245},  {30,35,54,255},  {52,60,92,255},   {247,118,142,255},{122,162,247,255},{187,154,247,255},{115,218,202,255},{192,202,245,255}},
    {"redroom",   {10,3,5,255},    {22,8,12,235},   {15,4,7,245},    {38,12,18,255},  {85,22,30,255},   {255,20,50,255},  {240,45,65,255},  {255,140,0,255},  {0,230,180,255},  {235,210,215,255}},
    {"amber",     {12,8,2,255},    {24,16,5,230},   {16,10,3,245},   {40,26,8,255},   {90,60,15,255},   {240,60,30,255},  {255,175,0,255},  {220,120,0,255},  {180,240,80,255}, {255,210,130,255}},
    {"cyberpunk", {18,10,26,255},  {30,16,42,230},  {22,12,32,245},  {45,20,60,255},  {100,30,120,255}, {255,0,85,255},   {0,230,255,255},  {255,215,0,255},  {50,255,120,255}, {220,200,240,255}},
    {"nord",      {15,20,28,255},  {24,30,42,230},  {18,24,34,245},  {35,45,60,255},  {60,75,100,255},  {191,97,106,255}, {136,192,208,255},{208,135,112,255},{163,190,140,255},{216,222,233,255}},
    {"dracula",   {18,16,26,255},  {28,25,40,230},  {20,18,30,245},  {40,35,58,255},  {70,60,95,255},   {255,85,85,255},  {139,233,253,255},{255,184,108,255},{80,250,123,255}, {248,248,242,255}},
    {"synthwave", {12,6,24,255},   {26,12,48,230},  {16,8,32,245},   {42,18,75,255},  {90,35,140,255},  {255,30,130,255}, {0,240,255,255},  {255,160,0,255},  {120,255,180,255},{235,210,255,255}},
    {"cobalt",    {2,12,28,255},   {8,22,48,230},   {4,16,36,245},   {14,34,70,255},  {30,65,120,255},  {255,60,90,255},  {0,190,255,255},  {255,180,40,255}, {40,240,180,255},{180,215,245,255}},
    {"monokai",   {20,20,20,255},  {32,32,32,230},  {24,24,24,245},  {45,45,45,255},  {80,80,80,255},   {255,97,136,255}, {120,220,232,255},{252,152,103,255},{166,226,46,255}, {248,248,242,255}},
    {"gruvbox",   {20,20,18,255},  {32,30,26,230},  {24,22,18,245},  {48,44,38,255},  {85,78,66,255},   {251,73,52,255},  {131,165,152,255},{254,128,25,255}, {184,187,38,255},{235,219,178,255}},
    {"abyss",     {2,6,12,255},    {6,14,26,230},   {4,10,20,245},   {10,24,42,255},  {20,50,80,255},   {230,40,70,255},  {0,180,200,255},  {0,140,180,255},  {0,240,160,255}, {140,180,200,255}},
    {"solaris",   {18,6,2,255},    {32,12,4,230},   {22,8,3,245},    {52,20,8,255},   {100,40,15,255},  {255,40,20,255},  {255,140,0,255},  {255,200,0,255},  {220,240,50,255},{255,225,180,255}},
    {"ghost",     {12,14,18,255},  {20,24,30,230},  {15,18,24,245},  {32,38,48,255},  {65,75,90,255},   {240,80,100,255}, {160,210,245,255},{200,190,220,255},{140,230,210,255},{220,230,240,255}},
    {"matrix",    {4,10,6,255},    {8,22,12,230},   {5,15,8,245},    {14,38,20,255},  {25,80,40,255},   {240,40,70,255},  {0,210,255,255},  {40,240,100,255}, {50,255,120,255},{200,255,215,255}},
};
static const int kThemeCount = sizeof(kThemes) / sizeof(kThemes[0]);

void SetActiveTheme(const char* name) {
    const ThemeDef* t = &kThemes[0]; // default: classic
    for (int i = 0; i < kThemeCount; i++) {
        if (strcmp(kThemes[i].name, name) == 0) { t = &kThemes[i]; break; }
    }
    COLOR_BLACK  = t->bg;
    COLOR_PANEL  = t->panel;
    COLOR_CLI_BG = t->cliBg;
    COLOR_URLBAR = t->urlbar;
    COLOR_BORDER = t->border;
    COLOR_BLOOD  = t->blood;
    COLOR_CYAN   = t->cyan;
    COLOR_AMBER  = t->amber;
    COLOR_TOXIC  = t->toxic;
    COLOR_GHOST  = t->ghost;
}

// ============================================================
// SCALED PRIMITIVES (reference-space -> real screen space)
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
        DrawTextEx(g_fontVCR, text, {SX(x), SY(y)}, fontSize * g_uiScale, 1.0f, color);
    } else {
        DrawText(text, (int)SX(x), (int)SY(y), (int)(fontSize * g_uiScale), color);
    }
}

void DrawScaledLine(float x1, float y1, float x2, float y2, Color color) {
    DrawLine((int)SX(x1), (int)SY(y1), (int)SX(x2), (int)SY(y2), color);
}

float MeasureScaledTextWidth(const char* text, float fontSize) {
    if (g_fontVCR.texture.id != 0) {
        Vector2 sz = MeasureTextEx(g_fontVCR, text, fontSize * g_uiScale, 1.0f);
        return sz.x / g_uiScale;
    }
    return (float)MeasureText(text, (int)fontSize) ; // fallback approx, unscaled font
}

Vector2 GetRefMousePos(void) {
    Vector2 m = GetMousePosition();
    Vector2 ref;
    ref.x = (m.x - g_offsetX) / g_uiScale;
    ref.y = (m.y - g_offsetY) / g_uiScale;
    return ref;
}

bool RefRectHover(float x, float y, float w, float h, Vector2 refMouse) {
    return (refMouse.x >= x && refMouse.x <= x + w && refMouse.y >= y && refMouse.y <= y + h);
}

// ============================================================
// ASSET MANAGEMENT
// ============================================================

void InitColors(void) {
    SetActiveTheme("classic");
}

void LoadAssets(void) {
    g_fontVCR = LoadFont("assets/VCR_OSD_MONO_1.001.ttf");
    if (g_fontVCR.texture.id == 0) {
        printf("Warning: Could not load VCR_OSD_MONO font. Using default font.\n");
    } else {
        SetTextureFilter(g_fontVCR.texture, TEXTURE_FILTER_BILINEAR);
        printf("VCR_OSD_MONO font loaded successfully.\n");
    }
    InitColors();
}

void UnloadAssets(void) {
    if (g_fontVCR.texture.id != 0) {
        UnloadFont(g_fontVCR);
    }
}

// ============================================================
// MARKUP TAG PARSING HELPERS
// ============================================================

// Returns true and fills `body` with the bracketed payload if `line`
// starts with "[TAG:" and has a matching "]". e.g. for prefix "[LINK:"
// and line "[LINK:market.vnet] >> text", body = "market.vnet",
// rest = " >> text".
static bool ParseTaggedField(const std::string& line, const char* prefix,
                              std::string& body, std::string& rest) {
    size_t plen = strlen(prefix);
    if (line.size() < plen || line.compare(0, plen, prefix) != 0) return false;
    size_t close = line.find(']', plen);
    if (close == std::string::npos) return false;
    body = line.substr(plen, close - plen);
    rest = (close + 1 < line.size()) ? line.substr(close + 1) : "";

    // trimming white spaces
    while (!rest.empty() && (rest[0] == ' ' || rest[0] == '\t')) {
        rest.erase(0, 1);
    }
    return true;
}

bool StartsWith(const std::string& s, const char* prefix) {
    size_t plen = strlen(prefix);
    return s.size() >= plen && s.compare(0, plen, prefix) == 0;
}

// Splits "a:b" (first colon only) into a/b. Used for [GAUGE:pct:label]
// and [BADGE:text:color] payloads.
static void SplitOnce(const std::string& s, char delim, std::string& a, std::string& b) {
    size_t idx = s.find(delim);
    if (idx == std::string::npos) { a = s; b = ""; return; }
    a = s.substr(0, idx);
    b = s.substr(idx + 1);
}

// ============================================================
// MARKUP PAGE RENDERER
// ============================================================

void DrawMarkupPage(float contentX, float contentY, float contentW, float contentH) {
    Vector2 refMouse = GetRefMousePos();
    bool mouseInPanel = RefRectHover(contentX, contentY, contentW, contentH, refMouse);
    bool clicked = mouseInPanel && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)
                   && !g_player.cliOpen && !g_player.isConnecting;

    BeginScissorMode((int)SX(contentX), (int)SY(contentY), (int)(contentW * g_uiScale), (int)(contentH * g_uiScale));

    float y = contentY + 20.0f - g_player.pageScroll;
    const float lineH = 26.0f;
    const float leftX = contentX + 20.0f;
    const float textW = contentW - 40.0f;

    for (const std::string& raw : g_pageLines) {
        if (y < contentY - lineH || y > contentY + contentH + lineH) {
            y += lineH;
            continue;
        }

        // ============================================================
        // EXISTING TAGS (Keep these)
        // ============================================================
        if (StartsWith(raw, "[TITLE]")) {
            DrawScaledText(raw.c_str() + 8, leftX, y, 20, COLOR_BLOOD);
            y += lineH + 6.0f;
        }
        else if (StartsWith(raw, "[SUBTITLE]")) {
            DrawScaledText(raw.c_str() + 11, leftX, y, 15, COLOR_AMBER);
            y += lineH;
        }
        else if (StartsWith(raw, "[WARN]")) {
            DrawScaledText(raw.c_str() + 7, leftX, y, 13, COLOR_BLOOD);
            y += lineH;
        }
        else if (StartsWith(raw, "[BLOOD]")) {
            DrawScaledText(raw.c_str() + 8, leftX, y, 13, COLOR_BLOOD);
            y += lineH;
        }
        else if (StartsWith(raw, "[PULSE]")) {
            float pulse = sinf((float)GetTime() * 5.0f) * 0.5f + 0.5f;
            Color c = (pulse > 0.5f) ? COLOR_AMBER : COLOR_BLOOD;
            DrawScaledText(raw.c_str() + 8, leftX, y, 13, c);
            y += lineH;
        }
        else if (StartsWith(raw, "[GLITCH]")) {
            float t = (float)GetTime();
            float jx = sinf(t * 48.0f) * 1.5f;
            float jy = cosf(t * 32.0f) * 0.8f;
            const char* txt = raw.c_str() + 9;
            DrawScaledText(txt, leftX + jx + 1.2f, y + jy - 0.5f, 13, Fade(COLOR_BLOOD, 0.75f));
            DrawScaledText(txt, leftX + jx - 1.2f, y + jy + 0.5f, 13, Fade(COLOR_CYAN, 0.75f));
            DrawScaledText(txt, leftX + jx, y + jy, 13, COLOR_GHOST);
            y += lineH;
        }
        else if (StartsWith(raw, "[CODE]")) {
            DrawScaledText(raw.c_str() + 7, leftX, y, 13, COLOR_TOXIC);
            y += lineH;
        }
        else if (StartsWith(raw, "[BOX]")) {
            DrawScaledText(raw.c_str() + 6, leftX, y, 13, COLOR_CYAN);
            y += lineH;
        }
        else if (StartsWith(raw, "[TEXT]")) {
            const char* txt = (raw.size() > 7) ? raw.c_str() + 7 : "";
            DrawScaledText(txt, leftX, y, 13, COLOR_GHOST);
            y += lineH;
        }
        else if (raw == "[HR]") {
            DrawScaledLine(leftX, y + 10.0f, leftX + textW, y + 10.0f, COLOR_BORDER);
            y += 18.0f;
        }
        else if (StartsWith(raw, "[GAUGE:")) {
            std::string body, rest;
            ParseTaggedField(raw, "[GAUGE:", body, rest);
            std::string pctStr, label;
            SplitOnce(body, ':', pctStr, label);
            float pct = 0.0f;
            try { pct = std::stof(pctStr); } catch (...) { pct = 0.0f; }
            if (pct < 0.0f) pct = 0.0f;
            if (pct > 100.0f) pct = 100.0f;

            float gw = 340.0f, gh = 18.0f;
            DrawScaledRect(leftX, y, gw, gh, COLOR_BLACK);
            float fillW = (pct / 100.0f) * gw;
            if (fillW < 2.0f) fillW = 2.0f;
            Color barCol = (pct > 70.0f) ? COLOR_BLOOD : COLOR_TOXIC;
            DrawScaledRect(leftX, y, fillW, gh, barCol);
            DrawScaledRectLines(leftX, y, gw, gh, COLOR_BORDER);

            char labelBuf[160];
            snprintf(labelBuf, sizeof(labelBuf), "%s [%d%%]", label.c_str(), (int)pct);
            DrawScaledText(labelBuf, leftX + gw + 15.0f, y + 2.0f, 12, COLOR_AMBER);
            y += lineH + 4.0f;
        }
        else if (StartsWith(raw, "[BADGE:")) {
            std::string body, rest;
            ParseTaggedField(raw, "[BADGE:", body, rest);
            std::string text, colorName;
            SplitOnce(body, ':', text, colorName);
            Color bg = COLOR_AMBER;
            if (colorName == "BLOOD") bg = COLOR_BLOOD;
            else if (colorName == "TOXIC") bg = COLOR_TOXIC;
            else if (colorName == "CYAN") bg = COLOR_CYAN;

            float badgeW = MeasureScaledTextWidth(text.c_str(), 12) + 16.0f;
            DrawScaledRect(leftX, y, badgeW, 22.0f, bg);
            DrawScaledText(text.c_str(), leftX + 8.0f, y + 4.0f, 12, COLOR_BLACK);
            y += lineH + 2.0f;
        }
        else if (StartsWith(raw, "[LINK:")) {
            std::string url, rest;
            ParseTaggedField(raw, "[LINK:", url, rest);
            
            // Trim leading spaces from rest
            while (!rest.empty() && rest[0] == ' ') {
                rest.erase(0, 1);
            }
            
            // Also trim the >> if present (or any other prefix)
            if (rest.size() >= 2 && rest[0] == '>' && rest[1] == '>') {
                rest.erase(0, 2);
                while (!rest.empty() && rest[0] == ' ') {
                    rest.erase(0, 1);
                }
            }
            
            // If rest is empty, use the URL as the label
            if (rest.empty()) {
                rest = url;
            }
            
            float labelW = MeasureScaledTextWidth(rest.c_str(), 14);
            bool hover = RefRectHover(leftX, y, labelW + 20.0f, 24.0f, refMouse) && mouseInPanel;
            Color linkCol = hover ? COLOR_TOXIC : COLOR_CYAN;
            DrawScaledText(rest.c_str(), leftX, y, 14, linkCol);
            if (hover && clicked) {
                TriggerRouteNavigation(url.c_str());
            }
            y += lineH;
        }

        // ============================================================
        // NEW TAGS - PORTED FROM VYNE SOURCE
        // ============================================================

        // [COMMENT] - Hidden debug/developer comments (skip rendering)
        else if (StartsWith(raw, "[COMMENT]")) {
            // Do nothing - comments are hidden
            y += lineH;
        }

        // [ART:key] - ASCII Art
        else if (StartsWith(raw, "[ART:")) {
            std::string key = raw.substr(5, raw.find(']') - 5);
            float pulse = sinf((float)GetTime() * 5.0f) * 0.5f + 0.5f;
            Color artCol = (pulse > 0.5f) ? COLOR_BLOOD : COLOR_AMBER;
            
            std::string artLabel = "[ASCII ART: " + key + "]";
            DrawScaledText(artLabel.c_str(), leftX, y, 11, artCol);
            y += lineH * 2;
        }

        // [IMG:key] - Image
        else if (StartsWith(raw, "[IMG:")) {
            std::string key = raw.substr(5, raw.find(']') - 5);
            // For now, draw placeholder
            DrawScaledRect(leftX, y, 200, 120, COLOR_CLI_BG);
            DrawScaledRectLines(leftX, y, 200, 120, COLOR_BORDER);
            DrawScaledText(("[IMG:" + key + "]").c_str(), leftX + 10, y + 10, 11, COLOR_AMBER);
            y += 130.0f;
        }

        // [VIDEO:key:title] - Video Player
        else if (StartsWith(raw, "[VIDEO:")) {
            std::string body, rest;
            ParseTaggedField(raw, "[VIDEO:", body, rest);
            std::string vidKey, vidTitle;
            SplitOnce(body, ':', vidKey, vidTitle);
            
            float vw = 460.0f, vh = 250.0f;
            DrawScaledRect(leftX, y, vw, vh, COLOR_BLACK);
            DrawScaledRectLines(leftX, y, vw, vh, COLOR_BLOOD);
            
            // Header bar
            DrawScaledRect(leftX, y, vw, 22.0f, COLOR_PANEL);
            char vidHeader[128];
            snprintf(vidHeader, sizeof(vidHeader), "MEDIA_PLAYER // %s", vidTitle.c_str());
            DrawScaledText(vidHeader, leftX + 10, y + 5, 10, COLOR_AMBER);
            
            // Video area
            DrawScaledRect(leftX + 8, y + 28, vw - 16, 175, COLOR_CLI_BG);
            DrawScaledRectLines(leftX + 8, y + 28, vw - 16, 175, COLOR_BORDER);
            DrawScaledText("480p IR | 18.0 Hz", leftX + vw - 90, y + 35, 9, COLOR_TOXIC);
            
            // Scanline effect
            float scanY = y + 28 + fmodf((float)GetTime() * 90.0f, 175.0f);
            DrawScaledLine(leftX + 8, scanY, leftX + vw - 8, scanY, {255, 40, 40, 160});
            
            // Controls
            float ctrlY = y + 212;
            DrawScaledText("[> PLAY]", leftX + 10, ctrlY + 10, 9, COLOR_TOXIC);
            
            // Progress bar
            DrawScaledRect(leftX + 215, ctrlY + 12, 215, 6, COLOR_BLACK);
            DrawScaledRectLines(leftX + 215, ctrlY + 12, 215, 6, COLOR_BORDER);
            float progress = fmodf((float)GetTime() * 0.5f, 1.0f);
            DrawScaledRect(leftX + 215, ctrlY + 12, 215 * progress, 6, COLOR_BLOOD);
            
            y += vh + 10.0f;
        }

        // [HEX_STREAM:addr:length:scramble_rate]
        else if (StartsWith(raw, "[HEX_STREAM:")) {
            std::string body, rest;
            ParseTaggedField(raw, "[HEX_STREAM:", body, rest);
            
            // Parse params: addr, length, scramble_rate
            std::string addr, lenStr, rateStr;
            size_t pos1 = body.find(':');
            if (pos1 != std::string::npos) {
                addr = body.substr(0, pos1);
                size_t pos2 = body.find(':', pos1 + 1);
                if (pos2 != std::string::npos) {
                    lenStr = body.substr(pos1 + 1, pos2 - pos1 - 1);
                    rateStr = body.substr(pos2 + 1);
                }
            }
            
            int byteLen = 16;
            float scrambleRate = 12.0f;
            try { byteLen = std::stoi(lenStr); } catch (...) { byteLen = 16; }
            try { scrambleRate = std::stof(rateStr); } catch (...) { scrambleRate = 12.0f; }
            if (byteLen > 32) byteLen = 32;
            if (byteLen < 1) byteLen = 16;
            
            float hsX = leftX;
            float hsY = y;
            float hsW = 530.0f;
            
            int rows = (byteLen + 15) / 16;
            float hsH = 32.0f + rows * 22.0f;
            
            // Frame
            DrawScaledRect(hsX, hsY, hsW, hsH, COLOR_BLACK);
            DrawScaledRectLines(hsX, hsY, hsW, hsH, COLOR_BORDER);
            
            // Header
            DrawScaledRect(hsX, hsY, hsW, 20.0f, COLOR_PANEL);
            char hexHeader[128];
            snprintf(hexHeader, sizeof(hexHeader), "MEM_STREAM // %s [LIVE]", addr.c_str());
            DrawScaledText(hexHeader, hsX + 10, hsY + 4, 10, COLOR_AMBER);
            
            // Hex data
            float t = (float)GetTime();
            const char* hexChars = "0123456789ABCDEF";
            
            for (int r = 0; r < rows; r++) {
                float rowY = hsY + 26.0f + r * 22.0f;
                char rowAddr[32];
                snprintf(rowAddr, sizeof(rowAddr), "0x%04X:", r * 16);
                DrawScaledText(rowAddr, hsX + 10, rowY, 10, COLOR_CYAN);
                
                std::string hexLine;
                int bytesInRow = (r + 1) * 16 <= byteLen ? 16 : byteLen - r * 16;
                for (int b = 0; b < bytesInRow; b++) {
                    int byteVal = (int)(fmodf(t * scrambleRate * 15.0f + b * 17.0f, 255.0f));
                    if (byteVal < 0) byteVal = 0;
                    if (byteVal > 255) byteVal = 255;
                    hexLine += hexChars[byteVal / 16];
                    hexLine += hexChars[byteVal % 16];
                    hexLine += ' ';
                }
                DrawScaledText(hexLine.c_str(), hsX + 110, rowY, 10, COLOR_GHOST);
            }
            
            y += hsH + 10.0f;
        }

        // [SCANNER:var_id:alias] - Retinal Scanner UI
        else if (StartsWith(raw, "[SCANNER:")) {
            std::string body, rest;
            ParseTaggedField(raw, "[SCANNER:", body, rest);
            std::string varId, reqAlias;
            SplitOnce(body, ':', varId, reqAlias);
            
            float sx = leftX, sy = y;
            float sw = 420.0f, sh = 140.0f;
            
            DrawScaledRect(sx, sy, sw, sh, COLOR_BLACK);
            DrawScaledRectLines(sx, sy, sw, sh, COLOR_CYAN);
            
            // Ocular reticle
            float retX = sx + 20, retY = sy + 20;
            float retSz = 100.0f;
            DrawScaledRect(retX, retY, retSz, retSz, COLOR_PANEL);
            DrawScaledRectLines(retX, retY, retSz, retSz, COLOR_CYAN);
            
            // Center crosshair
            float cx = retX + retSz / 2, cy = retY + retSz / 2;
            float t = (float)GetTime();
            for (int i = 0; i < 4; i++) {
                float angle = t * 3.0f + i * 1.5708f;
                float rx = cx + cosf(angle) * 35.0f;
                float ry = cy + sinf(angle) * 35.0f;
                DrawScaledRect(rx, ry, 3, 3, COLOR_AMBER);
            }
            
            // Info panel
            float infoX = sx + 135;
            DrawScaledText("BIOMETRIC RETINAL SCANNER", infoX, sy + 18, 11, COLOR_CYAN);
            char reqText[128];
            snprintf(reqText, sizeof(reqText), "REQUIRED: ALIAS [%s]", reqAlias.c_str());
            DrawScaledText(reqText, infoX, sy + 38, 9, COLOR_AMBER);
            DrawScaledLine(infoX, sy + 52, sx + sw - 15, sy + 52, COLOR_BORDER);
            
            // Status
            float pulse = sinf(t * 2.0f) * 0.5f + 0.5f;
            DrawScaledText("STATUS: IDLE // AWAITING CONTACT", infoX, sy + 62, 10, COLOR_GHOST);
            DrawScaledText(">>> CLICK TO INITIATE SCAN <<<", infoX, sy + 80, 9, 
                          (pulse > 0.5f) ? COLOR_TOXIC : COLOR_AMBER);
            DrawScaledText("AWAITING OPTICAL RETINAL LOCK...", infoX, sy + 108, 9, COLOR_CYAN);
            
            // Interactive click detection
            bool hover = RefRectHover(sx, sy, sw, sh, refMouse) && mouseInPanel;
            if (hover && clicked) {
                PushCliLog("[SCANNER]: Engaged optical retinal scanner");
            }
            
            y += sh + 10.0f;
        }

        // [INPUT:id:placeholder] - Interactive Input Box
        else if (StartsWith(raw, "[INPUT:")) {
            std::string body, rest;
            ParseTaggedField(raw, "[INPUT:", body, rest);
            std::string inpId, placeholder;
            SplitOnce(body, ':', inpId, placeholder);
            
            float ix = leftX, iy = y;
            float iw = 375.0f, ih = 28.0f;
            
            DrawScaledRect(ix, iy, iw, ih, COLOR_BLACK);
            DrawScaledRectLines(ix, iy, iw, ih, COLOR_BORDER);
            
            char inputText[256];
            snprintf(inputText, sizeof(inputText), "> %s", placeholder.c_str());
            DrawScaledText(inputText, ix + 10, iy + 7, 10, COLOR_GHOST);
            
            // Interactive click
            bool hover = RefRectHover(ix, iy, iw, ih, refMouse) && mouseInPanel;
            if (hover && clicked) {
                PushCliLog("[INPUT]: Focused on %s", inpId.c_str());
            }
            
            y += ih + 10.0f;
        }

        // [BTN:action_id:label] - Interactive Button
        else if (StartsWith(raw, "[BTN:")) {
            std::string body, rest;
            ParseTaggedField(raw, "[BTN:", body, rest);
            std::string actionId, label;
            SplitOnce(body, ':', actionId, label);
            
            float bx = leftX, by = y;
            float labelW = MeasureScaledTextWidth(label.c_str(), 11);
            float bw = labelW + 30.0f, bh = 26.0f;
            
            bool hover = RefRectHover(bx, by, bw, bh, refMouse) && mouseInPanel;
            Color bgCol = hover ? COLOR_BLOOD : COLOR_PANEL;
            
            DrawScaledRect(bx, by, bw, bh, bgCol);
            DrawScaledRectLines(bx, by, bw, bh, COLOR_BLOOD);
            DrawScaledText(label.c_str(), bx + 15, by + 6, 11, hover ? COLOR_BLACK : COLOR_TOXIC);
            
            if (hover && clicked) {
                PushCliLog("[BTN]: Clicked '%s' (action: %s)", label.c_str(), actionId.c_str());
                // Handle special actions
                if (actionId == "buy_ice") {
                    // Purchase ICE shield
                }
                // Add more action handlers
            }
            
            y += bh + 10.0f;
        }

        // [COMMENT] - Already handled above

        // Unknown/Unsupported tags
        else {
            // Skip silently or log for debugging
            y += lineH;
        }
    }

    EndScissorMode();

    // Clamp scroll
    float contentTotal = y - (contentY + 20.0f - g_player.pageScroll) + g_player.pageScroll;
    float maxScroll = contentTotal - contentH + 20.0f;
    if (maxScroll < 0.0f) maxScroll = 0.0f;
    if (g_player.pageScroll > maxScroll) g_player.pageScroll = maxScroll;
}

// ============================================================
// ANIMATED BACKGROUND (Port from Vyne Source)
// ============================================================

static void DrawAnimatedBackground(void) {
    float t = (float)GetTime();
    
    // --- Wave gradient strips ---
    for (int i = 0; i < 20; i++) {
        float stripY = i * 40.0f;
        int waveVal = (int)(sinf(t * 2.0f + i * 0.3f) * 20.0f + 25.0f);
        DrawScaledRect(0, stripY, 1280, 40, {(unsigned char)waveVal, 2, 8, 255});
    }
    
    // --- Floating grid nodes with laser links ---
    float nodeX, nodeY, nodeX2, nodeY2;
    for (int i = 0; i < 8; i++) {
        nodeX = fmodf(i * 160.0f + t * 30.0f, 1380.0f) - 50.0f;
        nodeY = 100.0f + sinf(t * 1.5f + i) * 60.0f;
        nodeX2 = fmodf((i + 1) * 160.0f + t * 30.0f, 1380.0f) - 50.0f;
        nodeY2 = 100.0f + sinf(t * 1.5f + i + 1) * 60.0f;
        
        // Laser link between nodes
        if (i < 7) {
            DrawScaledLine(nodeX, nodeY, nodeX2, nodeY2, {140, 20, 40, 90});
        }
        DrawScaledRect(nodeX, nodeY, 6, 6, COLOR_BLOOD);
    }
}

// ============================================================
// CONNECTION SCREEN (Port from Vyne Source)
// ============================================================

static void DrawConnectionScreen(void) {
    float t = (float)GetTime();
    float pulse = sinf(t * 6.0f) * 0.5f + 0.5f;
    
    // --- Animated Background (BRIGHTER) ---
    for (int i = 0; i < 20; i++) {
        float stripY = i * 40.0f;
        int waveVal = (int)(sinf(t * 2.0f + i * 0.3f) * 30.0f + 45.0f);  // Increased range
        DrawScaledRect(0, stripY, 1280, 40, {(unsigned char)waveVal, 3, 12, 255});
    }
    
    // --- Floating grid nodes (BRIGHTER) ---
    float nodeX, nodeY, nodeX2, nodeY2;
    for (int i = 0; i < 8; i++) {
        nodeX = fmodf(i * 160.0f + t * 30.0f, 1380.0f) - 50.0f;
        nodeY = 100.0f + sinf(t * 1.5f + i) * 60.0f;
        nodeX2 = fmodf((i + 1) * 160.0f + t * 30.0f, 1380.0f) - 50.0f;
        nodeY2 = 100.0f + sinf(t * 1.5f + i + 1) * 60.0f;
        
        if (i < 7) {
            DrawScaledLine(nodeX, nodeY, nodeX2, nodeY2, {200, 30, 60, 150});  // Brighter
        }
        DrawScaledRect(nodeX, nodeY, 8, 8, COLOR_BLOOD);  // Slightly larger
    }
    
    // --- Glitch jitter ---
    float jx = (pulse > 0.85f) ? sinf(t * 40.0f) * 4.0f : 0.0f;
    float jy = (pulse > 0.85f) ? cosf(t * 35.0f) * 3.0f : 0.0f;
    
    // --- Central box ---
    float boxX = 240.0f + jx;
    float boxY = 140.0f + jy;
    
    DrawScaledRect(boxX, boxY, 800, 520, COLOR_PANEL);
    DrawScaledRectLines(boxX, boxY, 800, 520, COLOR_BLOOD);
    
    // --- Header (FIXED ALIGNMENT) ---
    DrawScaledText("VYNE VEKTRAOS v9.5 // OPERATION COLD SIGNAL", 
                   boxX + 40.0f, boxY + 25.0f, 14, COLOR_BLOOD);
    DrawScaledText("SUBTERRANEAN RELAY UPLINK TERMINAL [PORT: 8001]", 
                   boxX + 40.0f, boxY + 50.0f, 11, COLOR_CYAN);
    DrawScaledLine(boxX + 20, boxY + 70, boxX + 780, boxY + 70, COLOR_BORDER);
    
    // --- Lore box ---
    float loreX = boxX + 20, loreY = boxY + 80;
    float loreW = 760, loreH = 180;
    
    DrawScaledRect(loreX, loreY, loreW, loreH, COLOR_BLACK);
    DrawScaledRectLines(loreX, loreY, loreW, loreH, COLOR_BORDER);
    
    Color pulseCol = (pulse > 0.5f) ? COLOR_BLOOD : COLOR_AMBER;
    DrawScaledText("[CLASSIFIED TOP SECRET] ANKARA SECTOR 09 BLACK-SITE RELAY",
                   loreX + 20.0f, loreY + 15.0f, 10, pulseCol);
    
    DrawScaledText("> ANALOG ECHO INTERCEPTED FROM signal0.vnet VIA COPPER BUS", 
                   loreX + 20, loreY + 38, 10, COLOR_GHOST);
    DrawScaledText("> SAT-99 ORBITAL OPTICS LOCKED ON LOCAL CRT GLARE COORDINATES", 
                   loreX + 20, loreY + 61, 10, COLOR_GHOST);
    DrawScaledText("> MORPHOGENIC STATIC LEAKING THROUGH VFS MEMORY ALLOCATION STACKS", 
                   loreX + 20, loreY + 84, 10, COLOR_GHOST);
    DrawScaledText("> KAGUYA TRIAL EXPLOIT HOOKS READY. AWAITING UDP HANDSHAKE...", 
                   loreX + 20, loreY + 107, 10, COLOR_TOXIC);
    DrawScaledText("WARNING: UNREGISTERED EYE CONTACT DETECTED THROUGH MONITOR GLASS",
                   loreX + 20, loreY + 140, 10, COLOR_BLOOD);
    
    // --- IP input label (FIXED ALIGNMENT) ---
    float labelY = boxY + 272;
    DrawScaledText("TARGET GATEWAY HOST / SERVER IP ADDRESS:", 
                   boxX + 40.0f, labelY, 11, COLOR_CYAN);
    
    // --- IP input box ---
    float inputX = boxX + 200, inputY = boxY + 290;
    float inputW = 400, inputH = 40;
    
    DrawScaledRect(inputX, inputY, inputW, inputH, COLOR_BLACK);
    DrawScaledRectLines(inputX, inputY, inputW, inputH, 
                        g_player.ipBoxFocused ? COLOR_BLOOD : COLOR_BORDER);
    
    // --- IP input text (FIXED ALIGNMENT) ---
    char ipDisplay[64];
    snprintf(ipDisplay, sizeof(ipDisplay), "%s%s", 
             g_player.ipInputBuffer, 
             (fmodf(t * 2.0f, 1.0f) > 0.5f) ? "_" : "");
    float ipW = MeasureScaledTextWidth(ipDisplay, 12);
    DrawScaledText(ipDisplay, 
                   inputX + (inputW - ipW) / 2.0f,  // CENTERED
                   inputY + 13, 12, COLOR_TOXIC);
    
    // --- Connect button ---
    float btnX = boxX + 270, btnY = boxY + 370;
    float btnW = 260, btnH = 40;
    
    Vector2 refMouse = GetRefMousePos();
    bool btnHover = RefRectHover(btnX, btnY, btnW, btnH, refMouse);
    
    DrawScaledRect(btnX, btnY, btnW, btnH, btnHover ? COLOR_BLOOD : COLOR_CLI_BG);
    DrawScaledRectLines(btnX, btnY, btnW, btnH, COLOR_BLOOD);
    
    // --- Connect button text (FIXED ALIGNMENT) ---
    float btnTextW = MeasureScaledTextWidth("INITIALIZE UPLINK", 11);
    DrawScaledText("INITIALIZE UPLINK", 
                   btnX + (btnW - btnTextW) / 2.0f,  // CENTERED
                   btnY + 14, 11, 
                   btnHover ? COLOR_BLACK : COLOR_TOXIC);
    
    // --- Footer (FIXED ALIGNMENT) ---
    DrawScaledText("'THE FAMILY HAS NO IP ADDRESS BECAUSE THE FAMILY IS EVERY OPEN PORT.'",
                   boxX + 40.0f, boxY + 470, 9, COLOR_AMBER);
    DrawScaledText("PRESS [ENTER] TO ESTABLISH P2P UDP SOCKET HANDSHAKE WITH SERVER",
                   boxX + 40.0f, boxY + 490, 9, COLOR_GHOST);
    
    // --- Scanlines (LIGHTER overlay) ---
    DrawScaledRect(0, 0, 1280, 800, {0, 0, 0, 60});  // Reduced alpha from 90 to 60
}

// ============================================================
// CONNECTION UI OVERLAY (Port from Vyne Source)
// ============================================================

void DrawConnectionOverlay(void) {
    float t = (float)GetTime();
    float pulse = sinf(t * 8.0f) * 0.5f + 0.5f;
    float jitterX = (g_player.glitchTrigger > 0.0f) ? sinf(t * 50.0f) * (g_player.glitchTrigger * 12.0f) : 0.0f;
    float jitterY = (g_player.glitchTrigger > 0.0f) ? cosf(t * 30.0f) * (g_player.glitchTrigger * 10.0f) : 0.0f;
    
    // --- Main container ---
    float boxX = 40.0f + jitterX;
    float boxY = 140.0f + jitterY;
    float boxW = 850.0f;
    float boxH = 500.0f;
    
    DrawScaledRect(boxX, boxY, boxW, boxH, COLOR_BLACK);
    DrawScaledRectLines(boxX, boxY, boxW, boxH, COLOR_AMBER);
    
    // --- Corner reticles ---
    DrawScaledRect(boxX - 5, boxY - 5, 20, 2, COLOR_BLOOD);
    DrawScaledRect(boxX - 5, boxY - 5, 2, 20, COLOR_BLOOD);
    DrawScaledRect(boxX + boxW - 15, boxY - 5, 20, 2, COLOR_BLOOD);
    DrawScaledRect(boxX + boxW - 5, boxY - 5, 2, 20, COLOR_BLOOD);
    DrawScaledRect(boxX - 5, boxY + boxH - 5, 20, 2, COLOR_BLOOD);
    DrawScaledRect(boxX - 5, boxY + boxH - 15, 2, 20, COLOR_BLOOD);
    DrawScaledRect(boxX + boxW - 15, boxY + boxH - 5, 20, 2, COLOR_BLOOD);
    DrawScaledRect(boxX + boxW - 5, boxY + boxH - 15, 2, 20, COLOR_BLOOD);
    
    // --- Remaining time ---
    float remaining = g_player.targetConnectTime - g_player.connectTimer;
    if (remaining < 0.0f) remaining = 0.0f;
    float ratio = (g_player.targetConnectTime > 0.0f) ? 
                  g_player.connectTimer / g_player.targetConnectTime : 0.0f;
    if (ratio < 0.02f) ratio = 0.02f;
    if (ratio > 1.0f) ratio = 1.0f;
    
    float centerX = boxX + boxW / 2.0f;
    
    // --- Header text ---
    Color headerCol = (pulse > 0.5f) ? COLOR_AMBER : COLOR_BLOOD;
    DrawScaledText("[ TOR PROXY CIRCUIT HANDSHAKE ACTIVE ]", 
                   centerX - 150.0f, boxY + 60.0f, 16, headerCol);
    
    // --- Target URL ---
    char urlStr[128];
    snprintf(urlStr, sizeof(urlStr), "RESOLVING DESTINATION: vnet://%s", g_player.pendingURL);
    float urlW = MeasureScaledTextWidth(urlStr, 13);
    DrawScaledText(urlStr, centerX - urlW / 2.0f, boxY + 105.0f, 13, COLOR_CYAN);
    
    // --- Rotating crosshair ---
    float rotOff = t * 4.0f;
    for (int i = 0; i < 4; i++) {
        float angle = rotOff + (float)i * 1.5708f;
        float rx = centerX + cosf(angle) * 35.0f;
        float ry = boxY + 170.0f + sinf(angle) * 20.0f;
        DrawScaledRect(rx, ry, 4, 4, COLOR_BLOOD);
    }
    
    // --- Latency status ---
    char latencyStr[128];
    snprintf(latencyStr, sizeof(latencyStr), "LATENCY BUFFER: %.0fs REMAINING [HOPS: 3/3]", remaining);
    float latW = MeasureScaledTextWidth(latencyStr, 11);
    DrawScaledText(latencyStr, centerX - latW / 2.0f, boxY + 215.0f, 11, COLOR_TOXIC);
    
    // --- Progress bar ---
    float barX = centerX - 250.0f;
    float barY = boxY + 245.0f;
    float barW = 500.0f;
    float barH = 22.0f;
    
    DrawScaledRect(barX, barY, barW, barH, COLOR_PANEL);
    DrawScaledRectLines(barX, barY, barW, barH, COLOR_BORDER);
    
    float fillW = barW * ratio;
    if (fillW < 2.0f) fillW = 2.0f;
    DrawScaledRect(barX, barY, fillW, barH, COLOR_TOXIC);
    
    // Leading edge glow
    if (fillW > 5.0f) {
        DrawScaledRect(barX + fillW - 4.0f, barY, 4, barH, COLOR_BLOOD);
    }
    
    // Percentage text
    char pctStr[16];
    snprintf(pctStr, sizeof(pctStr), "%d%%", (int)(ratio * 100.0f));
    float pctW = MeasureScaledTextWidth(pctStr, 11);
    DrawScaledText(pctStr, centerX - pctW / 2.0f, barY + 4.0f, 11, 
                   (ratio > 0.5f) ? COLOR_BLACK : COLOR_CYAN);
    
    // --- Hex stream telemetry ---
    int hexTick = (int)fmodf(t * 20.0f, 99.0f);
    char hexStr[128];
    snprintf(hexStr, sizeof(hexStr), "0x88F9_NODE_HOP_OK // ENCRYPTING PACKET SUBNET SECTOR #%d", hexTick);
    float hexW = MeasureScaledTextWidth(hexStr, 10);
    DrawScaledText(hexStr, centerX - hexW / 2.0f, boxY + 295.0f, 10, COLOR_AMBER);
    
    // --- Glitching footer ---
    float glitchOffset = sinf(t * 40.0f) * 4.0f;
    char footerStr[] = "SPOOFING MAC ADDRESS & MIRRORING PACKETS VIA ARCHIVAL.VNET...";
    float footW = MeasureScaledTextWidth(footerStr, 10);
    DrawScaledText(footerStr, centerX - footW / 2.0f + glitchOffset, boxY + 330.0f, 10, COLOR_GHOST);
}

// ============================================================
// HELLROOM CHATROOM UI (Port from Vyne Source)
// ============================================================

void DrawHellroomUI(float jx, float jy, Vector2 refMouse, bool clicked) {
    float t = (float)GetTime();
    float pulse = sinf(t * 5.0f) * 0.5f + 0.5f;

    // Header
    DrawScaledText("HELLROOM.VNET // DEMONIC P2P UNENCRYPTED CHATROOM", 
                   40 + jx, 95 + jy, 14, COLOR_BLOOD);
    DrawScaledLine(40 + jx, 115 + jy, 890 + jx, 115 + jy, COLOR_BORDER);

    // Input bar background
    DrawScaledRect(35 + jx, 125 + jy, 855, 52, COLOR_CLI_BG);
    DrawScaledLine(35 + jx, 125 + jy, 890 + jx, 125 + jy, COLOR_BORDER);
    DrawScaledLine(35 + jx, 177 + jy, 890 + jx, 177 + jy, COLOR_BORDER);

    // Handle input box
    float handleX = 45 + jx;
    float handleY = 135 + jy;
    float handleW = 180;
    float handleH = 32;

    bool handleHover = RefRectHover(handleX, handleY, handleW, handleH, refMouse);
    if (clicked && handleHover) {
        g_hellroom.handleFocused = true;
        g_hellroom.chatFocused = false;
    }

    DrawScaledRect(handleX, handleY, handleW, handleH, COLOR_BLACK);
    DrawScaledRectLines(handleX, handleY, handleW, handleH, 
                        g_hellroom.handleFocused ? COLOR_BLOOD : COLOR_BORDER);

    char handleDisplay[64];
    if (g_hellroom.handleFocused) {
        // Show cursor blink when focused
        const char* cursor = (fmodf(t * 2.0f, 1.0f) > 0.5f) ? "_" : "";
        snprintf(handleDisplay, sizeof(handleDisplay), "NICK: %s%s", 
                 g_hellroom.handleInputBuffer, cursor);
    } else {
        snprintf(handleDisplay, sizeof(handleDisplay), "NICK: %s", 
                 g_hellroom.handleInputBuffer);
    }
    DrawScaledText(handleDisplay, 52 + jx, 145 + jy, 11, 
                   g_hellroom.handleFocused ? COLOR_TOXIC : COLOR_AMBER);

    // Chat input box
    float chatX = 235 + jx;
    float chatY = 135 + jy;
    float chatW = 520;
    float chatH = 32;

    bool chatHover = RefRectHover(chatX, chatY, chatW, chatH, refMouse);
    if (clicked && chatHover) {
        g_hellroom.chatFocused = true;
        g_hellroom.handleFocused = false;
    }

    DrawScaledRect(chatX, chatY, chatW, chatH, COLOR_BLACK);
    DrawScaledRectLines(chatX, chatY, chatW, chatH, 
                        g_hellroom.chatFocused ? COLOR_BLOOD : COLOR_BORDER);

    char chatDisplay[128];
    if (g_hellroom.chatFocused) {
        const char* cursor = (fmodf(t * 2.0f, 1.0f) > 0.5f) ? "_" : "";
        snprintf(chatDisplay, sizeof(chatDisplay), "MSG> %s%s", 
                 g_hellroom.chatInputBuffer, cursor);
    } else {
        snprintf(chatDisplay, sizeof(chatDisplay), "MSG> %s", 
                 g_hellroom.chatInputBuffer);
    }
    // Truncate if too long (55 chars in Vyne)
    if (strlen(chatDisplay) > 55) {
        chatDisplay[55] = '\0';
    }
    DrawScaledText(chatDisplay, 242 + jx, 145 + jy, 11, 
                   g_hellroom.chatFocused ? COLOR_TOXIC : COLOR_CYAN);

    // Transmit button
    float btnX = 765 + jx;
    float btnY = 135 + jy;
    float btnW = 115;
    float btnH = 32;

    bool btnHover = RefRectHover(btnX, btnY, btnW, btnH, refMouse);
    DrawScaledRect(btnX, btnY, btnW, btnH, btnHover ? COLOR_BLOOD : COLOR_PANEL);
    DrawScaledRectLines(btnX, btnY, btnW, btnH, COLOR_BLOOD);
    DrawScaledText("TRANSMIT", 785 + jx, 145 + jy, 11, 
                   btnHover ? COLOR_BLACK : COLOR_BLOOD);

    // Handle transmit button click
    if (btnHover && clicked) {
        if (strlen(g_hellroom.chatInputBuffer) > 0) {
            // Check for whisper command: /w <nickname> <message> or /pm <nickname> <message>
            if (strlen(g_hellroom.chatInputBuffer) >= 3 && 
                (strncmp(g_hellroom.chatInputBuffer, "/w ", 3) == 0 || 
                 strncmp(g_hellroom.chatInputBuffer, "/pm ", 4) == 0)) {
                // Whisper command
                char* cmdStart = g_hellroom.chatInputBuffer;
                if (cmdStart[0] == '/' && cmdStart[1] == 'w') {
                    cmdStart += 3;
                } else if (cmdStart[0] == '/' && cmdStart[1] == 'p' && cmdStart[2] == 'm') {
                    cmdStart += 4;
                }
                // Skip spaces
                while (*cmdStart == ' ') cmdStart++;
                
                char target[64] = {0};
                char msg[256] = {0};
                if (sscanf(cmdStart, "%63s %255[^\n]", target, msg) == 2) {
                    // Send whisper
                    std::string payload = std::string(g_player.handle) + ":" + target + ":" + msg;
                    if (IsVNetConnected()) {
                        VNetSendRaw(std::string(VNetCmd::WHISPER) + ":" + payload);
                    }
                    // Local reflection for sender
                    char buffer[256];
                    snprintf(buffer, sizeof(buffer), "[WHISPER TO <%s>]: %s", target, msg);
                    PushCliLog(buffer);
                    PushFeedLog(buffer);
                } else {
                    PushCliLog("[ERROR]: Usage: /w <handle> <message>");
                }
            } else {
                // Regular chat
                std::string payload = std::string(g_player.handle) + ":" + g_hellroom.chatInputBuffer;
                if (IsVNetConnected()) {
                    VNetSendRaw(std::string(VNetCmd::CHAT) + ":" + payload);
                }
            }
            // Clear input
            g_hellroom.chatInputBuffer[0] = '\0';
            TriggerJitter(0.2f);
        }
    }

    // Chat log area
    float logX = 35 + jx;
    float logY = 190 + jy;
    float logW = 855;
    float logH = 545;

    DrawScaledRect(logX, logY, logW, logH, COLOR_BLACK);
    DrawScaledRectLines(logX, logY, logW, logH, COLOR_BORDER);

    DrawScaledText("=== LIVE VNET BROADCAST STREAM & PEER CHAT LOG ===", 
                   50 + jx, 202 + jy, 11, COLOR_AMBER);
    DrawScaledLine(50 + jx, 220 + jy, 875 + jx, 220 + jy, COLOR_BORDER);

    // Render chat feed (reverse order - newest at bottom)
    BeginScissorMode((int)SX(35 + jx), (int)SY(190 + jy), 
                     (int)(855 * g_uiScale), (int)(545 * g_uiScale));

    int feedCount = g_feedLogCount;
    int visibleLines = 22; // Approximate lines visible in 545px height / 22px per line
    
    // Calculate start index to show newest at bottom
    int startIdx = feedCount - visibleLines;
    if (startIdx < 0) startIdx = 0;

    // Reverse scroll: positive scroll moves up to show older messages
    int scrollLines = (int)(g_hellroom.chatScrollY / 22.0f);
    startIdx -= scrollLines;
    if (startIdx < 0) startIdx = 0;
    if (startIdx > feedCount) startIdx = feedCount;

    for (int i = startIdx; i < feedCount; i++) {
        int row = i - startIdx;
        float y = 228.0f + jy + row * 22.0f;
        if (y > 720.0f + jy) break;

        const char* txt = g_feedLogs[i];
        // Truncate to 90 chars (matching Vyne)
        char truncated[128];
        strncpy(truncated, txt, 90);
        truncated[90] = '\0';

        Color col = COLOR_GHOST;
        
        if (strstr(txt, "[WHISPER FROM")) {
            col = COLOR_AMBER;  // Bright Amber/Magenta accent for incoming whispers
        } else if (strstr(txt, "[WHISPER TO")) {
            col = COLOR_CYAN;   // Cyan accent for outgoing whispers
        } else if (strstr(txt, "[CHAT]")) {
            col = COLOR_TOXIC;
        } else if (strstr(txt, "[FEED]")) {
            col = COLOR_CYAN;
        }

        DrawScaledText(truncated, 50 + jx, y, 10, col);
    }

    EndScissorMode();

    // Update chat scroll from mouse wheel (handled in game.cpp)
}

// ============================================================
// MAIN UI SHELL
// ============================================================

void DrawUI(void) {
    ClearBackground(COLOR_BLACK);
    
    // Update jitter
    UpdateJitter(GetFrameTime());

    // ============================================================
    // CONNECTION MENU - ALWAYS FIRST (overrides everything)
    // ============================================================
    if (g_player.isInConnectionMenu) {
        DrawConnectionScreen();
        return;
    }

    // ============================================================
    // DESKTOP - Always active after connection
    // ============================================================
    // Desktop draws the entire UI including browser, terminal, etc.
    GetDesktop().Draw();
    
    // Draw FPS overlay if enabled (on top of desktop)
    if (g_game.showFPS) {
        char fpsText[32];
        snprintf(fpsText, sizeof(fpsText), "FPS: %d", g_game.currentFPS);
        DrawText(fpsText, 10, 10, 18, COLOR_TOXIC);
    }
}
// ============================================================
// TERMINAL UI
// ============================================================

void DrawTerminal(void) {
    DrawScaledRect(20, 80, 890, 520, COLOR_CLI_BG);
    DrawScaledRectLines(20, 80, 890, 520, COLOR_BLOOD);

    DrawScaledText("SYSTEM TERMINAL OVERLAY | REAL-TIME CLI & CYBERWARFARE HUB",
                    35, 95, 14, COLOR_BLOOD);
    DrawScaledLine(35, 118, 895, 118, COLOR_BORDER);

    BeginScissorMode((int)SX(30), (int)SY(125), (int)(870 * g_uiScale), (int)(430 * g_uiScale));

    int total = g_cliLogCount;
    int visibleLines = 18;
    int start = (total > visibleLines) ? total - visibleLines : 0;
    // Basic scroll: shift the start index by scroll amount (in lines).
    int scrollLines = (int)(g_player.cliScroll / 22.0f);
    start -= scrollLines;
    if (start < 0) start = 0;
    if (start > total) start = total;

    for (int i = start; i < total; i++) {
        int row = i - start;
        float y = 128.0f + row * 22.0f;
        if (y > 545.0f) break;

        Color col = COLOR_CYAN;
        const char* txt = g_cliLogs[i];
        if (strncmp(txt, "> ", 2) == 0) col = COLOR_TOXIC;
        if (strstr(txt, "[ERROR]") || strstr(txt, "[ERR]")) col = COLOR_BLOOD;
        if (strstr(txt, "[WARNING]")) col = COLOR_AMBER;

        DrawScaledText(txt, 35, y, 13, col);
    }

    EndScissorMode();

    // Input line
    DrawScaledLine(20, 560, 910, 560, COLOR_BORDER);
    char prompt[300];
    snprintf(prompt, sizeof(prompt), "CMD> %s", g_player.inputBuffer);
    DrawScaledText(prompt, 35, 572, 14, COLOR_TOXIC);

    if (fmodf((float)GetTime(), 0.8f) > 0.4f) {
        float textWidth = MeasureScaledTextWidth(prompt, 14);
        DrawScaledRect(35.0f + textWidth + 2.0f, 572, 8, 16, COLOR_TOXIC);
    }
}

// ============================================================
// FEED PANEL / THREAT RADAR
// ============================================================

void DrawFeedPanel(void) {
    DrawScaledText("SYSTEM THREAT RADAR", 1000, 92, 13, COLOR_AMBER);
    DrawScaledLine(930, 110, 1260, 110, COLOR_BORDER);

    // Trace level gauge
    DrawScaledText("TRACE LEVEL GAUGE:", 945, 122, 12, COLOR_CYAN);
    DrawScaledRect(945, 138, 300, 14, COLOR_BLACK);
    float traceW = (g_player.traceLevel / 100.0f) * 300.0f;
    Color traceColor = (g_player.traceLevel > 70) ? COLOR_BLOOD : COLOR_AMBER;
    DrawScaledRect(945, 138, traceW, 14, traceColor);
    DrawScaledRectLines(945, 138, 300, 14, COLOR_BORDER);

    char traceStr[32];
    snprintf(traceStr, sizeof(traceStr), "%d%% TRACED", g_player.traceLevel);
    DrawScaledText(traceStr, 945, 156, 12, COLOR_AMBER);
    DrawScaledLine(945, 176, 1260, 176, COLOR_BORDER);

    // CRT heat
    char heatStr[64];
    snprintf(heatStr, sizeof(heatStr), "CRT GLASS TEMP: %.0f C", g_player.crtHeat);
    DrawScaledText(heatStr, 945, 186, 11, (g_player.crtHeat > 75.0f) ? COLOR_BLOOD : COLOR_AMBER);
    DrawScaledRect(945, 202, 300, 10, COLOR_BLACK);
    float heatW = ((g_player.crtHeat - 35.0f) / 65.0f) * 300.0f;
    if (heatW < 0) heatW = 0;
    Color heatColor = (g_player.crtHeat > 75.0f) ? COLOR_BLOOD : COLOR_AMBER;
    DrawScaledRect(945, 202, heatW, 10, heatColor);
    DrawScaledLine(945, 220, 1260, 220, COLOR_BORDER);

    // Neural paranoia
    char paraStr[64];
    snprintf(paraStr, sizeof(paraStr), "NEURAL PARANOIA: %.0f%%", g_player.neuralParanoia);
    DrawScaledText(paraStr, 945, 230, 11, (g_player.neuralParanoia > 60.0f) ? COLOR_BLOOD : COLOR_AMBER);
    DrawScaledRect(945, 246, 300, 10, COLOR_BLACK);
    float paraW = (g_player.neuralParanoia / 100.0f) * 300.0f;
    Color paraColor = (g_player.neuralParanoia > 60.0f) ? COLOR_BLOOD : COLOR_AMBER;
    DrawScaledRect(945, 246, paraW, 10, paraColor);
    DrawScaledLine(945, 264, 1260, 264, COLOR_BORDER);

    // VCOIN & ICE
    char walletStr[64];
    snprintf(walletStr, sizeof(walletStr), "VCOIN: %.2f  ICE: %d/3", g_player.vcoin, g_player.iceShields);
    DrawScaledText(walletStr, 945, 274, 11, COLOR_TOXIC);
    DrawScaledLine(945, 294, 1260, 294, COLOR_BORDER);

    // Feed box
    DrawScaledRect(945, 302, 300, 396, COLOR_BLACK);
    DrawScaledRectLines(945, 302, 300, 396, COLOR_BORDER);

    BeginScissorMode((int)SX(945), (int)SY(302), (int)(300 * g_uiScale), (int)(396 * g_uiScale));
    int feedStart = (g_feedLogCount > 18) ? g_feedLogCount - 18 : 0;
    for (int i = feedStart; i < g_feedLogCount; i++) {
        int row = i - feedStart;
        float y = 310.0f + row * 20.0f;
        if (y > 690.0f) break;

        const char* txt = g_feedLogs[i];
        Color col = COLOR_CYAN;
        if (strstr(txt, "[CHAT]")) col = COLOR_TOXIC;
        if (strstr(txt, "[TRACE SPIKE]")) col = COLOR_AMBER;
        if (strstr(txt, "[DOS ATTACK]")) col = COLOR_BLOOD;
        if (strstr(txt, "[OVERLOAD]")) col = COLOR_BLOOD;

        DrawScaledText(txt, 950, y, 10, col);
    }
    EndScissorMode();
}

// ============================================================
// PULSE ANIMATION
// ============================================================

void DrawVPulse(float x, float y, float size, float time) {
    float pulse = sinf(time * 8.0f) * 0.5f + 0.5f;
    float radius = size * (0.5f + pulse * 0.5f);
    unsigned char alpha = (unsigned char)(pulse * 200 + 55);
    Color circleColor = {220, 20, 40, alpha};
    DrawCircle((int)SX(x), (int)SY(y), radius * g_uiScale, circleColor);
}