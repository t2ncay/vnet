#include "login_screen.h"
#include "../render.h"
#include "../../shared/vnet.h"
#include "../vnet_client.h"
#include "../game.h"

#include <cstdio>
#include <cstring>
#include <cmath>

extern Player g_player;

// ============================================================
// INITIALIZATION
// ============================================================

void InitLoginScreen(LoginScreen& login) {
    login.isActive = true;
    login.ipBoxFocused = true;
    login.selectedField = 0;
    login.animTime = 0.0f;
    login.cursorBlinkTimer = 0.0f;
    login.showCursor = true;
    login.glitchTimer = 0.0f;
    login.scanlineOffset = 0.0f;
    
    // Loading state
    login.isLoading = false;
    login.loadingProgress = 0.0f;
    login.loadingTimer = 0.0f;
    login.loadingComplete = false;
    login.completionTimer = 0.0f;
    strcpy(login.loadingStatus, "INITIALIZING UPLINK...");
    
    strcpy(login.ipInputBuffer, "127.0.0.1");
    strcpy(login.handleBuffer, g_player.handle);
}

// ============================================================
// START LOADING
// ============================================================

void StartLoading(LoginScreen& login) {
    login.isLoading = true;
    login.loadingProgress = 0.0f;
    login.loadingTimer = 0.0f;
    login.loadingComplete = false;
    login.completionTimer = 0.0f;
    strcpy(login.loadingStatus, "INITIALIZING UPLINK...");
}

// ============================================================
// HELPER DRAW FUNCTIONS - MOVED BEFORE THEY'RE USED!
// ============================================================

static void DrawAnimatedGrid(float time) {
    // Floating grid nodes
    for (int i = 0; i < 30; i++) {
        float x = fmodf(i * 65.0f + time * 15.0f, REF_WIDTH);
        float y = 50.0f + sinf(time * 0.5f + i * 0.4f) * 80.0f + i * 35.0f;
        
        // Grid lines
        DrawScaledRect(x - 1, y - 1, 2, 2, {0, 220, 240, (unsigned char)(60 + sinf(time + i) * 20 + 20)});
        
        // Connection lines between nodes
        if (i > 0 && i < 29) {
            float x2 = fmodf((i + 1) * 65.0f + time * 15.0f, REF_WIDTH);
            float y2 = 50.0f + sinf(time * 0.5f + (i + 1) * 0.4f) * 80.0f + (i + 1) * 35.0f;
            if (fabsf(x2 - x) < 300.0f) {
                DrawScaledLine(x, y, x2, y2, {0, 220, 240, 20});
            }
        }
    }
}

static void DrawVektraLogo(float x, float y, float size, float time) {
    float pulse = sinf(time * 1.5f) * 0.15f + 0.85f;
    float glowPulse = sinf(time * 2.0f) * 0.3f + 0.7f;
    
    // Main logo glow
    DrawScaledRect(x - 40 * size, y - 20 * size, 80 * size, 40 * size, 
                   {0, 220, 240, (unsigned char)(glowPulse * 20)});
    
    // VEKTRA text
    float fontSize = 48.0f * size;
    float textW = MeasureScaledTextWidth("VEKTRA", fontSize);
    DrawScaledText("VEKTRA", x - textW / 2, y - 20 * size, fontSize, 
                   {0, 220, 240, (unsigned char)(pulse * 200 + 55)});
    
    // Subtitle
    float subSize = 14.0f * size;
    float subW = MeasureScaledTextWidth("OPERATING SYSTEM // COLD SIGNAL v9.5", subSize);
    DrawScaledText("OPERATING SYSTEM // COLD SIGNAL v9.5", x - subW / 2, y + 8 * size, subSize,
                   {100, 140, 180, (unsigned char)(pulse * 150 + 50)});
    
    // Decorative lines
    float lineY = y + 14 * size;
    float lineW = 200 * size;
    DrawScaledLine(x - lineW / 2, lineY, x - 30 * size, lineY, {0, 220, 240, 80});
    DrawScaledLine(x + 30 * size, lineY, x + lineW / 2, lineY, {0, 220, 240, 80});
    
    // Diamond accent
    float diamondSize = 6 * size;
    DrawScaledRect(x - diamondSize, lineY - diamondSize, diamondSize * 2, diamondSize * 2, {0, 220, 240, 100});
    DrawScaledRect(x - diamondSize / 2, lineY - diamondSize / 2, diamondSize, diamondSize, {0, 220, 240, 200});
}

static void DrawInputBox(float x, float y, float w, float h, const char* label, 
                         const char* value, bool focused, bool showCursor,
                         Color focusColor, Color textColor) {
    // Background
    Color bg = focused ? Color{16, 20, 30, 255} : Color{8, 10, 18, 200};
    DrawScaledRect(x, y, w, h, bg);
    
    // Border
    Color border = focused ? focusColor : Color{40, 50, 70, 150};
    DrawScaledRectLines(x, y, w, h, border);
    
    // Inner glow when focused
    if (focused) {
        DrawScaledRect(x + 2, y + 2, w - 4, h - 4, {focusColor.r, focusColor.g, focusColor.b, 30});
    }
    
    // Label
    float labelSize = 10.0f;
    float labelW = MeasureScaledTextWidth(label, labelSize);
    DrawScaledText(label, x + 12, y - labelSize - 4, labelSize, {100, 140, 180, 200});
    
    // Value text
    char display[128];
    if (focused && showCursor) {
        snprintf(display, sizeof(display), "%s_", value);
    } else {
        snprintf(display, sizeof(display), "%s", value);
    }
    
    float textSize = 14.0f;
    float textW = MeasureScaledTextWidth(display, textSize);
    DrawScaledText(display, x + (w - textW) / 2.0f, y + (h - textSize * 1.5f) / 2.0f + 2.0f, 
                   textSize, textColor);
}

