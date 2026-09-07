#include "render_core.h"
#include "render_themes.h"
#include "render_effects.h"
#include "../desktop/desktop.h"
#include "../raid/raid.h"
#include "../game.h"
#include "../connection/login_screen.h"

static Texture2D g_cursorTexture = {0};
static bool g_cursorLoaded = false;

static RenderTexture2D g_crtTarget = {0};
static bool g_crtTargetInit = false;
static Shader g_crtShader = {0};

extern GameState g_game;
extern LoginScreen g_loginScreen;

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

// ============================================================
// CUSTOM CURSOR
// ============================================================
void DrawCustomCursor(void) {
    Vector2 mouse = GetMousePosition();
    float t = (float)GetTime();
    float pulse = sinf(t * 4.0f) * 0.3f + 0.7f;
    
    // Convert to reference space for consistent sizing
    Vector2 refMouse = GetRefMousePos();
    float x = refMouse.x;
    float y = refMouse.y;
    
    // Draw a crosshair reticle
    float size = 12.0f;
    float gap = 4.0f;
    float thickness = 1.5f;
    
    Color col = {0, 220, 240, (unsigned char)(pulse * 200 + 55)};
    
    // Top arm
    DrawScaledLine(x - thickness/2, y - size, x - thickness/2, y - gap, col);
    // Bottom arm
    DrawScaledLine(x - thickness/2, y + gap, x - thickness/2, y + size, col);
    // Left arm
    DrawScaledLine(x - size, y - thickness/2, x - gap, y - thickness/2, col);
    // Right arm
    DrawScaledLine(x + gap, y - thickness/2, x + size, y - thickness/2, col);
    
    // Center dot
    float dotSize = 2.0f + pulse * 1.0f;
    DrawScaledRect(x - dotSize/2, y - dotSize/2, dotSize, dotSize, col);
    
    // Outer ring (subtle)
    DrawScaledCircleLines(x, y, size + 4.0f, Fade(col, 0.2f));
}

// ============================================================
// MAIN UI SHELL
// ============================================================
void DrawUI(void) {
    ClearBackground(COLOR_BLACK);
    UpdateJitter(GetFrameTime());

    // ============================================================
    // CONNECTION MENU - ALWAYS FIRST (overrides everything)
    // ============================================================
    if (g_loginScreen.isActive) {
        DrawLoginScreen(g_loginScreen);
        DrawCustomCursor();
        return;
    }

    // ============================================================
    // DESKTOP - Always active after connection
    // ============================================================
    GetDesktop().Draw();

    // raid overlay
    DrawRaidOverlay();

    // raid sequence overlay
    DrawRaidSequenceOverlay();
    
    // Draw FPS overlay if enabled (on top of desktop)
    if (g_game.showFPS) {
        char fpsText[32];
        snprintf(fpsText, sizeof(fpsText), "FPS: %d", g_game.currentFPS);
        DrawText(fpsText, 10, 10, 18, COLOR_TOXIC);
    }
}