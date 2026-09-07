#include "render_themes.h"
#include "render_core.h"
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

// ---- THEME GRADIENT COLORS ----
Color THEME_GRADIENT_TOP = {18, 20, 30, 255};
Color THEME_GRADIENT_BOTTOM = {6, 8, 14, 255};
Color THEME_GRADIENT_ACCENT = {0, 220, 240, 255};

// ---- THEME MOOD GLOBALS ----
float THEME_JITTER_BASELINE = 0.05f;
float THEME_SCANLINE_INTENSITY = 1.0f;
float THEME_GLOW_STRENGTH = 0.8f;
float THEME_DATA_RAIN_DENSITY = 0.3f;
float THEME_WARMTH = 0.5f;

// ---- THEME TABLE WITH GRADIENTS ----
struct ThemeDef {
    const char* name;
    // Base colors
    Color bg, panel, cliBg, urlbar, border, blood, cyan, amber, toxic, ghost;
    // Gradient colors (top, bottom, accent)
    Color gradTop, gradBottom, gradAccent;
    // Mood values
    float jitterBaseline;
    float scanlineIntensity;
    float glowStrength;
    float dataRainDensity;
    float warmth;
};

static const ThemeDef kThemes[] = {
    // ============================================================
    // EXISTING THEMES (with gradients)
    // ============================================================
    {"classic",   
        {2,2,4,255}, {10,12,16,255}, {6,8,12,245}, {16,20,26,255}, {40,50,60,255},
        {220,20,40,255}, {0,220,240,255}, {255,150,0,255}, {40,240,100,255}, {160,170,185,255},
        {18,20,30,255}, {6,8,14,255}, {0,220,240,255},
        0.05f, 1.0f, 0.8f, 0.3f, 0.5f
    },
    {"tokyo",   
        {16,18,28,255}, {22,25,38,230}, {14,16,26,245}, {30,35,54,255}, {52,60,92,255},
        {247,118,142,255}, {122,162,247,255}, {187,154,247,255}, {115,218,202,255}, {192,202,245,255},
        {30,35,55,255}, {16,18,28,255}, {247,118,142,255},
        0.08f, 0.8f, 1.2f, 0.4f, 0.3f
    },
    {"redroom",   
        {10,3,5,255}, {22,8,12,235}, {15,4,7,245}, {38,12,18,255}, {85,22,30,255},
        {255,20,50,255}, {240,45,65,255}, {255,140,0,255}, {0,230,180,255}, {235,210,215,255},
        {35,8,12,255}, {10,3,5,255}, {255,20,50,255},
        0.2f, 1.8f, 1.0f, 0.2f, 0.9f
    },
    {"amber",   
        {12,8,2,255}, {24,16,5,230}, {16,10,3,245}, {40,26,8,255}, {90,60,15,255},
        {240,60,30,255}, {255,175,0,255}, {220,120,0,255}, {180,240,80,255}, {255,210,130,255},
        {40,25,8,255}, {12,8,2,255}, {255,175,0,255},
        0.1f, 1.2f, 0.9f, 0.3f, 0.8f
    },
    {"cyberpunk",   
        {18,10,26,255}, {30,16,42,230}, {22,12,32,245}, {45,20,60,255}, {100,30,120,255},
        {255,0,85,255}, {0,230,255,255}, {255,215,0,255}, {50,255,120,255}, {220,200,240,255},
        {35,18,50,255}, {18,10,26,255}, {255,0,85,255},
        0.15f, 1.5f, 1.5f, 0.6f, 0.4f
    },
    {"nord",   
        {15,20,28,255}, {24,30,42,230}, {18,24,34,245}, {35,45,60,255}, {60,75,100,255},
        {191,97,106,255}, {136,192,208,255}, {208,135,112,255}, {163,190,140,255}, {216,222,233,255},
        {30,38,52,255}, {15,20,28,255}, {136,192,208,255},
        0.03f, 0.6f, 0.6f, 0.2f, 0.2f
    },
    {"dracula",   
        {18,16,26,255}, {28,25,40,230}, {20,18,30,245}, {40,35,58,255}, {70,60,95,255},
        {255,85,85,255}, {139,233,253,255}, {255,184,108,255}, {80,250,123,255}, {248,248,242,255},
        {35,30,50,255}, {18,16,26,255}, {255,85,85,255},
        0.07f, 1.0f, 1.1f, 0.3f, 0.5f
    },
    {"synthwave",   
        {12,6,24,255}, {26,12,48,230}, {16,8,32,245}, {42,18,75,255}, {90,35,140,255},
        {255,30,130,255}, {0,240,255,255}, {255,160,0,255}, {120,255,180,255}, {235,210,255,255},
        {30,12,55,255}, {12,6,24,255}, {255,30,130,255},
        0.12f, 1.3f, 1.6f, 0.5f, 0.6f
    },
    {"cobalt",   
        {2,12,28,255}, {8,22,48,230}, {4,16,36,245}, {14,34,70,255}, {30,65,120,255},
        {255,60,90,255}, {0,190,255,255}, {255,180,40,255}, {40,240,180,255}, {180,215,245,255},
        {8,25,50,255}, {2,12,28,255}, {0,190,255,255},
        0.06f, 0.9f, 1.3f, 0.4f, 0.2f
    },
    {"monokai",   
        {20,20,20,255}, {32,32,32,230}, {24,24,24,245}, {45,45,45,255}, {80,80,80,255},
        {255,97,136,255}, {120,220,232,255}, {252,152,103,255}, {166,226,46,255}, {248,248,242,255},
        {40,40,40,255}, {20,20,20,255}, {255,97,136,255},
        0.04f, 0.7f, 0.9f, 0.3f, 0.5f
    },
    {"gruvbox",   
        {20,20,18,255}, {32,30,26,230}, {24,22,18,245}, {48,44,38,255}, {85,78,66,255},
        {251,73,52,255}, {131,165,152,255}, {254,128,25,255}, {184,187,38,255}, {235,219,178,255},
        {40,38,32,255}, {20,20,18,255}, {251,73,52,255},
        0.05f, 0.8f, 0.7f, 0.2f, 0.7f
    },
    {"abyss",   
        {2,6,12,255}, {6,14,26,230}, {4,10,20,245}, {10,24,42,255}, {20,50,80,255},
        {230,40,70,255}, {0,180,200,255}, {0,140,180,255}, {0,240,160,255}, {140,180,200,255},
        {6,16,30,255}, {2,6,12,255}, {0,180,200,255},
        0.18f, 1.6f, 0.9f, 0.8f, 0.1f
    },
    {"solaris",   
        {18,6,2,255}, {32,12,4,230}, {22,8,3,245}, {52,20,8,255}, {100,40,15,255},
        {255,40,20,255}, {255,140,0,255}, {255,200,0,255}, {220,240,50,255}, {255,225,180,255},
        {40,14,6,255}, {18,6,2,255}, {255,140,0,255},
        0.08f, 1.1f, 1.4f, 0.3f, 0.9f
    },
    {"ghost",   
        {12,14,18,255}, {20,24,30,230}, {15,18,24,245}, {32,38,48,255}, {65,75,90,255},
        {240,80,100,255}, {160,210,245,255}, {200,190,220,255}, {140,230,210,255}, {220,230,240,255},
        {28,32,40,255}, {12,14,18,255}, {160,210,245,255},
        0.02f, 0.5f, 0.5f, 0.1f, 0.3f
    },
    {"matrix",   
        {4,10,6,255}, {8,22,12,230}, {5,15,8,245}, {14,38,20,255}, {25,80,40,255},
        {240,40,70,255}, {0,210,255,255}, {40,240,100,255}, {50,255,120,255}, {200,255,215,255},
        {10,28,14,255}, {4,10,6,255}, {40,240,100,255},
        0.12f, 1.0f, 1.2f, 1.0f, 0.4f
    },
    
    // ============================================================
    // NEW THEMES WITH GRADIENTS
    // ============================================================
    {"midnight",  
        {6,4,12,255}, {16,12,28,230}, {10,8,20,245}, {28,20,48,255}, {60,40,100,255},
        {180,60,220,255}, {100,180,255,255}, {255,200,80,255}, {80,255,180,255}, {200,190,230,255},
        {15,8,30,255}, {6,4,12,255}, {180,60,220,255},
        0.1f, 1.2f, 1.5f, 0.5f, 0.4f
    },
    {"sunset",   
        {20,8,6,255}, {40,18,14,230}, {28,12,10,245}, {60,28,20,255}, {120,60,40,255},
        {255,80,60,255}, {255,180,80,255}, {255,220,100,255}, {180,255,120,255}, {255,210,180,255},
        {50,18,12,255}, {20,8,6,255}, {255,180,80,255},
        0.07f, 1.0f, 1.3f, 0.2f, 1.0f
    },
    {"ice",      
        {8,12,20,255}, {18,24,40,230}, {12,18,30,245}, {30,42,68,255}, {60,85,130,255},
        {80,200,255,255}, {150,230,255,255}, {200,240,255,255}, {100,255,230,255}, {220,240,255,255},
        {16,28,44,255}, {8,12,20,255}, {150,230,255,255},
        0.02f, 0.4f, 0.9f, 0.2f, 0.0f
    },
    {"lava",     
        {12,4,2,255}, {28,10,6,230}, {20,6,4,245}, {48,16,10,255}, {100,35,20,255},
        {255,60,40,255}, {255,120,60,255}, {255,200,80,255}, {200,255,100,255}, {255,200,160,255},
        {40,12,6,255}, {12,4,2,255}, {255,120,60,255},
        0.2f, 1.8f, 1.6f, 0.6f, 1.0f
    },
    {"neon_dream",
        {10,4,18,255}, {22,10,36,230}, {16,8,26,245}, {40,18,60,255}, {90,35,130,255},
        {255,40,180,255}, {0,255,220,255}, {255,220,0,255}, {150,255,100,255}, {230,200,255,255},
        {28,10,44,255}, {10,4,18,255}, {255,40,180,255},
        0.15f, 1.4f, 1.8f, 0.7f, 0.5f
    },
    {"hacker",   
        {6,8,6,255}, {14,22,14,230}, {10,16,10,245}, {24,44,24,255}, {40,80,40,255},
        {40,240,60,255}, {0,255,180,255}, {200,255,100,255}, {100,255,150,255}, {180,255,200,255},
        {12,30,14,255}, {6,8,6,255}, {40,240,60,255},
        0.08f, 0.8f, 1.0f, 1.2f, 0.4f
    },
    {"bloodmoon",
        {16,2,4,255}, {36,6,10,230}, {24,4,6,245}, {56,12,18,255}, {120,25,40,255},
        {255,20,40,255}, {200,40,60,255}, {255,120,40,255}, {40,240,120,255}, {255,180,180,255},
        {50,6,10,255}, {16,2,4,255}, {255,20,40,255},
        0.25f, 2.0f, 1.4f, 0.3f, 1.0f
    },
    {"deepweb",  
        {2,4,8,255}, {6,12,24,230}, {4,8,18,245}, {10,24,44,255}, {20,50,90,255},
        {0,200,255,255}, {0,160,220,255}, {40,220,255,255}, {0,255,200,255}, {160,210,240,255},
        {6,16,30,255}, {2,4,8,255}, {0,200,255,255},
        0.06f, 1.0f, 1.1f, 0.9f, 0.1f
    },
    {"vaporwave",
        {20,10,30,255}, {40,20,56,230}, {30,14,42,245}, {60,30,85,255}, {120,60,160,255},
        {255,30,180,255}, {0,240,255,255}, {255,200,0,255}, {120,255,200,255}, {240,210,255,255},
        {50,18,64,255}, {20,10,30,255}, {255,30,180,255},
        0.12f, 1.2f, 1.7f, 0.5f, 0.5f
    },
};
static const int kThemeCount = sizeof(kThemes) / sizeof(kThemes[0]);

