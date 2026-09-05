#include "wallpaper.h"
#include "render.h"
#include <cmath>
#include <ctime>

void DrawVektraWallpaper(float width, float height) {
    float t = (float)GetTime();
    
    Color baseBg = {18, 20, 24, 255};
    
    DrawScaledRect(0, 0, width, height, baseBg);
    
    // ============================================================
    // SUBTLE GRID
    // ============================================================
    for (int x = 0; x < width; x += 60) {
        DrawScaledLine(x, 0, x, height, {40, 42, 50, 40});
    }
    for (int y = 0; y < height; y += 60) {
        DrawScaledLine(0, y, width, y, {40, 42, 50, 40});
    }
    
    // ============================================================
    // TOP DECORATIVE BANNER - REDUCED WIDTH
    // ============================================================
    float bannerY = 0;
    float bannerH = 70;  // Slightly smaller
    float bannerX = 20;   // Added margin from left
    float bannerW = width - 40;  // Reduced width with margins
    
    DrawScaledRect(bannerX, bannerY, bannerW, bannerH, {22, 24, 30, 220});
    DrawScaledLine(bannerX, bannerY + bannerH, bannerX + bannerW, bannerY + bannerH, {60, 65, 80, 150});
    
    // Agency seal placeholder
    float sealX = bannerX + 20, sealY = 8, sealSize = 50;  // Slightly smaller
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
    
    // Agency name - adjusted positions
    DrawScaledText("VEKTRA INTELLIGENCE DIVISION", sealX + sealSize + 16, sealY + 14, 14, {160, 165, 180, 255});
    DrawScaledText("CLASSIFIED // EYES ONLY", sealX + sealSize + 16, sealY + 34, 9, {100, 105, 115, 200});
    
    // Top right - classification badge (moved left)
    float badgeX = bannerX + bannerW - 160;
    DrawScaledRect(badgeX, 8, 140, 50, {40, 20, 25, 220});
    DrawScaledRectLines(badgeX, 8, 140, 50, {160, 30, 40, 180});
    DrawScaledText("TOP SECRET", badgeX + 30, 18, 12, {200, 50, 60, 255});
    DrawScaledText("COMSEC // NOFORN", badgeX + 20, 38, 9, {160, 100, 110, 200});

    // ============================================================
    // CENTRAL VEKTRA LOGO / EAGLE
    // ============================================================
    float logoX = width / 2 - 120;
    float logoY = height / 2 - 120;
    
    // Eagle silhouette (simplified)
    DrawScaledRect(logoX + 40, logoY, 40, 120, {40, 42, 50, 100});
    DrawScaledRect(logoX + 80, logoY + 20, 40, 100, {40, 42, 50, 100});
    DrawScaledRect(logoX + 120, logoY + 40, 40, 80, {40, 42, 50, 100});
    DrawScaledRect(logoX + 160, logoY + 60, 40, 60, {40, 42, 50, 100});
    DrawScaledRect(logoX + 200, logoY + 80, 40, 40, {40, 42, 50, 100});
    
    // Wings spread
    for (int i = 0; i < 12; i++) {
        float angle = -2.0f + i * 0.35f;
        float wingX = logoX + 120 + i * 25;
        float wingY = logoY + 40 + sinf(angle) * 60;
        DrawScaledRect(wingX, wingY, 15, 3, {60, 65, 80, 100});
    }
    
    // Large VEKTRA text
    DrawScaledText("VEKTRA", width/2 - 180, height/2 - 40, 60, {120, 125, 140, 180});
    
    // Subtitle
    DrawScaledText("SILENT WATCH // COLD SIGNAL", width/2 - 130, height/2 + 30, 16, {80, 85, 100, 180});
    
    // ============================================================
    // DECORATIVE LINES
    // ============================================================
    float lineY = height/2 + 60;
    DrawScaledLine(width/2 - 200, lineY, width/2 - 60, lineY, {60, 65, 80, 120});
    DrawScaledLine(width/2 + 60, lineY, width/2 + 200, lineY, {60, 65, 80, 120});
    
    // Diamond accent
    DrawScaledRect(width/2 - 8, lineY - 8, 16, 16, {60, 65, 80, 150});
    DrawScaledRect(width/2 - 3, lineY - 3, 6, 6, {120, 125, 140, 200});
    
    // ============================================================
    // BOTTOM STATUS BAR - REDUCED WIDTH
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
        float alpha = 4.0f + sinf(t * 0.5f + y * 0.1f) * 2.0f;
        DrawScaledRect(0, y, width, 1, {0, 0, 0, (unsigned char)alpha});
    }
    
    // ============================================================
    // RANDOM STATIC GLITCHES (subtle)
    // ============================================================
    if (fmodf(t * 0.3f, 1.0f) > 0.97f) {
        float glitchX = rand() % (int)width;
        float glitchW = 20 + rand() % 80;
        DrawScaledRect(glitchX, 0, glitchW, height, {20, 22, 30, 15});
    }
}