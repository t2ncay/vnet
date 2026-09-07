#include "render_themes.h"
#include "render_core.h"  // for Fade? Not needed if you use raylib's Fade
#include <cstring>
#include <cmath>

// ---- COLOR GLOBALS ----
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

Color COLOR_VOID         = {1, 1, 2, 255};
Color COLOR_SIGNAL       = {40, 240, 200, 255};
Color COLOR_NEON_PURPLE  = {150, 90, 220, 255};
Color COLOR_NEON_PINK    = {255, 90, 130, 255};
Color COLOR_STATIC       = {160, 170, 185, 40};

Color COLOR_SUCCESS = {40, 240, 100, 255};
Color COLOR_WARNING = {255, 150, 0, 255};
Color COLOR_ERROR   = {220, 20, 40, 255};
Color COLOR_INFO    = {0, 220, 240, 255};

// ---- THEME TABLE ----
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

static Color BlendColor(Color a, Color b, float t, float boost = 1.0f) {
    auto lerp8 = [](unsigned char x, unsigned char y, float w) -> unsigned char {
        float v = x + (y - x) * w;
        return (unsigned char)(v < 0 ? 0 : (v > 255 ? 255 : v));
    };
    Color out = { lerp8(a.r, b.r, t), lerp8(a.g, b.g, t), lerp8(a.b, b.b, t), 255 };
    if (boost != 1.0f) {
        out.r = (unsigned char)fminf(255.0f, out.r * boost);
        out.g = (unsigned char)fminf(255.0f, out.g * boost);
        out.b = (unsigned char)fminf(255.0f, out.b * boost);
    }
    return out;
}

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

    // ---- Extended accents, derived per-theme ----
    COLOR_VOID        = BlendColor(t->bg, BLACK, 0.4f);              // darker than bg, for deep vignette layers
    COLOR_SIGNAL       = BlendColor(t->cyan, t->toxic, 0.5f, 1.15f);  // "live data" accent
    COLOR_NEON_PURPLE  = BlendColor(t->cyan, t->blood, 0.5f, 1.05f);  // gradient midpoint
    COLOR_NEON_PINK    = BlendColor(t->blood, t->amber, 0.4f, 1.05f); // gradient midpoint
    COLOR_STATIC       = Fade(t->ghost, 0.16f);                       // faint CRT noise tint

    // ---- Semantic aliases ----
    COLOR_SUCCESS = t->toxic;
    COLOR_WARNING = t->amber;
    COLOR_ERROR   = t->blood;
    COLOR_INFO    = t->cyan;
}

void InitColors(void) { SetActiveTheme("classic"); }