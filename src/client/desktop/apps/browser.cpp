#include "../desktop.h"
#include "../../render.h"
#include "vnet.h"
#include "../../../shared/vnet_sites.h"
#include "vnet_protocol.h"
#include "vnet_client.h"

void Desktop::DrawBrowserConnectionOverlay(float contentX, float contentY, float contentW, float contentH) {
    float t = (float)GetTime();
    float pulse = sinf(t * 8.0f) * 0.5f + 0.5f;
    
    // Center of the browser content area
    float centerX = contentX + contentW / 2.0f;
    float centerY = contentY + contentH / 2.0f;
    
    // Connection box - slightly larger
    float boxW = contentW * 0.80f;
    float boxH = contentH * 0.75f;
    float boxX = centerX - boxW / 2.0f;
    float boxY = centerY - boxH / 2.0f;
    
    // ---- NO SHAKE - just subtle CRT wobble ----
    float wobbleX = sinf(t * 0.7f) * 0.5f;
    float wobbleY = cosf(t * 0.9f + 0.5f) * 0.3f;
    
    // ---- BACKGROUND SCANLINES ----
    for (int i = 0; i < (int)(boxH / 3.0f); i++) {
        float scanY = boxY + i * 3.0f + fmodf(t * 60.0f, 3.0f);
        float scanAlpha = (i % 2 == 0) ? 12 : 4;
        DrawScaledRect(boxX + wobbleX, scanY + wobbleY, boxW, 1, 
                       {0, 0, 0, (unsigned char)scanAlpha});
    }
    
    // ---- CRT VIGNETTE ----
    for (int i = 0; i < 3; i++) {
        float size = i * 30.0f;
        DrawScaledRect(boxX + wobbleX + size, boxY + wobbleY + size, 
                       boxW - size * 2, boxH - size * 2, 
                       {0, 0, 0, (unsigned char)(5 - i * 2)});
    }
    
    // ---- CORNER RETICLES (GLOWING) ----
    float cornerSize = 20.0f;
    float glowPulse = sinf(t * 2.5f) * 0.3f + 0.7f;
    Color glowCol = {220, 20, 40, (unsigned char)(glowPulse * 200 + 55)};
    
    // Top-left
    DrawScaledRect(boxX + wobbleX - 4, boxY + wobbleY - 4, cornerSize, 3, glowCol);
    DrawScaledRect(boxX + wobbleX - 4, boxY + wobbleY - 4, 3, cornerSize, glowCol);
    // Top-right
    DrawScaledRect(boxX + boxW + wobbleX + 1, boxY + wobbleY - 4, cornerSize, 3, glowCol);
    DrawScaledRect(boxX + boxW + wobbleX - 1, boxY + wobbleY - 4, 3, cornerSize, glowCol);
    // Bottom-left
    DrawScaledRect(boxX + wobbleX - 4, boxY + boxH + wobbleY + 1, cornerSize, 3, glowCol);
    DrawScaledRect(boxX + wobbleX - 4, boxY + boxH + wobbleY - 1, 3, cornerSize, glowCol);
    // Bottom-right
    DrawScaledRect(boxX + boxW + wobbleX + 1, boxY + boxH + wobbleY + 1, cornerSize, 3, glowCol);
    DrawScaledRect(boxX + boxW + wobbleX - 1, boxY + boxH + wobbleY - 1, 3, cornerSize, glowCol);
    
    // ---- CORNER GLOW BLOOM ----
    DrawScaledRect(boxX + wobbleX - 6, boxY + wobbleY - 6, 3, 3, {220, 20, 40, 80});
    DrawScaledRect(boxX + boxW + wobbleX + 3, boxY + wobbleY - 6, 3, 3, {220, 20, 40, 80});
    DrawScaledRect(boxX + wobbleX - 6, boxY + boxH + wobbleY + 3, 3, 3, {220, 20, 40, 80});
    DrawScaledRect(boxX + boxW + wobbleX + 3, boxY + boxH + wobbleY + 3, 3, 3, {220, 20, 40, 80});
    
    // ---- REMAINING TIME ----
    float remaining = g_player.targetConnectTime - g_player.connectTimer;
    if (remaining < 0.0f) remaining = 0.0f;
    float ratio = (g_player.targetConnectTime > 0.0f) ? 
                  g_player.connectTimer / g_player.targetConnectTime : 0.0f;
    if (ratio < 0.02f) ratio = 0.02f;
    if (ratio > 1.0f) ratio = 1.0f;
    
    float headerY = boxY + 30.0f + wobbleY;
    
    // ---- ANIMATED HEADER WITH PULSE (CENTERED) ----
    Color headerCol = (pulse > 0.5f) ? COLOR_AMBER : COLOR_BLOOD;
    headerCol.a = (unsigned char)(pulse * 150 + 105);
    
    const char* headerText = "[ TOR PROXY CIRCUIT HANDSHAKE ACTIVE ]";
    float headerW = MeasureScaledTextWidth(headerText, 16);
    
    // Header background glow
    float headerGlow = sinf(t * 3.0f) * 0.3f + 0.7f;
    DrawScaledRect(centerX - headerW / 2 - 20 + wobbleX, headerY - 6, 
                   headerW + 40, 34, 
                   {220, 20, 40, (unsigned char)(headerGlow * 25)});
    
    DrawScaledText(headerText, 
                   centerX - headerW / 2 + wobbleX, 
                   headerY + 2, 16, headerCol);
    
    // ---- TARGET URL (CENTERED, LARGER) ----
    char urlStr[128];
    snprintf(urlStr, sizeof(urlStr), "RESOLVING: vnet://%s", g_player.pendingURL);
    float urlW = MeasureScaledTextWidth(urlStr, 14);
    
    DrawScaledText(urlStr, 
                   centerX - urlW / 2 + wobbleX, 
                   headerY + 42.0f + wobbleY, 
                   14, COLOR_CYAN);
    
    // URL subtle pulse underline
    float linePulse = sinf(t * 2.0f) * 0.3f + 0.7f;
    float lineY = headerY + 62.0f + wobbleY;
    DrawScaledRect(centerX - urlW / 2 - 10 + wobbleX, lineY, 
                   urlW + 20, 1, 
                   {0, 220, 240, (unsigned char)(linePulse * 80)});
    
    // ---- ROTATING CROSSHAIR (CENTERED, LARGER) ----
    float crosshairY = headerY + 85.0f + wobbleY;
    float rotOff = t * 3.5f;
    float crosshairRadius = 40.0f;
    float crosshairHeight = 24.0f;
    
    // Crosshair trail
    for (int trail = 0; trail < 4; trail++) {
        float trailOff = t * 3.5f - trail * 0.12f;
        float trailAlpha = 50 - trail * 12;
        for (int i = 0; i < 4; i++) {
            float angle = trailOff + (float)i * 1.5708f;
            float rx = centerX + cosf(angle) * (crosshairRadius - trail * 6.0f) + wobbleX;
            float ry = crosshairY + sinf(angle) * (crosshairHeight - trail * 3.5f) + wobbleY;
            DrawScaledRect(rx - 1.5f, ry - 1.5f, 3, 3, 
                          {220, 20, 40, (unsigned char)trailAlpha});
        }
    }
    
    // Main crosshair (larger)
    for (int i = 0; i < 4; i++) {
        float angle = rotOff + (float)i * 1.5708f;
        float rx = centerX + cosf(angle) * crosshairRadius + wobbleX;
        float ry = crosshairY + sinf(angle) * crosshairHeight + wobbleY;
        float size = 5.0f + sinf(t * 5.0f + i) * 1.5f;
        DrawScaledRect(rx - size/2, ry - size/2, size, size, COLOR_BLOOD);
    }
    
    // Center dot (larger, pulsing)
    float dotPulse = sinf(t * 4.0f) * 0.3f + 0.7f;
    float dotSize = 4.0f + dotPulse * 2.0f;
    DrawScaledRect(centerX - dotSize/2 + wobbleX, 
                   crosshairY - dotSize/2 + wobbleY, 
                   dotSize, dotSize, 
                   {220, 20, 40, (unsigned char)(dotPulse * 200 + 55)});
    
    // ---- LATENCY STATUS (CENTERED) ----
    char latencyStr[128];
    snprintf(latencyStr, sizeof(latencyStr), "LATENCY BUFFER: %.0fs REMAINING  •  HOPS: 3/3", remaining);
    float latW = MeasureScaledTextWidth(latencyStr, 12);
    DrawScaledText(latencyStr, 
                   centerX - latW / 2 + wobbleX, 
                   crosshairY + 45.0f + wobbleY, 
                   12, COLOR_TOXIC);
    
    // ---- PROGRESS BAR (CENTERED, LARGER) ----
    float barW = 440.0f;
    float barH = 26.0f;
    float barX = centerX - barW / 2 + wobbleX;
    float barY = crosshairY + 75.0f + wobbleY;
    
    // Bar background
    DrawScaledRect(barX, barY, barW, barH, COLOR_PANEL);
    DrawScaledRectLines(barX, barY, barW, barH, COLOR_BORDER);
    
    // Bar scanlines
    for (int i = 0; i < (int)barH; i += 2) {
        DrawScaledRect(barX, barY + i, barW, 1, {0, 0, 0, (unsigned char)(8 + i * 2)});
    }
    
    // Progress fill with gradient
    float fillW = barW * ratio;
    if (fillW < 2.0f) fillW = 2.0f;
    if (fillW > barW) fillW = barW;
    
    // Gradient fill (smoother)
    for (int x = 0; x < (int)fillW; x += 2) {
        float progress = (float)x / barW;
        unsigned char r = (unsigned char)(30 + progress * 190);
        unsigned char g = (unsigned char)(230 - progress * 140);
        unsigned char b = (unsigned char)(80 - progress * 60);
        DrawScaledRect(barX + x, barY, 2, barH, {r, g, b, 255});
    }
    
    // Glow effect on fill
    if (fillW > 10.0f) {
        DrawScaledRect(barX + fillW - 10.0f, barY - 3, 10, barH + 6, 
                       {40, 240, 100, 50});
        DrawScaledRect(barX + fillW - 4.0f, barY, 4, barH, 
                       {40, 240, 100, 130});
    }
    
    // ---- PERCENTAGE TEXT (CENTERED IN BAR) ----
    char pctStr[16];
    snprintf(pctStr, sizeof(pctStr), "%d%%", (int)(ratio * 100.0f));
    float pctW = MeasureScaledTextWidth(pctStr, 13);
    Color pctCol = (ratio > 0.5f) ? COLOR_BLACK : COLOR_CYAN;
    DrawScaledText(pctStr, 
                   centerX - pctW / 2 + wobbleX, 
                   barY + 5.0f, 13, pctCol);
    
    // ---- DATA PACKET ANIMATION (ABOVE PROGRESS BAR) ----
    float packetY = barY - 35.0f + wobbleY;
    for (int p = 0; p < 6; p++) {
        float offset = fmodf(t * 25.0f + p * 30.0f, barW - 30);
        float packetX = barX + 15 + offset;
        float packetSize = 5.0f + sinf(t * 3.5f + p) * 1.5f;
        Color packetCol = (p % 2 == 0) ? COLOR_TOXIC : COLOR_CYAN;
        packetCol.a = (unsigned char)(160 + sinf(t * 4.0f + p * 2.0f) * 50 + 50);
        DrawScaledRect(packetX, packetY + p * 2.5f, packetSize, packetSize, packetCol);
        
        // Packet trail
        for (int trail = 1; trail < 5; trail++) {
            float trailX = packetX - trail * 4.5f;
            if (trailX > barX) {
                DrawScaledRect(trailX, packetY + p * 2.5f, 2, 2, 
                               {packetCol.r, packetCol.g, packetCol.b, 
                                (unsigned char)(70 - trail * 14)});
            }
        }
    }
    
    // ---- HEX STREAM TELEMETRY (CENTERED) ----
    int hexTick = (int)fmodf(t * 18.0f, 99.0f);
    char hexStr[128];
    snprintf(hexStr, sizeof(hexStr), "0x88F9_NODE_HOP_OK  •  ENCRYPTING PACKET SUBNET SECTOR #%d", hexTick);
    float hexW = MeasureScaledTextWidth(hexStr, 10);
    DrawScaledText(hexStr, 
                   centerX - hexW / 2 + wobbleX, 
                   barY + 38.0f + wobbleY, 
                   10, COLOR_AMBER);
    
    // ---- FOOTER (CENTERED) ----
    const char* footerStr = "SPOOFING MAC ADDRESS • MIRRORING VIA ARCHIVAL.VNET";
    float footW = MeasureScaledTextWidth(footerStr, 10);
    DrawScaledText(footerStr, 
                   centerX - footW / 2 + wobbleX, 
                   barY + 58.0f + wobbleY, 
                   10, COLOR_GHOST);
    
    // ---- VERTICAL SCANLINE OVERLAY ----
    for (int x = 0; x < (int)boxW; x += 5) {
        float scanAlpha = 4 + sinf(t * 1.5f + x * 0.08f) * 3 + 3;
        DrawScaledRect(boxX + wobbleX + x, boxY + wobbleY, 1, boxH, 
                       {0, 0, 0, (unsigned char)scanAlpha});
    }
    
    // ---- CRT SCREEN FLICKER (SUBTLE) ----
    if (fmodf(t * 0.5f, 1.0f) > 0.97f) {
        DrawScaledRect(boxX + wobbleX, boxY + wobbleY, boxW, boxH, 
                       {0, 0, 0, (unsigned char)(15 + rand() % 25)});
    }
}

