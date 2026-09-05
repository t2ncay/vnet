#include "../desktop.h"
#include "../../render.h"
#include "../../../shared/vnet.h"

void Desktop::DrawProfile(const AppWindow& win) {
    float cx = win.x + 4;
    float cy = win.y + m_windowTitleHeight + 4;
    float cw = win.w - 8;
    float ch = win.h - m_windowTitleHeight - 8;
    
    // ---- BACKGROUND ----
    DrawScaledRect(cx, cy, cw, ch, Color{8, 10, 14, 255});
    DrawScaledRectLines(cx, cy, cw, ch, COLOR_BORDER);
    
    // ---- HEADER ----
    float headerH = 60.0f;
    DrawScaledRect(cx, cy, cw, headerH, Color{14, 18, 26, 255});
    DrawScaledLine(cx, cy + headerH, cx + cw, cy + headerH, Color{40, 220, 120, 80});
    
    // Avatar circle
    float avatarSize = 40.0f;
    float avatarX = cx + 20.0f;
    float avatarY = cy + 10.0f;
    DrawScaledRect(avatarX, avatarY, avatarSize, avatarSize, Color{20, 25, 35, 255});
    DrawScaledRectLines(avatarX, avatarY, avatarSize, avatarSize, Color{40, 220, 120, 150});
    DrawScaledText("👤", avatarX + 8, avatarY + 8, 22, COLOR_CYAN);
    
    // Handle and title
    DrawScaledText(g_player.handle, avatarX + avatarSize + 15, avatarY + 8, 14, COLOR_TOXIC);
    DrawScaledText("OPERATOR // VEKTRAOS v9.5", avatarX + avatarSize + 15, avatarY + 32, 10, COLOR_GHOST);
    
    // Port badge (right side)
    char portBadge[32];
    snprintf(portBadge, sizeof(portBadge), "PORT %d", g_player.port);
    float portW = MeasureScaledTextWidth(portBadge, 11);
    DrawScaledRect(cx + cw - portW - 30, cy + 12, portW + 20, 30, Color{20, 25, 35, 255});
    DrawScaledRectLines(cx + cw - portW - 30, cy + 12, portW + 20, 30, Color{0, 220, 240, 100});
    DrawScaledText(portBadge, cx + cw - portW - 20, cy + 22, 11, COLOR_CYAN);
    
    // ---- CONTENT AREA ----
    float contentY = cy + headerH + 10.0f;
    float contentH = ch - headerH - 10.0f;
    float leftCol = cx + 20.0f;
    float rightCol = cx + cw / 2.0f + 10.0f;
    float rowH = 36.0f;
    float rowY = contentY;
    
    // ---- LEFT COLUMN: STATS ----
    struct StatItem {
        const char* label;
        const char* value;
        Color color;
        float progress;
    };
    
    // Build stat items
    std::vector<StatItem> stats;
    
    char vcoinStr[32];
    snprintf(vcoinStr, sizeof(vcoinStr), "%.2f VCOIN", g_player.vcoin);
    stats.push_back({"VCOIN", vcoinStr, COLOR_TOXIC, g_player.vcoin / 50.0f});
    
    char traceStr[32];
    snprintf(traceStr, sizeof(traceStr), "%d%%", g_player.traceLevel);
    stats.push_back({"TRACE LEVEL", traceStr, 
                    g_player.traceLevel > 70 ? COLOR_BLOOD : COLOR_AMBER, 
                    g_player.traceLevel / 100.0f});
    
    char iceStr[32];
    snprintf(iceStr, sizeof(iceStr), "%d/3", g_player.iceShields);
    stats.push_back({"ICE SHIELDS", iceStr, COLOR_CYAN, g_player.iceShields / 3.0f});
    
    char heatStr[32];
    snprintf(heatStr, sizeof(heatStr), "%.0f°C", g_player.crtHeat);
    stats.push_back({"CRT HEAT", heatStr, 
                    g_player.crtHeat > 75.0f ? COLOR_BLOOD : COLOR_AMBER, 
                    (g_player.crtHeat - 35.0f) / 65.0f});
    
    char paranoiaStr[32];
    snprintf(paranoiaStr, sizeof(paranoiaStr), "%.0f%%", g_player.neuralParanoia);
    stats.push_back({"NEURAL PARANOIA", paranoiaStr, 
                    g_player.neuralParanoia > 60.0f ? COLOR_BLOOD : COLOR_AMBER, 
                    g_player.neuralParanoia / 100.0f});
    
    // Draw stats with progress bars
    for (int i = 0; i < (int)stats.size(); i++) {
        float y = rowY + i * (rowH + 6.0f);
        if (y > contentY + contentH - 30) break;
        
        const auto& stat = stats[i];
        
        // Stat label
        DrawScaledText(stat.label, leftCol, y + 2, 10, COLOR_GHOST);
        
        // Progress bar background
        float barX = leftCol + 95.0f;
        float barY = y + 4;
        float barW = 140.0f;
        float barH = 14.0f;
        
        DrawScaledRect(barX, barY, barW, barH, Color{12, 15, 20, 255});
        DrawScaledRectLines(barX, barY, barW, barH, Color{30, 35, 45, 150});
        
        // Progress fill
        float fillW = barW * stat.progress;
        if (fillW < 2.0f) fillW = 2.0f;
        if (fillW > barW) fillW = barW;
        DrawScaledRect(barX, barY, fillW, barH, stat.color);
        
        // Value text
        float valW = MeasureScaledTextWidth(stat.value, 10);
        DrawScaledText(stat.value, barX + barW - valW - 6.0f, barY + 2, 10, stat.color);
    }
    
    // ---- RIGHT COLUMN: SITES DISCOVERED ----
    float rightY = rowY;
    
    // Section header
    DrawScaledText("DISCOVERED SITES", rightCol, rightY, 11, COLOR_AMBER);
    DrawScaledLine(rightCol, rightY + 18, rightCol + 220, rightY + 18, Color{30, 35, 45, 150});
    rightY += 28.0f;
    
    // Site count
    char siteCountStr[64];
    snprintf(siteCountStr, sizeof(siteCountStr), "%d / 20 SITES", g_player.assignedCount);
    DrawScaledText(siteCountStr, rightCol, rightY, 10, COLOR_CYAN);
    rightY += 22.0f;
    
    // Site progress bar
    float siteBarW = 210.0f;
    float siteBarH = 10.0f;
    DrawScaledRect(rightCol, rightY, siteBarW, siteBarH, Color{12, 15, 20, 255});
    DrawScaledRectLines(rightCol, rightY, siteBarW, siteBarH, Color{30, 35, 45, 150});
    float siteFill = (g_player.assignedCount / 20.0f) * siteBarW;
    if (siteFill < 2.0f) siteFill = 2.0f;
    DrawScaledRect(rightCol, rightY, siteFill, siteBarH, COLOR_TOXIC);
    rightY += 20.0f;
    
    // List discovered sites (limit to 8 visible)
    int maxSites = (int)((contentH - (rightY - rowY)) / 20.0f);
    if (maxSites > 8) maxSites = 8;
    if (maxSites < 1) maxSites = 1;
    
    // Create a scroll offset if there are more sites than fit
    static int siteScrollOffset = 0;
    
    // Handle mouse wheel for site list scrolling (using pageScroll as a hack)
    // We'll use a separate static or member variable later
    
    int startIdx = 0;
    int endIdx = g_player.assignedCount;
    if (endIdx > maxSites) {
        // Show only maxSites sites, starting from the most recent (end)
        startIdx = endIdx - maxSites;
    }
    
    for (int i = startIdx; i < endIdx && i < g_player.assignedCount; i++) {
        float y = rightY + (i - startIdx) * 20.0f;
        if (y > contentY + contentH - 10) break;
        
        const char* site = g_player.assignedSites[i];
        
        // Check if it's a core node
        bool isCore = false;
        const char* coreNodes[] = {"market.vnet", "vault.vnet", "terminal.vnet", "crypto.vnet", "hellroom.vnet"};
        for (int c = 0; c < 5; c++) {
            if (strcmp(site, coreNodes[c]) == 0) {
                isCore = true;
                break;
            }
        }
        
        // Dot indicator
        Color dotColor = isCore ? COLOR_BLOOD : COLOR_CYAN;
        DrawScaledRect(rightCol, y + 6, 5, 5, dotColor);
        
        // Site name
        Color siteColor = isCore ? COLOR_AMBER : COLOR_GHOST;
        DrawScaledText(site, rightCol + 14, y + 2, 10, siteColor);
    }
    
    // ---- DIVIDER ----
    float dividerX = cx + cw / 2.0f;
    DrawScaledLine(dividerX, contentY, dividerX, cy + ch - 10, Color{30, 35, 45, 80});
    
    // ---- STATUS BADGES ----
    float badgeY = cy + ch - 36;
    
    // Status indicator
    DrawScaledRect(cx + 20, badgeY, 12, 12, COLOR_TOXIC);
    DrawScaledText("ONLINE", cx + 38, badgeY, 10, COLOR_TOXIC);
    
    // Runtime
    char runtimeStr[64];
    int hours = (int)(g_player.runTime / 3600.0f);
    int minutes = (int)((g_player.runTime - hours * 3600) / 60.0f);
    int seconds = (int)(g_player.runTime - hours * 3600 - minutes * 60);
    snprintf(runtimeStr, sizeof(runtimeStr), "UPTIME: %02d:%02d:%02d", hours, minutes, seconds);
    DrawScaledText(runtimeStr, cx + 110, badgeY, 10, COLOR_GHOST);
    
    // Version
    DrawScaledText("VEKTRAOS v9.5", cx + cw - 110, badgeY, 10, Color{80, 90, 110, 180});
}