static void DrawSystemStatus(float x, float y, float time) {
    float pulse = sinf(time * 2.0f) * 0.3f + 0.7f;
    
    DrawScaledText("┌───────────────────────────────────────────┐", x, y, 9, {50, 60, 80, 150});
    DrawScaledText("│ SYSTEM STATUS                          │", x, y + 16, 9, {50, 60, 80, 150});
    DrawScaledText("├───────────────────────────────────────────┤", x, y + 32, 9, {50, 60, 80, 150});
    
    // Status items
    float statusY = y + 50;
    
    // Network
    DrawScaledRect(x + 10, statusY + 4, 8, 8, {40, 240, 100, (unsigned char)(pulse * 200 + 55)});
    DrawScaledText("NETWORK:", x + 24, statusY, 9, {100, 140, 180, 200});
    DrawScaledText("READY", x + 110, statusY, 9, {40, 240, 100, 200});
    statusY += 20;
    
    // Encryption
    DrawScaledRect(x + 10, statusY + 4, 8, 8, {40, 240, 100, (unsigned char)(pulse * 200 + 55)});
    DrawScaledText("ENCRYPTION:", x + 24, statusY, 9, {100, 140, 180, 200});
    DrawScaledText("8192-BIT", x + 110, statusY, 9, {40, 240, 100, 200});
    statusY += 20;
    
    // Uptime
    DrawScaledRect(x + 10, statusY + 4, 8, 8, {40, 240, 100, (unsigned char)(pulse * 200 + 55)});
    DrawScaledText("UPTIME:", x + 24, statusY, 9, {100, 140, 180, 200});
    DrawScaledText("00:00:00", x + 110, statusY, 9, {40, 240, 100, 200});
    statusY += 20;
    
    // Port
    DrawScaledRect(x + 10, statusY + 4, 8, 8, {0, 220, 240, (unsigned char)(pulse * 200 + 55)});
    DrawScaledText("PORT:", x + 24, statusY, 9, {100, 140, 180, 200});
    char portStr[16];
    snprintf(portStr, sizeof(portStr), "%d", g_player.port);
    DrawScaledText(portStr, x + 110, statusY, 9, {0, 220, 240, 200});
    statusY += 20;
    
    DrawScaledText("└───────────────────────────────────────────┘", x, statusY, 9, {50, 60, 80, 150});
}

static void DrawLoginFooter(float x, float y, float time) {
    float pulse = sinf(time * 1.0f) * 0.3f + 0.7f;
    
    char versionStr[64];
    snprintf(versionStr, sizeof(versionStr), "VEKTRAOS v9.5 // COLD SIGNAL // %s", __DATE__);
    float w = MeasureScaledTextWidth(versionStr, 9);
    DrawScaledText(versionStr, x - w / 2, y, 9, {60, 80, 100, 150});
    
    // Warning text
    const char* warning = "⚠ UNREGISTERED SYSTEM // AUTHORIZED PERSONNEL ONLY";
    float w2 = MeasureScaledTextWidth(warning, 9);
    float pulse2 = sinf(time * 0.7f) * 0.5f + 0.5f;
    Color warnCol = {220, 50, 50, (unsigned char)(pulse2 * 150 + 50)};
    DrawScaledText(warning, x - w2 / 2, y + 18, 9, warnCol);
}

static void DrawCRTScanlines(float x, float y, float w, float h, float offset) {
    for (int i = 0; i < (int)h; i += 3) {
        float scanY = y + i + fmodf(offset, 3.0f);
        unsigned char alpha = (i % 2 == 0) ? 6 : 3;
        DrawScaledRect(x, scanY, w, 1, {0, 0, 0, alpha});
    }
}

// ============================================================
// DRAW LOADING SCREEN
// ============================================================

// ============================================================
// DRAW LOADING SCREEN – HOLOGRAPHIC CRT ENHANCED
// ============================================================

