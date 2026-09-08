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
                    float margin = 12.0f;
                    float winX = rightX + margin;
                    float winY = 40.0f;
                    float winW = rightW - margin * 2;
                    float winH = REF_HEIGHT - 80.0f;
                    
                    // ---- TERMINAL WINDOW FRAME ----
                    DrawScaledRect(winX + 4, winY + 4, winW, winH, Color{0, 0, 0, 80});
                    DrawScaledRect(winX, winY, winW, winH, COLOR_PANEL);
                    DrawScaledRectLines(winX, winY, winW, winH, Fade(COLOR_CYAN, 0.25f));
                    
                    // ---- TITLE BAR ----
                    float titleH = 30.0f;
                    DrawScaledRect(winX, winY, winW, titleH, Color{16, 18, 28, 220});
                    DrawScaledLine(winX, winY + titleH, winX + winW, winY + titleH, Fade(COLOR_CYAN, 0.15f));
                    
                    // Title with status indicator
                    DrawScaledText("█ RAID TERMINAL", winX + 12, winY + 8, 10, COLOR_TOXIC);
                    
                    // Live indicator (right side)
                    float livePulse = sinf(t * 2.0f) * 0.3f + 0.7f;
                    DrawScaledRect(winX + winW - 40, winY + 8, 6, 6, 
                                Fade(COLOR_TOXIC, livePulse));
                    DrawScaledText("LIVE", winX + winW - 28, winY + 6, 8, Fade(COLOR_TOXIC, 0.7f));
                    
                    // Timer display
                    float remaining = GetRaidRemainingTime();
                    char timerStr[32];
                    snprintf(timerStr, sizeof(timerStr), "⏱ %.0fs", remaining);
                    float timerW = MeasureScaledTextWidth(timerStr, 8);
                    DrawScaledText(timerStr, winX + winW - timerW - 50, winY + 7, 8,
                                remaining < 5.0f ? COLOR_BLOOD : COLOR_AMBER);
                    
                    // ---- TERMINAL CONTENT AREA ----
                    float padding = 6.0f;
                    float contentX = winX + padding;
                    float contentY = winY + titleH + padding;
                    float contentW = winW - padding * 2;
                    float contentH = winH - titleH - padding * 2;
                    
                    // Terminal background
                    DrawScaledRect(contentX, contentY, contentW, contentH, COLOR_CLI_BG);
                    DrawScaledRectLines(contentX, contentY, contentW, contentH, Fade(COLOR_CYAN, 0.05f));
                    
                    // ---- SCISSOR FOR LOGS ----
                    float logPadding = 4.0f;
                    float logX = contentX + logPadding;
                    float logY = contentY + logPadding;
                    float logW = contentW - logPadding * 2;
                    float logH = contentH - logPadding * 2;
                    
                    BeginScissorMode(
                        (int)SX(logX),
                        (int)SY(logY),
                        (int)(logW * g_uiScale),
                        (int)(logH * g_uiScale)
                    );
                    
                    // ---- DRAW LOGS ----
                    float lineHeight = 15.0f;
                    float fontSize = 8.5f;
                    int maxLines = (int)(logH / lineHeight) - 1;
                    if (maxLines < 2) maxLines = 2;
                    
                    int totalLines = g_cliLogCount;
                    int startLine = totalLines - maxLines;
                    if (startLine < 0) startLine = 0;
                    
                    // Apply scroll
                    if (g_player.cliScroll > 0.0f) {
                        int scrollLines = (int)(g_player.cliScroll / lineHeight);
                        startLine = totalLines - maxLines - scrollLines;
                        if (startLine < 0) startLine = 0;
                    }
                    
                    // Draw each log line
                    float drawY = logY + 2;
                    for (int i = startLine; i < totalLines && i < startLine + maxLines; i++) {
                        if (i < 0) continue;
                        
                        const char* log = g_cliLogs[i];
                        
                        // Colorize
                        Color logColor = COLOR_GHOST;
                        if (strstr(log, "[FEDERAL RAID]") || strstr(log, "[RAID]")) logColor = COLOR_BLOOD;
                        else if (strstr(log, "[ERROR]") || strstr(log, "[ERR]")) logColor = COLOR_BLOOD;
                        else if (strstr(log, "[WARNING]") || strstr(log, "[WARN]")) logColor = COLOR_AMBER;
                        else if (strstr(log, "[SUCCESS]") || strstr(log, "[MINER]")) logColor = COLOR_TOXIC;
                        else if (strstr(log, "[SCAN]") || strstr(log, "[PAGE]")) logColor = COLOR_CYAN;
                        else if (strstr(log, "[SERVER]")) logColor = COLOR_AMBER;
                        else if (strstr(log, "[WHISPER]")) logColor = COLOR_AMBER;
                        
                        // Truncate if too long
                        char truncated[256];
                        strncpy(truncated, log, 220);
                        truncated[220] = '\0';
                        
                        DrawScaledText(truncated, logX + 2, drawY, fontSize, logColor);
                        drawY += lineHeight;
                    }
                    
                    EndScissorMode();
                    
                    // ---- SCROLLBAR ----
                    if (totalLines > maxLines) {
                        float scrollbarX = contentX + contentW - 8;
                        float scrollbarY = contentY + 2;
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
                        float thumbY = scrollbarY + (scrollbarH - thumbH) * scrollRatio;
                        
                        DrawScaledRect(scrollbarX, scrollbarY, 4, scrollbarH, Color{30, 35, 50, 80});
                        DrawScaledRect(scrollbarX, thumbY, 4, thumbH, Fade(COLOR_CYAN, 0.3f));
                    }
                    
                    // ---- INPUT LINE (at bottom) ----
                    float inputH = 22.0f;
                    float inputY = contentY + contentH - inputH - 2;
                    float inputPad = 2.0f;
                    
                    // Input area background
                    DrawScaledRect(
                        contentX + inputPad,
                        inputY,
                        contentW - inputPad * 2,
                        inputH,
                        Color{8, 10, 14, 220}
                    );
                    DrawScaledRectLines(
                        contentX + inputPad,
                        inputY,
                        contentW - inputPad * 2,
                        inputH,
                        Fade(COLOR_CYAN, 0.15f)
                    );
                    
                    // Prompt
                    DrawScaledText(">", contentX + 8, inputY + 4, fontSize, COLOR_TOXIC);
                    
                    // Input text (with cursor)
                    float inputTextX = contentX + 20;
                    char inputDisplay[256];
                    bool cursorVisible = (int)(GetTime() * 2.0f) % 2 == 0;
                    
                    if (cursorVisible && g_player.cliOpen) {
                        snprintf(inputDisplay, sizeof(inputDisplay), "%s_", g_player.inputBuffer);
                    } else {
                        snprintf(inputDisplay, sizeof(inputDisplay), "%s", g_player.inputBuffer);
                    }
                    
                    DrawScaledText(inputDisplay, inputTextX, inputY + 4, fontSize, COLOR_GHOST);
                    
                    // ---- STATUS BAR (very bottom) ----
                    float statusH = 16.0f;
                    float statusY = contentY + contentH - inputH - statusH - 2;
                    
                    // Status background
                    DrawScaledRect(
                        contentX + inputPad,
                        statusY,
                        contentW - inputPad * 2,
                        statusH,
                        Color{6, 8, 12, 180}
                    );
                    
                    // Status text
                    char statusText[128];
                    snprintf(statusText, sizeof(statusText), 
                            "VCOIN: %.2f  |  ICE: %d/3  |  TRACE: %d%%  |  PORT: %d",
                            g_player.vcoin, g_player.iceShields, g_player.traceLevel, g_player.port);
                    DrawScaledText(statusText, contentX + 8, statusY + 3, 7, Fade(COLOR_GHOST, 0.5f));
                }
            
            // ---- DIVIDER LINE ----
            DrawScaledLine(leftW, 0, leftW, REF_HEIGHT, Fade(COLOR_CYAN, 0.05f));
            
            break;
        }
        case RAID_SEQ_SUCCESS: {
            float t = g_player.raidSeqTimer;
            float duration = 2.0f;
            float progress = t / duration;
            float alpha = 1.0f - progress;
            
            // ---- 1. BACKGROUND ----
            DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, Color{0, 6, 0, 255});
            
            // ---- 2. PULSING RADIAL GLOW ----
            float pulse = sinf(t * 8.0f) * 0.3f + 0.7f;
            float glowSize = 300.0f + pulse * 100.0f;
            float cx = REF_WIDTH / 2.0f;
            float cy = REF_HEIGHT / 2.0f;
            
            // Outer glow
            for (int i = 5; i >= 1; i--) {
                float r = glowSize * (1.0f + i * 0.15f);
                float a = (0.15f / i) * alpha;
                DrawScaledCircle(cx, cy, r, Fade(COLOR_TOXIC, a));
            }
            DrawScaledCircle(cx, cy, glowSize * 0.6f, Fade(COLOR_TOXIC, 0.2f * alpha));
            
            // ---- 3. SUCCESS ICON (Animated Checkmark) ----
            float iconSize = 80.0f;
            float iconY = cy - 60.0f;
            float iconPulse = sinf(t * 4.0f) * 0.15f + 0.85f;
            float scale = (progress < 0.3f) ? progress / 0.3f : 1.0f;
            scale *= iconPulse;
            
            // Circle behind icon
            DrawScaledCircle(cx, iconY, iconSize * 0.7f * scale, Fade(COLOR_TOXIC, 0.15f * alpha));
            DrawScaledCircleLines(cx, iconY, iconSize * 0.7f * scale, Fade(COLOR_TOXIC, 0.4f * alpha));
            
            // Checkmark (drawn with lines)
            float checkSize = 35.0f * scale;
            float checkX = cx;
            float checkY = iconY;
            
            // Checkmark left stroke
            for (int i = 0; i < 10; i++) {
                float pct = i / 9.0f;
                float x1 = checkX - checkSize * 0.5f + pct * checkSize * 0.4f;
                float y1 = checkY + pct * checkSize * 0.4f;
                float x2 = checkX - checkSize * 0.5f + (pct + 0.05f) * checkSize * 0.4f;
                float y2 = checkY + (pct + 0.05f) * checkSize * 0.4f;
                float lineAlpha = (pct < 0.6f) ? alpha : alpha * (1.0f - (pct - 0.6f) / 0.4f);
                DrawScaledLine(x1, y1, x2, y2, Fade(COLOR_TOXIC, lineAlpha * 0.8f));
            }
            
            // Checkmark right stroke (longer)
            for (int i = 0; i < 15; i++) {
                float pct = i / 14.0f;
                float x1 = checkX - checkSize * 0.1f + pct * checkSize * 0.6f;
                float y1 = checkY + checkSize * 0.3f + pct * checkSize * 0.6f;
                float x2 = checkX - checkSize * 0.1f + (pct + 0.05f) * checkSize * 0.6f;
                float y2 = checkY + checkSize * 0.3f + (pct + 0.05f) * checkSize * 0.6f;
                float lineAlpha = (pct < 0.7f) ? alpha : alpha * (1.0f - (pct - 0.7f) / 0.3f);
                DrawScaledLine(x1, y1, x2, y2, Fade(COLOR_TOXIC, lineAlpha * 0.8f));
            }
            
            // Checkmark glow
            DrawScaledRect(cx - checkSize * 0.8f, checkY - checkSize * 0.5f,
                        checkSize * 1.6f, checkSize * 1.4f,
                        Fade(COLOR_TOXIC, 0.05f * alpha));
            
            // ---- 4. MAIN TEXT "RAID BYPASSED" ----
            float fontMain = 44.0f;
            const char* mainMsg = "⚡ RAID BYPASSED ⚡";
            float mainW = MeasureScaledTextWidth(mainMsg, fontMain);
            float mainY = iconY + iconSize * 0.8f + 10.0f;
            
            // Glow behind text
            for (int i = 4; i >= 1; i--) {
                float offset = i * 2.0f;
                DrawScaledText(mainMsg, cx - mainW / 2 + offset, mainY + offset,
                            fontMain, Fade(COLOR_TOXIC, 0.04f * alpha * (5 - i) / 4.0f));
            }
            DrawScaledText(mainMsg, cx - mainW / 2, mainY, fontMain,
                        Fade(COLOR_TOXIC, alpha));
            
            // ---- 5. SUBTEXT ----
            float fontSub = 16.0f;
            const char* subMsg = "FEDERAL SWARM NEUTRALIZED • THREAT ELIMINATED";
            float subW = MeasureScaledTextWidth(subMsg, fontSub);
            float subY = mainY + fontMain + 16.0f;
            DrawScaledText(subMsg, cx - subW / 2, subY, fontSub,
                        Fade(COLOR_GHOST, alpha * 0.7f));
            
            // ---- 6. REWARD STATS ----
            float rewardY = subY + fontSub + 24.0f;
            char rewardStr[128];
            float rewardAmount = 0.0f;
            switch (g_player.raidType) {
                case RAID_EVADE: rewardAmount = 0.20f; break;
                case RAID_ESCAPE: rewardAmount = 0.15f; break;
                case RAID_BURN: rewardAmount = 0.10f; break;
            }
            snprintf(rewardStr, sizeof(rewardStr), "+%.2f VCOIN  •  TRACE -%d%%  •  +1 RAID SURVIVED",
                    rewardAmount, g_player.raidType == RAID_EVADE ? 20 : (g_player.raidType == RAID_ESCAPE ? 100 : 90));
            float rewardW = MeasureScaledTextWidth(rewardStr, 12);
            DrawScaledText(rewardStr, cx - rewardW / 2, rewardY, 12,
                        Fade(COLOR_CYAN, alpha * 0.6f));
            
            // ---- 7. ANIMATED PARTICLES (CONFETTI EFFECT) ----
            static float particlePositions[20][2];
            static float particleSpeeds[20];
            static bool particleInit = false;
            
            if (!particleInit) {
                for (int i = 0; i < 20; i++) {
                    particlePositions[i][0] = (rand() % 1000) / 1000.0f * REF_WIDTH;
                    particlePositions[i][1] = (rand() % 1000) / 1000.0f * REF_HEIGHT;
                    particleSpeeds[i] = 50.0f + rand() % 100;
                }
                particleInit = true;
            }
            
            for (int i = 0; i < 20; i++) {
                float px = particlePositions[i][0] + sinf(t * particleSpeeds[i] * 0.01f + i) * 30.0f;
                float py = particlePositions[i][1] - t * particleSpeeds[i] * 0.3f;
                if (py < 0) {
                    py = REF_HEIGHT;
                    px = (rand() % 1000) / 1000.0f * REF_WIDTH;
                }
                float pSize = 2.0f + sinf(t * 2.0f + i) * 1.0f;
                float pAlpha = alpha * (0.3f + 0.7f * sinf(t * 3.0f + i * 0.5f));
                Color pColor = (i % 3 == 0) ? Fade(COLOR_TOXIC, pAlpha) :
                            (i % 3 == 1) ? Fade(COLOR_CYAN, pAlpha * 0.6f) :
                            Fade(COLOR_AMBER, pAlpha * 0.5f);
                DrawScaledRect(px, py, pSize, pSize, pColor);
            }
            
            // ---- 8. SCANLINE OVERLAY ----
            for (int y = 0; y < REF_HEIGHT; y += 4) {
                float scanAlpha = 3.0f + sinf(t * 10.0f + y * 0.1f) * 1.5f;
                DrawScaledRect(0, y, REF_WIDTH, 1, Fade(BLACK, scanAlpha * 0.3f * alpha));
            }
            
            // ---- 9. VIGNETTE ----
            for (int i = 0; i < 6; i++) {
                float size = i * 40.0f;
                DrawScaledRect(size, size, REF_WIDTH - size * 2, REF_HEIGHT - size * 2,
                            Fade(BLACK, (6 - i) * 0.03f * alpha));
            }
            
            // ---- 10. CORNER ACCENTS ----
            float cornerSize = 30.0f;
            float cornerOffset = 20.0f;
            Color cornerCol = Fade(COLOR_TOXIC, alpha * 0.5f);
            
            // Top-left
            DrawScaledRect(cornerOffset, cornerOffset, cornerSize, 2, cornerCol);
            DrawScaledRect(cornerOffset, cornerOffset, 2, cornerSize, cornerCol);
            // Top-right
            DrawScaledRect(REF_WIDTH - cornerOffset - cornerSize, cornerOffset, cornerSize, 2, cornerCol);
            DrawScaledRect(REF_WIDTH - cornerOffset, cornerOffset, 2, cornerSize, cornerCol);
            // Bottom-left
            DrawScaledRect(cornerOffset, REF_HEIGHT - cornerOffset - cornerSize, cornerSize, 2, cornerCol);
            DrawScaledRect(cornerOffset, REF_HEIGHT - cornerOffset, 2, cornerSize, cornerCol);
            // Bottom-right
            DrawScaledRect(REF_WIDTH - cornerOffset - cornerSize, REF_HEIGHT - cornerOffset - cornerSize, cornerSize, 2, cornerCol);
            DrawScaledRect(REF_WIDTH - cornerOffset, REF_HEIGHT - cornerOffset, 2, cornerSize, cornerCol);
            
            break;
        }

        case RAID_SEQ_FAILURE: {
            float t = g_player.raidSeqTimer;
            float duration = 2.0f;
            float progress = t / duration;
            float alpha = 1.0f - progress;
            float pulse = sinf(t * 6.0f) * 0.3f + 0.7f;
            
            // ---- 1. BACKGROUND ----
            DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, Color{6, 0, 0, 255});
            
            // ---- 2. GLOWING RED RINGS ----
            float cx = REF_WIDTH / 2.0f;
            float cy = REF_HEIGHT / 2.0f;
            float ringSize = 200.0f + pulse * 50.0f;
            
            for (int i = 5; i >= 1; i--) {
                float r = ringSize * (1.0f + i * 0.2f);
                float a = (0.12f / i) * alpha;
                DrawScaledCircle(cx, cy, r, Fade(COLOR_BLOOD, a));
            }
            DrawScaledCircle(cx, cy, ringSize * 0.5f, Fade(COLOR_BLOOD, 0.2f * alpha));
            
            // ---- 3. WARNING ICON (Animated Skull/Cross) ----
            float iconSize = 70.0f;
            float iconY = cy - 50.0f;
            float scale = (progress < 0.3f) ? progress / 0.3f : 1.0f;
            scale *= (0.85f + 0.15f * sinf(t * 5.0f));
            
            // Circle behind icon
            DrawScaledCircle(cx, iconY, iconSize * 0.7f * scale, Fade(COLOR_BLOOD, 0.15f * alpha));
            DrawScaledCircleLines(cx, iconY, iconSize * 0.7f * scale, Fade(COLOR_BLOOD, 0.4f * alpha));
            
            // Skull X (crossed lines)
            float crossSize = 30.0f * scale;
            float crossX = cx;
            float crossY = iconY;
            
            // First diagonal
            for (int i = 0; i < 10; i++) {
                float pct = i / 9.0f;
                float x1 = crossX - crossSize + pct * crossSize * 2;
                float y1 = crossY - crossSize + pct * crossSize * 2;
                float x2 = crossX - crossSize + (pct + 0.05f) * crossSize * 2;
                float y2 = crossY - crossSize + (pct + 0.05f) * crossSize * 2;
                float lineAlpha = (pct < 0.5f) ? alpha : alpha * (1.0f - (pct - 0.5f) / 0.5f);
                DrawScaledLine(x1, y1, x2, y2, Fade(COLOR_BLOOD, lineAlpha * 0.9f));
            }
            
            // Second diagonal
            for (int i = 0; i < 10; i++) {
                float pct = i / 9.0f;
                float x1 = crossX + crossSize - pct * crossSize * 2;
                float y1 = crossY - crossSize + pct * crossSize * 2;
                float x2 = crossX + crossSize - (pct + 0.05f) * crossSize * 2;
                float y2 = crossY - crossSize + (pct + 0.05f) * crossSize * 2;
                float lineAlpha = (pct < 0.5f) ? alpha : alpha * (1.0f - (pct - 0.5f) / 0.5f);
                DrawScaledLine(x1, y1, x2, y2, Fade(COLOR_BLOOD, lineAlpha * 0.9f));
            }
            
            // ---- 4. MAIN TEXT "RAID BREACHED" ----
            float fontMain = 44.0f;
            const char* mainMsg = "💀 RAID BREACHED 💀";
            float mainW = MeasureScaledTextWidth(mainMsg, fontMain);
            float mainY = iconY + iconSize * 0.8f + 10.0f;
            
            // Glow behind text
            for (int i = 4; i >= 1; i--) {
                float offset = i * 2.0f;
                DrawScaledText(mainMsg, cx - mainW / 2 + offset, mainY + offset,
                            fontMain, Fade(COLOR_BLOOD, 0.04f * alpha * (5 - i) / 4.0f));
            }
            DrawScaledText(mainMsg, cx - mainW / 2, mainY, fontMain,
                        Fade(COLOR_BLOOD, alpha));
            
            // ---- 5. SUBTEXT ----
            float fontSub = 16.0f;
            const char* subMsg = "FEDERAL AGENTS BREACHED YOUR NODE";
            float subW = MeasureScaledTextWidth(subMsg, fontSub);
            float subY = mainY + fontMain + 16.0f;
            DrawScaledText(subMsg, cx - subW / 2, subY, fontSub,
                        Fade(COLOR_GHOST, alpha * 0.6f));
            
            // ---- 6. PENALTY STATS ----
            float penaltyY = subY + fontSub + 24.0f;
            char penaltyStr[128];
            float seized = g_player.vcoin * 0.25f;
            snprintf(penaltyStr, sizeof(penaltyStr),
                    "❌ VCOIN -%.2f  ❌ ICE 0/3  ❌ TRACE 100%%  ❌ SITE BURNED",
                    seized);
            float penaltyW = MeasureScaledTextWidth(penaltyStr, 11);
            DrawScaledText(penaltyStr, cx - penaltyW / 2, penaltyY, 11,
                        Fade(COLOR_BLOOD, alpha * 0.7f));
            
            // ---- 7. WARNING LINES (flashing) ----
            if (pulse > 0.7f) {
                float lineAlpha = (pulse - 0.7f) / 0.3f * alpha * 0.5f;
                DrawScaledRect(0, 0, REF_WIDTH, 3, Fade(COLOR_BLOOD, lineAlpha));
                DrawScaledRect(0, REF_HEIGHT - 3, REF_WIDTH, 3, Fade(COLOR_BLOOD, lineAlpha));
                DrawScaledRect(0, 0, 3, REF_HEIGHT, Fade(COLOR_BLOOD, lineAlpha));
                DrawScaledRect(REF_WIDTH - 3, 0, 3, REF_HEIGHT, Fade(COLOR_BLOOD, lineAlpha));
            }
            
            // ---- 8. SCANLINE OVERLAY ----
            for (int y = 0; y < REF_HEIGHT; y += 4) {
                float scanAlpha = 3.0f + sinf(t * 12.0f + y * 0.1f) * 2.0f;
                DrawScaledRect(0, y, REF_WIDTH, 1, Fade(COLOR_BLOOD, scanAlpha * 0.4f * alpha));
            }
            
            // ---- 9. GLITCH FLICKER ----
            if (fmodf(t * 3.0f, 1.0f) > 0.95f) {
                float glitchX = (rand() % (int)REF_WIDTH);
                float glitchW = 20 + rand() % 60;
                DrawScaledRect(glitchX, 0, glitchW, REF_HEIGHT,
                            Fade(COLOR_BLOOD, 0.1f * alpha * (fmodf(t * 3.0f, 1.0f) - 0.95f) * 20.0f));
            }
            
            // ---- 10. VIGNETTE ----
            for (int i = 0; i < 6; i++) {
                float size = i * 40.0f;
                DrawScaledRect(size, size, REF_WIDTH - size * 2, REF_HEIGHT - size * 2,
                            Fade(BLACK, (6 - i) * 0.04f * alpha));
            }
            
            // ---- 11. CORNER ACCENTS ----
            float cornerSize = 30.0f;
            float cornerOffset = 20.0f;
            Color cornerCol = Fade(COLOR_BLOOD, alpha * 0.5f * (0.5f + 0.5f * pulse));
            
            // Top-left
            DrawScaledRect(cornerOffset, cornerOffset, cornerSize, 2, cornerCol);
            DrawScaledRect(cornerOffset, cornerOffset, 2, cornerSize, cornerCol);
            // Top-right
            DrawScaledRect(REF_WIDTH - cornerOffset - cornerSize, cornerOffset, cornerSize, 2, cornerCol);
            DrawScaledRect(REF_WIDTH - cornerOffset, cornerOffset, 2, cornerSize, cornerCol);
            // Bottom-left
            DrawScaledRect(cornerOffset, REF_HEIGHT - cornerOffset - cornerSize, cornerSize, 2, cornerCol);
            DrawScaledRect(cornerOffset, REF_HEIGHT - cornerOffset, 2, cornerSize, cornerCol);
            // Bottom-right
            DrawScaledRect(REF_WIDTH - cornerOffset - cornerSize, REF_HEIGHT - cornerOffset - cornerSize, cornerSize, 2, cornerCol);
            DrawScaledRect(REF_WIDTH - cornerOffset, REF_HEIGHT - cornerOffset, 2, cornerSize, cornerCol);
            
            break;
        }
    }
}