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
// DRAW OVERLOADED PAGE - KERNEL PANIC STYLE
// ============================================================
// ============================================================
// DRAW OVERLOADED PAGE - KERNEL PANIC STYLE (ENHANCED)
// ============================================================
void DrawOverloadedPage(float x, float y, float w, float h) {
    float t = (float)GetTime();
    float timer = g_player.siteOverloadTimer;
    float total = g_player.siteOverloadTotal;
    float ratio = timer / total;
    float intensity = g_player.overloadGlitchIntensity;
    float scanOffset = g_player.overloadScanlineOffset;
    
    // ---- BACKGROUND (glitching dark red/black) ----
    Color bgCol = {8, 0, 0, 255};
    float glitch = sinf(t * 200.0f) * 0.3f + 0.7f;
    if (glitch > 0.85f) {
        bgCol = {18, 0, 0, 255};
    }
    DrawScaledRect(x, y, w, h, bgCol);
    
    // ---- SUBTLE GRID OVERLAY ----
    Color gridCol = {220, 20, 40, (unsigned char)(intensity * 15)};
    for (int gx = 0; gx < w; gx += 40) {
        DrawScaledLine(x + gx, y, x + gx, y + h, gridCol);
    }
    for (int gy = 0; gy < h; gy += 40) {
        DrawScaledLine(x, y + gy, x + w, y + gy, gridCol);
    }
    
    // ---- CRASHING SCANLINES (more aggressive) ----
    for (int i = 0; i < (int)h; i += 1) {
        float scanY = y + i + fmodf(scanOffset + i * 0.5f, h);
        unsigned char alpha = (unsigned char)(intensity * 60 + 10 + sinf(t * 10.0f + i * 0.1f) * 20);
        Color scanCol = {220, 20, 40, alpha};
        if (fmodf(i * 0.5f + t * 40.0f, 80.0f) > 70.0f) {
            scanCol = {255, 80, 80, (unsigned char)(alpha * 1.8f)};
        }
        DrawScaledRect(x, scanY, w, 1, scanCol);
    }
    
    // ---- DATA CORRUPTION BLOCKS (more dynamic) ----
    int blockCount = 40 + (int)(intensity * 60);
    for (int i = 0; i < blockCount; i++) {
        float bx = x + fmodf((i * 137.5f + t * 60.0f * (0.5f + i * 0.015f)), w);
        float by = y + fmodf((i * 97.3f + t * 40.0f * (0.3f + i * 0.01f)), h);
        float bw = 3.0f + fmodf(t * 25.0f + i * 7.0f, 40.0f);
        float bh = 1.0f + fmodf(t * 20.0f + i * 11.0f, 25.0f);
        unsigned char alpha = (unsigned char)(intensity * 180 * (0.2f + 0.8f * sinf(t * 6.0f + i * 0.7f)));
        Color blockCol = {220, 20, 40, alpha};
        if (i % 3 == 0) blockCol = {255, 120, 0, (unsigned char)(alpha * 0.7f)};
        if (i % 5 == 0) blockCol = {180, 0, 80, (unsigned char)(alpha * 0.8f)};
        if (i % 7 == 0) blockCol = {200, 60, 20, (unsigned char)(alpha * 0.9f)};
        DrawScaledRect(bx, by, bw, bh, blockCol);
    }
    
    // ---- CORRUPTED HEX CHARACTERS (falling rain) ----
    const char* hexChars = "0123456789ABCDEF";
    int hexCount = 50 + (int)(intensity * 80);
    for (int i = 0; i < hexCount; i++) {
        float hx = x + fmodf((i * 73.7f + t * 100.0f * (0.3f + i * 0.012f)), w);
        float hy = y + fmodf((i * 59.3f + t * 80.0f * (0.2f + i * 0.008f)), h);
        char hex = hexChars[(int)(fmodf(t * 15.0f + i * 7.0f, 16))];
        unsigned char alpha = (unsigned char)(intensity * 200 * (0.2f + 0.8f * sinf(t * 4.0f + i * 0.4f)));
        float fontSize = 6.0f + fmodf(t * 8.0f + i * 1.5f, 12.0f);
        DrawScaledText(&hex, hx, hy, fontSize, {220, 20, 40, alpha});
    }
    
    // ---- CRT WARP EFFECT ----
    for (int i = 0; i < 25; i++) {
        float warpY = y + i * (h / 25.0f) + sinf(t * 10.0f + i * 0.4f) * 25.0f;
        float warpX = x + sinf(t * 12.0f + i * 0.6f) * 40.0f;
        float warpW = w + sinf(t * 8.0f + i * 0.3f) * 50.0f;
        unsigned char alpha = (unsigned char)(intensity * 50 * (0.3f + 0.7f * sinf(t * 5.0f + i * 0.5f)));
        DrawScaledRect(warpX, warpY, warpW, 2, {220, 20, 40, alpha});
    }
    
    // ---- CENTRAL KERNEL PANIC MESSAGE (CENTERED) ----
    float centerX = x + w / 2.0f;
    float centerY = y + h / 2.0f;
    
    // Box dimensions - centered
    float boxW = 700.0f;
    float boxH = 380.0f;
    float boxX = centerX - boxW / 2.0f + sinf(t * 2.0f) * 2.0f;
    float boxY = centerY - boxH / 2.0f + cosf(t * 3.0f) * 2.0f;

    float pulse = sinf(t * 5.0f) * 0.3f + 0.7f;
    
    // ---- OUTER GLOW (more layers) ----
    for (int i = 8; i >= 1; i--) {
        float pad = i * 5.0f;
        unsigned char alpha = (unsigned char)(intensity * 50 * (i / 8.0f) * pulse);
        DrawScaledRectLines(boxX - pad, boxY - pad, boxW + pad * 2, boxH + pad * 2, 
                           {220, 20, 40, alpha});
    }
    
    // ---- PULSING BORDER ----
    for (int i = 3; i >= 1; i--) {
        float pad = i * 3.0f;
        unsigned char alpha = (unsigned char)(intensity * 80 * (i / 3.0f) * pulse);
        DrawScaledRectLines(boxX - pad, boxY - pad, boxW + pad * 2, boxH + pad * 2, 
                           {255, 50, 50, alpha});
    }
    
    // ---- BOX BACKGROUND ----
    DrawScaledRect(boxX, boxY, boxW, boxH, {6, 0, 0, 220});
    DrawScaledRectLines(boxX, boxY, boxW, boxH, {255, 30, 30, (unsigned char)(pulse * 200 + 55)});
    
    // ---- SCANNING LINE ACROSS BOX ----
    float scanLinePos = fmodf(t * 60.0f, boxH);
    DrawScaledRect(boxX + 10, boxY + scanLinePos, boxW - 20, 2, {255, 50, 50, 60});
    
    // ---- KERNEL PANIC TITLE (with glitch) ----
    float titleGlitch = sinf(t * 30.0f) * 2.0f;
    float titleY = boxY + 20;
    
    // Top border
    DrawScaledText("╔═══════════════════════════════════════════════════════════╗", 
                   boxX + 20 + titleGlitch * 0.5f, titleY, 11, {220, 20, 40, 255});
    
    // Main title with glitch
    DrawScaledText("║          ██████  KERNEL PANIC!  ██████                    ║", 
                   boxX + 20 + titleGlitch, titleY + 22, 13, {255, 60, 60, 255});
    DrawScaledText("║          ██████  SITE OVERLOADED ██████                   ║", 
                   boxX + 20 - titleGlitch * 0.7f, titleY + 42, 13, {255, 40, 40, 255});
    
    // Bottom border
    DrawScaledText("╚═══════════════════════════════════════════════════════════╝", 
                   boxX + 20 + titleGlitch * 0.3f, titleY + 62, 11, {220, 20, 40, 255});
    
    float contentY = titleY + 80;
    
    // ---- OVERLOADED SITE NAME (with glow) ----
    char siteMsg[128];
    snprintf(siteMsg, sizeof(siteMsg), "▸ vnet://%s ◂", g_player.overloadedSite);
    float siteW = MeasureScaledTextWidth(siteMsg, 17);
    float siteX = centerX - siteW / 2 + sinf(t * 4.0f) * 3.0f;
    
    // Glow behind site name
    for (int i = 3; i >= 1; i--) {
        DrawScaledText(siteMsg, siteX + i * 0.5f, contentY + i * 0.5f, 17, {255, 120, 0, (unsigned char)(20 * i)});
    }
    DrawScaledText(siteMsg, siteX, contentY, 17, {255, 150, 50, 255});
    contentY += 32;
    
    // ---- ERROR CODE ----
    char errorCode[64];
    snprintf(errorCode, sizeof(errorCode), "ERROR: 0x%04X_OVERLOAD_CASCADE", (int)(t * 100.0f) % 65535);
    float errW = MeasureScaledTextWidth(errorCode, 11);
    DrawScaledText(errorCode, centerX - errW / 2, contentY, 11, {200, 100, 100, 200});
    contentY += 26;
    
    // ---- SYSTEM MESSAGE (scrolling through messages) ----
    char memoryMsg[128];
    snprintf(memoryMsg, sizeof(memoryMsg), "MEMORY CORRUPTION AT 0x7FFF%04dA", 
            (int)(t * 100.0f) % 10000);

    const char* messages[] = {
        "CRITICAL: VOLTAGE SPIKE DETECTED IN VFS CORE",
        "UDP STACK CORRUPTION: PACKET LOSS > 95%",
        "BGP ROUTER DROPPED SUBNET - NO ROUTE TO HOST",
        "KERNEL TASK: vfs_worker hung (state: D)",
        memoryMsg,
        "CPU EXCEPTION: DIVIDE BY ZERO IN LINE 1337",
        "SECTOR NULL: UNALLOCATED ADDRESS SPACE",
        "ICE FIREWALL: CRITICAL FAILURE - SHIELDS DOWN"
    };
    int msgIdx = (int)(t * 0.4f) % 8;
    float msgGlitch = sinf(t * 25.0f) * 2.0f;
    DrawScaledText(messages[msgIdx], centerX - 260 + msgGlitch, contentY, 10, {255, 180, 150, 220});
    contentY += 22;
    
    // ---- TIMESTAMP / SYSTEM INFO ----
    char sysInfo[64];
    snprintf(sysInfo, sizeof(sysInfo), "[%s] SYSTEM: CRITICAL | MEM: 0x%04X", 
             "18.0 Hz", (int)(t * 1000.0f) % 65535);
    float sysW = MeasureScaledTextWidth(sysInfo, 9);
    DrawScaledText(sysInfo, centerX - sysW / 2, contentY, 9, {150, 80, 80, 180});
    contentY += 28;
    
    // ---- RECOVERY COUNTDOWN (larger, more dramatic) ----
    char timerStr[32];
    snprintf(timerStr, sizeof(timerStr), "RECOVERY IN: %.1fs", timer);
    float timerW = MeasureScaledTextWidth(timerStr, 20);
    float timerY = contentY;
    
    // Glow behind timer
    for (int i = 4; i >= 1; i--) {
        DrawScaledText(timerStr, centerX - timerW / 2 + i * 0.5f, timerY + i * 0.5f, 20, 
                      {timer > 10 ? 40 : 220, timer > 10 ? 240 : 20, timer > 10 ? 100 : 40, (unsigned char)(15 * i)});
    }
    DrawScaledText(timerStr, centerX - timerW / 2, timerY, 20, 
                  timer > 10 ? COLOR_TOXIC : COLOR_BLOOD);
    contentY += 34;
    
    // ---- PROGRESS BAR (more dramatic) ----
    float barX = boxX + 60;
    float barY = contentY;
    float barW = boxW - 120;
    float barH = 20.0f;
    
    // Bar background with glow
    DrawScaledRect(barX - 2, barY - 2, barW + 4, barH + 4, {220, 20, 40, 40});
    DrawScaledRect(barX, barY, barW, barH, {8, 0, 0, 255});
    DrawScaledRectLines(barX, barY, barW, barH, {220, 20, 40, 150});
    
    float fillW = barW * (1.0f - ratio);
    if (fillW < 2.0f) fillW = 2.0f;
    
    // Gradient fill with glow
    for (int x = 0; x < (int)fillW; x += 2) {
        float progress = (float)x / barW;
        unsigned char r = (unsigned char)(200 + progress * 55);
        unsigned char g = (unsigned char)(20 - progress * 15);
        unsigned char b = (unsigned char)(30 - progress * 25);
        DrawScaledRect(barX + x, barY, 2, barH, {r, g, b, 255});
    }
    
    // Glow at leading edge
    if (fillW > 10.0f) {
        DrawScaledRect(barX + fillW - 8.0f, barY - 4, 8, barH + 8, {255, 50, 50, 40});
    }
    
    // Scan light on progress bar
    float scanPos = fmodf(t * 100.0f, barW);
    DrawScaledRect(barX + scanPos - 4.0f, barY - 3, 8, barH + 6, {255, 80, 80, 60});
    DrawScaledRect(barX + scanPos - 1.0f, barY, 2, barH, {255, 200, 200, 200});
    
    contentY += 34;
    
    // ---- PACKET SNIFFER TRAFFIC (more dynamic) ----
    float snifferY = contentY;
    const char* snifferMsgs[] = {
        "[TCP] SYN - 192.168.1.23:443",
        "[UDP] DROP - 10.0.0.45:8080",
        "[ICMP] TIMEOUT - 8.8.8.8",
        "[DNS] REFUSED - vnet.local",
        "[HTTP] 503 - Service Unavailable",
        "[TCP] RST - 192.168.1.100:22",
        "[UDP] FLOOD - 0.0.0.0:0",
        "[BGP] DOWN - AS64500"
    };
    for (int i = 0; i < 4; i++) {
        int idx = (int)(t * 0.3f + i * 2.7f) % 8;
        float alpha = 180 - i * 40 + sinf(t * 3.0f + i) * 20;
        float offset = fmodf(t * 50.0f + i * 20.0f, 40.0f) - 20.0f;
        DrawScaledText(snifferMsgs[idx], boxX + 25 + offset, snifferY + i * 18.0f, 9, 
                      {100, 200, 255, (unsigned char)alpha});
    }
    
    // ---- NETWORK TRAFFIC SPARKLINES ----
    for (int i = 0; i < 15; i++) {
        float sx = x + fmodf(t * 80.0f + i * 60.0f, w);
        float sy = y + fmodf(t * 50.0f + i * 40.0f, h);
        float len = 15 + fmodf(t * 40.0f + i * 8.0f, 50.0f);
        Color sparkCol = {0, 200, 255, (unsigned char)(intensity * 60 + sinf(t * 5.0f + i) * 20)};
        DrawScaledLine(sx, sy, sx + len, sy + len * 0.4f, sparkCol);
        DrawScaledLine(sx + len * 0.2f, sy - len * 0.2f, sx + len * 0.8f, sy + len * 0.2f, Fade(sparkCol, 0.3f));
    }
    
    // ---- CRT VIGNETTE ----
    for (int i = 0; i < 8; i++) {
        float size = i * 25.0f;
        unsigned char alpha = (unsigned char)(10 - i * 1.2f);
        DrawScaledRect(x + size, y + size, w - size * 2, h - size * 2, {0, 0, 0, alpha});
    }
    
    // ---- GLITCH FLASHES (more frequent) ----
    if (fmodf(t * 2.5f, 1.0f) > 0.97f) {
        DrawScaledRect(x, y, w, h, {255, 0, 0, (unsigned char)(intensity * 50)});
    }
    if (fmodf(t * 1.7f, 1.0f) > 0.96f) {
        DrawScaledRect(x, y, w, h, {0, 0, 0, (unsigned char)(intensity * 70)});
    }
    if (fmodf(t * 3.3f, 1.0f) > 0.98f) {
        float flashX = x + fmodf(t * 200.0f, w);
        DrawScaledRect(flashX, y, 10 + fmodf(t * 100.0f, 80.0f), h, {255, 255, 255, (unsigned char)(intensity * 30)});
    }
    
    // ---- STATUS TEXT (bottom) ----
    char statusText[128];
    snprintf(statusText, sizeof(statusText), "OVERLOAD: %s | RECOVERY: %.1fs | PACKETS DROPPED: %d | CRC: 0x%04X",
             g_player.overloadedSite, timer, (int)(t * 100.0f) % 9999, (int)(t * 200.0f) % 65535);
    float statusW = MeasureScaledTextWidth(statusText, 9);
    DrawScaledText(statusText, centerX - statusW / 2, y + h - 25, 9, {150, 80, 80, 180});
    
    // ---- INTERLACING EFFECT (subtle) ----
    for (int i = 0; i < (int)h; i += 3) {
        DrawScaledRect(x, y + i, w, 1, {0, 0, 0, 6});
    }
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
    } else if (g_player.siteOverloaded) {
        DrawOverloadedPage(gameX, gameY, gameW, gameH);
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