static void DrawLoadingScreen(const LoginScreen& login) {
    float t = login.animTime;
    float cx = REF_WIDTH / 2.0f;
    float cy = REF_HEIGHT / 2.0f;
    float progress = login.loadingProgress;
    float pulse = sinf(t * 3.0f) * 0.3f + 0.7f;
    float glitchPulse = sinf(t * 7.0f) * 0.5f + 0.5f;
    
    // ---- RESET TYPEWRITER STATE ON NEW LOAD ----
    if (progress == 0.0f) {
        static bool resetDone = false;
        if (!resetDone) {
            // Reset static variables (declared below) by reinitializing them
            // We'll use a static bool to trigger reset once.
            // We'll reset inside the typewriter block.
            resetDone = true;
        }
        // But we need to reset the static variables themselves.
        // We'll do that inside their own blocks using a flag.
    }
    
    // ---- BACKGROUND ----
    DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, Color{2, 2, 6, 255});
    
    // ---- ANIMATED GRID ----
    DrawAnimatedGrid(t);
    
    // ---- CRT VIGNETTE (darker edges) ----
    for (int i = 0; i < 10; i++) {
        float size = i * 18.0f;
        DrawScaledRect(size, size, REF_WIDTH - size * 2, REF_HEIGHT - size * 2, 
                       {0, 0, 0, (unsigned char)(6 - i * 0.5f)});
    }
    
    // ---- HOLOGRAPHIC GLOW BEHIND MAIN CONTAINER ----
    DrawScaledRect(190, 70, 520, 520, {0, 220, 240, 8});
    DrawScaledRect(195, 75, 510, 510, {0, 220, 240, 6});
    
    // ---- MAIN CONTAINER ----
    float boxX = 200.0f;
    float boxY = 80.0f;
    float boxW = 500.0f;
    float boxH = 500.0f;
    
    DrawScaledRect(boxX, boxY, boxW, boxH, Color{8, 10, 16, 230});
    DrawScaledRectLines(boxX, boxY, boxW, boxH, {0, 220, 240, 60});
    
    // ---- CORNER RETICLES (with glow) ----
    Color retCol = {0, 220, 240, (unsigned char)(sinf(t * 0.5f) * 50 + 100)};
    float retSize = 28.0f;
    
    // Top-left
    DrawScaledRect(boxX - 3, boxY - 3, retSize, 3, retCol);
    DrawScaledRect(boxX - 3, boxY - 3, 3, retSize, retCol);
    // Top-right
    DrawScaledRect(boxX + boxW - retSize, boxY - 3, retSize, 3, retCol);
    DrawScaledRect(boxX + boxW, boxY - 3, 3, retSize, retCol);
    // Bottom-left
    DrawScaledRect(boxX - 3, boxY + boxH, retSize, 3, retCol);
    DrawScaledRect(boxX - 3, boxY + boxH - retSize, 3, retSize, retCol);
    // Bottom-right
    DrawScaledRect(boxX + boxW - retSize, boxY + boxH, retSize, 3, retCol);
    DrawScaledRect(boxX + boxW, boxY + boxH - retSize, 3, retSize, retCol);
    
    // ---- VEKTRA LOGO (with chromatic aberration) ----
    // Draw red offset
    float offsetX = 1.5f;
    float offsetY = 1.0f;
    DrawVektraLogo(cx + offsetX, boxY + 50 + offsetY, 0.55f, t);
    // Draw cyan offset
    DrawVektraLogo(cx - offsetX, boxY + 50 - offsetY, 0.55f, t);
    // Draw main logo
    DrawVektraLogo(cx, boxY + 50, 0.55f, t);
    
    // ---- HOLOGRAPHIC SCANNING LINE UNDER LOGO ----
    float scanLineY = boxY + 105 + sinf(t * 2.5f) * 5.0f;
    for (int i = 0; i < 3; i++) {
        float alpha = (i == 0) ? (unsigned char)(pulse * 150 + 50) : (unsigned char)(20);
        DrawScaledLine(boxX + 50 - i*2, scanLineY + i*2, boxX + boxW - 50 + i*2, scanLineY + i*2, 
                    {0, 220, 240, alpha});
    }
    
    // ---- DECORATIVE LINE ----
    float lineY = boxY + 115;
    DrawScaledLine(boxX + 30, lineY, boxX + boxW - 30, lineY, {0, 220, 240, 40});
    
    // ---- BOOT SEQUENCE TITLE (typewriter with glow) ----
    static float bootTimer = 0.0f;
    static int bootCharIndex = 0;
    static bool bootStarted = false;
    static const char* bootTitle = "> SYSTEM BOOT SEQUENCE v9.5";
    
    // Reset on new load
    if (progress == 0.0f) {
        bootStarted = false;
        bootTimer = 0.0f;
        bootCharIndex = 0;
    }
    
    if (!bootStarted) {
        bootStarted = true;
        bootTimer = 0.0f;
        bootCharIndex = 0;
    }
    
    bootTimer += GetFrameTime();
    if (bootTimer > 0.04f && bootCharIndex < (int)strlen(bootTitle)) {
        bootTimer = 0.0f;
        bootCharIndex++;
    }
    
    char bootDisplay[128];
    strncpy(bootDisplay, bootTitle, bootCharIndex);
    bootDisplay[bootCharIndex] = '\0';
    // Glow behind boot text
    DrawScaledText(bootDisplay, boxX + 30 + 1, boxY + 135 + 1, 12, Fade(COLOR_TOXIC, 0.2f));
    DrawScaledText(bootDisplay, boxX + 30 - 1, boxY + 135 - 1, 12, Fade(COLOR_CYAN, 0.2f));
    DrawScaledText(bootDisplay, boxX + 30, boxY + 135, 12, COLOR_TOXIC);
    
    // Blinking cursor after boot title
    if ((int)(GetTime() * 2.0f) % 2 == 0 && bootCharIndex >= (int)strlen(bootTitle)) {
        DrawScaledText("_", boxX + 30 + MeasureScaledTextWidth(bootTitle, 12), boxY + 135, 12, COLOR_TOXIC);
    }
    
    // ---- TYPEWRITER STATUS MESSAGES (with holographic glow) ----
    static float statusTimer = 0.0f;
    static int statusCharIndex = 0;
    static char currentStatus[128] = "";
    static float statusPingTimes[3] = { -10.0f, -10.0f, -10.0f };
    static int statusPingHead = 0;
    
    // Update status text based on progress (from existing logic)
    const char* statusText = login.loadingStatus;
    
    // If status changed, reset typewriter
    if (strcmp(currentStatus, statusText) != 0) {
        strcpy(currentStatus, statusText);
        statusCharIndex = 0;
        statusTimer = 0.0f;
        // Spawn a ping ring off the wheel — a small visual "beat" every
        // time the handshake advances to its next stage.
        statusPingTimes[statusPingHead] = t;
        statusPingHead = (statusPingHead + 1) % 3;
    }
    
    statusTimer += GetFrameTime();
    if (statusTimer > 0.03f && statusCharIndex < (int)strlen(statusText)) {
        statusTimer = 0.0f;
        statusCharIndex++;
    }
    
    char statusDisplay[128];
    strncpy(statusDisplay, statusText, statusCharIndex);
    statusDisplay[statusCharIndex] = '\0';
    
    // Glowing status text with chromatic aberration
    float statusPulse = sinf(t * 4.0f) * 0.3f + 0.7f;
    Color statusCol = (statusPulse > 0.5f) ? COLOR_CYAN : COLOR_AMBER;
    statusCol.a = (unsigned char)(statusPulse * 200 + 55);
    
    float statusW = MeasureScaledTextWidth(statusDisplay, 13);
    float statusX = cx - statusW / 2;
    float statusY = boxY + 175;
    
    // Chromatic aberration
    DrawScaledText(statusDisplay, statusX + 1.5f, statusY + 1.0f, 13, Fade(COLOR_BLOOD, 0.25f));
    DrawScaledText(statusDisplay, statusX - 1.5f, statusY - 1.0f, 13, Fade(COLOR_CYAN, 0.25f));
    DrawScaledText(statusDisplay, statusX, statusY, 13, statusCol);
    
    // ---- ROTATING CRYPTOGRAPHIC WHEEL (with holographic glow) ----
    float wheelY = boxY + 290;
    float wheelRadius = 48.0f;
    float rotOff = t * 1.8f;
    int segments = 12;
    float segmentAngle = 2.0f * PI / segments;
    
    // Glow ring behind wheel
    DrawScaledCircle(cx, wheelY, wheelRadius + 8, Fade(COLOR_CYAN, 0.08f));
    
    for (int i = 0; i < segments; i++) {
        float angle = rotOff + i * segmentAngle;
        float x1 = cx + cosf(angle) * wheelRadius;
        float y1 = wheelY + sinf(angle) * wheelRadius;
        float x2 = cx + cosf(angle + segmentAngle * 0.4f) * (wheelRadius * 0.55f);
        float y2 = wheelY + sinf(angle + segmentAngle * 0.4f) * (wheelRadius * 0.55f);
        
        Color segCol = (i % 2 == 0) ? COLOR_TOXIC : COLOR_CYAN;
        segCol.a = (unsigned char)(pulse * 180 + 50);
        // Glow lines
        DrawScaledLine(x1 + 1, y1 + 1, x2 + 1, y2 + 1, Fade(segCol, 0.3f));
        DrawScaledLine(x1, y1, x2, y2, segCol);
    }
    
    // Inner ring of dots (pulsing)
    for (int i = 0; i < 8; i++) {
        float angle = -rotOff * 0.7f + i * 0.7854f;
        float rx = cx + cosf(angle) * 24.0f;
        float ry = wheelY + sinf(angle) * 24.0f;
        float size = 3.0f + sinf(t * 5.0f + i) * 2.0f;
        Color dotCol = {0, 220, 240, (unsigned char)(pulse * 200 + 55)};
        DrawScaledRect(rx - size/2, ry - size/2, size, size, dotCol);
        // Glow
        DrawScaledRect(rx - size/2 - 2, ry - size/2 - 2, size + 4, size + 4, Fade(dotCol, 0.2f));
    }
    
    // ---- STATUS-CHANGE PING RINGS ----
    // One expanding ring per handshake stage advance — reads as the
    // system "pulsing" forward rather than the wheel just spinning idly.
    for (int i = 0; i < 3; i++) {
        float age = t - statusPingTimes[i];
        if (age >= 0.0f && age < 1.1f) {
            float ringT = age / 1.1f;
            float ringR = wheelRadius + ringT * 90.0f;
            DrawScaledCircleLines(cx, wheelY, ringR, Fade(COLOR_TOXIC, (1.0f - ringT) * 0.35f));
        }
    }
    
    // ---- PROGRESS BAR (HOLOGRAPHIC) ----
    float barX = boxX + 50;
    float barY = wheelY + 65;
    float barW = boxW - 100;
    float barH = 10.0f;
    
    // Background with border glow
    DrawScaledRect(barX - 2, barY - 2, barW + 4, barH + 4, {0, 220, 240, 40});
    DrawScaledRect(barX, barY, barW, barH, Color{12, 15, 20, 255});
    DrawScaledRectLines(barX, barY, barW, barH, {30, 35, 50, 100});
    
    // Progress fill with gradient
    float fillW = barW * progress;
    if (fillW < 2.0f) fillW = 2.0f;
    
    for (int x = 0; x < (int)fillW; x += 2) {
        float prog = (float)x / barW;
        unsigned char r = (unsigned char)(0 + prog * 220);
        unsigned char g = (unsigned char)(220 - prog * 20);
        unsigned char b = (unsigned char)(240 - prog * 40);
        DrawScaledRect(barX + x, barY, 2, barH, {r, g, b, 255});
    }
    
    // Scanning light on progress bar (moving)
    float scanPos = fmodf(t * 60.0f, barW);
    DrawScaledRect(barX + scanPos - 4.0f, barY - 2, 8.0f, barH + 4, {0, 220, 240, 60});
    DrawScaledRect(barX + scanPos - 1.0f, barY, 2.0f, barH, {0, 220, 240, 200});
    
    // Glow at leading edge
    if (fillW > 5.0f) {
        DrawScaledRect(barX + fillW - 6.0f, barY - 3, 6, barH + 6, {0, 220, 240, 40});
    }
    
    // ---- PERCENTAGE (with pulse and glow) ----
    char pctStr[16];
    snprintf(pctStr, sizeof(pctStr), "%d%%", (int)(progress * 100.0f));
    float pctW = MeasureScaledTextWidth(pctStr, 14);
    Color pctCol = (progress > 0.5f) ? COLOR_TOXIC : COLOR_CYAN;
    pctCol.a = (unsigned char)(pulse * 200 + 55);
    // Glow
    DrawScaledText(pctStr, cx - pctW / 2 + 1, barY + 22 + 1, 14, Fade(pctCol, 0.3f));
    DrawScaledText(pctStr, cx - pctW / 2, barY + 22, 14, pctCol);
    
    // ---- ANTICIPATION GLOW (container border brightens as we approach 100%) ----
    if (progress > 0.85f) {
        float anticipation = (progress - 0.85f) / 0.15f;
        DrawScaledRectLines(boxX - 1, boxY - 1, boxW + 2, boxH + 2,
                             Fade(COLOR_TOXIC, anticipation * (sinf(t * 6.0f) * 0.2f + 0.3f)));
    }
    
    // ---- FLOATING DATA PACKETS (with trails) ----
    float packetY = boxY + 310;
    for (int i = 0; i < 8; i++) {
        float offset = fmodf(t * 25.0f + i * 35.0f, boxW - 60);
        float px = boxX + 30 + offset;
        float size = 4.0f + sinf(t * 4.0f + i) * 1.5f;
        Color col = (i % 2 == 0) ? COLOR_TOXIC : COLOR_CYAN;
        col.a = (unsigned char)(120 + sinf(t * 3.0f + i) * 80 + 40);
        DrawScaledRect(px, packetY + i * 3.0f, size, size, col);
        // Trail
        for (int trail = 1; trail < 4; trail++) {
            float trailX = px - trail * 6.0f;
            if (trailX > boxX) {
                DrawScaledRect(trailX, packetY + i * 3.0f, 2, 2, 
                               {col.r, col.g, col.b, (unsigned char)(80 - trail * 20)});
            }
        }
    }
    
    // ---- VHS NOISE / STATIC ----
    // Random horizontal noise bars
    for (int i = 0; i < 8; i++) {
        if (fmodf(t * 3.0f + i * 1.7f, 1.0f) > 0.85f) {
            float yNoise = boxY + (rand() % (int)boxH);
            float hNoise = 2 + rand() % 6;
            float alpha = 20 + rand() % 80;
            DrawScaledRect(boxX, yNoise, boxW, hNoise, {0, 0, 0, (unsigned char)alpha});
        }
    }
    // Random color interference (chromatic shift)
    if (fmodf(t * 0.7f, 1.0f) > 0.9f) {
        float xOff = (rand() % 100) / 100.0f * 20.0f - 10.0f;
        float yOff = (rand() % 100) / 100.0f * 10.0f - 5.0f;
        DrawScaledRect(boxX + xOff, boxY + yOff, 20, 20, {0, 220, 240, 20});
        DrawScaledRect(boxX - xOff, boxY - yOff, 20, 20, {220, 20, 40, 20});
    }
    
    // ---- HEX STREAM ANIMATION (background layer) ----
    float hexY = boxY + boxH - 40;
    int hexTick = (int)fmodf(t * 30.0f, 99.0f);
    char hexStr[128];
    snprintf(hexStr, sizeof(hexStr), "0x%04X_HANDSHAKE_SEQUENCE_%02d", 
             (int)(t * 100.0f) % 65535, hexTick);
    float hexW2 = MeasureScaledTextWidth(hexStr, 9);
    DrawScaledText(hexStr, cx - hexW2 / 2, hexY, 9, COLOR_AMBER);
    
    // ---- GLITCHING FOOTER (with offset) ----
    float glitchOffset = sinf(t * 40.0f) * 2.0f;
    const char* footerMsg = "VEKTRAOS v9.5 // COLD SIGNAL // SECURE HANDSHAKE IN PROGRESS";
    float footW = MeasureScaledTextWidth(footerMsg, 9);
    DrawScaledText(footerMsg, cx - footW / 2 + glitchOffset, boxY + boxH - 20, 9, {60, 80, 100, 150});
    
    // ---- CRT SCANLINES (heavy) ----
    DrawCRTScanlines(0, 0, REF_WIDTH, REF_HEIGHT, login.scanlineOffset);
    
    // ---- GLITCH EFFECT (random) ----
    if (IsJittering()) {
        float jx = GetJitterX();
        float jy = GetJitterY();
        if (fabsf(jx) > 1.0f || fabsf(jy) > 1.0f) {
            DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, {0, 0, 0, 30});
        }
    }
    
    // ---- VIGNETTE BORDER ----
    DrawScaledRect(0, 0, REF_WIDTH, 2, {0, 0, 0, 200});
    DrawScaledRect(0, REF_HEIGHT - 2, REF_WIDTH, 2, {0, 0, 0, 200});
    DrawScaledRect(0, 0, 2, REF_HEIGHT, {0, 0, 0, 200});
    DrawScaledRect(REF_WIDTH - 2, 0, 2, REF_HEIGHT, {0, 0, 0, 200});
    
    // ============================================================
    // COMPLETION OUTRO — smooths the handoff to the desktop.
    // Runs on top of everything above for the ~1.4s window the
    // caller holds this screen open after loadingComplete flips true,
    // so by the time it actually switches views the frame is already
    // solid black instead of a hard, jarring cut.
    // ============================================================
    if (login.loadingComplete) {
        const float kOutroDuration = 1.35f;
        float outroT = login.completionTimer / kOutroDuration;
        if (outroT > 1.0f) outroT = 1.0f;
        
        // ---- STAGE A (0.00–0.18): connection flash ----
        float flashSpan = 0.18f;
        if (outroT < flashSpan) {
            float flashT = outroT / flashSpan;
            float flashAlpha = sinf(flashT * PI); // rises then falls, one clean pulse
            DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, Fade(COLOR_CYAN, flashAlpha * 0.30f));
            DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, Fade(WHITE, flashAlpha * flashAlpha * 0.22f));
        }
        
        // ---- STAGE B (0.12–0.62): "ACCESS GRANTED" stamp over the box ----
        float stampStart = 0.12f, stampEnd = 0.62f;
        if (outroT > stampStart) {
            float stampT = (outroT - stampStart) / (stampEnd - stampStart);
            if (stampT > 1.0f) stampT = 1.0f;
            float easeOut = 1.0f - powf(1.0f - stampT, 3.0f);
            float scale = 0.6f + 0.4f * easeOut;
            float alpha = (stampT < 0.85f) ? (stampT / 0.85f) : (1.0f - (stampT - 0.85f) / 0.15f);
            if (alpha < 0.0f) alpha = 0.0f;
            
            const char* stamp = "ACCESS GRANTED";
            float stampSize = 22.0f * scale;
            float stampW = MeasureScaledTextWidth(stamp, stampSize);
            float stampX = cx - stampW / 2.0f;
            float stampY = wheelY - 10.0f;
            
            // Impact rings, like the stamp is "landing"
            for (int ring = 0; ring < 3; ring++) {
                float ringT = stampT - ring * 0.12f;
                if (ringT > 0.0f && ringT < 1.0f) {
                    float ringR = ringT * 140.0f;
                    DrawScaledCircleLines(cx, stampY + 6, ringR, Fade(COLOR_TOXIC, (1.0f - ringT) * 0.4f));
                }
            }
            
            DrawScaledText(stamp, stampX + 1.5f, stampY + 1.5f, stampSize, Fade(COLOR_BLOOD, alpha * 0.3f));
            DrawScaledText(stamp, stampX - 1.5f, stampY - 1.5f, stampSize, Fade(COLOR_CYAN, alpha * 0.3f));
            DrawScaledText(stamp, stampX, stampY, stampSize, Fade(COLOR_TOXIC, alpha));
        }
        
        // ---- STAGE C (0.45–1.00): CRT-style wipe to black ----
        float wipeStart = 0.45f;
        if (outroT > wipeStart) {
            float wipeT = (outroT - wipeStart) / (1.0f - wipeStart);
            if (wipeT > 1.0f) wipeT = 1.0f;
            float easeIn = wipeT * wipeT; // accelerates into the cut
            
            // Two bars closing from top and bottom, like a CRT collapsing
            // to a horizontal line before it powers off.
            float barsH = (REF_HEIGHT * 0.5f) * easeIn;
            DrawScaledRect(0, 0, REF_WIDTH, barsH, COLOR_BLACK);
            DrawScaledRect(0, REF_HEIGHT - barsH, REF_WIDTH, barsH, COLOR_BLACK);
            
            // Bright seam at each closing edge
            float seamAlpha = (1.0f - wipeT) * 0.8f + 0.2f;
            DrawScaledRect(0, barsH - 1.5f, REF_WIDTH, 3.0f, Fade(COLOR_CYAN, seamAlpha));
            DrawScaledRect(0, REF_HEIGHT - barsH - 1.5f, REF_WIDTH, 3.0f, Fade(COLOR_CYAN, seamAlpha));
            
            // Final clean hold to solid black so the eventual switch to
            // the desktop lands on a frame with nothing left to hide.
            if (wipeT > 0.85f) {
                float finalFade = (wipeT - 0.85f) / 0.15f;
                DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, Fade(COLOR_BLACK, finalFade));
            }
        }
    }
}

