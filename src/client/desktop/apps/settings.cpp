#include "../desktop.h"
#include "../../render.h"
#include "../../../shared/vnet.h"
#include "../widgets/music_player.h"

// Forward declarations for category drawers (now in separate files)
void DrawSettingsTheme(float x, float y, float w, float h);
void DrawSettingsAudio(float x, float y, float w, float h);
void DrawSettingsDisplay(float x, float y, float w, float h);
void DrawSettingsSecurity(float x, float y, float w, float h);
void DrawSettingsSystem(float x, float y, float w, float h);

void Desktop::DrawSettings(const AppWindow& win) {
    float cx = win.x + 4;
    float cy = win.y + m_windowTitleHeight + 4;
    float cw = win.w - 8;
    float ch = win.h - m_windowTitleHeight - 8;
    
    float t = (float)GetTime();
    float pulse = sinf(t * 2.0f) * 0.5f + 0.5f;
    
    // ---- BACKGROUND ----
    DrawScaledRect(cx, cy, cw, ch, Color{6, 8, 14, 255});
    DrawScaledRectLines(cx, cy, cw, ch, COLOR_BORDER);
    
    // ---- HEADER ----
    float headerH = 50.0f;
    DrawScaledRect(cx, cy, cw, headerH, Color{14, 18, 28, 255});
    DrawScaledLine(cx, cy + headerH, cx + cw, cy + headerH, Color{30, 35, 50, 150});
    
    DrawScaledText("⚙ SETTINGS // VEKTRAOS CONTROL PANEL", cx + 16, cy + 14, 14, COLOR_AMBER);
    
    // Live system status indicator
    float statusPulse = sinf(t * 3.0f) * 0.3f + 0.7f;
    Color statusColor = {40, 240, 100, (unsigned char)(statusPulse * 200 + 55)};
    DrawScaledRect(cx + cw - 130, cy + 14, 8, 8, statusColor);
    DrawScaledText("SYSTEM LIVE", cx + cw - 115, cy + 13, 10, COLOR_TOXIC);
    
    // ---- SIDEBAR ----
    float sidebarW = 180.0f;
    float sidebarX = cx;
    float sidebarY = cy + headerH;
    float sidebarH = ch - headerH;
    
    DrawScaledRect(sidebarX, sidebarY, sidebarW, sidebarH, Color{10, 12, 20, 220});
    DrawScaledLine(sidebarX + sidebarW, sidebarY, sidebarX + sidebarW, sidebarY + sidebarH, Color{30, 35, 50, 100});
    
    // Sidebar items
    static int selectedCategory = 0;
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    const char* categories[] = {
        "🎨 Theme",
        "🔊 Audio",
        "🖥 Display",
        "🔒 Security",
        "📦 System"
    };
    
    for (int i = 0; i < 5; i++) {
        float itemY = sidebarY + 20 + i * 45.0f;
        bool hover = RefRectHover(sidebarX + 8, itemY, sidebarW - 16, 36, refMouse);
        bool active = (i == selectedCategory);
        
        Color bg = active ? Color{40, 45, 70, 200} : (hover ? Color{30, 35, 55, 150} : Color{0,0,0,0});
        DrawScaledRect(sidebarX + 8, itemY, sidebarW - 16, 36, bg);
        
        if (active) {
            DrawScaledRect(sidebarX + 8, itemY, 3, 36, COLOR_BLOOD);
        }
        
        DrawScaledText(categories[i], sidebarX + 20, itemY + 10, 11, 
                      active ? COLOR_TOXIC : (hover ? COLOR_CYAN : COLOR_GHOST));
        
        if (clicked && hover) {
            selectedCategory = i;
        }
    }
    
    // Version info at bottom of sidebar
    DrawScaledLine(sidebarX + 8, sidebarY + sidebarH - 50, sidebarX + sidebarW - 8, sidebarY + sidebarH - 50, Color{30, 35, 50, 80});
    DrawScaledText("VEKTRAOS v9.5", sidebarX + 20, sidebarY + sidebarH - 32, 9, COLOR_GHOST);
    DrawScaledText("CYBERWARFARE ENGINE", sidebarX + 20, sidebarY + sidebarH - 18, 8, {80, 90, 110, 150});
    
    // ---- CONTENT AREA ----
    float contentX = cx + sidebarW + 12;
    float contentY = cy + headerH + 8;
    float contentW = cw - sidebarW - 20;
    float contentH = ch - headerH - 16;
    
    // Content background
    DrawScaledRect(contentX, contentY, contentW, contentH, Color{8, 10, 18, 200});
    DrawScaledRectLines(contentX, contentY, contentW, contentH, Color{30, 35, 50, 80});
    
    // ---- DRAW SELECTED CATEGORY CONTENT ----
    float contentOffsetX = contentX + 20;
    float contentOffsetY = contentY + 16;
    
    switch (selectedCategory) {
        case 0: DrawSettingsTheme(contentOffsetX, contentOffsetY, contentW - 40, contentH - 32); break;
        case 1: DrawSettingsAudio(contentOffsetX, contentOffsetY, contentW - 40, contentH - 32); break;
        case 2: DrawSettingsDisplay(contentOffsetX, contentOffsetY, contentW - 40, contentH - 32); break;
        case 3: DrawSettingsSecurity(contentOffsetX, contentOffsetY, contentW - 40, contentH - 32); break;
        case 4: DrawSettingsSystem(contentOffsetX, contentOffsetY, contentW - 40, contentH - 32); break;
    }
}