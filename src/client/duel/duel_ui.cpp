#include "duel.h"
#include "../render.h"
#include "../../shared/vnet.h"
#include "../game.h"
#include <cmath>

extern Duel g_duel;

// ============================================================
// FORWARD DECLARATIONS
// ============================================================
static void DrawPlayerPanel(float x, float y, float w, float h,
                            const DuelPlayer& player, DuelRole role);
static void DrawDuelTerminal(float x, float y, float w, float h);
static void DrawDuelIntro(void);
static void DrawHexRain(float x, float y, float w, float h,
                        int streamCount, float baseSpeed,
                        float alphaMul, int colorMode);

// ============================================================
// HEX RAIN – FALLING CHARACTERS (0‑9A‑F)
// ============================================================
static void DrawHexRain(float x, float y, float w, float h,
                        int streamCount, float baseSpeed,
                        float alphaMul, int colorMode) {
    // colorMode: 0 = toxic/cyan/ghost, 1 = blood/amber, etc.
    static float streamOffsets[60] = {0};
    static float streamSpeeds[60] = {0};
    static int streamLengths[60] = {0};
    static float streamX[60] = {0};
    static bool initialized = false;

    if (!initialized) {
        for (int i = 0; i < 60; i++) {
            streamOffsets[i] = (float)(rand() % 1000) / 1000.0f * 2.0f;
            streamSpeeds[i] = 0.3f + (rand() % 100 / 100.0f) * 0.7f;
            streamLengths[i] = 5 + rand() % 20;
            streamX[i] = x + (rand() % (int)(w * 0.95f)) + w * 0.025f;
        }
        initialized = true;
    }

    float t = (float)GetTime();
    char hexChars[] = "0123456789ABCDEF";
    float hexH = 18.0f;

    // Clamp stream count to avoid overflow
    int count = (streamCount > 60) ? 60 : streamCount;
    if (count < 1) count = 1;

    for (int s = 0; s < count; s++) {
        float speed = streamSpeeds[s] * baseSpeed;
        float offset = streamOffsets[s];
        float progress = fmodf(t * speed + offset, 2.0f);
        if (progress > 1.0f) continue;

        float alpha = (1.0f - progress) * 0.7f + 0.2f;
        alpha *= alphaMul;
        float yPos = y + (1.0f - progress) * h;
        float xPos = streamX[s];

        // Color based on mode
        Color col;
        int colorType = s % 3;
        switch (colorMode) {
            case 0: // default: toxic/cyan/ghost
                if (colorType == 0) col = Fade(COLOR_TOXIC, alpha);
                else if (colorType == 1) col = Fade(COLOR_CYAN, alpha * 0.7f);
                else col = Fade(COLOR_GHOST, alpha * 0.5f);
                break;
            case 1: // amber/blood
                if (colorType == 0) col = Fade(COLOR_AMBER, alpha);
                else if (colorType == 1) col = Fade(COLOR_BLOOD, alpha * 0.7f);
                else col = Fade(COLOR_GHOST, alpha * 0.4f);
                break;
            default:
                col = Fade(COLOR_TOXIC, alpha);
                break;
        }

        int len = streamLengths[s];
        for (int j = 0; j < len; j++) {
            char c = hexChars[(rand() % 16)];
            float yDraw = yPos + j * hexH;
            if (yDraw < y || yDraw > y + h) continue;

            float size = 12.0f + (rand() % 10);
            if (j % 3 == 0) size *= 1.2f;
            Color charCol = col;
            if (rand() % 5 == 0) charCol = Fade(COLOR_BLOOD, alpha * 0.5f);

            DrawScaledText(&c, xPos, yDraw, size, charCol);
        }
    }
}

