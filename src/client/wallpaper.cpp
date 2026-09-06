#include "wallpaper.h"
#include "render.h"
#include <cmath>
#include <ctime>

// ============================================================
// HELPER: Draw crisp, scaled text with proper size
// ============================================================
static void DrawCrispText(const char* text, float x, float y, float fontSize, Color color) {
    // Use DrawScaledText which handles the scaling properly
    DrawScaledText(text, x, y, fontSize, color);
}

void DrawVektraWallpaper(float width, float height) {
    float t = (float)GetTime();
    
    // ============================================================
    // BACKGROUND
    // ============================================================
    Color baseBg = {18, 20, 24, 255};
    DrawScaledRect(0, 0, width, height, baseBg);
    
    // Subtle gradient overlay (dark to slightly lighter)
    for (int y = 0; y < height; y += 2) {
        float prog = (float)y / height;
        unsigned char alpha = (unsigned char)(prog * 15);
        DrawScaledRect(0, y, width, 2, {30, 35, 45, alpha});
    }
    
    // ============================================================
    // SUBTLE GRID
    // ============================================================
    for (int x = 0; x < width; x += 60) {
        DrawScaledLine(x, 0, x, height, {40, 42, 50, 30});
    }
    for (int y = 0; y < height; y += 60) {
        DrawScaledLine(0, y, width, y, {40, 42, 50, 30});
    }
    
    // ============================================================
    // AMBIENT PARTICLES (floating dots)
    // ============================================================
    for (int i = 0; i < 50; i++) {
        float px = fmodf(i * 137.5f + t * 8.0f, width);
        float py = fmodf(i * 97.3f + sinf(t * 0.3f + i * 0.7f) * 40.0f, height);
        unsigned char alpha = (unsigned char)(30 + sinf(t * 0.5f + i * 1.2f) * 20 + 20);
        DrawScaledRect(px, py, 2, 2, {60, 65, 80, alpha});
    }
    
    // ============================================================
    // TOP DECORATIVE BANNER - UNCHANGED
    // ============================================================
    float bannerY = 0;
    float bannerH = 70;
    float bannerX = 20;
    float bannerW = width - 40;
    
    DrawScaledRect(bannerX, bannerY, bannerW, bannerH, {22, 24, 30, 220});
    DrawScaledLine(bannerX, bannerY + bannerH, bannerX + bannerW, bannerY + bannerH, {60, 65, 80, 150});
    
    // Agency seal placeholder
    float sealX = bannerX + 20, sealY = 8, sealSize = 50;
    DrawScaledRect(sealX, sealY, sealSize, sealSize, {35, 38, 45, 255});
    DrawScaledRectLines(sealX, sealY, sealSize, sealSize, {80, 85, 100, 200});
    
    // Star in seal
    float cx = sealX + sealSize/2, cy = sealY + sealSize/2;
    for (int i = 0; i < 5; i++) {
        float angle = -1.5708f + i * 2.0944f;
        float x1 = cx + cosf(angle) * 16;
        float y1 = cy + sinf(angle) * 16;
        float x2 = cx + cosf(angle + 1.0472f) * 7;
        float y2 = cy + sinf(angle + 1.0472f) * 7;
        DrawScaledLine(x1, y1, x2, y2, {140, 145, 160, 200});
        DrawScaledLine(x2, y2, cx + cosf(angle + 2.0944f) * 16, cy + sinf(angle + 2.0944f) * 16, {140, 145, 160, 200});
    }
    
    DrawScaledText("VEKTRA INTELLIGENCE DIVISION", sealX + sealSize + 16, sealY + 14, 14, {160, 165, 180, 255});
    DrawScaledText("CLASSIFIED // EYES ONLY", sealX + sealSize + 16, sealY + 34, 9, {100, 105, 115, 200});
    
    // Top right - classification badge
    float badgeX = bannerX + bannerW - 160;
    DrawScaledRect(badgeX, 8, 140, 50, {40, 20, 25, 220});
    DrawScaledRectLines(badgeX, 8, 140, 50, {160, 30, 40, 180});
    DrawScaledText("TOP SECRET", badgeX + 30, 18, 12, {200, 50, 60, 255});
    DrawScaledText("COMSEC // NOFORN", badgeX + 20, 38, 9, {160, 100, 110, 200});

    // ============================================================
    // CENTRAL VEKTRA LOGO / EAGLE - IMPROVED
    // ============================================================
    float centerX = width / 2.0f;
    float centerY = height / 2.0f;
    float baseSize = 1.0f;
    
    // ---- EAGLE SILHOUETTE (more detailed) ----
    float eagleX = centerX - 80;
    float eagleY = centerY - 60;
    float eagleScale = 1.2f;
    
    // Body
    DrawScaledRect(eagleX + 60 * eagleScale, eagleY + 10 * eagleScale, 
                   40 * eagleScale, 100 * eagleScale, {45, 48, 55, 120});
    DrawScaledRect(eagleX + 100 * eagleScale, eagleY + 30 * eagleScale, 
                   30 * eagleScale, 80 * eagleScale, {45, 48, 55, 120});
    
    // Head
    DrawScaledRect(eagleX + 70 * eagleScale, eagleY, 
                   25 * eagleScale, 20 * eagleScale, {45, 48, 55, 140});
    // Beak
    DrawScaledRect(eagleX + 95 * eagleScale, eagleY + 5 * eagleScale, 
                   15 * eagleScale, 8 * eagleScale, {55, 58, 65, 150});
    
    // Left wing (spread)
    for (int i = 0; i < 15; i++) {
        float wingAngle = -1.8f + i * 0.25f;
        float wingX = eagleX + 80 * eagleScale - i * 12 * eagleScale;
        float wingY = eagleY + 40 * eagleScale + sinf(wingAngle) * 50 * eagleScale;
        float wingW = 8 * eagleScale + (15 - i) * 1.5f * eagleScale;
        
        int alphaVal = 100 - i * 3;
        if (alphaVal < 0) alphaVal = 0;
        if (alphaVal > 255) alphaVal = 255;
        DrawScaledRect(wingX, wingY, wingW, 2 * eagleScale, {55, 58, 68, (unsigned char)alphaVal});
    }
    
    // Right wing (spread)
    for (int i = 0; i < 15; i++) {
        float wingAngle = -1.8f + i * 0.25f;
        float wingX = eagleX + 120 * eagleScale + i * 12 * eagleScale;
        float wingY = eagleY + 40 * eagleScale + sinf(wingAngle) * 50 * eagleScale;
        float wingW = 8 * eagleScale + (15 - i) * 1.5f * eagleScale;

        int alphaVal = 100 - i * 3;
        if (alphaVal < 0) alphaVal = 0;
        if (alphaVal > 255) alphaVal = 255;
        DrawScaledRect(wingX, wingY, wingW, 2 * eagleScale, {55, 58, 68, (unsigned char)alphaVal});
    }
    
    // Tail
    for (int i = 0; i < 8; i++) {
        float tailX = eagleX + 65 * eagleScale + i * 8 * eagleScale;
        float tailY = eagleY + 100 * eagleScale;
        DrawScaledRect(tailX, tailY, 4 * eagleScale, 20 * eagleScale, {45, 48, 55, 80});
    }
    
    // ============================================================
    // VEKTRA TEXT - FIXED PIXELATION
    // ============================================================
    float textY = eagleY + 130 * eagleScale;
    
    // Glow behind text
    float glowSize = 140.0f;
    DrawScaledRect(centerX - glowSize, textY - 30, glowSize * 2, 80, 
                   {0, 180, 200, 15});
    
    // Main VEKTRA text - using proper scaled font
    // The font size is larger but DrawScaledText handles scaling properly
    float mainFontSize = 52.0f;
    float textWidth = MeasureScaledTextWidth("VEKTRA", mainFontSize);
    DrawScaledText("VEKTRA", centerX - textWidth / 2, textY, mainFontSize, 
                   {140, 145, 160, 230});
    
    // Glow outline effect (subtle)
    for (int i = 1; i <= 3; i++) {
        float offset = i * 1.0f;
        Color glowColor = {0, 180, 200, (unsigned char)(15 / i)};
        DrawScaledText("VEKTRA", centerX - textWidth / 2 + offset, textY + offset, 
                       mainFontSize, glowColor);
        DrawScaledText("VEKTRA", centerX - textWidth / 2 - offset, textY - offset, 
                       mainFontSize, glowColor);
    }
    
    // Redraw main text on top (crisp)
    DrawScaledText("VEKTRA", centerX - textWidth / 2, textY, mainFontSize, 
                   {160, 165, 180, 255});
    
    // ============================================================
    // SUBTITLE - FIXED PIXELATION
    // ============================================================
    float subY = textY + mainFontSize + 20;
    float subFontSize = 16.0f;
    
    const char* subtitle = "SILENT WATCH // COLD SIGNAL";
    float subWidth = MeasureScaledTextWidth(subtitle, subFontSize);
    DrawScaledText(subtitle, centerX - subWidth / 2, subY, subFontSize, 
                   {100, 105, 115, 200});
    
    // Decorative line under subtitle
    float lineY2 = subY + subFontSize + 12;
    float lineW2 = 280.0f;
    DrawScaledLine(centerX - lineW2 / 2, lineY2, centerX - 40, lineY2, 
                   {60, 65, 80, 80});
    DrawScaledLine(centerX + 40, lineY2, centerX + lineW2 / 2, lineY2, 
                   {60, 65, 80, 80});
    
    // Diamond accent
    float diamondSize = 8.0f;
    DrawScaledRect(centerX - diamondSize, lineY2 - diamondSize, 
                   diamondSize * 2, diamondSize * 2, {60, 65, 80, 120});
    DrawScaledRect(centerX - diamondSize / 2, lineY2 - diamondSize / 2, 
                   diamondSize, diamondSize, {140, 145, 160, 180});
    
    // ============================================================
    // DEPARTMENT TAGLINE
    // ============================================================
    float tagY = lineY2 + 30;
    const char* tagline = "VEKTRA INTELLIGENCE DIVISION // CYBER OPERATIONS";
    float tagW = MeasureScaledTextWidth(tagline, 10);
    DrawScaledText(tagline, centerX - tagW / 2, tagY, 10, {80, 85, 95, 150});
    
    // ============================================================
    // BOTTOM STATUS BAR - UNCHANGED
    // ============================================================
    float footerY = height - 55;
    float footerH = 55;
    float footerX = 20;
    float footerW = width - 40;

    DrawScaledRect(footerX, footerY, footerW, footerH, {22, 24, 30, 220});
    DrawScaledLine(footerX, footerY, footerX + footerW, footerY, {60, 65, 80, 150});

    // Left: system info
    char infoStr[128];
    time_t now = time(nullptr);
    struct tm* local = localtime(&now);
    char timeStr[32];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", local);
    snprintf(infoStr, sizeof(infoStr), "VEKTRA_OS // COMSEC v9.5 // %s // TERMINAL: ACTIVE", timeStr);
    DrawScaledText(infoStr, footerX + 20, footerY + 20, 9, {100, 105, 115, 180});

    // Right: classification
    DrawScaledText("CLASSIFICATION: TOP SECRET // NOFORN", footerX + footerW - 330, footerY + 20, 9, {160, 30, 40, 180});

    // Pulsing dot
    float pulse = sinf(t * 2.0f) * 0.3f + 0.7f;
    Color dotColor = {40, 200, 60, (unsigned char)(pulse * 200 + 55)};
    DrawScaledRect(footerX + footerW - 110, footerY + 18, 8, 8, dotColor);
    DrawScaledText("LIVE", footerX + footerW - 95, footerY + 17, 8, {160, 165, 180, 200});
    
    // ============================================================
    // SCANLINE OVERLAY (subtle)
    // ============================================================
    for (int y = 0; y < height; y += 4) {
        float alpha = 3.0f + sinf(t * 0.5f + y * 0.1f) * 1.5f;
        DrawScaledRect(0, y, width, 1, {0, 0, 0, (unsigned char)alpha});
    }
    
    // ============================================================
    // RANDOM STATIC GLITCHES (subtle)
    // ============================================================
    if (fmodf(t * 0.3f, 1.0f) > 0.97f) {
        float glitchX = rand() % (int)width;
        float glitchW = 20 + rand() % 80;
        DrawScaledRect(glitchX, 0, glitchW, height, {20, 22, 30, 12});
    }
    
    // ============================================================
    // VIGNETTE (subtle darkening at edges)
    // ============================================================
    for (int i = 0; i < 6; i++) {
        float size = i * 40.0f;
        unsigned char alpha = (unsigned char)(6 - i * 0.8f);
        DrawScaledRect(size, size, width - size * 2, height - size * 2, 
                       {0, 0, 0, alpha});
    }
}