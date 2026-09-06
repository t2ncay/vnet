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
    float spacing = 12.0f;
    int perRow = 4;
    
    const char* themeNames[] = {
        "classic", "tokyo", "redroom", "amber",
        "cyberpunk", "nord", "dracula", "synthwave",
        "cobalt", "monokai", "gruvbox", "abyss",
        "solaris", "ghost", "matrix"
    };
    int themeCount = sizeof(themeNames) / sizeof(themeNames[0]);
    
    // Color previews for each theme
    Color themeColors[][3] = {
        {{2,2,4,255}, {220,20,40,255}, {0,220,240,255}},    // classic
        {{16,18,28,255}, {247,118,142,255}, {122,162,247,255}}, // tokyo
        {{10,3,5,255}, {255,20,50,255}, {240,45,65,255}},   // redroom
        {{12,8,2,255}, {240,60,30,255}, {255,175,0,255}},   // amber
        {{18,10,26,255}, {255,0,85,255}, {0,230,255,255}},  // cyberpunk
        {{15,20,28,255}, {191,97,106,255}, {136,192,208,255}}, // nord
        {{18,16,26,255}, {255,85,85,255}, {139,233,253,255}}, // dracula
        {{12,6,24,255}, {255,30,130,255}, {0,240,255,255}}, // synthwave
        {{2,12,28,255}, {255,60,90,255}, {0,190,255,255}},  // cobalt
        {{20,20,20,255}, {255,97,136,255}, {120,220,232,255}}, // monokai
        {{20,20,18,255}, {251,73,52,255}, {131,165,152,255}}, // gruvbox
        {{2,6,12,255}, {230,40,70,255}, {0,180,200,255}},   // abyss
        {{18,6,2,255}, {255,40,20,255}, {255,140,0,255}},   // solaris
        {{12,14,18,255}, {240,80,100,255}, {160,210,245,255}}, // ghost
        {{4,10,6,255}, {240,40,70,255}, {0,210,255,255}},   // matrix
    };
    
    // Get current theme
    std::string currentTheme = "classic";
    
    for (int i = 0; i < themeCount; i++) {
        int col = i % perRow;
        int row = i / perRow;
        float tx = x + col * (themeSize + spacing);
        float ty = rowY + row * (themeSize + spacing + 30);
        
        if (ty > y + h - 20) break;
        
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
            SetActiveTheme(themeNames[i]);
            PushCliLog("[SETTINGS]: Theme set to %s", themeNames[i]);
            // Force UI refresh
            TriggerJitter(0.15f);
        }
    }
    
    // Current theme info
    DrawScaledLine(x, y + h - 40, x + w, y + h - 40, Color{30, 35, 50, 80});
    char themeInfo[128];
    snprintf(themeInfo, sizeof(themeInfo), "ACTIVE THEME: %s", currentTheme.c_str());
    DrawScaledText(themeInfo, x + 10, y + h - 22, 10, COLOR_TOXIC);
    DrawScaledText("Click any theme card to apply", x + 250, y + h - 22, 9, COLOR_GHOST);
}