// ============================================================
// DRAW DUEL INTRO
// ============================================================
static void DrawDuelIntro(void) {
    float t = (float)GetTime();
    float progress = 1.0f - (g_duel.introTimer / 6.0f); // 0→1 over 6s

    DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, Color{4, 6, 10, 240});

    // Hex rain as background
    int streamCount = 28 + (int)(progress * 12);
    float baseSpeed = 100.0f + progress * 150.0f;
    DrawHexRain(0, 0, REF_WIDTH, REF_HEIGHT,
                streamCount, baseSpeed, 1.0f, 0);

    // Glitch overlay (pulsing)
    float glitch = sinf(t * 25.0f) * 0.5f + 0.5f;
    glitch *= 0.3f;
    DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT,
                   {220, 20, 40, (unsigned char)(glitch * 80)});

    // Random glitch bars
    for (int i = 0; i < 6; i++) {
        float y = (rand() % REF_HEIGHT);
        float h = 2 + rand() % 12;
        float alpha = (rand() % 100) / 400.0f;
        DrawScaledRect(0, y, REF_WIDTH, h,
                       {220, 20, 40, (unsigned char)(alpha * 200)});
    }

    // MAIN TEXT (glitchy)
    float pulse = sinf(t * 6.0f) * 0.2f + 0.8f;
    char line1[64] = "INITIALIZING EXPLOIT VECTOR...";
    char line2[64] = "BREACHING TARGET NODE...";

    float jx = sinf(t * 48.0f) * 2.0f;
    float jy = cosf(t * 32.0f) * 1.5f;

    Color col = Fade(COLOR_TOXIC, pulse * 0.9f + 0.1f);

    DrawScaledText(line1, REF_WIDTH/2 - 250 + jx, REF_HEIGHT/2 - 40 + jy, 28, col);
    DrawScaledText(line1, REF_WIDTH/2 - 250 - jx, REF_HEIGHT/2 - 40 - jy, 28, Fade(COLOR_BLOOD, 0.3f));
    DrawScaledText(line1, REF_WIDTH/2 - 250, REF_HEIGHT/2 - 40, 28, col);

    // Typewriter second line
    int charCount = (int)(t * 12.0f) % 30;
    char display[64] = {0};
    strncpy(display, line2, charCount);
    if (charCount < (int)strlen(line2)) {
        strcat(display, "_");
    }
    DrawScaledText(display, REF_WIDTH/2 - 140, REF_HEIGHT/2 + 10, 18, COLOR_CYAN);

    // Progress bar
    float barX = REF_WIDTH/2 - 200;
    float barY = REF_HEIGHT/2 + 60;
    float barW = 400;
    float barH = 20;

    DrawScaledRect(barX, barY, barW, barH, COLOR_BLACK);
    DrawScaledRectLines(barX, barY, barW, barH, COLOR_BORDER);

    float fill = progress;
    for (int x = 0; x < (int)(barW * fill); x += 2) {
        float p = (float)x / barW;
        unsigned char r = (unsigned char)(40 + p * 200);
        unsigned char g = (unsigned char)(200 - p * 150);
        unsigned char b = (unsigned char)(50 + p * 50);
        DrawScaledRect(barX + x, barY, 2, barH, {r, g, b, 255});
    }

    char pct[16];
    snprintf(pct, sizeof(pct), "%.0f%%", progress * 100.0f);
    float tw = MeasureScaledTextWidth(pct, 16);
    DrawScaledText(pct, barX + barW/2 - tw/2, barY - 22, 16, COLOR_TOXIC);

    // Scanlines
    for (int i = 0; i < (int)REF_HEIGHT; i += 4) {
        float scanY = i + fmodf(t * 60.0f, 4.0f);
        DrawScaledRect(0, scanY, REF_WIDTH, 1, {0, 0, 0, 6});
    }

    // Vignette
    DrawScaledRect(0, 0, REF_WIDTH, 2, {0, 0, 0, 200});
    DrawScaledRect(0, REF_HEIGHT - 2, REF_WIDTH, 2, {0, 0, 0, 200});
    DrawScaledRect(0, 0, 2, REF_HEIGHT, {0, 0, 0, 200});
    DrawScaledRect(REF_WIDTH - 2, 0, 2, REF_HEIGHT, {0, 0, 0, 200});
}