// ============================================================
// UPDATE
// ============================================================

void UpdateLoginScreen(LoginScreen& login, float dt) {
    login.animTime += dt;
    login.cursorBlinkTimer += dt;
    login.scanlineOffset += dt * 2.0f;
    
    // Cursor blink
    if (login.cursorBlinkTimer >= 0.5f) {
        login.cursorBlinkTimer = 0.0f;
        login.showCursor = !login.showCursor;
    }
    
    // Random glitch
    login.glitchTimer += dt;
    if (login.glitchTimer > 2.0f + (rand() % 100) / 50.0f) {
        login.glitchTimer = 0.0f;
        if ((rand() % 100) < 15) {
            TriggerGlitch(0.3f + (rand() % 100) / 200.0f);
        }
    }
    
    // ============================================================
    // LOADING UPDATE
    // ============================================================
    if (login.isLoading && !login.loadingComplete) {
        login.loadingTimer += dt;
        login.loadingProgress += dt * 0.12f;   // was 0.15f — slightly longer handshake
        
        if (login.loadingProgress > 1.0f) {
            login.loadingProgress = 1.0f;
            login.loadingComplete = true;
        }
        
        // Update status messages based on progress
        if (login.loadingProgress < 0.15f) {
            strcpy(login.loadingStatus, "INITIALIZING UPLINK...");
        } else if (login.loadingProgress < 0.30f) {
            strcpy(login.loadingStatus, "NEGOTIATING HANDSHAKE...");
        } else if (login.loadingProgress < 0.45f) {
            strcpy(login.loadingStatus, "ESTABLISHING SECURE CHANNEL...");
        } else if (login.loadingProgress < 0.60f) {
            strcpy(login.loadingStatus, "EXCHANGING CRYPTOGRAPHIC KEYS...");
        } else if (login.loadingProgress < 0.75f) {
            strcpy(login.loadingStatus, "AUTHENTICATING OPERATOR...");
        } else if (login.loadingProgress < 0.90f) {
            strcpy(login.loadingStatus, "SYNCHRONIZING VFS MEMORY STACKS...");
        } else {
            strcpy(login.loadingStatus, "ESTABLISHING P2P SOCKET...");
        }
    } else if (login.isLoading && login.loadingComplete) {
        // Drives the outro sequence in DrawLoadingScreen (flash / stamp /
        // wipe). The caller (game.cpp) holds the login screen visible for
        // ~1.5s after loadingComplete before switching to the desktop, so
        // this has that whole window to resolve to a clean black frame.
        login.completionTimer += dt;
    }
}

