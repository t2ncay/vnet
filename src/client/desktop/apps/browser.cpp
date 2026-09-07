#include "../desktop.h"
#include "../../render.h"
#include "../../../shared/vnet.h"
#include "../../../shared/vnet_sites.h"
#include "../../../shared/vnet_protocol.h"
#include "../../vnet_client.h"

// ============================================================
// BROWSER CONNECTION OVERLAY (with gradients)
// ============================================================
void Desktop::DrawBrowserConnectionOverlay(float contentX, float contentY, float contentW, float contentH) {
    float t = (float)GetTime();
    float pulse = sinf(t * 8.0f) * 0.5f + 0.5f;
    
    float centerX = contentX + contentW / 2.0f;
    float centerY = contentY + contentH / 2.0f;
    
    float boxW = contentW * 0.80f;
    float boxH = contentH * 0.75f;
    float boxX = centerX - boxW / 2.0f;
    float boxY = centerY - boxH / 2.0f;
    
    float wobbleX = sinf(t * 0.7f) * 0.5f;
    float wobbleY = cosf(t * 0.9f + 0.5f) * 0.3f;
    
    // ---- GRADIENT BACKGROUND FOR CONNECTION BOX ----
    DrawGradientBackground(boxX + wobbleX, boxY + wobbleY, boxW, boxH, 
                          THEME_GRADIENT_TOP, THEME_GRADIENT_BOTTOM);
    DrawScaledRectLines(boxX + wobbleX, boxY + wobbleY, boxW, boxH, 
                        Fade(THEME_GRADIENT_ACCENT, 0.3f));
    
    // ---- CRT VIGNETTE ----
    for (int i = 0; i < 3; i++) {
        float size = i * 30.0f;
        DrawScaledRect(boxX + wobbleX + size, boxY + wobbleY + size, 
                       boxW - size * 2, boxH - size * 2, 
                       {0, 0, 0, (unsigned char)(5 - i * 2)});
    }
    
    // ---- CORNER RETICLES (using gradient accent) ----
    float cornerSize = 20.0f;
    float glowPulse = sinf(t * 2.5f) * 0.3f + 0.7f;
    Color glowCol = THEME_GRADIENT_ACCENT;
    glowCol.a = (unsigned char)(glowPulse * 200 + 55);
    
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
    
    // ---- REMAINING TIME ----
    float remaining = g_player.targetConnectTime - g_player.connectTimer;
    if (remaining < 0.0f) remaining = 0.0f;
    float ratio = (g_player.targetConnectTime > 0.0f) ? 
                  g_player.connectTimer / g_player.targetConnectTime : 0.0f;
    if (ratio < 0.02f) ratio = 0.02f;
    if (ratio > 1.0f) ratio = 1.0f;
    
    float headerY = boxY + 30.0f + wobbleY;
    Color headerCol = (pulse > 0.5f) ? THEME_GRADIENT_ACCENT : COLOR_AMBER;
    headerCol.a = (unsigned char)(pulse * 150 + 105);
    
    const char* headerText = "[ TOR PROXY CIRCUIT HANDSHAKE ACTIVE ]";
    float headerW = MeasureScaledTextWidth(headerText, 16);
    
    // Header background glow (using gradient accent)
    float headerGlow = sinf(t * 3.0f) * 0.3f + 0.7f;
    DrawScaledRect(centerX - headerW / 2 - 20 + wobbleX, headerY - 6, 
                   headerW + 40, 34, 
                   Fade(THEME_GRADIENT_ACCENT, headerGlow * 0.15f));
    
    DrawScaledText(headerText, 
                   centerX - headerW / 2 + wobbleX, 
                   headerY + 2, 16, headerCol);
    
    // ---- TARGET URL (using gradient accent) ----
    char urlStr[128];
    snprintf(urlStr, sizeof(urlStr), "RESOLVING: vnet://%s", g_player.pendingURL);
    float urlW = MeasureScaledTextWidth(urlStr, 14);
    DrawScaledText(urlStr, 
                   centerX - urlW / 2 + wobbleX, 
                   headerY + 42.0f + wobbleY, 
                   14, COLOR_CYAN);
    
    // URL underline (using gradient accent)
    float linePulse = sinf(t * 2.0f) * 0.3f + 0.7f;
    float lineY = headerY + 62.0f + wobbleY;
    DrawScaledRect(centerX - urlW / 2 - 10 + wobbleX, lineY, 
                   urlW + 20, 1, 
                   Fade(THEME_GRADIENT_ACCENT, linePulse * 0.5f));
    
    // ---- ROTATING CROSSHAIR (using gradient accent) ----
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
                          Fade(THEME_GRADIENT_ACCENT, trailAlpha / 255.0f));
        }
    }
    
    // Main crosshair (using gradient accent)
    for (int i = 0; i < 4; i++) {
        float angle = rotOff + (float)i * 1.5708f;
        float rx = centerX + cosf(angle) * crosshairRadius + wobbleX;
        float ry = crosshairY + sinf(angle) * crosshairHeight + wobbleY;
        float size = 5.0f + sinf(t * 5.0f + i) * 1.5f;
        DrawScaledRect(rx - size/2, ry - size/2, size, size, THEME_GRADIENT_ACCENT);
    }
    
    // Center dot (pulsing gradient accent)
    float dotPulse = sinf(t * 4.0f) * 0.3f + 0.7f;
    float dotSize = 4.0f + dotPulse * 2.0f;
    Color dotCol = THEME_GRADIENT_ACCENT;
    dotCol.a = (unsigned char)(dotPulse * 200 + 55);
    DrawScaledRect(centerX - dotSize/2 + wobbleX, 
                   crosshairY - dotSize/2 + wobbleY, 
                   dotSize, dotSize, dotCol);
    
    // ---- PROGRESS BAR (with gradient fill) ----
    float barW = 440.0f;
    float barH = 26.0f;
    float barX = centerX - barW / 2 + wobbleX;
    float barY = crosshairY + 75.0f + wobbleY;
    
    // Bar background
    DrawScaledRect(barX, barY, barW, barH, COLOR_BLACK);
    DrawScaledRectLines(barX, barY, barW, barH, Fade(THEME_GRADIENT_ACCENT, 0.3f));
    
    // Gradient progress fill
    float fillW = barW * ratio;
    if (fillW < 2.0f) fillW = 2.0f;
    if (fillW > barW) fillW = barW;
    
    // Use theme gradient for the bar fill
    for (int x = 0; x < (int)fillW; x += 2) {
        float progress = (float)x / barW;
        Color gradCol = BlendColor(THEME_GRADIENT_TOP, THEME_GRADIENT_ACCENT, progress);
        DrawScaledRect(barX + x, barY, 2, barH, gradCol);
    }
    
    // Glow at leading edge (using gradient accent)
    if (fillW > 10.0f) {
        DrawScaledRect(barX + fillW - 10.0f, barY - 3, 10, barH + 6, 
                       Fade(THEME_GRADIENT_ACCENT, 0.3f));
        DrawScaledRect(barX + fillW - 4.0f, barY, 4, barH, 
                       Fade(THEME_GRADIENT_ACCENT, 0.7f));
    }
    
    // ---- PERCENTAGE TEXT ----
    char pctStr[16];
    snprintf(pctStr, sizeof(pctStr), "%d%%", (int)(ratio * 100.0f));
    float pctW = MeasureScaledTextWidth(pctStr, 13);
    Color pctCol = (ratio > 0.5f) ? COLOR_BLACK : THEME_GRADIENT_ACCENT;
    DrawScaledText(pctStr, centerX - pctW / 2 + wobbleX, barY + 5.0f, 13, pctCol);
    
    // ---- FOOTER ----
    const char* footerStr = "SPOOFING MAC ADDRESS • MIRRORING VIA ARCHIVAL.VNET";
    float footW = MeasureScaledTextWidth(footerStr, 10);
    DrawScaledText(footerStr, 
                   centerX - footW / 2 + wobbleX, 
                   barY + 58.0f + wobbleY, 
                   10, Fade(THEME_GRADIENT_ACCENT, 0.4f));
}