// ============================================================
// DRAW MAIN DUEL SCREEN
// ============================================================
void DrawDuelScreen(void) {
    if (!g_duel.active) return;

    // ---- BACKGROUND ----
    DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, Color{4, 6, 10, 240});

    if (g_duel.state == DuelState::STARTING) {
        DrawDuelIntro();
        return;
    }

    float t = (float)GetTime();

    // ---- 1. HEX RAIN BACKGROUND (more immersive) ----
    // Use a dense but not overwhelming stream
    DrawHexRain(0, 0, REF_WIDTH, REF_HEIGHT, 35, 130.0f, 0.6f, 0);

    // ---- 2. FAINT GRID OVERLAY ----
    // Horizontal lines (subtle)
    for (int y = 0; y < REF_HEIGHT; y += 40) {
        DrawScaledLine(0, y, REF_WIDTH, y, Fade(COLOR_CYAN, 0.03f));
    }
    // Vertical lines
    for (int x = 0; x < REF_WIDTH; x += 60) {
        DrawScaledLine(x, 0, x, REF_HEIGHT, Fade(COLOR_CYAN, 0.02f));
    }

    // ---- 3. CORNER BRACKETS (tech‑style) ----
    Color bracketCol = Fade(COLOR_CYAN, 0.3f);
    float br = 30.0f;
    float off = 15.0f;
    // Top-left
    DrawScaledLine(off, off + br, off, off, bracketCol);
    DrawScaledLine(off, off, off + br, off, bracketCol);
    // Top-right
    DrawScaledLine(REF_WIDTH - off - br, off, REF_WIDTH - off, off, bracketCol);
    DrawScaledLine(REF_WIDTH - off, off, REF_WIDTH - off, off + br, bracketCol);
    // Bottom-left
    DrawScaledLine(off, REF_HEIGHT - off - br, off, REF_HEIGHT - off, bracketCol);
    DrawScaledLine(off, REF_HEIGHT - off, off + br, REF_HEIGHT - off, bracketCol);
    // Bottom-right
    DrawScaledLine(REF_WIDTH - off - br, REF_HEIGHT - off, REF_WIDTH - off, REF_HEIGHT - off, bracketCol);
    DrawScaledLine(REF_WIDTH - off, REF_HEIGHT - off - br, REF_WIDTH - off, REF_HEIGHT - off, bracketCol);

    // ---- 4. HEADER ----
    float headerY = 20;
    DrawScaledText("🏴‍☠️ HACKING DUEL — CTF STYLE", REF_WIDTH/2 - 200, headerY, 20, COLOR_BLOOD);
    DrawScaledText("CAPTURE THE FLAG // CYBER OPERATIONS", REF_WIDTH/2 - 180, headerY + 26, 12, COLOR_CYAN);
    DrawScaledLine(40, headerY + 48, REF_WIDTH - 40, headerY + 48, Color{30, 35, 50, 100});

    // ---- 5. TIMER ----
    float remaining = g_duel.maxDuration - g_duel.timer;
    char timerStr[32];
    snprintf(timerStr, sizeof(timerStr), "⏱ %.0fs REMAINING", remaining);
    float timerW = MeasureScaledTextWidth(timerStr, 16);
    DrawScaledText(timerStr, REF_WIDTH - timerW - 40, headerY + 10, 16,
                   remaining < 10.0f ? COLOR_BLOOD : COLOR_TOXIC);

    // ---- 6. PLAYER PANELS ----
    float panelW = (REF_WIDTH - 80) / 2.0f;
    float panelX1 = 40;
    float panelX2 = REF_WIDTH / 2.0f + 20;
    float panelY = headerY + 56;
    float panelH = REF_HEIGHT - 240;

    DrawPlayerPanel(panelX1, panelY, panelW, panelH, g_duel.attacker, DuelRole::ATTACKER);
    DrawPlayerPanel(panelX2, panelY, panelW, panelH, g_duel.defender, DuelRole::DEFENDER);

    // ---- 7. TERMINAL ----
    float termY = REF_HEIGHT - 160;
    float termH = 140;
    DrawDuelTerminal(40, termY, REF_WIDTH - 80, termH);

    // ---- 8. GLITCH OVERLAY ----
    if (g_duel.glitchIntensity > 0.05f) {
        float alpha = g_duel.glitchIntensity * 0.2f;
        DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, {220, 20, 40, (unsigned char)(alpha * 255)});
        for (int i = 0; i < (int)(g_duel.glitchIntensity * 10); i++) {
            float y = rand() % REF_HEIGHT;
            float h = 2 + rand() % 8;
            DrawScaledRect(0, y, REF_WIDTH, h, {220, 20, 40, (unsigned char)(alpha * 150)});
        }
    }

    // ---- 9. SCANLINES ----
    for (int i = 0; i < (int)REF_HEIGHT; i += 4) {
        float scanY = i + fmodf(g_duel.scanlineOffset, 4.0f);
        DrawScaledRect(0, scanY, REF_WIDTH, 1, {0, 0, 0, 4});
    }

    // ---- 10. VIGNETTE ----
    DrawScaledRect(0, 0, REF_WIDTH, 2, {0, 0, 0, 200});
    DrawScaledRect(0, REF_HEIGHT - 2, REF_WIDTH, 2, {0, 0, 0, 200});
    DrawScaledRect(0, 0, 2, REF_HEIGHT, {0, 0, 0, 200});
    DrawScaledRect(REF_WIDTH - 2, 0, 2, REF_HEIGHT, {0, 0, 0, 200});
}

