#include "../../desktop.h"
#include "../../../render.h"
#include "../../../../shared/vnet.h"
#include "../../../vnet_client.h"

// Forward declarations for VDEC sub-tabs
void DrawVDECKeyRing(float x, float y, float w, float h);
void DrawVDECDecrypt(float x, float y, float w, float h);
void DrawVDECEncrypt(float x, float y, float w, float h);
void DrawVDECHash(float x, float y, float w, float h);
void DrawVDECMinigame(float x, float y, float w, float h);

void Desktop::DrawVDEC(const AppWindow& win) {
    float cx = win.x + 4;
    float cy = win.y + m_windowTitleHeight + 4;
    float cw = win.w - 8;
    float ch = win.h - m_windowTitleHeight - 8;
    
    float t = (float)GetTime();
    float pulse = sinf(t * 3.0f) * 0.3f + 0.7f;
    
    // ---- BACKGROUND ----
    DrawScaledRect(cx, cy, cw, ch, Color{4, 6, 14, 255});
    DrawScaledRectLines(cx, cy, cw, ch, COLOR_BORDER);
    
    // ---- HEADER ----
    float headerH = 50.0f;
    DrawScaledRect(cx, cy, cw, headerH, Color{10, 14, 24, 255});
    DrawScaledLine(cx, cy + headerH, cx + cw, cy + headerH, Color{30, 35, 50, 150});
    
    // Animated VDEC logo
    float glowPulse = sinf(t * 2.0f) * 0.3f + 0.7f;
    DrawScaledText("🔐 VDEC v2.0 // VEKTRA DECRYPTION ENGINE", cx + 16, cy + 14, 14, 
                   Color{0, 220, 240, (unsigned char)(glowPulse * 200 + 55)});
    
    // Status indicator
    Color statusCol = {40, 240, 100, (unsigned char)(pulse * 200 + 55)};
    DrawScaledRect(cx + cw - 120, cy + 14, 8, 8, statusCol);
    DrawScaledText("ACTIVE", cx + cw - 105, cy + 13, 10, COLOR_TOXIC);
    
    // ---- TAB BAR ----
    float tabY = cy + headerH;
    float tabH = 36.0f;
    const char* tabs[] = {"🔑 Key Ring", "🔓 Decrypt", "🔒 Encrypt", "🔢 Hash", "🎯 Minigame"};
    int tabCount = 5;
    float tabW = cw / tabCount;
    
    DrawScaledRect(cx, tabY, cw, tabH, Color{6, 8, 16, 255});
    DrawScaledLine(cx, tabY + tabH, cx + cw, tabY + tabH, Color{30, 35, 50, 100});
    
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    for (int i = 0; i < tabCount; i++) {
        float tx = cx + i * tabW;
        bool hover = RefRectHover(tx, tabY, tabW, tabH, refMouse);
        bool active = (i == m_vdec.selectedTab);
        
        Color bg = active ? Color{20, 25, 45, 200} : (hover ? Color{15, 18, 35, 150} : Color{0,0,0,0});
        DrawScaledRect(tx, tabY, tabW, tabH, bg);
        
        if (active) {
            DrawScaledRect(tx, tabY + tabH - 3, tabW, 3, COLOR_CYAN);
        }
        
        DrawScaledText(tabs[i], tx + 8, tabY + 10, 11, 
                      active ? COLOR_CYAN : (hover ? COLOR_TOXIC : COLOR_GHOST));
        
        if (clicked && hover) {
            m_vdec.selectedTab = i;
            m_vdec.minigameActive = false;
        }
    }
    
    // ---- CONTENT AREA ----
    float contentX = cx + 12;
    float contentY = tabY + tabH + 8;
    float contentW = cw - 24;
    float contentH = ch - headerH - tabH - 16;
    
    // Content background with subtle scanlines
    DrawScaledRect(contentX, contentY, contentW, contentH, Color{6, 8, 16, 200});
    DrawScaledRectLines(contentX, contentY, contentW, contentH, Color{30, 35, 50, 80});
    
    // Scanline overlay
    for (int i = 0; i < (int)contentH; i += 4) {
        float scanY = contentY + i + fmodf(t * 30.0f, 4.0f);
        DrawScaledRect(contentX, scanY, contentW, 1, {0, 0, 0, 4});
    }
    
    // ---- DRAW TAB CONTENT ----
    switch (m_vdec.selectedTab) {
        case 0: DrawVDECKeyRing(contentX, contentY, contentW, contentH); break;
        case 1: DrawVDECDecrypt(contentX, contentY, contentW, contentH); break;
        case 2: DrawVDECEncrypt(contentX, contentY, contentW, contentH); break;
        case 3: DrawVDECHash(contentX, contentY, contentW, contentH); break;
        case 4: DrawVDECMinigame(contentX, contentY, contentW, contentH); break;
    }
}