void Desktop::DrawBrowser(const AppWindow& win) {
    float cx = win.x + 4;
    float cy = win.y + m_windowTitleHeight + 4;
    float cw = win.w - 8;
    float ch = win.h - m_windowTitleHeight - 8;
    
    // Browser background
    DrawScaledRect(cx, cy, cw, ch, COLOR_BLACK);
    
    // ---- TOOLBAR ----
    float toolbarY = cy;
    float toolbarH = 48.0f;
    float navX = cx + 8;
    float navY = toolbarY + 8;
    float navSize = 30.0f;
    float spacing = 6.0f;
    
    // Toolbar background
    DrawScaledRect(cx, toolbarY, cw, toolbarH, Color{18, 20, 28, 230});
    DrawScaledLine(cx, toolbarY + toolbarH, cx + cw, toolbarY + toolbarH, COLOR_BORDER);
    
    // ---- NAVIGATION BUTTONS ----
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    // Back button
    bool backHover = RefRectHover(navX, navY, navSize, navSize, refMouse);
    DrawScaledRect(navX, navY, navSize, navSize, backHover ? Color{40, 45, 65, 255} : Color{0,0,0,0});
    DrawScaledText("◄", navX + 8, navY + 6, 14, backHover ? COLOR_TOXIC : COLOR_GHOST);
    navX += navSize + spacing;
    
    // Forward button
    bool fwdHover = RefRectHover(navX, navY, navSize, navSize, refMouse);
    DrawScaledRect(navX, navY, navSize, navSize, fwdHover ? Color{40, 45, 65, 255} : Color{0,0,0,0});
    DrawScaledText("►", navX + 8, navY + 6, 14, fwdHover ? COLOR_TOXIC : COLOR_GHOST);
    navX += navSize + spacing;
    
    // Reload button
    bool reloadHover = RefRectHover(navX, navY, navSize, navSize, refMouse);
    DrawScaledRect(navX, navY, navSize, navSize, reloadHover ? Color{40, 45, 65, 255} : Color{0,0,0,0});
    DrawScaledText("⟳", navX + 6, navY + 6, 16, reloadHover ? COLOR_TOXIC : COLOR_GHOST);
    navX += navSize + spacing;
    
    // Home button
    bool homeHover = RefRectHover(navX, navY, navSize, navSize, refMouse);
    DrawScaledRect(navX, navY, navSize, navSize, homeHover ? Color{40, 45, 65, 255} : Color{0,0,0,0});
    DrawScaledText("⌂", navX + 6, navY + 4, 16, homeHover ? COLOR_TOXIC : COLOR_GHOST);
    navX += navSize + spacing + 10;
    
    // ---- URL BAR ----
    float urlX = navX;
    float urlY = navY;
    float urlW = cw - (urlX - cx) - 20;
    float urlH = navSize;
    
    // ============================================================
    // FIX: Track if URL bar is focused
    // ============================================================
    static bool urlBarFocused = false;
    static char urlInputBuffer[256] = "";
    static float cursorBlinkTimer = 0.0f;
    static bool showCursor = true;
    
    bool urlHover = RefRectHover(urlX, urlY, urlW, urlH, refMouse);
    
    // Click to focus
    if (clicked && urlHover) {
        urlBarFocused = true;
        // Copy current URL to input buffer when focusing
        if (strlen(urlInputBuffer) == 0) {
            strncpy(urlInputBuffer, g_player.currentURL, sizeof(urlInputBuffer) - 1);
        }
    } else if (clicked && !urlHover) {
        urlBarFocused = false;
    }
    
    // Cursor blink
    cursorBlinkTimer += GetFrameTime();
    if (cursorBlinkTimer >= 0.5f) {
        cursorBlinkTimer = 0.0f;
        showCursor = !showCursor;
    }
    
    // ---- URL BAR DRAWING ----
    DrawScaledRect(urlX, urlY, urlW, urlH, urlBarFocused ? Color{16, 20, 30, 255} : COLOR_URLBAR);
    DrawScaledRectLines(urlX, urlY, urlW, urlH, urlBarFocused ? COLOR_BLOOD : COLOR_BORDER);
    
    // Show URL text
    char urlDisplay[256];
    if (urlBarFocused) {
        const char* cursor = showCursor ? "_" : "";
        if (strlen(urlInputBuffer) == 0 && !showCursor) {
            snprintf(urlDisplay, sizeof(urlDisplay), "%s", cursor);
        } else {
            snprintf(urlDisplay, sizeof(urlDisplay), "%s%s", urlInputBuffer, cursor);
        }
    } else {
        if (g_player.isConnecting) {
            snprintf(urlDisplay, sizeof(urlDisplay), "vnet://%s", g_player.pendingURL);
        } else {
            snprintf(urlDisplay, sizeof(urlDisplay), "vnet://%s", g_player.currentURL);
        }
    }
    
    Color urlColor = urlBarFocused ? COLOR_TOXIC : (g_player.isConnecting ? COLOR_AMBER : COLOR_CYAN);
    DrawScaledText(urlDisplay, urlX + 8, urlY + 8, 11, urlColor);
    
    // ---- URL BAR INPUT HANDLING ----
    if (urlBarFocused) {
        // Character input
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126) {
                size_t len = strlen(urlInputBuffer);
                if (len < sizeof(urlInputBuffer) - 1) {
                    urlInputBuffer[len] = (char)key;
                    urlInputBuffer[len + 1] = '\0';
                }
            }
            key = GetCharPressed();
        }
        
        // Backspace
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = (int)strlen(urlInputBuffer);
            if (len > 0) {
                urlInputBuffer[len - 1] = '\0';
            }
        }
        
        // Enter - Navigate!
        if (IsKeyPressed(KEY_ENTER) && strlen(urlInputBuffer) > 0) {
            // Clean the URL
            char cleanURL[256];
            strncpy(cleanURL, urlInputBuffer, sizeof(cleanURL) - 1);
            cleanURL[sizeof(cleanURL) - 1] = '\0';
            
            // Remove vnet:// prefix if present
            if (strncmp(cleanURL, "vnet://", 7) == 0) {
                memmove(cleanURL, cleanURL + 7, strlen(cleanURL) - 6);
            }
            
            // If no .vnet and not vnet.dir, add .vnet
            if (strstr(cleanURL, ".vnet") == NULL && strcmp(cleanURL, "vnet.dir") != 0) {
                strcat(cleanURL, ".vnet");
            }
            
            // Navigate!
            TriggerRouteNavigation(cleanURL);
            
            // Clear input buffer and unfocus
            urlInputBuffer[0] = '\0';
            urlBarFocused = false;
            PushCliLog("[BROWSER]: Navigated to %s", cleanURL);
        }
        
        // Escape - unfocus
        if (IsKeyPressed(KEY_ESCAPE)) {
            urlBarFocused = false;
            urlInputBuffer[0] = '\0';
        }
    }
    
    // ---- PAGE CONTENT ----
    float gameX = cx + 8;
    float gameY = toolbarY + toolbarH + 4;
    float gameW = cw - 16;
    float gameH = ch - toolbarH - 8;
    
    // Page background
    DrawScaledRect(gameX, gameY, gameW, gameH, COLOR_PANEL);
    DrawScaledRectLines(gameX, gameY, gameW, gameH, COLOR_BORDER);
    
    // ============================================================
    // CONNECTION OVERLAY - Show inside browser when connecting
    // ============================================================
    if (g_player.isConnecting) {
        DrawBrowserConnectionOverlay(gameX, gameY, gameW, gameH);
    } else {
        // ---- PAGE TITLE ----
        const VEXSiteData* site = SiteManager::Get().GetSiteData(g_player.currentURL);
        char titleStr[128];
        if (site) {
            snprintf(titleStr, sizeof(titleStr), "// %s", site->title);
        } else {
            snprintf(titleStr, sizeof(titleStr), "// %s", g_player.currentURL);
        }
        DrawScaledText(titleStr, gameX + 12, gameY + 8, 13, COLOR_BLOOD);
        DrawScaledLine(gameX + 12, gameY + 28, gameX + gameW - 12, gameY + 28, COLOR_BORDER);
        
        // ---- PAGE CONTENT ----
        float contentX = gameX + 4;
        float contentY = gameY + 34;
        float contentW = gameW - 8;
        float contentH = gameH - 38;
        
        if (contentH > 20) {
            DrawMarkupPage(contentX, contentY, contentW, contentH);
        }
    }
    
    // ---- STATUS BAR ----
    float statusY = cy + ch - 22;
    DrawScaledRect(cx, statusY, cw, 22, Color{18, 20, 28, 220});
    DrawScaledLine(cx, statusY, cx + cw, statusY, COLOR_BORDER);
    
    char status[128];
    snprintf(status, sizeof(status), "PORT: %d | VCOIN: %.2f | TRACE: %d%% | ICE: %d/3", 
             g_player.port, g_player.vcoin, g_player.traceLevel, g_player.iceShields);
    DrawScaledText(status, cx + 12, statusY + 5, 9, COLOR_GHOST);
    
    // ---- BUTTON INTERACTIONS ----
    if (clicked && !g_player.isConnecting) {
        if (homeHover) {
            TriggerRouteNavigation("vnet.dir");
            urlBarFocused = false;
            urlInputBuffer[0] = '\0';
        }
        else if (reloadHover) {
            RefreshPage();
            TriggerJitter(0.15f);
        }
        else if (backHover && strlen(g_player.prevURL) > 0) {
            TriggerRouteNavigation(g_player.prevURL);
            urlBarFocused = false;
            urlInputBuffer[0] = '\0';
        }
        else if (fwdHover) {
            PushCliLog("[BROWSER]: Forward navigation not implemented yet");
        }
    }
}