// ============================================================
// MAIN DRAW FUNCTION
// ============================================================

void DrawLoginScreen(const LoginScreen& login) {
    // ============================================================
    // SHOW LOADING SCREEN IF LOADING
    // ============================================================
    if (login.isLoading) {
        DrawLoadingScreen(login);
        return;
    }
    
    float t = login.animTime;
    float cx = REF_WIDTH / 2.0f;
    float cy = REF_HEIGHT / 2.0f;
    
    // ---- BACKGROUND ----
    DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, Color{4, 6, 10, 255});
    
    // ---- ANIMATED GRID ----
    DrawAnimatedGrid(t);
    
    // ---- CRT VIGNETTE ----
    for (int i = 0; i < 8; i++) {
        float size = i * 20.0f;
        DrawScaledRect(size, size, REF_WIDTH - size * 2, REF_HEIGHT - size * 2, 
                       {0, 0, 0, (unsigned char)(4 - i * 0.3f)});
    }
    
    // ---- MAIN CONTAINER ----
    float boxX = 200.0f;
    float boxY = 80.0f;
    float boxW = 500.0f;
    float boxH = 500.0f;
    
    // Glow behind box
    DrawScaledRect(boxX - 20, boxY - 20, boxW + 40, boxH + 40, {0, 220, 240, 10});
    
    // Box background
    DrawScaledRect(boxX, boxY, boxW, boxH, Color{8, 10, 16, 230});
    DrawScaledRectLines(boxX, boxY, boxW, boxH, {0, 220, 240, 60});
    
    // ---- CORNER RETICLES ----
    float retSize = 25.0f;
    Color retCol = {0, 220, 240, (unsigned char)(sinf(t * 0.5f) * 50 + 100)};
    
    // Top-left
    DrawScaledRect(boxX - 2, boxY - 2, retSize, 2, retCol);
    DrawScaledRect(boxX - 2, boxY - 2, 2, retSize, retCol);
    // Top-right
    DrawScaledRect(boxX + boxW - retSize, boxY - 2, retSize, 2, retCol);
    DrawScaledRect(boxX + boxW, boxY - 2, 2, retSize, retCol);
    // Bottom-left
    DrawScaledRect(boxX - 2, boxY + boxH, retSize, 2, retCol);
    DrawScaledRect(boxX - 2, boxY + boxH - retSize, 2, retSize, retCol);
    // Bottom-right
    DrawScaledRect(boxX + boxW - retSize, boxY + boxH, retSize, 2, retCol);
    DrawScaledRect(boxX + boxW, boxY + boxH - retSize, 2, retSize, retCol);
    
    // ---- LOGO ----
    DrawVektraLogo(cx, boxY + 60, 0.7f, t);
    
    // ---- DECORATIVE LINE ----
    float lineY = boxY + 130;
    DrawScaledLine(boxX + 30, lineY, boxX + boxW - 30, lineY, {0, 220, 240, 40});
    
    // ---- SYSTEM STATUS ----
    DrawSystemStatus(boxX + boxW + 30, boxY + 20, t);
    
    // ---- INPUT SECTION ----
    float inputY = boxY + 180;
    float inputW = boxW - 60;
    float inputH = 50.0f;
    float spacing = 30.0f;
    
    // IP Address label
    DrawScaledText("TARGET GATEWAY", boxX + 30, inputY - 18, 11, {100, 150, 200, 200});
    
    // IP Input
    bool ipFocused = (login.selectedField == 0);
    DrawInputBox(boxX + 30, inputY, inputW, inputH, "", 
                 login.ipInputBuffer, ipFocused, login.showCursor,
                 COLOR_CYAN, COLOR_TOXIC);
    inputY += inputH + spacing;
    
    // Handle label
    DrawScaledText("OPERATOR HANDLE", boxX + 30, inputY - 18, 11, {100, 150, 200, 200});
    
    // Handle Input
    bool handleFocused = (login.selectedField == 1);
    DrawInputBox(boxX + 30, inputY, inputW, inputH, "",
                 login.handleBuffer, handleFocused, login.showCursor,
                 COLOR_AMBER, COLOR_CYAN);
    inputY += inputH + spacing + 20;
    
    // ---- CONNECT BUTTON ----
    float btnW = 200;
    float btnH = 48;
    float btnX = cx - btnW / 2;
    float btnY = inputY;
    
    bool btnHover = RefRectHover(btnX, btnY, btnW, btnH, GetRefMousePos());
    float btnPulse = sinf(t * 2.0f) * 0.2f + 0.8f;
    
    Color btnBg = btnHover ? COLOR_BLOOD : Color{20, 25, 45, 200};
    Color btnBorder = btnHover ? COLOR_BLOOD : Color{0, 220, 240, (unsigned char)(btnPulse * 150 + 50)};
    
    DrawScaledRect(btnX, btnY, btnW, btnH, btnBg);
    DrawScaledRectLines(btnX, btnY, btnW, btnH, btnBorder);
    
    // Inner glow
    if (btnHover) {
        DrawScaledRect(btnX + 4, btnY + 4, btnW - 8, btnH - 8, {220, 20, 40, 40});
    }
    
    // Button text
    const char* btnText = "INITIALIZE UPLINK";
    float btnTextW = MeasureScaledTextWidth(btnText, 14);
    Color btnTextCol = btnHover ? COLOR_BLACK : COLOR_TOXIC;
    DrawScaledText(btnText, btnX + (btnW - btnTextW) / 2, btnY + 15, 14, btnTextCol);
    
    // ---- FOOTER ----
    DrawLoginFooter(cx, boxY + boxH - 20, t);
    
    // ---- CRT SCANLINES OVERLAY ----
    DrawCRTScanlines(0, 0, REF_WIDTH, REF_HEIGHT, login.scanlineOffset);
    
    // ---- GLITCH EFFECT ----
    if (IsJittering()) {
        float jx = GetJitterX();
        float jy = GetJitterY();
        if (fabsf(jx) > 1.0f || fabsf(jy) > 1.0f) {
            DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, {0, 0, 0, 30});
        }
    }
    
    // ---- VIGNETTE BORDER ----
    DrawScaledRect(0, 0, REF_WIDTH, 2, {0, 0, 0, 200});
    DrawScaledRect(0, REF_HEIGHT - 2, REF_WIDTH, 2, {0, 0, 0, 200});
    DrawScaledRect(0, 0, 2, REF_HEIGHT, {0, 0, 0, 200});
    DrawScaledRect(REF_WIDTH - 2, 0, 2, REF_HEIGHT, {0, 0, 0, 200});
}

