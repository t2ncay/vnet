#include "raid.h"
#include "../../shared/vnet.h"
#include "../render.h"
#include "../game.h"
#include <cstdio>
#include <cmath>

extern Player g_player;

// ============================================================
// RAID OVERLAY (Flashing border + badge)
// ============================================================

void DrawRaidOverlay(void) {
    if (!g_player.raidActive) return;
    
    float t = (float)GetTime();
    float pulse = sinf(t * 8.0f) * 0.5f + 0.5f;
    
    // Jitter effect (subtle during raid)
    float jx = (pulse > 0.85f) ? sinf(t * 40.0f) * 3.0f : 0.0f;
    float jy = (pulse > 0.85f) ? cosf(t * 35.0f) * 2.0f : 0.0f;
    
    // ---- RED ALERT BORDER ----
    int borderWidth = 4;
    Color alertColor = {220, 20, 40, (unsigned char)(pulse * 200 + 55)};
    DrawScaledRect(0, 0, REF_WIDTH, borderWidth, alertColor);
    DrawScaledRect(0, REF_HEIGHT - borderWidth, REF_WIDTH, borderWidth, alertColor);
    DrawScaledRect(0, 0, borderWidth, REF_HEIGHT, alertColor);
    DrawScaledRect(REF_WIDTH - borderWidth, 0, borderWidth, REF_HEIGHT, alertColor);
    
    // ---- CORNER GLOW RETICLES ----
    float glowPulse = sinf(t * 6.0f) * 0.3f + 0.7f;
    Color glowCol = {220, 20, 40, (unsigned char)(glowPulse * 200 + 55)};
    float retSize = 40.0f;
    float retOffset = 10.0f;
    
    // Top-left
    DrawScaledRect(retOffset, retOffset, retSize, 3, glowCol);
    DrawScaledRect(retOffset, retOffset, 3, retSize, glowCol);
    // Top-right
    DrawScaledRect(REF_WIDTH - retOffset - retSize, retOffset, retSize, 3, glowCol);
    DrawScaledRect(REF_WIDTH - retOffset, retOffset, 3, retSize, glowCol);
    // Bottom-left
    DrawScaledRect(retOffset, REF_HEIGHT - retOffset - retSize, retSize, 3, glowCol);
    DrawScaledRect(retOffset, REF_HEIGHT - retOffset, 3, retSize, glowCol);
    // Bottom-right
    DrawScaledRect(REF_WIDTH - retOffset - retSize, REF_HEIGHT - retOffset - retSize, retSize, 3, glowCol);
    DrawScaledRect(REF_WIDTH - retOffset, REF_HEIGHT - retOffset, 3, retSize, glowCol);
    
    // ---- TOP BADGE ----
    float badgeX = 20 + jx;
    float badgeY = 60 + jy;
    float badgeW = 200.0f;
    float badgeH = 32.0f;
    
    DrawScaledRect(badgeX, badgeY, badgeW, badgeH, {220, 20, 40, 220});
    DrawScaledRectLines(badgeX, badgeY, badgeW, badgeH, {255, 50, 50, 200});
    DrawScaledText("⚡ FEDERAL E-RAID", badgeX + 20, badgeY + 8, 14, COLOR_BLACK);
    
    // ---- TIMER (right side) ----
    if (g_player.raidStage == RAID_STAGE_CHOICE || g_player.raidStage == RAID_STAGE_MINIGAME) {
        float remaining = GetRaidRemainingTime();
        char timerStr[32];
        snprintf(timerStr, sizeof(timerStr), "⏱ %.1fs", remaining);
        float timerW = MeasureScaledTextWidth(timerStr, 16);
        Color timerCol = remaining < 3.0f ? COLOR_BLOOD : COLOR_TOXIC;
        DrawScaledText(timerStr, REF_WIDTH - timerW - 30, badgeY + 8, 16, timerCol);
        
        // Timer bar (subtle)
        float barX = REF_WIDTH - 200;
        float barY = badgeY + badgeH + 8;
        float barW = 180;
        float barH = 4;
        float ratio = remaining / 10.0f;
        if (ratio < 0.0f) ratio = 0.0f;
        if (ratio > 1.0f) ratio = 1.0f;
        
        DrawScaledRect(barX, barY, barW, barH, {30, 35, 50, 150});
        DrawScaledRect(barX, barY, barW * ratio, barH, 
                      remaining < 3.0f ? COLOR_BLOOD : COLOR_TOXIC);
    }
    
    // ---- STAGE INDICATOR ----
    const char* stageText = "";
    switch (g_player.raidStage) {
        case RAID_STAGE_ALERT: stageText = "⚠ ALERT: FEDERAL SWEEP DETECTED"; break;
        case RAID_STAGE_CHOICE: stageText = "🎯 CHOOSE EVASION STRATEGY"; break;
        case RAID_STAGE_MINIGAME: 
            switch (g_player.raidType) {
                case RAID_EVADE: stageText = "🕵️ EVADE: Deploy decoy nodes"; break;
                case RAID_ESCAPE: stageText = "🏃 ESCAPE: Port migration"; break;
                case RAID_BURN: stageText = "🔥 BURN: Scorched earth purge"; break;
            }
            break;
        case RAID_STAGE_RESULT: 
            stageText = g_player.raidSuccess ? "✅ RAID ESCAPED!" : "💀 RAID BREACHED!";
            break;
        default: return;
    }
    
    float stageW = MeasureScaledTextWidth(stageText, 13);
    DrawScaledText(stageText, (REF_WIDTH - stageW) / 2, 100, 13, 
                   g_player.raidStage == RAID_STAGE_RESULT ? 
                   (g_player.raidSuccess ? COLOR_TOXIC : COLOR_BLOOD) : COLOR_AMBER);
    
    // ---- SCANLINE EFFECT (subtle during raid) ----
    for (int i = 0; i < (int)REF_HEIGHT; i += 6) {
        float scanY = i + fmodf(t * 120.0f, 6.0f);
        unsigned char alpha = (unsigned char)((pulse > 0.7f) ? 8 : 4);
        DrawScaledRect(0, scanY, REF_WIDTH, 1, {0, 0, 0, alpha});
    }
}

