#include "../../render.h"
#include "../../../shared/vnet.h"
#include "../../music_player.h"

void DrawSettingsAudio(float x, float y, float w, float h) {
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    bool mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    float t = (float)GetTime();
    
    DrawScaledText("🔊 AUDIO CONFIGURATION", x, y, 16, COLOR_CYAN);
    DrawScaledLine(x, y + 24, x + w, y + 24, Color{30, 35, 50, 100});
    
    float rowY = y + 44;
    float rowH = 50.0f;
    
    // ---- MASTER VOLUME ----
    DrawScaledText("MASTER VOLUME", x, rowY, 12, COLOR_GHOST);
    
    float sliderX = x + 180;
    float sliderY = rowY + 4;
    float sliderW = w - 200;
    float sliderH = 18.0f;
    
    DrawScaledRect(sliderX, sliderY, sliderW, sliderH, Color{12, 15, 20, 255});
    DrawScaledRectLines(sliderX, sliderY, sliderW, sliderH, Color{30, 35, 50, 100});
    
    float volume = GetMusicPlayer().GetVolume();
    float fillW = sliderW * volume;
    if (fillW < 2.0f) fillW = 2.0f;
    DrawScaledRect(sliderX, sliderY, fillW, sliderH, COLOR_TOXIC);
    
    // Volume knob
    float knobX = sliderX + fillW - 6;
    DrawScaledRect(knobX, sliderY - 3, 12, sliderH + 6, Color{40, 45, 65, 200});
    DrawScaledRectLines(knobX, sliderY - 3, 12, sliderH + 6, COLOR_BORDER);
    
    // Volume percentage
    char volStr[16];
    snprintf(volStr, sizeof(volStr), "%d%%", (int)(volume * 100.0f));
    DrawScaledText(volStr, sliderX + sliderW + 12, sliderY + 2, 11, COLOR_TOXIC);
    
    bool hoverSlider = RefRectHover(sliderX, sliderY - 10, sliderW, sliderH + 20, refMouse);
    if ((mouseDown && hoverSlider) || (clicked && RefRectHover(knobX - 6, sliderY - 6, 24, sliderH + 12, refMouse))) {
        Vector2 refMouse = GetRefMousePos();
        float newVol = (refMouse.x - sliderX) / sliderW;
        if (newVol < 0.0f) newVol = 0.0f;
        if (newVol > 1.0f) newVol = 1.0f;
        GetMusicPlayer().SetVolume(newVol);
    }
    
    rowY += rowH + 8;
    DrawScaledLine(x + 10, rowY, x + w - 10, rowY, Color{30, 35, 50, 60});
    rowY += 12;
    
    // ---- CURRENT TRACK INFO ----
    DrawScaledText("NOW PLAYING", x, rowY, 12, COLOR_GHOST);
    rowY += 22;
    
    const MusicTrack* track = GetMusicPlayer().GetCurrentTrack();
    if (track) {
        DrawScaledText(track->title.c_str(), x + 10, rowY, 14, track->accentColor);
        DrawScaledText(track->artist.c_str(), x + 10, rowY + 22, 10, COLOR_GHOST);
        
        char durationStr[32];
        int mins = (int)track->duration / 60;
        int secs = (int)track->duration % 60;
        snprintf(durationStr, sizeof(durationStr), "%02d:%02d", mins, secs);
        DrawScaledText(durationStr, x + w - 60, rowY + 6, 11, COLOR_GHOST);
    } else {
        DrawScaledText("No track loaded", x + 10, rowY + 6, 12, COLOR_GHOST);
    }
    
    rowY += 44;
    DrawScaledLine(x + 10, rowY, x + w - 10, rowY, Color{30, 35, 50, 60});
    rowY += 12;
    
    // ---- AUDIO VISUALIZER SETTINGS ----
    DrawScaledText("VISUALIZER", x, rowY, 12, COLOR_GHOST);
    rowY += 22;
    
    DrawScaledText("Sensitivity", x + 10, rowY + 4, 10, COLOR_CYAN);
    // Sensitivity slider
    float sensX = x + 130;
    float sensY = rowY + 4;
    float sensW = 150.0f;
    float sensH = 12.0f;
    DrawScaledRect(sensX, sensY, sensW, sensH, Color{12, 15, 20, 255});
    DrawScaledRectLines(sensX, sensY, sensW, sensH, Color{30, 35, 50, 100});
    DrawScaledRect(sensX, sensY, sensW * 0.7f, sensH, COLOR_AMBER);
    DrawScaledText("70%", sensX + sensW + 10, sensY, 9, COLOR_AMBER);
    
    rowY += 30;
}