// ============================================================
// DRAW PLAYER PANEL (enhanced)
// ============================================================
void DrawPlayerPanel(float x, float y, float w, float h, const DuelPlayer& player, DuelRole role) {
    bool isAttacker = (role == DuelRole::ATTACKER);
    Color accent = isAttacker ? COLOR_TOXIC : COLOR_CYAN;
    const char* roleText = isAttacker ? "⚡ ATTACKER" : "🛡️ DEFENDER";

    // ---- PANEL BACKGROUND (slightly transparent with double border) ----
    DrawScaledRect(x, y, w, h, Color{6, 8, 14, 200});
    // Outer border (thin)
    DrawScaledRectLines(x, y, w, h, Fade(accent, 0.2f));
    // Inner border (thicker, with glow)
    DrawScaledRectLines(x + 4, y + 4, w - 8, h - 8, Fade(accent, 0.08f));

    // ---- HEADER ----
    float headerH = 36.0f;
    DrawScaledRect(x, y, w, headerH, Fade(accent, 0.1f));
    DrawScaledLine(x, y + headerH, x + w, y + headerH, Fade(accent, 0.2f));
    // Small accent line above header
    DrawScaledLine(x + 8, y + 2, x + w - 8, y + 2, Fade(accent, 0.15f));

    DrawScaledText(roleText, x + 12, y + 8, 12, accent);
    DrawScaledText(player.handle.c_str(), x + 140, y + 8, 12, COLOR_GHOST);
    char portStr[16];
    snprintf(portStr, sizeof(portStr), "%d", player.port);
    DrawScaledText("PORT:", x + 300, y + 8, 10, COLOR_GHOST);
    DrawScaledText(portStr, x + 340, y + 8, 10, COLOR_AMBER);

    // ---- PROGRESS BAR ----
    float progY = y + headerH + 8;
    float progW = w - 24;
    DrawScaledText("PROGRESS", x + 12, progY, 9, COLOR_GHOST);
    DrawScaledRect(x + 12, progY + 14, progW, 6, Color{12, 15, 20, 255});
    float fill = player.progress / 100.0f;
    DrawScaledRect(x + 12, progY + 14, progW * fill, 6, accent);
    char progStr[16];
    snprintf(progStr, sizeof(progStr), "%.0f%%", player.progress);
    float progW2 = MeasureScaledTextWidth(progStr, 9);
    DrawScaledText(progStr, x + w - progW2 - 12, progY, 9, accent);

    // ---- OBJECTIVES LIST ----
    float objY = progY + 30;
    DrawScaledText("🎯 OBJECTIVES", x + 12, objY, 10, COLOR_AMBER);
    objY += 18;
    for (const auto& obj : player.objectives) {
        float objX = x + 20;
        Color objCol = obj.completed ? COLOR_TOXIC : COLOR_GHOST;
        const char* status = obj.completed ? "✅" : "⬜";
        DrawScaledText(status, objX, objY, 10, objCol);
        DrawScaledText(obj.name.c_str(), objX + 20, objY, 9, objCol);
        // Difficulty indicator
        float diffX = x + w - 60;
        Color diffCol = obj.difficulty < 0.33f ? COLOR_TOXIC :
                        obj.difficulty < 0.66f ? COLOR_AMBER : COLOR_BLOOD;
        char diffStr[16];
        snprintf(diffStr, sizeof(diffStr), "%.0f%%", obj.difficulty * 100);
        DrawScaledText(diffStr, diffX, objY, 8, diffCol);
        objY += 20;
        if (objY - y > h - 40) break;
    }

    // ---- STATS (bottom) ----
    float statsY = y + h - 50;
    DrawScaledLine(x + 10, statsY, x + w - 10, statsY, Fade(accent, 0.1f));
    statsY += 6;
    char stats[128];
    snprintf(stats, sizeof(stats), "TRACE: %d%%  |  ICE: %d/3  |  VCOIN: %.2f",
             player.trace, player.iceShields, player.vcoin);
    DrawScaledText(stats, x + 12, statsY, 8, Fade(COLOR_GHOST, 0.7f));

    // ---- SCANLINE IN PANEL (subtle) ----
    float t = (float)GetTime();
    for (int i = 0; i < (int)h; i += 8) {
        float scanY = y + i + fmodf(t * 30.0f, 8.0f);
        DrawScaledRect(x + 4, scanY, w - 8, 1, Fade(BLACK, 0.03f));
    }
}

