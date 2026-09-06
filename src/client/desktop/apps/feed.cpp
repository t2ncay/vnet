#include "../desktop.h"
#include "../../render.h"
#include "../../../shared/vnet.h"

void Desktop::DrawFeed(const AppWindow& win) {
    float cx = win.x + 4;
    float cy = win.y + m_windowTitleHeight + 4;
    float cw = win.w - 8;
    float ch = win.h - m_windowTitleHeight - 8;
    
    float t = (float)GetTime();
    float pulse = sinf(t * 3.0f) * 0.5f + 0.5f;
    
    // ---- BACKGROUND ----
    DrawScaledRect(cx, cy, cw, ch, Color{6, 8, 14, 255});
    DrawScaledRectLines(cx, cy, cw, ch, COLOR_BORDER);
    
    // ---- HEADER WITH LIVE INDICATOR ----
    float headerH = 48.0f;
    DrawScaledRect(cx, cy, cw, headerH, Color{14, 18, 28, 255});
    DrawScaledLine(cx, cy + headerH, cx + cw, cy + headerH, Color{30, 35, 50, 150});
    
    // Title
    DrawScaledText("◈ SYSTEM FEED", cx + 16, cy + 14, 14, COLOR_AMBER);
    
    // LIVE indicator with pulse
    float livePulse = sinf(t * 4.0f) * 0.3f + 0.7f;
    Color liveCol = {40, 240, 100, (unsigned char)(livePulse * 200 + 55)};
    DrawScaledRect(cx + cw - 70, cy + 14, 8, 8, liveCol);
    DrawScaledText("LIVE", cx + cw - 56, cy + 13, 10, COLOR_TOXIC);
    
    // Feed counter
    char counterStr[32];
    snprintf(counterStr, sizeof(counterStr), "MSG: %d", g_feedLogCount);
    DrawScaledText(counterStr, cx + cw - 130, cy + 13, 10, COLOR_GHOST);
    
    // ---- CONTENT LAYOUT ----
    float contentY = cy + headerH + 6.0f;
    float contentH = ch - headerH - 10.0f;
    float leftPanelW = cw * 0.70f;
    float rightPanelW = cw - leftPanelW - 12.0f;
    
    // ---- LEFT PANEL: FEED LOGS ----
    float logX = cx + 6;
    float logY = contentY;
    float logW = leftPanelW - 4;
    float logH = contentH - 4;
    
    // Panel background with subtle scanlines
    DrawScaledRect(logX, logY, logW, logH, Color{8, 10, 18, 220});
    DrawScaledRectLines(logX, logY, logW, logH, Color{30, 35, 50, 100});
    
    // Scanline overlay on log panel
    for (int i = 0; i < (int)logH; i += 4) {
        float scanY = logY + i + fmodf(t * 30.0f, 4.0f);
        DrawScaledRect(logX, scanY, logW, 1, {0, 0, 0, 4});
    }
    
    // ---- FEED LOGS WITH SCROLL ----
    BeginScissorMode((int)SX(logX + 4), (int)SY(logY + 4), 
                     (int)((logW - 8) * g_uiScale), (int)((logH - 8) * g_uiScale));
    
    int total = g_feedLogCount;
    int visibleLines = (int)((logH - 16) / 20.0f);
    int startIdx = total - visibleLines;
    if (startIdx < 0) startIdx = 0;
    
    // Reverse scroll: positive scroll moves up
    int scrollLines = (int)(g_player.feedScroll / 20.0f);
    startIdx += scrollLines;
    if (startIdx < 0) startIdx = 0;
    if (startIdx > total) startIdx = total;
    
    for (int i = startIdx; i < total; i++) {
        int row = i - startIdx;
        float y = logY + 8 + row * 20.0f;
        if (y > logY + logH - 12) break;
        
        const char* txt = g_feedLogs[i];
        
        // Truncate long messages
        char truncated[192];
        strncpy(truncated, txt, 120);
        truncated[120] = '\0';
        
        // Determine color based on message type
        Color col = COLOR_GHOST;
        Color barCol = {0, 0, 0, 0};
        bool hasBar = false;
        
        if (strstr(txt, "[CHAT]")) {
            col = COLOR_TOXIC;
            barCol = {40, 240, 100, 80};
            hasBar = true;
        } else if (strstr(txt, "[WHISPER FROM")) {
            col = COLOR_AMBER;
            barCol = {255, 150, 0, 80};
            hasBar = true;
        } else if (strstr(txt, "[WHISPER TO")) {
            col = COLOR_CYAN;
            barCol = {0, 220, 240, 80};
            hasBar = true;
        } else if (strstr(txt, "[DOS ATTACK]") || strstr(txt, "[TRACE SPIKE]")) {
            col = COLOR_BLOOD;
            barCol = {220, 20, 40, 100};
            hasBar = true;
        } else if (strstr(txt, "[OVERLOAD]")) {
            col = COLOR_BLOOD;
            barCol = {220, 20, 40, 120};
            hasBar = true;
        } else if (strstr(txt, "[GRID BLACKOUT]") || strstr(txt, "[SYSTEM OVERRIDE]")) {
            col = COLOR_AMBER;
            barCol = {255, 150, 0, 150};
            hasBar = true;
        } else if (strstr(txt, "[FEDERAL E-RAID]")) {
            col = COLOR_BLOOD;
            barCol = {255, 0, 50, 150};
            hasBar = true;
        } else if (strstr(txt, "[MINER]") || strstr(txt, "[WHALE ALERT]")) {
            col = COLOR_TOXIC;
            barCol = {40, 240, 100, 60};
            hasBar = true;
        } else if (strstr(txt, "[FEED_INIT]") || strstr(txt, "[FEED]")) {
            col = COLOR_CYAN;
            barCol = {0, 220, 240, 40};
            hasBar = true;
        }
        
        // Accent bar on the left
        if (hasBar) {
            DrawScaledRect(logX + 4, y - 2, 3, 16, barCol);
        }
        
        // Message text
        float textX = hasBar ? logX + 12 : logX + 8;
        DrawScaledText(truncated, textX, y, 10, col);
        
        // Time stamp (fake)
        char timeStamp[12];
        int hours = (i * 7 + 13) % 24;
        int mins = (i * 13 + 42) % 60;
        snprintf(timeStamp, sizeof(timeStamp), "%02d:%02d", hours, mins);
        float timeW = MeasureScaledTextWidth(timeStamp, 8);
        DrawScaledText(timeStamp, logX + logW - timeW - 8, y + 1, 8, {80, 90, 110, 120});
    }
    
    EndScissorMode();
    
    // ---- SCROLL INDICATOR ----
    if (total > visibleLines) {
        float scrollRatio = (float)startIdx / (float)(total - visibleLines);
        float indicatorY = logY + 8 + scrollRatio * (logH - 24);
        DrawScaledRect(logX + logW - 6, indicatorY, 3, 16, {80, 90, 110, 150});
    }
    
    // ---- RIGHT PANEL: THREAT RADAR & STATS ----
    float rightX = cx + leftPanelW + 8;
    float rightY = contentY;
    float rightW = rightPanelW - 4;
    float rightH = contentH - 4;
    
    // Panel background
    DrawScaledRect(rightX, rightY, rightW, rightH, Color{8, 10, 18, 220});
    DrawScaledRectLines(rightX, rightY, rightW, rightH, Color{30, 35, 50, 100});
    
    float panelY = rightY + 8;
    float panelX = rightX + 8;
    float panelW = rightW - 16;
    
    // ---- SECTION: THREAT RADAR ----
    DrawScaledText("⚡ THREAT RADAR", panelX, panelY, 11, COLOR_BLOOD);
    DrawScaledLine(panelX, panelY + 16, panelX + panelW, panelY + 16, Color{30, 35, 50, 100});
    panelY += 24;
    
    // Trace level gauge
    DrawScaledText("TRACE", panelX, panelY + 2, 9, COLOR_CYAN);
    float traceBarX = panelX + 50;
    float traceBarY = panelY + 2;
    float traceBarW = panelW - 54;
    float traceBarH = 12.0f;
    
    DrawScaledRect(traceBarX, traceBarY, traceBarW, traceBarH, Color{12, 15, 20, 255});
    DrawScaledRectLines(traceBarX, traceBarY, traceBarW, traceBarH, Color{30, 35, 50, 100});
    
    float traceFill = (g_player.traceLevel / 100.0f) * traceBarW;
    if (traceFill < 2.0f) traceFill = 2.0f;
    Color traceCol = g_player.traceLevel > 70 ? COLOR_BLOOD : COLOR_AMBER;
    DrawScaledRect(traceBarX, traceBarY, traceFill, traceBarH, traceCol);
    
    // Trace percentage
    char tracePct[16];
    snprintf(tracePct, sizeof(tracePct), "%d%%", g_player.traceLevel);
    float pctW = MeasureScaledTextWidth(tracePct, 9);
    DrawScaledText(tracePct, traceBarX + traceBarW - pctW - 4, traceBarY + 1, 9, 
                   g_player.traceLevel > 50 ? COLOR_BLACK : COLOR_GHOST);
    
    panelY += 20;
    
    // ICE Shields
    DrawScaledText("ICE", panelX, panelY + 2, 9, COLOR_CYAN);
    float iceBarX = panelX + 30;
    float iceBarY = panelY + 2;
    float iceBarW = panelW - 34;
    float iceBarH = 12.0f;
    
    DrawScaledRect(iceBarX, iceBarY, iceBarW, iceBarH, Color{12, 15, 20, 255});
    DrawScaledRectLines(iceBarX, iceBarY, iceBarW, iceBarH, Color{30, 35, 50, 100});
    
    float iceFill = (g_player.iceShields / 3.0f) * iceBarW;
    if (iceFill < 2.0f) iceFill = 2.0f;
    DrawScaledRect(iceBarX, iceBarY, iceFill, iceBarH, COLOR_CYAN);
    
    char iceText[16];
    snprintf(iceText, sizeof(iceText), "%d/3", g_player.iceShields);
    float iceW = MeasureScaledTextWidth(iceText, 9);
    DrawScaledText(iceText, iceBarX + iceBarW - iceW - 4, iceBarY + 1, 9, COLOR_BLACK);
    
    panelY += 20;
    
    // CRT Heat
    DrawScaledText("HEAT", panelX, panelY + 2, 9, COLOR_CYAN);
    float heatBarX = panelX + 38;
    float heatBarY = panelY + 2;
    float heatBarW = panelW - 42;
    float heatBarH = 12.0f;
    
    DrawScaledRect(heatBarX, heatBarY, heatBarW, heatBarH, Color{12, 15, 20, 255});
    DrawScaledRectLines(heatBarX, heatBarY, heatBarW, heatBarH, Color{30, 35, 50, 100});
    
    float heatFill = ((g_player.crtHeat - 35.0f) / 65.0f) * heatBarW;
    if (heatFill < 0.0f) heatFill = 0.0f;
    if (heatFill < 2.0f && g_player.crtHeat > 35.0f) heatFill = 2.0f;
    Color heatCol = g_player.crtHeat > 75.0f ? COLOR_BLOOD : COLOR_AMBER;
    DrawScaledRect(heatBarX, heatBarY, heatFill, heatBarH, heatCol);
    
    char heatText[16];
    snprintf(heatText, sizeof(heatText), "%.0f°C", g_player.crtHeat);
    float heatW = MeasureScaledTextWidth(heatText, 9);
    DrawScaledText(heatText, heatBarX + heatBarW - heatW - 4, heatBarY + 1, 9, 
                   g_player.crtHeat > 60.0f ? COLOR_BLACK : COLOR_GHOST);
    
    panelY += 24;
    DrawScaledLine(panelX, panelY, panelX + panelW, panelY, Color{30, 35, 50, 80});
    panelY += 8;
    
    // ---- SECTION: VCOIN WALLET ----
    DrawScaledText("💰 WALLET", panelX, panelY, 11, COLOR_TOXIC);
    DrawScaledLine(panelX, panelY + 16, panelX + panelW, panelY + 16, Color{30, 35, 50, 100});
    panelY += 24;
    
    char vcoinDisplay[32];
    snprintf(vcoinDisplay, sizeof(vcoinDisplay), "%.2f VCOIN", g_player.vcoin);
    float vcoinW = MeasureScaledTextWidth(vcoinDisplay, 16);
    DrawScaledText(vcoinDisplay, panelX + (panelW - vcoinW) / 2, panelY, 16, COLOR_TOXIC);
    panelY += 28;
    
    // VCOIN progress to 25 (victory threshold)
    float vcoinGoal = g_player.vcoin / 25.0f;
    if (vcoinGoal > 1.0f) vcoinGoal = 1.0f;
    float goalBarX = panelX;
    float goalBarY = panelY;
    float goalBarW = panelW;
    float goalBarH = 6.0f;
    
    DrawScaledRect(goalBarX, goalBarY, goalBarW, goalBarH, Color{12, 15, 20, 255});
    DrawScaledRectLines(goalBarX, goalBarY, goalBarW, goalBarH, Color{30, 35, 50, 100});
    
    float goalFill = vcoinGoal * goalBarW;
    if (goalFill < 2.0f) goalFill = 2.0f;
    DrawScaledRect(goalBarX, goalBarY, goalFill, goalBarH, COLOR_TOXIC);
    
    char goalText[32];
    snprintf(goalText, sizeof(goalText), "GOAL: %.1f/25.0 VCOIN", g_player.vcoin);
    float goalW = MeasureScaledTextWidth(goalText, 8);
    DrawScaledText(goalText, panelX + (panelW - goalW) / 2, goalBarY + 10, 8, COLOR_GHOST);
    
    panelY += 32;
    DrawScaledLine(panelX, panelY, panelX + panelW, panelY, Color{30, 35, 50, 80});
    panelY += 8;
    
    // ---- SECTION: ACTIVE NODES ----
    DrawScaledText("🔗 ACTIVE NODES", panelX, panelY, 11, COLOR_CYAN);
    DrawScaledLine(panelX, panelY + 16, panelX + panelW, panelY + 16, Color{30, 35, 50, 100});
    panelY += 24;
    
    int nodeCount = g_player.assignedCount;
    char nodeStr[32];
    snprintf(nodeStr, sizeof(nodeStr), "%d / 54 SITES", nodeCount);
    float nodeW = MeasureScaledTextWidth(nodeStr, 12);
    DrawScaledText(nodeStr, panelX + (panelW - nodeW) / 2, panelY, 12, 
                   nodeCount >= 20 ? COLOR_TOXIC : COLOR_AMBER);
    panelY += 22;
    
    // Node progress bar
    float nodeBarX = panelX;
    float nodeBarY = panelY;
    float nodeBarW = panelW;
    float nodeBarH = 8.0f;
    
    DrawScaledRect(nodeBarX, nodeBarY, nodeBarW, nodeBarH, Color{12, 15, 20, 255});
    DrawScaledRectLines(nodeBarX, nodeBarY, nodeBarW, nodeBarH, Color{30, 35, 50, 100});
    
    float nodeFill = (nodeCount / 54.0f) * nodeBarW;
    if (nodeFill < 2.0f) nodeFill = 2.0f;
    DrawScaledRect(nodeBarX, nodeBarY, nodeFill, nodeBarH, 
                   nodeCount >= 20 ? COLOR_TOXIC : COLOR_CYAN);
    
    panelY += 16;
    
    // ---- SECTION: RECENT ACTIVITY TICKER ----
    panelY += 8;
    DrawScaledLine(panelX, panelY, panelX + panelW, panelY, Color{30, 35, 50, 80});
    panelY += 8;
    
    DrawScaledText("📡 ACTIVITY", panelX, panelY, 10, COLOR_AMBER);
    panelY += 18;
    
    // Show last 3 feed entries as quick preview
    int previewStart = g_feedLogCount - 3;
    if (previewStart < 0) previewStart = 0;
    for (int i = previewStart; i < g_feedLogCount; i++) {
        float y = panelY + (i - previewStart) * 16.0f;
        if (y > rightY + rightH - 12) break;
        
        const char* txt = g_feedLogs[i];
        char preview[80];
        strncpy(preview, txt, 50);
        preview[50] = '\0';
        
        // Shorten long messages
        if (strlen(preview) > 45) {
            preview[44] = '.';
            preview[45] = '.';
            preview[46] = '\0';
        }
        
        Color previewCol = COLOR_GHOST;
        if (strstr(txt, "[CHAT]")) previewCol = COLOR_TOXIC;
        else if (strstr(txt, "[WHISPER")) previewCol = COLOR_AMBER;
        else if (strstr(txt, "[DOS") || strstr(txt, "[TRACE")) previewCol = COLOR_BLOOD;
        else if (strstr(txt, "[OVERLOAD]")) previewCol = COLOR_BLOOD;
        
        DrawScaledText(preview, panelX, y, 8, previewCol);
    }
    
    // ---- BOTTOM STATUS BAR ----
    float statusY = cy + ch - 22;
    DrawScaledRect(cx, statusY, cw, 22, Color{14, 18, 28, 220});
    DrawScaledLine(cx, statusY, cx + cw, statusY, Color{30, 35, 50, 100});
    
    char statusText[128];
    snprintf(statusText, sizeof(statusText), "UPTIME: %.0fs | FEED: %d | TRACE: %d%% | ICE: %d/3 | VCOIN: %.2f",
             g_player.runTime, g_feedLogCount, g_player.traceLevel, g_player.iceShields, g_player.vcoin);
    DrawScaledText(statusText, cx + 12, statusY + 5, 8, COLOR_GHOST);
}