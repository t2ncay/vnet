#include "../../render.h"
#include "../../../shared/vnet.h"
#include "../../vnet_client.h"

#include <cmath>

void DrawSettingsSecurity(float x, float y, float w, float h) {
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    float t = (float)GetTime();
    
    DrawScaledText("🔒 SECURITY & PRIVACY", x, y, 16, COLOR_CYAN);
    DrawScaledLine(x, y + 24, x + w, y + 24, Color{30, 35, 50, 100});
    
    float rowY = y + 44;
    
    // ---- ICE SHIELDS ----
    DrawScaledText("ICE FIREWALL SHIELDS", x, rowY, 12, COLOR_GHOST);
    rowY += 22;
    
    for (int i = 0; i < 3; i++) {
        float sx = x + 10 + i * 80.0f;
        bool active = (i < g_player.iceShields);
        bool hover = RefRectHover(sx, rowY, 70, 50, refMouse);
        
        DrawScaledRect(sx, rowY, 70, 50, active ? Color{40, 45, 70, 200} : Color{12, 15, 20, 200});
        DrawScaledRectLines(sx, rowY, 70, 50, active ? COLOR_CYAN : Color{30, 35, 50, 100});
        
        char shieldStr[16];
        snprintf(shieldStr, sizeof(shieldStr), "ICE %d", i + 1);
        DrawScaledText(shieldStr, sx + 15, rowY + 8, 10, active ? COLOR_TOXIC : COLOR_GHOST);
        DrawScaledText(active ? "ON" : "OFF", sx + 20, rowY + 28, 12, active ? COLOR_TOXIC : COLOR_BLOOD);
        
        if (active) {
            float pulse = sinf(t * 3.0f + i) * 0.3f + 0.7f;
            DrawScaledRect(sx + 30, rowY + 8, 6, 6, {40, 240, 100, (unsigned char)(pulse * 200 + 55)});
        }
    }
    
    rowY += 60;
    DrawScaledLine(x + 10, rowY, x + w - 10, rowY, Color{30, 35, 50, 60});
    rowY += 12;
    
    // ---- TRACE PROTECTION ----
    DrawScaledText("TRACE PROTECTION", x, rowY, 12, COLOR_GHOST);
    rowY += 22;
    
    // Trace bar
    float barX = x + 10;
    float barY = rowY + 4;
    float barW = w - 20;
    float barH = 20.0f;
    
    DrawScaledRect(barX, barY, barW, barH, Color{12, 15, 20, 255});
    DrawScaledRectLines(barX, barY, barW, barH, Color{30, 35, 50, 100});
    
    float traceFill = (g_player.traceLevel / 100.0f) * barW;
    if (traceFill < 2.0f) traceFill = 2.0f;
    Color traceColor = g_player.traceLevel > 70 ? COLOR_BLOOD : (g_player.traceLevel > 40 ? COLOR_AMBER : COLOR_TOXIC);
    DrawScaledRect(barX, barY, traceFill, barH, traceColor);
    
    char traceStr[32];
    snprintf(traceStr, sizeof(traceStr), "TRACE: %d%%", g_player.traceLevel);
    float traceW = MeasureScaledTextWidth(traceStr, 12);
    DrawScaledText(traceStr, barX + (barW - traceW) / 2.0f, barY + 3, 12, 
                  g_player.traceLevel > 50 ? COLOR_BLACK : COLOR_GHOST);
    
    rowY += 32;
    
    // ---- FLUSH TRACE BUTTON ----
    bool hoverFlush = RefRectHover(x + 10, rowY, 180, 32, refMouse);
    DrawScaledRect(x + 10, rowY, 180, 32, hoverFlush ? COLOR_BLOOD : Color{30, 35, 55, 200});
    DrawScaledRectLines(x + 10, rowY, 180, 32, COLOR_BLOOD);
    DrawScaledText("FLUSH TRACE (0.10 VCOIN)", x + 20, rowY + 8, 11, hoverFlush ? COLOR_BLACK : COLOR_TOXIC);
    
    if (clicked && hoverFlush) {
        if (g_player.vcoin < 0.10f) {
            PushCliLog("[SETTINGS]: Insufficient VCOIN for flush");
        } else {
            g_player.vcoin -= 0.10f;
            g_player.traceLevel = (int)(g_player.traceLevel * 0.7f);
            if (g_player.traceLevel < 0) g_player.traceLevel = 0;
            PushCliLog("[SETTINGS]: Trace flushed!");
            TriggerJitter(0.3f);
        }
    }
    
    // ---- BUY ICE BUTTON ----
    bool hoverIce = RefRectHover(x + 200, rowY, 160, 32, refMouse);
    DrawScaledRect(x + 200, rowY, 160, 32, hoverIce ? COLOR_CYAN : Color{30, 35, 55, 200});
    DrawScaledRectLines(x + 200, rowY, 160, 32, COLOR_CYAN);
    DrawScaledText("BUY ICE (0.30 VCOIN)", x + 210, rowY + 8, 11, hoverIce ? COLOR_BLACK : COLOR_CYAN);
    
    if (clicked && hoverIce) {
        if (g_player.iceShields >= 3) {
            PushCliLog("[SETTINGS]: Max ICE shields reached");
        } else if (g_player.vcoin < 0.30f) {
            PushCliLog("[SETTINGS]: Insufficient VCOIN");
        } else {
            g_player.vcoin -= 0.30f;
            g_player.iceShields++;
            PushCliLog("[SETTINGS]: ICE shield purchased! (%d/3)", g_player.iceShields);
            TriggerJitter(0.2f);
        }
    }
}