// ============================================================
// DRAW DUEL TERMINAL
// ============================================================
void DrawDuelTerminal(float x, float y, float w, float h) {
    float t = (float)GetTime();

    // Background
    DrawScaledRect(x, y, w, h, Color{4, 6, 10, 220});
    DrawScaledRectLines(x, y, w, h, Fade(COLOR_CYAN, 0.2f));

    // Inner border (tech glow)
    DrawScaledRectLines(x + 2, y + 2, w - 4, h - 4, Fade(COLOR_CYAN, 0.05f));

    // Header
    DrawScaledText("█ DUEL TERMINAL", x + 12, y + 8, 10, COLOR_TOXIC);
    DrawScaledLine(x + 10, y + 24, x + w - 10, y + 24, Fade(COLOR_CYAN, 0.1f));

    // Action log (scissor)
    BeginScissorMode((int)SX(x + 4), (int)SY(y + 30),
                     (int)((w - 8) * g_uiScale), (int)((h - 60) * g_uiScale));

    float logY = y + 34;
    int startIdx = (int)g_duel.actionLog.size() - 12;
    if (startIdx < 0) startIdx = 0;

    for (int i = startIdx; i < (int)g_duel.actionLog.size(); i++) {
        const char* log = g_duel.actionLog[i].c_str();
        Color col = COLOR_GHOST;
        if (strstr(log, "[SYSTEM]")) col = COLOR_CYAN;
        else if (strstr(log, "[SUCCESS]")) col = COLOR_TOXIC;
        else if (strstr(log, "[FAIL]")) col = COLOR_BLOOD;
        else if (strstr(log, "[FLAG]")) col = COLOR_AMBER;
        DrawScaledText(log, x + 8, logY, 9, col);
        logY += 18;
    }
    EndScissorMode();

    // Input line
    float inputY = y + h - 32;
    DrawScaledRect(x + 4, inputY, w - 8, 24, Color{8, 10, 14, 200});
    DrawScaledRectLines(x + 4, inputY, w - 8, 24, Fade(COLOR_CYAN, 0.15f));

    extern char g_duelInputBuffer[256];
    DrawScaledText(">", x + 12, inputY + 5, 10, COLOR_TOXIC);
    if (strlen(g_duelInputBuffer) > 0) {
        DrawScaledText(g_duelInputBuffer, x + 24, inputY + 5, 10, COLOR_CYAN);
    }
    // Blinking cursor
    if ((int)(t * 2.0f) % 2 == 0) {
        float cursorX = x + 24 + MeasureScaledTextWidth(g_duelInputBuffer, 10);
        DrawScaledRect(cursorX, inputY + 2, 6, 18, COLOR_TOXIC);
    }

    // Help text
    DrawScaledText("[ENTER] send  |  [TAB] focus  |  [↑↓] scroll",
                   x + 12, y + h - 56, 7, Fade(COLOR_GHOST, 0.3f));
}