// ============================================================
// INTRUDER DETECTOR WINDOW
// ============================================================

void DrawIntruderDetector(float x, float y, float w, float h) {
    float t = (float)GetTime();
    float pulse = sinf(t * 8.0f) * 0.5f + 0.5f;
    float pulseSlow = sinf(t * 2.0f) * 0.3f + 0.7f;
    
    // ---- WINDOW BACKGROUND ----
    DrawScaledRect(x, y, w, h, {6, 4, 8, 245});
    DrawScaledRectLines(x, y, w, h, {220, 20, 40, (unsigned char)(pulse * 200 + 55)});
    
    // ---- CORNER RETICLES ----
    Color retCol = {220, 20, 40, (unsigned char)(pulse * 150 + 50)};
    float retSize = 20.0f;
    DrawScaledRect(x + 4, y + 4, retSize, 2, retCol);
    DrawScaledRect(x + 4, y + 4, 2, retSize, retCol);
    DrawScaledRect(x + w - 4 - retSize, y + 4, retSize, 2, retCol);
    DrawScaledRect(x + w - 4, y + 4, 2, retSize, retCol);
    DrawScaledRect(x + 4, y + h - 4 - retSize, retSize, 2, retCol);
    DrawScaledRect(x + 4, y + h - 4, 2, retSize, retCol);
    DrawScaledRect(x + w - 4 - retSize, y + h - 4 - retSize, retSize, 2, retCol);
    DrawScaledRect(x + w - 4, y + h - 4, 2, retSize, retCol);
    
    // ---- HEADER ----
    float headerH = 36.0f;
    DrawScaledRect(x, y, w, headerH, {220, 20, 40, (unsigned char)(pulse * 100 + 50)});
    DrawScaledLine(x, y + headerH, x + w, y + headerH, {220, 20, 40, 200});
    
    // Title with pulse
    Color titleCol = {255, 255, 255, (unsigned char)(pulse * 200 + 55)};
    DrawScaledText("INTRUDER DETECTOR v3.2", x + 16, y + 9, 13, titleCol);
    
    // Status indicator (blinking red)
    float blink = fmodf(t, 0.8f) < 0.4f;
    Color statusCol = blink ? COLOR_BLOOD : Color{220, 20, 40, 80};
    DrawScaledRect(x + w - 40, y + 8, 16, 16, statusCol);
    DrawScaledRectLines(x + w - 40, y + 8, 16, 16, COLOR_BLOOD);
    
    // ---- CONTENT ----
    float contentY = y + headerH + 12;
    
    // Alert message
    float alertPulse = sinf(t * 10.0f) * 0.5f + 0.5f;
    Color alertCol = {255, 50, 50, (unsigned char)(alertPulse * 200 + 55)};
    DrawScaledText("⚠ FEDERAL E-RAID DETECTED", x + 20, contentY, 16, alertCol);
    contentY += 28;
    
    // Port info
    char portInfo[64];
    snprintf(portInfo, sizeof(portInfo), "TARGET PORT: %d", g_player.port);
    DrawScaledText(portInfo, x + 20, contentY, 12, COLOR_AMBER);
    contentY += 22;
    
    // Trace threat
    char traceInfo[64];
    snprintf(traceInfo, sizeof(traceInfo), "TRACE THREAT: %d%%", g_player.traceLevel);
    DrawScaledText(traceInfo, x + 20, contentY, 12, 
                   g_player.traceLevel > 70 ? COLOR_BLOOD : COLOR_AMBER);
    contentY += 22;
    
    // Time window
    float remaining = GetRaidRemainingTime();
    char timeInfo[64];
    if (remaining > 0.0f) {
        snprintf(timeInfo, sizeof(timeInfo), "RESPONSE WINDOW: %.1fs", remaining);
    } else {
        snprintf(timeInfo, sizeof(timeInfo), "RESPONSE WINDOW: EXPIRED");
    }
    DrawScaledText(timeInfo, x + 20, contentY, 12, 
                   remaining < 3.0f ? COLOR_BLOOD : COLOR_TOXIC);
    contentY += 28;
    
    // ---- PROGRESS BAR (time remaining) ----
    float barX = x + 20;
    float barY = contentY;
    float barW = w - 40;
    float barH = 8.0f;
    float ratio = 0.0f;
    if (g_player.raidStage == RAID_STAGE_CHOICE || g_player.raidStage == RAID_STAGE_MINIGAME) {
        ratio = remaining / 10.0f;
        if (ratio < 0.0f) ratio = 0.0f;
        if (ratio > 1.0f) ratio = 1.0f;
    } else {
        ratio = 0.5f + sinf(t * 0.5f) * 0.3f;  // Pulsing during alert
    }
    
    DrawScaledRect(barX, barY, barW, barH, {30, 35, 50, 200});
    DrawScaledRectLines(barX, barY, barW, barH, {50, 55, 70, 150});
    DrawScaledRect(barX, barY, barW * ratio, barH, 
                   remaining < 3.0f ? COLOR_BLOOD : COLOR_TOXIC);
    contentY += 18;
    
    // ---- INSTRUCTION ----
    if (g_player.raidStage == RAID_STAGE_CHOICE) {
        DrawScaledText("TYPE 'raid 1', 'raid 2', or 'raid 3' TO RESPOND", 
                       x + 20, contentY, 11, COLOR_CYAN);
        contentY += 22;
        DrawScaledText("  1. EVADE   - Deploy decoy nodes", x + 30, contentY, 10, COLOR_GHOST);
        contentY += 18;
        DrawScaledText("  2. ESCAPE  - Port migration", x + 30, contentY, 10, COLOR_GHOST);
        contentY += 18;
        DrawScaledText("  3. BURN    - Scorched earth purge", x + 30, contentY, 10, COLOR_GHOST);
    } else if (g_player.raidStage == RAID_STAGE_MINIGAME) {
        switch (g_player.raidType) {
            case RAID_EVADE:
                DrawScaledText("TYPE: decoy <node> <port>", x + 20, contentY, 11, COLOR_TOXIC);
                contentY += 20;
                DrawScaledText("Example: decoy cult.vnet 8080", x + 30, contentY, 10, COLOR_GHOST);
                break;
            case RAID_ESCAPE:
                DrawScaledText("TYPE: patch <port>", x + 20, contentY, 11, COLOR_TOXIC);
                contentY += 20;
                DrawScaledText("Example: patch 8050", x + 30, contentY, 10, COLOR_GHOST);
                break;
            case RAID_BURN:
                DrawScaledText("TYPE: purge <target>", x + 20, contentY, 11, COLOR_TOXIC);
                contentY += 20;
                DrawScaledText("Example: purge access.log", x + 30, contentY, 10, COLOR_GHOST);
                break;
        }
    } else if (g_player.raidStage == RAID_STAGE_RESULT) {
        const char* resultText = g_player.raidSuccess ? 
            "✅ RAID ESCAPED SUCCESSFULLY!" : 
            "💀 FEDERAL RAID BREACHED YOUR NODE!";
        Color resultCol = g_player.raidSuccess ? COLOR_TOXIC : COLOR_BLOOD;
        DrawScaledText(resultText, x + 20, contentY, 14, resultCol);
    }
    
    // ---- FOOTER ----
    float footerY = y + h - 24;
    DrawScaledLine(x + 8, footerY, x + w - 8, footerY, {220, 20, 40, (unsigned char)(pulse * 100 + 50)});
    DrawScaledText("FEDERAL INTELLIGENCE DIVISION // COMSEC", x + 20, footerY + 6, 8, {80, 90, 110, 150});
    
    // ---- SCANLINES ----
    for (int i = 0; i < (int)h; i += 4) {
        float scanY = y + i + fmodf(t * 60.0f, 4.0f);
        DrawScaledRect(x, scanY, w, 1, {0, 0, 0, 4});
    }
    
    // ---- GLITCH EFFECT ----
    if (pulse > 0.92f) {
        float glitchX = x + rand() % (int)w;
        DrawScaledRect(glitchX, y, 20 + rand() % 40, h, {220, 20, 40, 15});
    }
}