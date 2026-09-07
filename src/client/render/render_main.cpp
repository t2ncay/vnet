#include "render_core.h"
#include "render_themes.h"
#include "render_effects.h"
#include "../desktop/desktop.h"
#include "../raid/raid.h"
#include "../game.h"
#include "../connection/login_screen.h"

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
// MAIN UI SHELL
// ============================================================
void DrawUI(void) {
    ClearBackground(COLOR_BLACK);
    
    // Update jitter
    UpdateJitter(GetFrameTime());

    // ============================================================
    // CONNECTION MENU - ALWAYS FIRST (overrides everything)
    // ============================================================
    if (g_loginScreen.isActive) {
        DrawLoginScreen(g_loginScreen);
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