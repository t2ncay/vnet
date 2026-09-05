#include "desktop.h"
#include "../render.h"
#include "../../shared/vnet.h"
#include "../game.h"

// ============================================================
// DESKTOP ICONS
// ============================================================

void Desktop::DrawDesktopIcons() {
    float iconSize = 80.0f;
    float spacing = 20.0f;
    float startX = 30.0f;
    float startY = 80.0f;
    int cols = 4;
    Vector2 refMouse = GetRefMousePos();
    
    for (int i = 0; i < (int)m_apps.size(); i++) {
        int col = i % cols;
        int row = i / cols;
        float x = startX + col * (iconSize + spacing);
        float y = startY + row * (iconSize + spacing + 30);
        
        bool hover = RefRectHover(x, y, iconSize, iconSize, refMouse);
        
        // Background
        DrawScaledRect(x, y, iconSize, iconSize, 
                       hover ? Color{40, 45, 65, 200} : Color{20, 22, 35, 150});
        DrawScaledRectLines(x, y, iconSize, iconSize, 
                            hover ? COLOR_BLOOD : Color{30, 35, 50, 100});
        
        // Draw icon texture instead of emoji
        Texture2D icon = GetIcon(m_apps[i].iconKey);
        if (icon.id != 0) {
            float iconDrawSize = 40.0f;
            float iconX = x + (iconSize - iconDrawSize) / 2.0f;
            float iconY = y + 8.0f;
            DrawTexturePro(
                icon,
                {0, 0, (float)icon.width, (float)icon.height},
                {SX(iconX), SY(iconY), iconDrawSize * g_uiScale, iconDrawSize * g_uiScale},
                {0, 0},
                0.0f,
                hover ? COLOR_TOXIC : COLOR_CYAN
            );
        } else {
            // Fallback: show text
            DrawScaledText(m_apps[i].iconKey.c_str(), 
                           x + (iconSize - 24) / 2, y + 8, 16, COLOR_CYAN);
        }
        
        // Icon label
        DrawScaledText(m_apps[i].name.c_str(), 
                       x + 10, y + iconSize - 18, 10, hover ? COLOR_TOXIC : COLOR_GHOST);
    }
}