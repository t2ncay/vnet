#include "wallpaper.h"
#include "render.h"
#include <cmath>
#include <ctime>
#include <cstring>

// ============================================================
// HELPER: Draw crisp, scaled text with proper size
// ============================================================
static void DrawCrispText(const char* text, float x, float y, float fontSize, Color color) {
    // Use DrawScaledText which handles the scaling properly
    DrawScaledText(text, x, y, fontSize, color);
}

// ============================================================
// HELPER: Draw text curved along a circle (CIA/Pentagon seal style)
// ============================================================
// centerAngleDeg is a clock-angle: 0 = top (12 o'clock), 90 = right (3),
// 180 = bottom (6), 270 = left (9), sweeping clockwise as it increases.
// `flip` should be true for text along the BOTTOM half of a ring so the
// letters stay upright (readable) instead of appearing upside-down.
static void DrawCircularText(const char* text, float cx, float cy, float radius,
                              float centerAngleDeg, float fontSize, Color color, bool flip) {
    if (g_fontVCR.texture.id == 0) return;  // rotation requires the loaded Font
    int len = (int)strlen(text);
    if (len == 0) return;

    float scaledFontSize = fontSize * g_uiScale * g_fontScale;
    Vector2 cellSize = MeasureTextEx(g_fontVCR, "M", scaledFontSize, 1.0f);  // monospace cell
    float charSpacing = cellSize.x + 1.0f;
    float circumference = 2.0f * PI * radius * g_uiScale;
    float anglePerChar = (circumference > 0.0f) ? (charSpacing / circumference) * 360.0f : 0.0f;

    float angle = centerAngleDeg - (anglePerChar * (len - 1)) / 2.0f;
    Vector2 origin = { cellSize.x / 2.0f, cellSize.y / 2.0f };

    for (int i = 0; i < len; i++) {
        char buf[2] = { text[i], '\0' };
        float rad = angle * DEG2RAD;
        float px = cx + radius * sinf(rad);
        float py = cy - radius * cosf(rad);
        Vector2 pos = { SX(px), SY(py) };
        float rotation = angle + (flip ? 180.0f : 0.0f);

        DrawTextPro(g_fontVCR, buf, pos, origin, rotation, scaledFontSize, 1.0f, color);
        angle += anglePerChar;
    }
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
    // CENTRAL SEAL - CIA / PENTAGON STYLE EMBLEM (VEKTRA)
    // ============================================================
    float centerX = width / 2.0f;
    float centerY = height / 2.0f - 6.0f;

    float sealRadius      = 148.0f;   // outer edge of the ring band
    float sealInnerRadius = 138.0f;   // inner edge of the ring band
    float coreRadius      = 110.0f;   // inner disc holding the emblem

    // ---- SOFT OUTER GLOW ----
    for (int i = 5; i >= 1; i--) {
        float gr = sealRadius + i * 6.0f;
        unsigned char alpha = (unsigned char)(3 * (6 - i));
        DrawScaledCircle(centerX, centerY, gr, {0, 180, 200, alpha});
    }

    // ---- OUTER RING BAND (double border, engraved hairlines) ----
    DrawScaledCircle(centerX, centerY, sealRadius, {68, 72, 84, 255});
    DrawScaledCircle(centerX, centerY, sealInnerRadius, baseBg);
    DrawScaledCircleLines(centerX, centerY, sealRadius, {150, 155, 168, 220});
    DrawScaledCircleLines(centerX, centerY, sealInnerRadius, {150, 155, 168, 160});
    DrawScaledCircleLines(centerX, centerY, sealRadius - 3.0f, {35, 37, 44, 200});
    DrawScaledCircleLines(centerX, centerY, sealInnerRadius + 3.0f, {35, 37, 44, 150});

    // ---- CURVED RING TEXT ----
    float ringTextRadius = (sealRadius + sealInnerRadius) / 2.0f;
    DrawCircularText("VEKTRA INTELLIGENCE DIVISION", centerX, centerY, ringTextRadius,
                      0.0f, 11.0f, {205, 210, 220, 235}, false);
    DrawCircularText("COLD SIGNAL // CYBER OPERATIONS", centerX, centerY, ringTextRadius,
                      180.0f, 10.0f, {185, 190, 202, 210}, true);

    // Small star markers at the seam between the two arcs (3 & 9 o'clock)
    for (int side = 0; side < 2; side++) {
        float markAngle = (side == 0) ? 90.0f : 270.0f;
        float rad = markAngle * DEG2RAD;
        float mx = centerX + sinf(rad) * ringTextRadius;
        float my = centerY - cosf(rad) * ringTextRadius;
        float starW = MeasureScaledTextWidth("*", 12);
        DrawScaledText("*", mx - starW / 2.0f, my - 7.0f, 12, {205, 210, 220, 220});
    }

    // ---- INNER CORE BORDER ----
    DrawScaledCircleLines(centerX, centerY, coreRadius, {90, 94, 106, 200});

    // ---- SUNBURST / RAYS OF GLORY (behind the emblem) ----
    int rayCount = 32;
    for (int i = 0; i < rayCount; i++) {
        float rayAngle = (360.0f / rayCount) * i;
        float rad = rayAngle * DEG2RAD;
        float innerR = coreRadius - 8.0f;
        float outerR = (i % 2 == 0) ? coreRadius - 34.0f : coreRadius - 22.0f;
        float x1 = centerX + sinf(rad) * innerR;
        float y1 = centerY - cosf(rad) * innerR;
        float x2 = centerX + sinf(rad) * outerR;
        float y2 = centerY - cosf(rad) * outerR;
        DrawScaledLine(x1, y1, x2, y2, {60, 64, 76, 90});
    }

    // ---- LAUREL WREATH (mirrored left / right) ----
    for (int side = 0; side < 2; side++) {
        float dir = (side == 0) ? -1.0f : 1.0f;
        for (int i = 0; i < 10; i++) {
            float t2 = i / 9.0f;
            float branchY = centerY + coreRadius - 30.0f - t2 * (coreRadius * 0.9f);
            float branchX = centerX + dir * (18.0f + t2 * (coreRadius - 40.0f));
            float leafLen = 10.0f - t2 * 4.0f;
            float leafAngleDeg = dir * (20.0f + t2 * 50.0f);
            float rad = leafAngleDeg * DEG2RAD;
            float dx = cosf(rad) * leafLen;
            float dy = sinf(rad) * leafLen;
            DrawScaledLine(branchX - dx, branchY - dy, branchX + dx, branchY + dy,
                           {110, 150, 110, 200});
            DrawScaledLine(branchX, branchY - 5.0f, branchX, branchY + 5.0f,
                           {80, 120, 80, 160});
        }
        DrawScaledLine(centerX + dir * 16.0f, centerY + coreRadius - 30.0f,
                       centerX + dir * (coreRadius - 44.0f), centerY - coreRadius * 0.35f,
                       {70, 100, 70, 150});
    }

    // ---- EAGLE EMBLEM (centered, symmetric) ----
    float eagleCenterY = centerY - 6.0f;

    // Body
    DrawScaledRect(centerX - 20.0f, eagleCenterY - 34.0f, 40.0f, 70.0f, {55, 58, 68, 235});
    // Head
    DrawScaledCircle(centerX, eagleCenterY - 46.0f, 14.0f, {60, 63, 73, 240});
    // Beak
    DrawScaledRect(centerX - 5.0f, eagleCenterY - 44.0f, 10.0f, 8.0f, {180, 150, 60, 220});

    // Wings (mirrored)
    for (int side = 0; side < 2; side++) {
        float dir = (side == 0) ? -1.0f : 1.0f;
        for (int i = 0; i < 14; i++) {
            float t2 = i / 13.0f;
            float baseX = centerX + dir * (22.0f + t2 * 78.0f);
            float wy = eagleCenterY - 20.0f + sinf(t2 * 2.2f) * 26.0f * (1.0f - t2 * 0.3f);
            float wLen = 10.0f + (1.0f - t2) * 8.0f;
            unsigned char alpha = (unsigned char)(210 - i * 8);
            float rectX = (dir > 0) ? baseX : baseX - wLen;
            DrawScaledRect(rectX, wy, wLen, 3.0f, {58, 61, 71, alpha});
        }
    }

    // Tail feathers
    for (int i = -3; i <= 3; i++) {
        float fx = centerX + i * 6.0f;
        DrawScaledRect(fx - 2.0f, eagleCenterY + 34.0f, 4.0f, 22.0f - fabsf((float)i) * 2.0f,
                       {55, 58, 68, 200});
    }

    // Shield on the chest
    float shieldW = 22.0f, shieldH = 30.0f;
    float shieldX = centerX - shieldW / 2.0f;
    float shieldY = eagleCenterY - 12.0f;
    DrawScaledRect(shieldX, shieldY, shieldW, shieldH, {40, 42, 50, 255});
    DrawScaledRectLines(shieldX, shieldY, shieldW, shieldH, {150, 155, 168, 220});
    for (int i = 0; i < 4; i++) {
        float stripeX = shieldX + 3.0f + i * 5.0f;
        DrawScaledLine(stripeX, shieldY + 2.0f, stripeX, shieldY + shieldH - 2.0f,
                       {90, 94, 106, 200});
    }

    // Talons grasping a laurel sprig (left) and a lightning bolt for cyber ops (right)
    DrawScaledLine(centerX - 8.0f, eagleCenterY + 36.0f, centerX - 26.0f, eagleCenterY + 54.0f,
                   {55, 58, 68, 200});
    DrawScaledLine(centerX + 8.0f, eagleCenterY + 36.0f, centerX + 26.0f, eagleCenterY + 54.0f,
                   {55, 58, 68, 200});
    DrawScaledCircle(centerX - 28.0f, eagleCenterY + 56.0f, 4.0f, {90, 130, 90, 220});
    DrawScaledCircle(centerX + 28.0f, eagleCenterY + 56.0f, 4.0f, {0, 180, 200, 220});

    // Constellation of 13 stars above the head
    int starCount = 13;
    float starArcRadius = 30.0f;
    for (int i = 0; i < starCount; i++) {
        float sAngle = -70.0f + (140.0f / (starCount - 1)) * i;
        float rad = sAngle * DEG2RAD;
        float sx = centerX + sinf(rad) * starArcRadius;
        float sy = (eagleCenterY - 66.0f) - cosf(rad) * (starArcRadius * 0.4f);
        DrawScaledCircle(sx, sy, 1.6f, {210, 215, 225, 230});
    }

    // ---- VEKTRA RIBBON BANNER ----
    float ribbonFontSize = 15.0f;
    float ribbonTextW = MeasureScaledTextWidth("VEKTRA", ribbonFontSize);
    float ribbonH = 22.0f;
    float ribbonW = ribbonTextW + 44.0f;
    float ribbonX = centerX - ribbonW / 2.0f;
    float ribbonY = eagleCenterY + 62.0f;

    DrawScaledRect(ribbonX, ribbonY, ribbonW, ribbonH, {30, 32, 40, 255});
    DrawScaledRectLines(ribbonX, ribbonY, ribbonW, ribbonH, {160, 165, 178, 220});
    DrawScaledLine(ribbonX, ribbonY, ribbonX - 8.0f, ribbonY + ribbonH / 2.0f, {160, 165, 178, 180});
    DrawScaledLine(ribbonX, ribbonY + ribbonH, ribbonX - 8.0f, ribbonY + ribbonH / 2.0f, {160, 165, 178, 180});
    DrawScaledLine(ribbonX + ribbonW, ribbonY, ribbonX + ribbonW + 8.0f, ribbonY + ribbonH / 2.0f,
                   {160, 165, 178, 180});
    DrawScaledLine(ribbonX + ribbonW, ribbonY + ribbonH, ribbonX + ribbonW + 8.0f, ribbonY + ribbonH / 2.0f,
                   {160, 165, 178, 180});
    DrawScaledText("VEKTRA", centerX - ribbonTextW / 2.0f, ribbonY + 4.0f, ribbonFontSize,
                   {220, 225, 235, 255});

    // ============================================================
    // SUBTITLE + CLEARANCE LINE (below the seal)
    // ============================================================
    float subY = centerY + sealRadius + 22.0f;
    float subFontSize = 15.0f;

    const char* subtitle = "SILENT WATCH // ADVANCED THREAT SIGNALS";
    float subWidth = MeasureScaledTextWidth(subtitle, subFontSize);
    DrawScaledText(subtitle, centerX - subWidth / 2.0f, subY, subFontSize,
                   {100, 105, 115, 200});

    // Decorative line under subtitle
    float lineY2 = subY + subFontSize + 12.0f;
    float lineW2 = 280.0f;
    DrawScaledLine(centerX - lineW2 / 2.0f, lineY2, centerX - 40.0f, lineY2,
                   {60, 65, 80, 80});
    DrawScaledLine(centerX + 40.0f, lineY2, centerX + lineW2 / 2.0f, lineY2,
                   {60, 65, 80, 80});

    // Diamond accent
    float diamondSize = 8.0f;
    DrawScaledRect(centerX - diamondSize, lineY2 - diamondSize,
                   diamondSize * 2.0f, diamondSize * 2.0f, {60, 65, 80, 120});
    DrawScaledRect(centerX - diamondSize / 2.0f, lineY2 - diamondSize / 2.0f,
                   diamondSize, diamondSize, {140, 145, 160, 180});

    // ============================================================
    // CLEARANCE TAGLINE
    // ============================================================
    float tagY = lineY2 + 30.0f;
    const char* tagline = "AUTHORIZED PERSONNEL ONLY // CLEARANCE REQUIRED";
    float tagW = MeasureScaledTextWidth(tagline, 10);
    DrawScaledText(tagline, centerX - tagW / 2.0f, tagY, 10, {80, 85, 95, 150});
    
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