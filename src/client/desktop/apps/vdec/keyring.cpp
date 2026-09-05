#include "../../../render.h"
#include "../../../../shared/vnet.h"
#include "../../../vnet_client.h"
#include "../../../../shared/vnet_protocol.h"
#include "../../desktop.h"

void Desktop::DrawVDECKeyRing(float x, float y, float w, float h) {
    float t = (float)GetTime();
    
    DrawScaledText("🔑 KEY RING // CRYPTOGRAPHIC KEY STORAGE", x, y, 14, COLOR_AMBER);
    DrawScaledLine(x, y + 22, x + w, y + 22, Color{30, 35, 50, 80});
    
    float rowY = y + 34;
    float keySize = 80.0f;
    float spacing = 12.0f;
    int perRow = 4;
    
    for (int i = 0; i < 8; i++) {
        int col = i % perRow;
        int row = i / perRow;
        float kx = x + col * (keySize + spacing);
        float ky = rowY + row * (keySize + spacing + 30);
        
        if (ky > y + h - 20) break;
        
        bool found = m_vdec.keysFound[i];
        bool hover = RefRectHover(kx, ky, keySize, keySize, GetRefMousePos());
        
        Color bg = found ? Color{20, 40, 30, 200} : Color{10, 12, 20, 200};
        if (hover) bg = found ? Color{30, 60, 40, 220} : Color{20, 22, 40, 200};
        
        DrawScaledRect(kx, ky, keySize, keySize, bg);
        DrawScaledRectLines(kx, ky, keySize, keySize, 
                           found ? COLOR_TOXIC : (hover ? COLOR_CYAN : Color{30, 35, 50, 100}));
        
        char slotLabel[16];
        snprintf(slotLabel, sizeof(slotLabel), "KEY %d", i + 1);
        DrawScaledText(slotLabel, kx + 6, ky + 4, 8, found ? COLOR_TOXIC : Color{60, 70, 90, 150});
        
        if (found) {
            float glow = sinf(t * 3.0f + i) * 0.3f + 0.7f;
            Color glowCol = {40, 240, 100, (unsigned char)(glow * 100 + 55)};
            DrawScaledRect(kx + 4, ky + 18, keySize - 8, 20, glowCol);
            DrawScaledText(m_vdec.keys[i], kx + 6, ky + 19, 14, COLOR_BLACK);
            DrawScaledText("✓ FOUND", kx + 6, ky + 48, 9, COLOR_TOXIC);
        } else {
            DrawScaledText("🔒", kx + 28, ky + 22, 28, Color{60, 70, 90, 150});
            DrawScaledText("MISSING", kx + 10, ky + 56, 8, Color{60, 70, 90, 150});
        }
    }
    
    float summaryY = y + h - 36;
    DrawScaledLine(x, summaryY - 4, x + w, summaryY - 4, Color{30, 35, 50, 80});
    
    char summary[128];
    snprintf(summary, sizeof(summary), "KEYS FOUND: %d/8  |  BIT-SHIFT OFFSET: %d", 
             m_vdec.keyCount, m_vdec.bitShiftOffset);
    DrawScaledText(summary, x + 8, summaryY + 4, 10, COLOR_CYAN);
}