// ---- THEME TRANSITION STATE ----
static struct {
    bool active;
    float timer;
    float duration;
    Color fromColors[18];  // Increased for gradient colors
    Color toColors[18];
} g_themeTransition = {false, 0.0f, 0.4f, {}, {}};

// ---- PRIVATE HELPERS ----
static void StoreCurrentColors(Color* dest) {
    dest[0] = COLOR_BLACK;
    dest[1] = COLOR_PANEL;
    dest[2] = COLOR_CLI_BG;
    dest[3] = COLOR_URLBAR;
    dest[4] = COLOR_BORDER;
    dest[5] = COLOR_BLOOD;
    dest[6] = COLOR_CYAN;
    dest[7] = COLOR_AMBER;
    dest[8] = COLOR_TOXIC;
    dest[9] = COLOR_GHOST;
    dest[10] = COLOR_VOID;
    dest[11] = COLOR_SIGNAL;
    dest[12] = COLOR_NEON_PURPLE;
    dest[13] = COLOR_NEON_PINK;
    dest[14] = COLOR_STATIC;
    dest[15] = THEME_GRADIENT_TOP;
    dest[16] = THEME_GRADIENT_BOTTOM;
    dest[17] = THEME_GRADIENT_ACCENT;
}

static void ApplyColorArray(const Color* src) {
    COLOR_BLACK = src[0];
    COLOR_PANEL = src[1];
    COLOR_CLI_BG = src[2];
    COLOR_URLBAR = src[3];
    COLOR_BORDER = src[4];
    COLOR_BLOOD = src[5];
    COLOR_CYAN = src[6];
    COLOR_AMBER = src[7];
    COLOR_TOXIC = src[8];
    COLOR_GHOST = src[9];
    COLOR_VOID = src[10];
    COLOR_SIGNAL = src[11];
    COLOR_NEON_PURPLE = src[12];
    COLOR_NEON_PINK = src[13];
    COLOR_STATIC = src[14];
    THEME_GRADIENT_TOP = src[15];
    THEME_GRADIENT_BOTTOM = src[16];
    THEME_GRADIENT_ACCENT = src[17];
}

