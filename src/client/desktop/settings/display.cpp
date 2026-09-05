#include "../../render.h"
#include "../../../shared/vnet.h"

void DrawSettingsDisplay(float x, float y, float w, float h) {
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    float t = (float)GetTime();
    
    DrawScaledText("🖥 DISPLAY SETTINGS", x, y, 16, COLOR_CYAN);
    DrawScaledLine(x, y + 24, x + w, y + 24, Color{30, 35, 50, 100});
    
    float rowY = y + 44;
    
    // ---- RESOLUTION ----
    DrawScaledText("RESOLUTION", x, rowY, 12, COLOR_GHOST);
    rowY += 22;
    
    const char* resolutions[] = {"1920x1080", "1680x1050", "1440x900", "1280x720"};
    for (int i = 0; i < 4; i++) {
        float rx = x + 10 + i * 110.0f;
        bool hover = RefRectHover(rx, rowY, 100, 28, refMouse);
        bool active = (i == 0);
        
        DrawScaledRect(rx, rowY, 100, 28, active ? Color{40, 45, 70, 200} : (hover ? Color{30, 35, 55, 150} : Color{12, 15, 20, 200}));
        DrawScaledRectLines(rx, rowY, 100, 28, active ? COLOR_TOXIC : (hover ? COLOR_CYAN : Color{30, 35, 50, 100}));
        DrawScaledText(resolutions[i], rx + 10, rowY + 6, 10, active ? COLOR_TOXIC : COLOR_GHOST);
        
        if (clicked && hover) {
            PushCliLog("[SETTINGS]: Resolution changed to %s", resolutions[i]);
            TriggerJitter(0.2f);
        }
    }
    
    rowY += 44;
    DrawScaledLine(x + 10, rowY, x + w - 10, rowY, Color{30, 35, 50, 60});
    rowY += 12;
    
    // ---- DISPLAY MODE ----
    DrawScaledText("DISPLAY MODE", x, rowY, 12, COLOR_GHOST);
    rowY += 22;
    
    const char* modes[] = {"Fullscreen", "Windowed", "Borderless"};
    for (int i = 0; i < 3; i++) {
        float rx = x + 10 + i * 120.0f;
        bool hover = RefRectHover(rx, rowY, 110, 28, refMouse);
        bool active = (i == 0);
        
        DrawScaledRect(rx, rowY, 110, 28, active ? Color{40, 45, 70, 200} : (hover ? Color{30, 35, 55, 150} : Color{12, 15, 20, 200}));
        DrawScaledRectLines(rx, rowY, 110, 28, active ? COLOR_TOXIC : (hover ? COLOR_CYAN : Color{30, 35, 50, 100}));
        DrawScaledText(modes[i], rx + 15, rowY + 6, 10, active ? COLOR_TOXIC : COLOR_GHOST);
        
        if (clicked && hover) {
            PushCliLog("[SETTINGS]: Display mode changed to %s", modes[i]);
            if (i == 0 && !IsWindowFullscreen()) {
                ToggleFullscreen();
            } else if (i != 0 && IsWindowFullscreen()) {
                ToggleFullscreen();
            }
        }
    }
    
    rowY += 44;
    DrawScaledLine(x + 10, rowY, x + w - 10, rowY, Color{30, 35, 50, 60});
    rowY += 12;
    
    // ---- CRT EFFECTS ----
    DrawScaledText("CRT EFFECTS", x, rowY, 12, COLOR_GHOST);
    rowY += 22;
    
    // Toggle switches
    const char* toggles[] = {"Scanlines", "Vignette", "Chromatic Aberration", "Screen Flicker"};
    bool toggleStates[] = {true, true, false, true};
    
    for (int i = 0; i < 4; i++) {
        float tx = x + 10 + (i % 2) * 200.0f;
        float ty = rowY + (i / 2) * 32.0f;
        bool hover = RefRectHover(tx, ty, 180, 24, refMouse);
        
        DrawScaledRect(tx, ty, 180, 24, hover ? Color{30, 35, 55, 150} : Color{12, 15, 20, 200});
        DrawScaledRectLines(tx, ty, 180, 24, hover ? COLOR_CYAN : Color{30, 35, 50, 80});
        
        // Toggle switch
        float toggleX = tx + 150;
        float toggleY = ty + 4;
        float toggleW = 22;
        float toggleH = 16;
        bool state = toggleStates[i];
        
        DrawScaledRect(toggleX, toggleY, toggleW, toggleH, state ? COLOR_TOXIC : Color{30, 35, 50, 200});
        DrawScaledRectLines(toggleX, toggleY, toggleW, toggleH, state ? COLOR_TOXIC : Color{50, 55, 70, 150});
        float knobX = state ? toggleX + toggleW - 12 : toggleX + 2;
        DrawScaledRect(knobX, toggleY + 2, 10, 12, state ? Color{40, 240, 100, 200} : Color{80, 90, 110, 150});
        
        DrawScaledText(toggles[i], tx + 8, ty + 5, 10, state ? COLOR_TOXIC : COLOR_GHOST);
        
        if (clicked && hover) {
            toggleStates[i] = !toggleStates[i];
            PushCliLog("[SETTINGS]: %s %s", toggles[i], toggleStates[i] ? "ENABLED" : "DISABLED");
            TriggerJitter(0.1f);
        }
    }
}