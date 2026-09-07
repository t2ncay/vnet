#include "raid.h"
#include "../../shared/vnet.h"
#include "../render.h"
#include "../game.h"
#include <cstdlib>
#include <cmath>
#include <ctime>

extern Player g_player;

// ============================================================
// NETWORK VISUALIZATION (private helper)
// ============================================================
// ============================================================
// NETWORK VISUALIZATION - Replaces 3D Operator Face
// ============================================================
void DrawNetworkVisualization(float x, float y, float w, float h) {
    float t = (float)GetTime();
    float pulse = sinf(t * 3.0f) * 0.3f + 0.7f;
    
    // ---- BACKGROUND PANEL ----
    DrawScaledRect(x, y, w, h, Color{6, 8, 14, 230});
    DrawScaledRectLines(x, y, w, h, Fade(COLOR_CYAN, 0.3f));
    
    // ---- HEADER ----
    DrawScaledText("█ PORT PROBE // NODE SWAP", x + 12, y + 8, 11, COLOR_TOXIC);
    DrawScaledLine(x + 10, y + 26, x + w - 10, y + 26, Fade(COLOR_CYAN, 0.2f));
    
    // ---- NETWORK NODES ----
    float centerX = x + w / 2.0f;
    float centerY = y + h / 2.0f + 10.0f;
    float radius = 60.0f;
    int nodeCount = 6;
    
    // Draw connection lines between nodes
    for (int i = 0; i < nodeCount; i++) {
        float angle1 = t * 0.3f + (i / (float)nodeCount) * 6.28318f;
        float angle2 = t * 0.3f + ((i + 1) / (float)nodeCount) * 6.28318f;
        float x1 = centerX + cosf(angle1) * radius;
        float y1 = centerY + sinf(angle1) * radius;
        float x2 = centerX + cosf(angle2) * radius;
        float y2 = centerY + sinf(angle2) * radius;
        
        // Data flow along the line (moving dots)
        float flowPos = fmodf(t * 0.8f + i * 0.2f, 1.0f);
        float fx = x1 + (x2 - x1) * flowPos;
        float fy = y1 + (y2 - y1) * flowPos;
        
        // Glowing connection
        DrawScaledLine(x1, y1, x2, y2, Fade(COLOR_CYAN, 0.15f + 0.1f * pulse));
        // Data packet
        DrawScaledRect(fx - 2, fy - 2, 4, 4, Fade(COLOR_TOXIC, 0.6f + 0.3f * pulse));
    }
    
    // Draw nodes with port numbers
    for (int i = 0; i < nodeCount; i++) {
        float angle = t * 0.3f + (i / (float)nodeCount) * 6.28318f;
        float nx = centerX + cosf(angle) * radius;
        float ny = centerY + sinf(angle) * radius;
        float nodePulse = sinf(t * 2.0f + i * 1.2f) * 0.3f + 0.7f;
        
        // Node glow
        DrawScaledCircle(nx, ny, 12.0f * nodePulse, Fade(COLOR_BLOOD, 0.15f));
        DrawScaledCircle(nx, ny, 8.0f, Fade(COLOR_CYAN, 0.25f));
        
        // Node core
        Color nodeColor = (i % 2 == 0) ? COLOR_CYAN : COLOR_TOXIC;
        DrawScaledRect(nx - 4, ny - 4, 8, 8, Fade(nodeColor, 0.8f));
        
        // Port label
        char portStr[8];
        int port = 8000 + i * 37 + (int)(t * 1.5f) % 100;
        snprintf(portStr, sizeof(portStr), ":%d", port % 1000 + 8000);
        DrawScaledText(portStr, nx - 12, ny + 14, 7, Fade(COLOR_GHOST, 0.6f));
    }
    
    // ---- SCANNING PULSE ----
    float scanAngle = t * 0.5f;
    float scanX = centerX + cosf(scanAngle) * (radius + 25.0f);
    float scanY = centerY + sinf(scanAngle) * (radius + 25.0f);
    DrawScaledCircle(scanX, scanY, 6.0f, Fade(COLOR_AMBER, 0.8f));
    DrawScaledCircle(scanX, scanY, 18.0f, Fade(COLOR_AMBER, 0.15f));
    
    // ---- STATS BAR ----
    float statsY = y + h - 28;
    DrawScaledLine(x + 10, statsY, x + w - 10, statsY, Fade(COLOR_CYAN, 0.15f));
    
    char stats[128];
    int packets = (int)(t * 12.3f) % 999 + 100;
    int hops = (int)(t * 0.7f) % 5 + 3;
    snprintf(stats, sizeof(stats), "PACKETS: %04d  |  HOPS: %d  |  TRACE: %d%%  |  PORT: %d",
             packets, hops, g_player.traceLevel, g_player.port);
    DrawScaledText(stats, x + 12, statsY + 6, 8, Fade(COLOR_GHOST, 0.7f));
    
    // ---- GLITCH BORDER ----
    if (pulse > 0.85f) {
        float glitchX = x + rand() % (int)w;
        DrawScaledRect(glitchX, y, 2 + rand() % 8, h, Fade(COLOR_BLOOD, 0.1f));
    }
}

