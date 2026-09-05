#include "../../desktop.h"
#include "../../render.h"
#include "../../../shared/vnet.h"
#include "../../vnet_client.h"

void Desktop::DrawTerminal(const AppWindow& win) {
    float cx = win.x + 4;
    float cy = win.y + m_windowTitleHeight + 4;
    float cw = win.w - 8;
    float ch = win.h - m_windowTitleHeight - 8;
    
    // ---- TERMINAL BACKGROUND ----
    DrawScaledRect(cx, cy, cw, ch, Color{8, 10, 14, 255});  // Deeper dark background
    
    // ---- TOP BAR WITH TABS ----
    float topBarH = 30.0f;
    DrawScaledRect(cx, cy, cw, topBarH, Color{16, 18, 24, 255});
    DrawScaledLine(cx, cy + topBarH, cx + cw, cy + topBarH, Color{30, 35, 45, 200});
    
    // Tab bar
    float tabX = cx + 12;
    float tabW = 120.0f;
    float tabH = 22.0f;
    float tabY = cy + 4;
    
    // Active tab
    DrawScaledRect(tabX, tabY, tabW, tabH, Color{22, 26, 34, 255});
    DrawScaledRectLines(tabX, tabY, tabW, tabH, Color{40, 220, 120, 150});
    DrawScaledText("● TERMINAL", tabX + 8, tabY + 4, 9, COLOR_TOXIC);
    
    // Inactive tab (darker)
    tabX += tabW + 4;
    DrawScaledRect(tabX, tabY, 100, tabH, Color{12, 14, 20, 255});
    DrawScaledRectLines(tabX, tabY, 100, tabH, Color{30, 35, 45, 100});
    DrawScaledText("○ SHELL", tabX + 8, tabY + 4, 9, COLOR_GHOST);
    
    // ---- STATUS INDICATORS (right side) ----
    float indicatorX = cx + cw - 12;
    DrawScaledText("●", indicatorX - 80, cy + 8, 8, COLOR_TOXIC);  // Green dot
    DrawScaledText("ONLINE", indicatorX - 65, cy + 6, 8, COLOR_GHOST);
    
    // Port indicator
    char portStr[32];
    snprintf(portStr, sizeof(portStr), "PORT %d", g_player.port);
    DrawScaledText(portStr, indicatorX - 20, cy + 6, 8, COLOR_CYAN);
    
    // ---- LOG AREA ----
    float logY = cy + topBarH + 4;
    float logH = ch - topBarH - 32 - 4;
    
    BeginScissorMode((int)SX(cx + 4), (int)SY(logY), 
                     (int)((cw - 8) * g_uiScale), (int)(logH * g_uiScale));
    
    int total = g_cliLogCount;
    int visibleLines = (int)(logH / 20.0f);
    int start = (total > visibleLines) ? total - visibleLines : 0;
    int scrollLines = (int)(g_player.cliScroll / 20.0f);
    start -= scrollLines;
    if (start < 0) start = 0;
    if (start > total) start = total;
    
    // Draw a subtle line number / timestamp for each log
    for (int i = start; i < total; i++) {
        int row = i - start;
        float y = logY + 4 + row * 20.0f;
        if (y > cy + ch - 30) break;
        
        const char* txt = g_cliLogs[i];
        
        // Determine color based on log type
        Color col = COLOR_GHOST;
        Color prefixCol = COLOR_GHOST;
        float prefixAlpha = 0.4f;
        
        if (strncmp(txt, "> ", 2) == 0) {
            col = COLOR_TOXIC;
            prefixCol = {40, 240, 100, 100};
        } else if (strstr(txt, "[ERROR]") || strstr(txt, "[ERR]")) {
            col = COLOR_BLOOD;
            prefixCol = {220, 20, 40, 150};
        } else if (strstr(txt, "[WARNING]") || strstr(txt, "[WARN]")) {
            col = COLOR_AMBER;
            prefixCol = {255, 150, 0, 150};
        } else if (strstr(txt, "[PAGE]")) {
            col = COLOR_CYAN;
            prefixCol = {0, 220, 240, 150};
        } else if (strstr(txt, "[SYS_INIT]")) {
            col = COLOR_TOXIC;
            prefixCol = {40, 240, 100, 150};
        } else if (strstr(txt, "[SCAN]")) {
            col = COLOR_AMBER;
            prefixCol = {255, 150, 0, 150};
        } else if (strstr(txt, "[MINER]")) {
            col = COLOR_TOXIC;
            prefixCol = {40, 240, 100, 150};
        } else if (strstr(txt, "[ICE]")) {
            col = COLOR_CYAN;
            prefixCol = {0, 220, 240, 150};
        }
        
        // Check for chat/whisper
        if (strstr(txt, "[CHAT]")) {
            col = COLOR_TOXIC;
            prefixCol = {40, 240, 100, 100};
        } else if (strstr(txt, "[WHISPER")) {
            col = COLOR_AMBER;
            prefixCol = {255, 150, 0, 150};
        } else if (strstr(txt, "[DOS") || strstr(txt, "[TRACE SPIKE]")) {
            col = COLOR_BLOOD;
            prefixCol = {220, 20, 40, 150};
        }
        
        // Render with subtle line number
        char lineNum[8];
        snprintf(lineNum, sizeof(lineNum), "%03d", i + 1);
        DrawScaledText(lineNum, cx + 8, y, 7, {80, 90, 110, 120});
        
        // Main log text with slight indent
        DrawScaledText(txt, cx + 40, y, 11, col);
        
        // Small accent bar on the left for error/warning
        if (strstr(txt, "[ERROR]") || strstr(txt, "[ERR]")) {
            DrawScaledRect(cx + 4, y + 2, 3, 14, COLOR_BLOOD);
        } else if (strstr(txt, "[WARNING]") || strstr(txt, "[WARN]")) {
            DrawScaledRect(cx + 4, y + 2, 3, 14, COLOR_AMBER);
        } else if (strncmp(txt, "> ", 2) == 0) {
            DrawScaledRect(cx + 4, y + 2, 3, 14, COLOR_TOXIC);
        }
    }
    
    EndScissorMode();
    
    // ---- INPUT AREA ----
    float inputY = cy + ch - 28;
    
    // Input area background
    DrawScaledRect(cx + 4, inputY, cw - 8, 24, Color{12, 15, 20, 220});
    DrawScaledLine(cx + 4, inputY, cx + cw - 4, inputY, Color{30, 35, 45, 180});
    
    // Input prompt with arrow
    bool focused = IsTerminalFocused();
    float t = (float)GetTime();
    bool cursorVisible = focused ? (fmodf(t, 0.8f) > 0.4f) : false;
    float fontSize = 12.0f;
    
    // Animated prompt arrow
    float arrowPulse = sinf(t * 3.0f) * 0.3f + 0.7f;
    Color arrowCol = {40, 240, 100, (unsigned char)(arrowPulse * 255)};
    DrawScaledText("➜", cx + 10, inputY + 5, 12, arrowCol);
    
    // Input text
    char prompt[300];
    snprintf(prompt, sizeof(prompt), " %s", g_player.inputBuffer);
    DrawScaledText(prompt, cx + 28, inputY + 5, fontSize, COLOR_GHOST);
    
    // Input cursor
    if (cursorVisible) {
        float textWidth = MeasureScaledTextWidth(prompt, fontSize);
        float cursorHeight = fontSize * g_fontScale;
        float cursorWidth = 6.0f;
        
        DrawScaledRect(
            cx + 28 + textWidth + 1.0f,
            inputY + 4,
            cursorWidth,
            cursorHeight,
            COLOR_TOXIC
        );
    }
    
    // ---- STATUS BAR (Bottom) ----
    float statusY = cy + ch - 4;
    DrawScaledLine(cx + 4, statusY, cx + cw - 4, statusY, Color{30, 35, 45, 100});
    
    // Status indicators
    char statusBuf[128];
    snprintf(statusBuf, sizeof(statusBuf), "VCOIN: %.2f  |  ICE: %d/3  |  TRACE: %d%%", 
             g_player.vcoin, g_player.iceShields, g_player.traceLevel);
    
    float statusW = MeasureScaledTextWidth(statusBuf, 8);
    DrawScaledText(statusBuf, cx + cw - statusW - 12, cy + ch - 12, 8, {80, 90, 110, 180});
    
    // Focus hint
    if (!focused) {
        DrawScaledText("[CLICK TO FOCUS]", cx + cw - 130, inputY + 5, 9, {255, 150, 0, 180});
    }
}