// ============================================================
// MAIN BROWSER DRAW (with gradients)
// ============================================================
void Desktop::DrawBrowser(const AppWindow& win) {
    float cx = win.x + 4;
    float cy = win.y + m_windowTitleHeight + 4;
    float cw = win.w - 8;
    float ch = win.h - m_windowTitleHeight - 8;
    
    // ---- GRADIENT BACKGROUND FOR BROWSER ----
    DrawGradientBackground(cx, cy, cw, ch, THEME_GRADIENT_TOP, THEME_GRADIENT_BOTTOM);
    
    // ---- TOOLBAR ----
    float toolbarY = cy;
    float toolbarH = 48.0f;
    float navX = cx + 8;
    float navY = toolbarY + 8;
    float navSize = 30.0f;
    float spacing = 6.0f;
    
    // Toolbar background (using gradient)
    DrawScaledRect(cx, toolbarY, cw, toolbarH, Fade(THEME_GRADIENT_BOTTOM, 0.8f));
    DrawScaledLine(cx, toolbarY + toolbarH, cx + cw, toolbarY + toolbarH, 
                   Fade(THEME_GRADIENT_ACCENT, 0.3f));
    
    // ---- NAVIGATION BUTTONS ----
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    // Back button
    bool backHover = RefRectHover(navX, navY, navSize, navSize, refMouse);
    DrawScaledRect(navX, navY, navSize, navSize, backHover ? Fade(THEME_GRADIENT_ACCENT, 0.3f) : Color{0,0,0,0});
    DrawScaledText("◄", navX + 8, navY + 6, 14, backHover ? THEME_GRADIENT_ACCENT : COLOR_GHOST);
    navX += navSize + spacing;
    
    // Forward button
    bool fwdHover = RefRectHover(navX, navY, navSize, navSize, refMouse);
    DrawScaledRect(navX, navY, navSize, navSize, fwdHover ? Fade(THEME_GRADIENT_ACCENT, 0.3f) : Color{0,0,0,0});
    DrawScaledText("►", navX + 8, navY + 6, 14, fwdHover ? THEME_GRADIENT_ACCENT : COLOR_GHOST);
    navX += navSize + spacing;
    
    // Reload button
    bool reloadHover = RefRectHover(navX, navY, navSize, navSize, refMouse);
    DrawScaledRect(navX, navY, navSize, navSize, reloadHover ? Fade(THEME_GRADIENT_ACCENT, 0.3f) : Color{0,0,0,0});
    DrawScaledText("⟳", navX + 6, navY + 6, 16, reloadHover ? THEME_GRADIENT_ACCENT : COLOR_GHOST);
    navX += navSize + spacing;
    
    // Home button
    bool homeHover = RefRectHover(navX, navY, navSize, navSize, refMouse);
    DrawScaledRect(navX, navY, navSize, navSize, homeHover ? Fade(THEME_GRADIENT_ACCENT, 0.3f) : Color{0,0,0,0});
    DrawScaledText("⌂", navX + 6, navY + 4, 16, homeHover ? THEME_GRADIENT_ACCENT : COLOR_GHOST);
    navX += navSize + spacing + 10;
    
    // ---- URL BAR ----
    float urlX = navX;
    float urlY = navY;
    float urlW = cw - (urlX - cx) - 20;
    float urlH = navSize;
    
    static bool urlBarFocused = false;
    static char urlInputBuffer[256] = "";
    static float cursorBlinkTimer = 0.0f;
    static bool showCursor = true;
    
    bool urlHover = RefRectHover(urlX, urlY, urlW, urlH, refMouse);
    
    if (clicked && urlHover) {
        urlBarFocused = true;
        if (strlen(urlInputBuffer) == 0) {
            strncpy(urlInputBuffer, g_player.currentURL, sizeof(urlInputBuffer) - 1);
        }
    } else if (clicked && !urlHover) {
        urlBarFocused = false;
    }
    
    cursorBlinkTimer += GetFrameTime();
    if (cursorBlinkTimer >= 0.5f) {
        cursorBlinkTimer = 0.0f;
        showCursor = !showCursor;
    }
    
    // URL Bar with gradient border
    DrawScaledRect(urlX, urlY, urlW, urlH, urlBarFocused ? Color{16, 20, 30, 255} : COLOR_URLBAR);
    DrawScaledRectLines(urlX, urlY, urlW, urlH, 
                        urlBarFocused ? THEME_GRADIENT_ACCENT : Fade(THEME_GRADIENT_ACCENT, 0.3f));
    
    // URL text with gradient accent when focused
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
    
    Color urlColor = urlBarFocused ? THEME_GRADIENT_ACCENT : 
                    (g_player.isConnecting ? COLOR_AMBER : COLOR_CYAN);
    DrawScaledText(urlDisplay, urlX + 8, urlY + 8, 11, urlColor);
    
    // ---- URL BAR INPUT HANDLING ----
    if (urlBarFocused) {
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
        
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = (int)strlen(urlInputBuffer);
            if (len > 0) {
                urlInputBuffer[len - 1] = '\0';
            }
        }
        
        if (IsKeyPressed(KEY_ENTER) && strlen(urlInputBuffer) > 0) {
            char cleanURL[256];
            strncpy(cleanURL, urlInputBuffer, sizeof(cleanURL) - 1);
            cleanURL[sizeof(cleanURL) - 1] = '\0';
            
            if (strncmp(cleanURL, "vnet://", 7) == 0) {
                memmove(cleanURL, cleanURL + 7, strlen(cleanURL) - 6);
            }
            
            if (strstr(cleanURL, ".vnet") == NULL && strcmp(cleanURL, "vnet.dir") != 0) {
                strcat(cleanURL, ".vnet");
            }
            
            TriggerRouteNavigation(cleanURL);
            urlInputBuffer[0] = '\0';
            urlBarFocused = false;
            PushCliLog("[BROWSER]: Navigated to %s", cleanURL);
        }
        
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
    
    // Page background (using gradient)
    DrawScaledRect(gameX, gameY, gameW, gameH, Fade(THEME_GRADIENT_BOTTOM, 0.5f));
    DrawScaledRectLines(gameX, gameY, gameW, gameH, Fade(THEME_GRADIENT_ACCENT, 0.15f));
    
    if (g_player.isConnecting) {
        DrawBrowserConnectionOverlay(gameX, gameY, gameW, gameH);
    } else {
        // ---- PAGE TITLE (using gradient accent) ----
        const VEXSiteData* site = SiteManager::Get().GetSiteData(g_player.currentURL);
        char titleStr[128];
        if (site) {
            snprintf(titleStr, sizeof(titleStr), "// %s", site->title.c_str());  // ← .c_str() added
        } else {
            snprintf(titleStr, sizeof(titleStr), "// %s", g_player.currentURL);
        }
        DrawScaledText(titleStr, gameX + 12, gameY + 8, 13, THEME_GRADIENT_ACCENT);
        DrawScaledLine(gameX + 12, gameY + 28, gameX + gameW - 12, gameY + 28, 
                       Fade(THEME_GRADIENT_ACCENT, 0.2f));
        
        // ---- PAGE CONTENT ----
        float contentX = gameX + 4;
        float contentY = gameY + 34;
        float contentW = gameW - 8;
        float contentH = gameH - 38;
        
        if (contentH > 20) {
            DrawMarkupPage(contentX, contentY, contentW, contentH);
        }
    }
    
    // ---- STATUS BAR (using gradient accent) ----
    float statusY = cy + ch - 22;
    DrawScaledRect(cx, statusY, cw, 22, Fade(THEME_GRADIENT_BOTTOM, 0.9f));
    DrawScaledLine(cx, statusY, cx + cw, statusY, Fade(THEME_GRADIENT_ACCENT, 0.3f));
    
    char status[128];
    snprintf(status, sizeof(status), "PORT: %d | VCOIN: %.2f | TRACE: %d%% | ICE: %d/3", 
             g_player.port, g_player.vcoin, g_player.traceLevel, g_player.iceShields);
    DrawScaledText(status, cx + 12, statusY + 5, 9, Fade(THEME_GRADIENT_ACCENT, 0.6f));
    
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