// ============================================================
// COLOR HELPERS
// ============================================================
Color BlendColor(Color a, Color b, float t, float boost) {  // ← NO default here
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

// ============================================================
// SetActiveTheme - WITH TRANSITION SUPPORT
// ============================================================
void SetActiveTheme(const char* name, bool instant) {
    const ThemeDef* t = &kThemes[0];
    for (int i = 0; i < kThemeCount; i++) {
        if (strcmp(kThemes[i].name, name) == 0) { t = &kThemes[i]; break; }
    }

    // ---- SET MOOD VALUES ----
    THEME_JITTER_BASELINE = t->jitterBaseline;
    THEME_SCANLINE_INTENSITY = t->scanlineIntensity;
    THEME_GLOW_STRENGTH = t->glowStrength;
    THEME_DATA_RAIN_DENSITY = t->dataRainDensity;
    THEME_WARMTH = t->warmth;

    // ---- SET GRADIENT COLORS ----
    THEME_GRADIENT_TOP = t->gradTop;
    THEME_GRADIENT_BOTTOM = t->gradBottom;
    THEME_GRADIENT_ACCENT = t->gradAccent;

    if (instant) {
        // ---- APPLY INSTANTLY ----
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

        COLOR_VOID        = BlendColor(t->bg, BLACK, 0.4f);
        COLOR_SIGNAL       = BlendColor(t->cyan, t->toxic, 0.5f, 1.15f);
        COLOR_NEON_PURPLE  = BlendColor(t->cyan, t->blood, 0.5f, 1.05f);
        COLOR_NEON_PINK    = BlendColor(t->blood, t->amber, 0.4f, 1.05f);
        COLOR_STATIC       = Fade(t->ghost, 0.16f);

        COLOR_SUCCESS = t->toxic;
        COLOR_WARNING = t->amber;
        COLOR_ERROR   = t->blood;
        COLOR_INFO    = t->cyan;
        
        g_themeTransition.active = false;
    } else {
        // ---- START TRANSITION ----
        StoreCurrentColors(g_themeTransition.fromColors);
        
        Color target[18] = {
            t->bg, t->panel, t->cliBg, t->urlbar, t->border,
            t->blood, t->cyan, t->amber, t->toxic, t->ghost,
            BlendColor(t->bg, BLACK, 0.4f),
            BlendColor(t->cyan, t->toxic, 0.5f, 1.15f),
            BlendColor(t->cyan, t->blood, 0.5f, 1.05f),
            BlendColor(t->blood, t->amber, 0.4f, 1.05f),
            Fade(t->ghost, 0.16f),
            t->gradTop,
            t->gradBottom,
            t->gradAccent
        };
        memcpy(g_themeTransition.toColors, target, sizeof(target));
        g_themeTransition.active = true;
        g_themeTransition.timer = 0.0f;
        g_themeTransition.duration = 0.4f;
    }
}

// ============================================================
// UpdateThemeTransition - Call this every frame
// ============================================================
void UpdateThemeTransition(float dt) {
    if (!g_themeTransition.active) return;
    
    g_themeTransition.timer += dt;
    float progress = g_themeTransition.timer / g_themeTransition.duration;
    if (progress >= 1.0f) {
        progress = 1.0f;
        g_themeTransition.active = false;
    }
    
    // Smooth step (ease in-out)
    float t = progress * progress * (3.0f - 2.0f * progress);
    
    Color current[18];
    for (int i = 0; i < 18; i++) {
        current[i] = BlendColor(g_themeTransition.fromColors[i], 
                                g_themeTransition.toColors[i], t);
    }
    ApplyColorArray(current);
}

// ============================================================
// InitColors - Legacy entry point
// ============================================================
void InitColors(void) { 
    SetActiveTheme("classic", true); 
}