// ============================================================
// INPUT HANDLING
// ============================================================

bool HandleLoginInput(LoginScreen& login) {
    // ============================================================
    // DON'T PROCESS INPUT DURING LOADING
    // ============================================================
    if (login.isLoading) {
        return false;
    }
    
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    // ---- MOUSE CLICKS ----
    if (clicked) {
        // Check IP box
        float ipX = REF_WIDTH / 2.0f - 220 + 30;
        float ipY = 80 + 180;
        float ipW = 500 - 60;
        float ipH = 50;
        
        if (RefRectHover(ipX, ipY, ipW, ipH, refMouse)) {
            login.selectedField = 0;
            login.ipBoxFocused = true;
            login.cursorBlinkTimer = 0.0f;
            login.showCursor = true;
        } else {
            // Check Handle box
            float handleY = ipY + ipH + 30;
            if (RefRectHover(ipX, handleY, ipW, ipH, refMouse)) {
                login.selectedField = 1;
                login.ipBoxFocused = false;
                login.cursorBlinkTimer = 0.0f;
                login.showCursor = true;
            } else {
                // Check Connect button
                float btnX = REF_WIDTH / 2.0f - 100;
                float btnY = handleY + ipH + 20;
                float btnW = 200;
                float btnH = 48;
                
                if (RefRectHover(btnX, btnY, btnW, btnH, refMouse)) {
                    if (strlen(login.ipInputBuffer) > 0) {
                        StartLoading(login);
                        return true;
                    }
                }
            }
        }
    }
    
    // ---- KEYBOARD INPUT ----
    if (login.selectedField == 0) {
        // IP input
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= '0' && key <= '9') || key == '.' || key == ':') {
                size_t len = strlen(login.ipInputBuffer);
                if (len < sizeof(login.ipInputBuffer) - 1) {
                    login.ipInputBuffer[len] = (char)key;
                    login.ipInputBuffer[len + 1] = '\0';
                }
            }
            key = GetCharPressed();
        }
        
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = (int)strlen(login.ipInputBuffer);
            if (len > 0) login.ipInputBuffer[len - 1] = '\0';
        }
        
        if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_ENTER)) {
            login.selectedField = 1;
            login.ipBoxFocused = false;
        }
    } else if (login.selectedField == 1) {
        // Handle input
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z') || 
                (key >= '0' && key <= '9') || key == '_' || key == '-') {
                size_t len = strlen(login.handleBuffer);
                if (len < sizeof(login.handleBuffer) - 1) {
                    login.handleBuffer[len] = (char)key;
                    login.handleBuffer[len + 1] = '\0';
                }
            }
            key = GetCharPressed();
        }
        
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = (int)strlen(login.handleBuffer);
            if (len > 0) login.handleBuffer[len - 1] = '\0';
        }
        
        if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_UP)) {
            login.selectedField = 0;
            login.ipBoxFocused = true;
        }
        
        if (IsKeyPressed(KEY_ENTER)) {
            if (strlen(login.ipInputBuffer) > 0) {
                StartLoading(login);
                return true;
            }
        }
    }
    
    return false;
}

// ============================================================
// GETTERS
// ============================================================

const char* GetLoginIP(const LoginScreen& login) {
    return login.ipInputBuffer;
}

const char* GetLoginHandle(const LoginScreen& login) {
    return login.handleBuffer;
}