void DrawRaidSequenceOverlay() {
    if (!g_player.raidActive) return;

    float t = g_player.raidSeqTimer;
    float pulse = sinf(GetTime() * 8.0f) * 0.5f + 0.5f;

    switch (g_player.raidSeqStage) {
        case RAID_SEQ_GLITCH:
            {
                float t = g_player.raidSeqTimer;
                float duration = g_player.raidSeqStageDuration;
                float progress = t / duration;  // 0 → 1
                float intensity = sinf(progress * 3.14159f) * 0.8f + 0.2f; // Peaks mid-way
                
                // ---- 1. SCREEN TEARING (horizontal displacement) ----
                int tearCount = (int)(5 + intensity * 15);
                for (int i = 0; i < tearCount; i++) {
                    float tearY = rand() % REF_HEIGHT;
                    float tearH = 2 + rand() % (int)(6 + intensity * 10);
                    float tearX = (rand() % (int)(REF_WIDTH * 0.3f)) - REF_WIDTH * 0.15f;
                    float tearW = REF_WIDTH + abs(tearX) * 2;
                    
                    Color tearColor = (i % 3 == 0) ? Fade(COLOR_BLOOD, 0.15f * intensity) :
                                    (i % 3 == 1) ? Fade(COLOR_TOXIC, 0.1f * intensity) :
                                    Fade(COLOR_CYAN, 0.08f * intensity);
                    DrawScaledRect(tearX, tearY, tearW, tearH, tearColor);
                }
                
                // ---- 2. CHROMATIC ABERRATION (color channel offset) ----
                if (intensity > 0.4f) {
                    float offset = (intensity - 0.4f) * 4.0f; // max 2.4px
                    float rOffset = (rand() % 100 / 100.0f) * offset;
                    float bOffset = (rand() % 100 / 100.0f) * offset;
                    // We'll apply via drawing red/blue overlay rectangles with blend
                    // Simulate with semi-transparent colored bars
                    for (int i = 0; i < (int)(5 + intensity * 20); i++) {
                        float x = rand() % REF_WIDTH;
                        float y = rand() % REF_HEIGHT;
                        float w = 20 + rand() % 100;
                        float h = 2 + rand() % 6;
                        if (i % 2 == 0)
                            DrawScaledRect(x + rOffset, y, w, h, Fade(COLOR_BLOOD, 0.08f * intensity));
                        else
                            DrawScaledRect(x - bOffset, y, w, h, Fade(COLOR_CYAN, 0.08f * intensity));
                    }
                }
                
                // ---- 3. DATA CORRUPTION BLOCKS (random hex characters) ----
                int blockCount = (int)(3 + intensity * 12);
                for (int i = 0; i < blockCount; i++) {
                    float bx = rand() % REF_WIDTH;
                    float by = rand() % REF_HEIGHT;
                    float bw = 20 + rand() % 60;
                    float bh = 12 + rand() % 24;
                    
                    // Background block
                    Color bgColor = (i % 3 == 0) ? Fade(COLOR_BLOOD, 0.1f * intensity) :
                                    (i % 3 == 1) ? Fade(COLOR_TOXIC, 0.08f * intensity) :
                                    Fade(COLOR_BLACK, 0.2f * intensity);
                    DrawScaledRect(bx, by, bw, bh, bgColor);
                    
                    // Hex characters inside block
                    int hexCount = 3 + rand() % 6;
                    for (int j = 0; j < hexCount; j++) {
                        char hexChars[] = "0123456789ABCDEF";
                        char c = hexChars[rand() % 16];
                        float hx = bx + 4 + (rand() % (int)(bw - 8));
                        float hy = by + 2 + (rand() % (int)(bh - 6));
                        Color col = (i % 2 == 0) ? Fade(COLOR_BLOOD, 0.5f * intensity) :
                                                Fade(COLOR_TOXIC, 0.4f * intensity);
                        DrawScaledText(&c, hx, hy, 10 + rand() % 6, col);
                    }
                }
                
                // ---- 4. AGGRESSIVE JITTER & SCANLINES ----
                float jitterIntensity = 0.5f + intensity * 1.5f;
                TriggerJitter(jitterIntensity * 0.2f, 0.02f + intensity * 0.08f);
                
                // Enhanced scanlines with varying frequency
                float scanlineFreq = 60.0f + intensity * 120.0f;
                float scanlineAlpha = 0.1f + intensity * 0.4f;
                DrawScanlineOverlay(0, 0, REF_WIDTH, REF_HEIGHT, 
                                    Fade(COLOR_BLOOD, scanlineAlpha * 0.5f), 
                                    scanlineFreq, 2.0f + intensity * 4.0f);
                
                // ---- 5. FLICKERING WHITE FLASHES (random) ----
                if (rand() % 100 < (int)(intensity * 15)) {
                    float flashAlpha = (rand() % 100 / 100.0f) * 0.2f * intensity;
                    DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, Fade(WHITE, flashAlpha));
                }
                
                // ---- 6. HORIZONTAL BARS (classic glitch) ----
                int barCount = (int)(3 + intensity * 20);
                for (int i = 0; i < barCount; i++) {
                    float x = rand() % REF_WIDTH;
                    float y = rand() % REF_HEIGHT;
                    float w = 10 + rand() % (int)(40 + intensity * 60);
                    float h = 2 + rand() % (int)(2 + intensity * 6);
                    Color barColor = (i % 4 == 0) ? Fade(COLOR_BLOOD, 0.15f * intensity) :
                                    (i % 4 == 1) ? Fade(COLOR_TOXIC, 0.1f * intensity) :
                                    (i % 4 == 2) ? Fade(COLOR_CYAN, 0.1f * intensity) :
                                    Fade(COLOR_GHOST, 0.1f * intensity);
                    DrawScaledRect(x, y, w, h, barColor);
                }
                
                // ---- 7. VERTICAL LINES (data corruption) ----
                int vLineCount = (int)(2 + intensity * 8);
                for (int i = 0; i < vLineCount; i++) {
                    float x = rand() % REF_WIDTH;
                    float y = rand() % REF_HEIGHT;
                    float w = 1 + rand() % 3;
                    float h = 10 + rand() % (int)(20 + intensity * 40);
                    DrawScaledRect(x, y, w, h, Fade(COLOR_BLOOD, 0.1f * intensity));
                }
                
                break;
            }

        case RAID_SEQ_FLASH:
            // Full white flash with extreme jitter
            {
                float flashAlpha = 1.0f - (t / g_player.raidSeqStageDuration);
                DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, Fade(WHITE, flashAlpha));
                TriggerJitter(1.0f, 0.05f);
                // Add chromatic aberration: offset red/blue channels?
                // For simplicity, we'll just use a bright flash.
            }
            break;

        case RAID_SEQ_HEX:
            {
                float t = g_player.raidSeqTimer;
                float duration = g_player.raidSeqStageDuration;
                float progress = t / duration;  // 0 → 1
                float intensity = sinf(progress * 3.14159f) * 0.8f + 0.2f;
                
                // ---- 1. MAIN DATA STREAM (multiple layers) ----
                int streamCount = 25 + (int)(intensity * 20);
                float baseSpeed = 120.0f + intensity * 200.0f;
                
                // Pre-compute stream data for consistency
                static float streamOffsets[50] = {0};
                static float streamSpeeds[50] = {0};
                static int streamLengths[50] = {0};
                static float streamX[50] = {0};
                
                // Initialize streams (only once per stage)
                static bool initialized = false;
                if (!initialized) {
                    for (int i = 0; i < 50; i++) {
                        streamOffsets[i] = (float)(rand() % 1000) / 1000.0f * 2.0f;
                        streamSpeeds[i] = 0.3f + (rand() % 100 / 100.0f) * 0.7f;
                        streamLengths[i] = 5 + rand() % 20;
                        streamX[i] = (rand() % (int)(REF_WIDTH * 0.9f)) + REF_WIDTH * 0.05f;
                    }
                    initialized = true;
                }
                
                // Update and draw streams
                float hexW = 18.0f;
                float hexH = 20.0f;
                char hexChars[] = "0123456789ABCDEF";
                
                // Draw multiple streams
                for (int s = 0; s < streamCount; s++) {
                    float speed = streamSpeeds[s] * baseSpeed;
                    float offset = streamOffsets[s];
                    float progress2 = fmodf(t * speed + offset, 2.0f);
                    if (progress2 > 1.0f) continue;
                    
                    float alpha = (1.0f - progress2) * 0.7f + 0.2f;
                    float yPos = (1.0f - progress2) * REF_HEIGHT;
                    float xPos = streamX[s];
                    
                    // Determine color
                    Color col;
                    int colorType = rand() % 3;
                    if (colorType == 0) col = Fade(COLOR_TOXIC, alpha);
                    else if (colorType == 1) col = Fade(COLOR_CYAN, alpha * 0.7f);
                    else col = Fade(COLOR_GHOST, alpha * 0.5f);
                    
                    // Draw stream characters with varying size
                    int len = streamLengths[s];
                    for (int j = 0; j < len; j++) {
                        char c = hexChars[(rand() % 16)];
                        float yOffset = j * hexH;
                        float yDraw = yPos + yOffset;
                        if (yDraw < 0 || yDraw > REF_HEIGHT) continue;
                        
                        // Random size variation per character
                        float size = 14.0f + (rand() % 12);
                        if (j % 3 == 0) size *= 1.2f;  // Some characters larger
                        
                        // Random color variation per character
                        Color charCol = col;
                        if (rand() % 5 == 0) charCol = Fade(COLOR_BLOOD, alpha * 0.6f);
                        
                        DrawScaledText(&c, xPos, yDraw, size, charCol);
                    }
                }
                
                // ---- 2. GLITCHY HEX BLOCKS (overlay) ----
                int blockCount = (int)(3 + intensity * 15);
                for (int i = 0; i < blockCount; i++) {
                    float bx = rand() % REF_WIDTH;
                    float by = rand() % REF_HEIGHT;
                    float bw = 30 + rand() % 80;
                    float bh = 15 + rand() % 40;
                    
                    // Random semitransparent block
                    Color bg = (i % 2 == 0) ? Fade(COLOR_BLOOD, 0.05f * intensity) :
                                            Fade(COLOR_TOXIC, 0.04f * intensity);
                    DrawScaledRect(bx, by, bw, bh, bg);
                    
                    // Random hex inside block
                    int inner = 2 + rand() % 5;
                    for (int j = 0; j < inner; j++) {
                        char c = hexChars[rand() % 16];
                        float hx = bx + 4 + (rand() % (int)(bw - 8));
                        float hy = by + 2 + (rand() % (int)(bh - 6));
                        DrawScaledText(&c, hx, hy, 12 + rand() % 8, 
                                    Fade(COLOR_TOXIC, 0.6f * intensity));
                    }
                }
                
                // ---- 3. DATA STREAKS (fast moving lines) ----
                int streakCount = (int)(2 + intensity * 6);
                for (int i = 0; i < streakCount; i++) {
                    float x = rand() % REF_WIDTH;
                    float y = rand() % REF_HEIGHT;
                    float w = 80 + rand() % 150;
                    float h = 1 + rand() % 3;
                    float speed = 0.2f + rand() % 100 / 100.0f;
                    float pos = fmodf(t * speed * 200.0f + i * 50.0f, REF_HEIGHT + 100.0f) - 50.0f;
                    DrawScaledRect(x, pos, w, h, Fade(COLOR_CYAN, 0.2f * intensity));
                }
                
                // ---- 4. SCREEN FLICKER (random white/color flashes) ----
                if (rand() % 100 < (int)(intensity * 20)) {
                    float flashAlpha = (rand() % 100 / 100.0f) * 0.15f * intensity;
                    Color flashColor = (rand() % 2 == 0) ? Fade(WHITE, flashAlpha) : Fade(COLOR_CYAN, flashAlpha * 0.8f);
                    DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, flashColor);
                }
                
                // ---- 5. SCANLINE OVERLAY (aggressive) ----
                float scanFreq = 40.0f + intensity * 100.0f;
                float scanAlpha = 0.1f + intensity * 0.3f;
                DrawScanlineOverlay(0, 0, REF_WIDTH, REF_HEIGHT, 
                                    Fade(COLOR_BLOOD, scanAlpha * 0.3f), 
                                    scanFreq, 2.0f + intensity * 3.0f);
                
                // ---- 6. DARKENING OVERLAY (pulsing) ----
                float darkAlpha = 0.2f + intensity * 0.2f;
                DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, Fade(BLACK, darkAlpha));
                
                break;
            }

        case RAID_SEQ_BLACKOUT:
            // Pure black
            DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, BLACK);
            break;

        case RAID_SEQ_ACTIVE: {
            // Desktop is locked, background is black
            float t = (float)GetTime();
            float pulse = sinf(t * 3.0f) * 0.3f + 0.7f;
            
            // ---- SPLIT LAYOUT: LEFT = HEX STREAM, RIGHT = TERMINAL ----
            float leftX = 0;
            float leftW = REF_WIDTH * 0.55f;
            float rightX = leftW;
            float rightW = REF_WIDTH * 0.45f;
            
            // ============================================================
            // LEFT PANEL: MODERN DASHBOARD (Hex Sidebar, Radar, Sniffer)
            // ============================================================
            {
                // 1. Clean Background (No chaotic lines, just a dark slate)
                DrawScaledRect(leftX, 0, leftW, REF_HEIGHT, Color{4, 6, 10, 255});
                
                // Subtle dot-matrix grid instead of heavy lines
                for (int x = 10; x < leftW; x += 20) {
                    for (int y = 10; y < REF_HEIGHT; y += 20) {
                        DrawScaledRect(leftX + x, y, 1, 1, Fade(COLOR_CYAN, 0.05f));
                    }
                }

                // ---- LAYOUT DEFINITIONS ----
                // Sidebar for Hex Dump
                float sideW = 60.0f;
                float sideX = leftX + 10.0f;
                
                // Main content area math
                float pad = 10.0f;
                float mainX = sideX + sideW + pad;
                float mainW = leftW - sideW - (pad * 3);
                
                float radarY = pad;
                float radarH = REF_HEIGHT * 0.35f;
                
                float snifferY = radarY + radarH + pad;
                float snifferH = REF_HEIGHT * 0.45f;
                
                float opY = snifferY + snifferH + pad;
                float opH = REF_HEIGHT - opY - pad;

                // ------------------------------------------------------------
                // MODULE 1: HEX DUMP SIDEBAR (Constrained, doesn't ruin readability)
                // ------------------------------------------------------------
                DrawScaledRect(sideX, pad, sideW, REF_HEIGHT - (pad*2), Color{6, 8, 14, 180});
                DrawScaledRectLines(sideX, pad, sideW, REF_HEIGHT - (pad*2), Fade(COLOR_CYAN, 0.1f));
                DrawScaledText("MEM", sideX + 20, pad + 5, 8, Fade(COLOR_CYAN, 0.5f));
                
                int hexCols = 4; // Down from 30, much cleaner
                int hexRows = 35;
                float hexW = sideW / hexCols;
                float hexH = (REF_HEIGHT - (pad*2)) / hexRows;
                
                for (int col = 0; col < hexCols; col++) {
                    for (int row = 2; row < hexRows - 1; row++) {
                        char hexChars[] = "0123456789ABCDEF";
                        char c = hexChars[rand() % 16];
                        
                        float speed = 0.8f + (col % 2) * 0.2f;
                        float phase = col * 0.5f + row * 0.1f;
                        float offset = fmodf(t * speed + phase, 2.0f);
                        
                        if (offset < 1.0f && (rand() % 100) < 60) { // Sparser density
                            float alpha = (1.0f - offset) * 0.6f;
                            Color colColor = (col % 2 == 0) ? Fade(COLOR_TOXIC, alpha) : Fade(COLOR_GHOST, alpha * 0.5f);
                            
                            float xPos = sideX + col * hexW + 6.0f;
                            float yPos = pad + row * hexH - offset * hexH;
                            DrawScaledText(&c, xPos, yPos, hexH * 0.8f, colColor);
                        }
                    }
                }

                // ------------------------------------------------------------
                // MODULE 2: RADAR & TARGET ACQUISITION
                // ------------------------------------------------------------
                DrawScaledRect(mainX, radarY, mainW, radarH, Color{6, 8, 14, 180});
                DrawScaledRectLines(mainX, radarY, mainW, radarH, Fade(COLOR_CYAN, 0.15f));
                
                // Tech Header
                DrawScaledRect(mainX, radarY, mainW, 18, Fade(COLOR_CYAN, 0.1f));
                DrawScaledText(":: SIGNAL GEO-LOCATION ::", mainX + 8, radarY + 4, 9, COLOR_CYAN);
                
                // Modern Radar Drawing
                float rX = mainX + (mainW / 2.0f);
                float rY = radarY + (radarH / 2.0f) + 8.0f;
                float maxR = radarH * 0.35f;
                
                // Crosshairs
                DrawScaledLine(mainX + 20, rY, mainX + mainW - 20, rY, Fade(COLOR_CYAN, 0.1f));
                DrawScaledLine(rX, radarY + 25, rX, radarY + radarH - 10, Fade(COLOR_CYAN, 0.1f));
                
                DrawScaledCircleLines(rX, rY, maxR, Fade(COLOR_CYAN, 0.3f));
                DrawScaledCircleLines(rX, rY, maxR * 0.66f, Fade(COLOR_CYAN, 0.15f));
                DrawScaledCircleLines(rX, rY, maxR * 0.33f, Fade(COLOR_CYAN, 0.05f));
                
                // Smooth Scanner Sweep
                float angle = t * 2.0f;
                DrawScaledLine(rX, rY, rX + cosf(angle) * maxR, rY + sinf(angle) * maxR, Fade(COLOR_TOXIC, 0.6f));
                
                // Blips
                for (int i = 0; i < 3; i++) {
                    float a = t * 0.2f + i * 2.1f;
                    float r = maxR * (0.4f + 0.4f * sinf(i * 1.5f));
                    float bx = rX + cosf(a) * r;
                    float by = rY + sinf(a) * r;
                    float blipAlpha = sinf(t * 3.0f - angle + a) > 0.8f ? 1.0f : 0.2f; // Flash when scanner passes
                    DrawScaledCircle(bx, by, 3.0f, Fade(COLOR_BLOOD, blipAlpha));
                    if (blipAlpha > 0.5f) {
                        DrawScaledRectLines(bx - 6, by - 6, 12, 12, Fade(COLOR_TOXIC, blipAlpha));
                    }
                }

                // ------------------------------------------------------------
                // MODULE 3: NETWORK SNIFFER (Tabular & Clean)
                // ------------------------------------------------------------
                DrawScaledRect(mainX, snifferY, mainW, snifferH, Color{6, 8, 14, 180});
                DrawScaledRectLines(mainX, snifferY, mainW, snifferH, Fade(COLOR_CYAN, 0.15f));
                
                DrawScaledRect(mainX, snifferY, mainW, 18, Fade(COLOR_CYAN, 0.1f));
                DrawScaledText(":: PACKET CAPTURE (PROMISCUOUS MODE) ::", mainX + 8, snifferY + 4, 9, COLOR_CYAN);
                
                // ---- COLUMN X POSITIONS (Relative to panel width) ----
                float colTime = mainX + 8;
                float colSrc  = mainX + mainW * 0.15f;
                float colDst  = mainX + mainW * 0.48f;
                float colType = mainX + mainW * 0.80f;
                float colSize = mainX + mainW * 0.90f;

                // Column Headers
                float colY = snifferY + 22;
                DrawScaledText("TIME", colTime, colY, 8, Fade(COLOR_GHOST, 0.6f));
                DrawScaledText("SRC IP:PORT", colSrc, colY, 8, Fade(COLOR_GHOST, 0.6f));
                DrawScaledText("DST IP:PORT", colDst, colY, 8, Fade(COLOR_GHOST, 0.6f));
                DrawScaledText("TYPE", colType, colY, 8, Fade(COLOR_GHOST, 0.6f));
                DrawScaledText("SIZE", colSize, colY, 8, Fade(COLOR_GHOST, 0.6f));
                DrawScaledLine(mainX + 5, colY + 12, mainX + mainW - 5, colY + 12, Fade(COLOR_CYAN, 0.2f));

                // Packet List (Scrolling cleanly upwards)
                float packetH = 14.0f;
                int maxPackets = (int)((snifferH - 50) / packetH);
                
                for (int i = 0; i < maxPackets; i++) {
                    float yPos = colY + 16 + (i * packetH);
                    
                    // Generate stable pseudo-random data per row based on time
                    int seed = (int)(t * 2.0f) + i; 
                    srand(seed); 
                    
                    char timeStr[16], srcStr[32], dstStr[32], typeStr[8], sizeStr[16];
                    snprintf(timeStr, sizeof(timeStr), "%04d", (int)(t * 10 + i) % 9999);
                    snprintf(srcStr, sizeof(srcStr), "%d.%d.%d.%d:%d", 192, 168, 1, rand()%255, 8000 + rand()%999);
                    snprintf(dstStr, sizeof(dstStr), "%d.%d.%d.%d:%d", rand()%255, rand()%255, rand()%255, rand()%255, 80);
                    
                    int type = rand() % 3;
                    strcpy(typeStr, type == 0 ? "TCP" : (type == 1 ? "UDP" : "HTTP"));
                    snprintf(sizeStr, sizeof(sizeStr), "%db", 64 + rand()%1400);
                    
                    // Row alternating color
                    Color rowCol = (i % 2 == 0) ? Fade(COLOR_GHOST, 0.8f) : Fade(COLOR_CYAN, 0.6f);
                    if (type == 2) rowCol = COLOR_TOXIC; // Highlight HTTP
                    
                    DrawScaledText(timeStr, colTime, yPos, 8, Fade(COLOR_GHOST, 0.4f));
                    DrawScaledText(srcStr, colSrc, yPos, 8, rowCol);
                    DrawScaledText(dstStr, colDst, yPos, 8, rowCol);
                    DrawScaledText(typeStr, colType, yPos, 8, (type==2) ? COLOR_TOXIC : Fade(COLOR_GHOST, 0.5f));
                    DrawScaledText(sizeStr, colSize, yPos, 8, Fade(COLOR_GHOST, 0.4f));
                }
                
                // Sniffer Stats Footer
                float statsY = snifferY + snifferH - 20;
                DrawScaledLine(mainX + 5, statsY, mainX + mainW - 5, statsY, Fade(COLOR_CYAN, 0.2f));
                char statsLine[128];
                snprintf(statsLine, sizeof(statsLine), "PKT/S: %d  |  TOTAL: %d  |  DROPS: 0", (int)(t * 34.7f) % 1000 + 200, (int)(t * 128.3f) % 99999 + 10000);
                DrawScaledText(statsLine, mainX + 8, statsY + 6, 8, Fade(COLOR_TOXIC, 0.7f));

                // ------------------------------------------------------------
                // MODULE 4: OPERATOR DIALOGUE
                // ------------------------------------------------------------
                DrawScaledRect(mainX, opY, mainW, opH, Color{10, 2, 2, 180}); // Slight red tint
                DrawScaledRectLines(mainX, opY, mainW, opH, Fade(COLOR_BLOOD, 0.4f));
                
                DrawScaledRect(mainX, opY, mainW, 16, Fade(COLOR_BLOOD, 0.2f));
                DrawScaledText(":: COMMS LINK ::", mainX + 8, opY + 4, 8, COLOR_BLOOD);
                
                // Blinking prompt
                if ((int)(t * 2.0f) % 2 == 0) {
                    DrawScaledRect(mainX + 8, opY + 24, 6, 12, COLOR_TOXIC);
                }
                
                DrawScaledText(g_player.raidSeqOperatorDialogue, mainX + 20, opY + 25, 11, COLOR_TOXIC);
            }
            
            // ============================================================
            // RIGHT PANEL: TERMINAL WINDOW
            // ============================================================
            {
                float winX = rightX + 8;
                float winY = 40;
                float winW = rightW - 16;
                float winH = REF_HEIGHT - 80;
                
                // ---- TERMINAL WINDOW FRAME ----
                DrawScaledRect(winX + 4, winY + 4, winW, winH, Color{0, 0, 0, 80});
                DrawScaledRect(winX, winY, winW, winH, COLOR_PANEL);
                DrawScaledRectLines(winX, winY, winW, winH, Fade(COLOR_CYAN, 0.25f));
                
                // ---- TITLE BAR ----
                float titleH = 28.0f;
                DrawScaledRect(winX, winY, winW, titleH, Color{16, 18, 28, 220});
                DrawScaledLine(winX, winY + titleH, winX + winW, winY + titleH, Fade(COLOR_CYAN, 0.15f));
                
                DrawScaledText("█ RAID TERMINAL", winX + 12, winY + 6, 10, COLOR_TOXIC);
                
                // ---- TERMINAL CONTENT ----
                float contentX = winX + 6;
                float contentY = winY + titleH + 6;
                float contentW = winW - 12;
                float contentH = winH - titleH - 12;
                
                DrawScaledRect(contentX, contentY, contentW, contentH, COLOR_CLI_BG);
                DrawScaledRectLines(contentX, contentY, contentW, contentH, Fade(COLOR_CYAN, 0.05f));
                
                // ---- CLI LOGS (compact) ----
                float lineY = contentY + 4;
                float fontSize = 9.0f;
                float lineHeight = 16.0f;
                int maxLines = (int)(contentH / lineHeight) - 2;
                int totalLines = g_cliLogCount;
                int startLine = totalLines - maxLines;
                if (startLine < 0) startLine = 0;
                
                if (g_player.cliScroll > 0.0f) {
                    int scrollLines = (int)(g_player.cliScroll / lineHeight);
                    startLine = totalLines - maxLines - scrollLines;
                    if (startLine < 0) startLine = 0;
                }
                
                for (int i = startLine; i < g_cliLogCount && i < startLine + maxLines; i++) {
                    if (i < 0) continue;
                    const char* log = g_cliLogs[i];
                    
                    Color logColor = COLOR_GHOST;
                    if (strstr(log, "[FEDERAL RAID]") || strstr(log, "[RAID]")) logColor = COLOR_BLOOD;
                    else if (strstr(log, "[ERROR]") || strstr(log, "[ERR]")) logColor = COLOR_BLOOD;
                    else if (strstr(log, "[WARNING]") || strstr(log, "[WARN]")) logColor = COLOR_AMBER;
                    else if (strstr(log, "[SUCCESS]") || strstr(log, "[MINER]")) logColor = COLOR_TOXIC;
                    else if (strstr(log, "[SCAN]") || strstr(log, "[PAGE]")) logColor = COLOR_CYAN;
                    
                    DrawScaledText(log, contentX + 4, lineY, fontSize, logColor);
                    lineY += lineHeight;
                }
                
                // ---- INPUT LINE ----
                float inputY = contentY + contentH - 24;
                DrawScaledRect(contentX + 2, inputY, contentW - 4, 20, Color{8, 10, 14, 220});
                DrawScaledRectLines(contentX + 2, inputY, contentW - 4, 20, Fade(COLOR_CYAN, 0.15f));
                
                DrawScaledText(">", contentX + 8, inputY + 4, fontSize, COLOR_TOXIC);
                DrawScaledText(g_player.inputBuffer, contentX + 20, inputY + 4, fontSize, COLOR_GHOST);
                
                if ((int)(GetTime() * 2.0f) % 2 == 0) {
                    float cursorX = contentX + 20 + MeasureScaledTextWidth(g_player.inputBuffer, fontSize);
                    DrawScaledRect(cursorX, inputY + 2, 5, 14, COLOR_TOXIC);
                }
                
                // ---- SCROLLBAR ----
                if (totalLines > maxLines) {
                    float scrollbarX = contentX + contentW - 6;
                    float scrollbarH = contentH - 4;
                    float visibleRatio = (float)maxLines / totalLines;
                    float thumbH = scrollbarH * visibleRatio;
                    if (thumbH < 12.0f) thumbH = 12.0f;
                    
                    float maxScrollPx = (totalLines - maxLines) * lineHeight;
                    float scrollRatio = 0.0f;
                    if (maxScrollPx > 0.0f) {
                        scrollRatio = g_player.cliScroll / maxScrollPx;
                        if (scrollRatio > 1.0f) scrollRatio = 1.0f;
                    }
                    float thumbY = contentY + 2 + (scrollbarH - thumbH) * scrollRatio;
                    
                    DrawScaledRect(scrollbarX, contentY + 2, 4, scrollbarH, Color{30, 35, 50, 80});
                    DrawScaledRect(scrollbarX, thumbY, 4, thumbH, Fade(COLOR_CYAN, 0.3f));
                }
                
                // ---- HELP TEXT ----
                DrawScaledText("[ENTER] send  |  [TAB] focus  |  [↑↓] scroll", 
                            contentX + 4, contentY + contentH - 46, 7, Fade(COLOR_GHOST, 0.3f));
            }
            
            // ---- DIVIDER LINE ----
            DrawScaledLine(leftW, 0, leftW, REF_HEIGHT, Fade(COLOR_CYAN, 0.05f));
            
            break;
        }
        case RAID_SEQ_SUCCESS:
            // Green flash with "RAID BYPASSED"
            {
                float alpha = 1.0f - (t / 2.0f);
                DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, Fade(GREEN, alpha * 0.6f));
                const char* msg = "⚡ RAID BYPASSED ⚡";
                float w = MeasureScaledTextWidth(msg, 48);
                // Shadow
                DrawScaledText(msg, (REF_WIDTH - w) / 2 + 2, REF_HEIGHT / 2 - 18 + 2, 48, Fade(BLACK, alpha * 0.5f));
                DrawScaledText(msg, (REF_WIDTH - w) / 2, REF_HEIGHT / 2 - 20, 48, Fade(COLOR_TOXIC, alpha));
            }
            break;

        case RAID_SEQ_FAILURE:
            // Red flash with "RAID SUCCESS"
            {
                float alpha = 1.0f - (t / 2.0f);
                DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, Fade(RED, alpha * 0.6f));
                const char* msg = "💀 RAID SUCCESS 💀";
                float w = MeasureScaledTextWidth(msg, 48);
                // Shadow
                DrawScaledText(msg, (REF_WIDTH - w) / 2 + 2, REF_HEIGHT / 2 - 18 + 2, 48, Fade(BLACK, alpha * 0.5f));
                DrawScaledText(msg, (REF_WIDTH - w) / 2, REF_HEIGHT / 2 - 20, 48, Fade(COLOR_BLOOD, alpha));
            }
            break;
    }
}