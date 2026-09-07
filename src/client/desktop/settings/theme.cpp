#include "../../render.h"
#include "../../../shared/vnet.h"
#include "../../vnet_client.h"
#include "../../desktop/widgets/music_player.h"

void DrawSettingsTheme(float x, float y, float w, float h) {
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    float t = (float)GetTime();
    
    DrawScaledText("🎨 THEME CONFIGURATOR", x, y, 16, COLOR_CYAN);
    DrawScaledLine(x, y + 24, x + w, y + 24, Color{30, 35, 50, 100});
    
    float rowY = y + 36;
    float themeSize = 60.0f;
    float spacing = 10.0f;  // Reduced spacing for 5 columns
    int perRow = 5;         // 5 columns instead of 4
    
    const char* themeNames[] = {
        "classic", "tokyo", "redroom", "amber",
        "cyberpunk", "nord", "dracula", "synthwave",
        "cobalt", "monokai", "gruvbox", "abyss",
        "solaris", "ghost", "matrix",
        "midnight", "sunset", "ice", "lava", 
        "neon_dream", "hacker", "bloodmoon", "deepweb", "vaporwave"
    };
    int themeCount = sizeof(themeNames) / sizeof(themeNames[0]);
    
    // Color previews for each theme (24 themes)
    Color themeColors[][3] = {
        {{2,2,4,255}, {220,20,40,255}, {0,220,240,255}},      // classic
        {{16,18,28,255}, {247,118,142,255}, {122,162,247,255}}, // tokyo
        {{10,3,5,255}, {255,20,50,255}, {240,45,65,255}},      // redroom
        {{12,8,2,255}, {240,60,30,255}, {255,175,0,255}},      // amber
        {{18,10,26,255}, {255,0,85,255}, {0,230,255,255}},     // cyberpunk
        {{15,20,28,255}, {191,97,106,255}, {136,192,208,255}}, // nord
        {{18,16,26,255}, {255,85,85,255}, {139,233,253,255}},  // dracula
        {{12,6,24,255}, {255,30,130,255}, {0,240,255,255}},    // synthwave
        {{2,12,28,255}, {255,60,90,255}, {0,190,255,255}},     // cobalt
        {{20,20,20,255}, {255,97,136,255}, {120,220,232,255}}, // monokai
        {{20,20,18,255}, {251,73,52,255}, {131,165,152,255}},  // gruvbox
        {{2,6,12,255}, {230,40,70,255}, {0,180,200,255}},      // abyss
        {{18,6,2,255}, {255,40,20,255}, {255,140,0,255}},      // solaris
        {{12,14,18,255}, {240,80,100,255}, {160,210,245,255}}, // ghost
        {{4,10,6,255}, {240,40,70,255}, {0,210,255,255}},      // matrix
        {{6,4,12,255}, {180,60,220,255}, {100,180,255,255}},   // midnight
        {{20,8,6,255}, {255,80,60,255}, {255,180,80,255}},     // sunset
        {{8,12,20,255}, {80,200,255,255}, {150,230,255,255}},  // ice
        {{12,4,2,255}, {255,60,40,255}, {255,120,60,255}},     // lava
        {{10,4,18,255}, {255,40,180,255}, {0,255,220,255}},    // neon_dream
        {{6,8,6,255}, {40,240,60,255}, {0,255,180,255}},       // hacker
        {{16,2,4,255}, {255,20,40,255}, {200,40,60,255}},      // bloodmoon
        {{2,4,8,255}, {0,200,255,255}, {0,160,220,255}},       // deepweb
        {{20,10,30,255}, {255,30,180,255}, {0,240,255,255}},   // vaporwave
    };
    
    // Get current theme
    std::string currentTheme = "classic";
    
    // Calculate if we need to scroll (if content exceeds visible area)
    int totalRows = (themeCount + perRow - 1) / perRow;
    float totalHeight = totalRows * (themeSize + spacing + 30) + 20;
    float maxHeight = h - 60;  // Leave room for header and footer
    
    // Simple scroll offset (static for now, but can be made interactive)
    static float themeScrollOffset = 0.0f;
    
    // Handle mouse wheel scrolling over the theme area
    float wheel = GetMouseWheelMove();
    if (RefRectHover(x, rowY, w, h - 50, refMouse)) {
        themeScrollOffset -= wheel * 20.0f;
        if (themeScrollOffset < 0.0f) themeScrollOffset = 0.0f;
        float maxScroll = totalHeight - maxHeight;
        if (maxScroll < 0.0f) maxScroll = 0.0f;
        if (themeScrollOffset > maxScroll) themeScrollOffset = maxScroll;
    }
    
    // Draw themes with scroll offset
    for (int i = 0; i < themeCount; i++) {
        int col = i % perRow;
        int row = i / perRow;
        float tx = x + col * (themeSize + spacing);
        float ty = rowY + row * (themeSize + spacing + 30) - themeScrollOffset;
        
        // Skip if outside visible area
        if (ty + themeSize < rowY - 10 || ty > rowY + maxHeight + 10) continue;
        
        bool hover = RefRectHover(tx, ty, themeSize, themeSize, refMouse);
        bool active = (strcmp(themeNames[i], currentTheme.c_str()) == 0);
        
        // Theme preview card
        DrawScaledRect(tx, ty, themeSize, themeSize, Color{20, 22, 35, 200});
        DrawScaledRectLines(tx, ty, themeSize, themeSize, 
                           active ? COLOR_TOXIC : (hover ? COLOR_CYAN : Color{30, 35, 50, 100}));
        
        // Color swatches
        float swatchSize = 14.0f;
        float swatchY = ty + 8;
        for (int c = 0; c < 3; c++) {
            float swatchX = tx + 6 + c * (swatchSize + 4);
            DrawScaledRect(swatchX, swatchY, swatchSize, swatchSize, themeColors[i][c]);
            DrawScaledRectLines(swatchX, swatchY, swatchSize, swatchSize, Color{30, 35, 50, 80});
        }
        
        // Theme name
        DrawScaledText(themeNames[i], tx + 2, ty + themeSize - 16, 8, 
                      active ? COLOR_TOXIC : COLOR_GHOST);
        
        // Active indicator
        if (active) {
            DrawScaledRect(tx + themeSize - 14, ty + 4, 10, 10, COLOR_TOXIC);
            DrawScaledText("✓", tx + themeSize - 12, ty + 3, 9, COLOR_BLACK);
        }
        
        if (clicked && hover) {
            SetActiveTheme(themeNames[i], false);  // Smooth transition!
            PushCliLog("[SETTINGS]: Theme set to %s", themeNames[i]);
            TriggerJitter(0.15f);
        }
    }
    
    // ---- Scroll indicator ----
    if (totalHeight > maxHeight) {
        float scrollRatio = themeScrollOffset / (totalHeight - maxHeight);
        float indicatorY = rowY + 8 + scrollRatio * (maxHeight - 24);
        float indicatorX = x + w - 8;
        DrawScaledRect(indicatorX, indicatorY, 4, 16, {80, 90, 110, 150});
    }
    
    // ---- Current theme description ----
    char descStr[128];
    snprintf(descStr, sizeof(descStr), "THEME: %s", currentTheme.c_str());
    DrawScaledText(descStr, x + 10, y + h - 36, 10, COLOR_TOXIC);
    
    // ---- Footer ----
    DrawScaledLine(x, y + h - 44, x + w, y + h - 44, Color{30, 35, 50, 80});
    DrawScaledText("Click any theme card to apply", x + 10, y + h - 22, 9, COLOR_GHOST);
}