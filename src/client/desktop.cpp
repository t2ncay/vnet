#include "desktop.h"
#include "render.h"
#include "vnet.h"
#include "game.h"
#include "vnet_client.h"
#include "vnet_sites.h"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <cmath>
#include <algorithm>

// ============================================================
// STATIC CALLBACKS - App Launchers
// ============================================================

static void LaunchBrowser() {
    GetDesktop().OpenApp(AppType::Browser, "VNET Browser");
}

static void LaunchTerminal() {
    GetDesktop().OpenApp(AppType::Terminal, "Terminal");
}

static void LaunchProfile() {
    GetDesktop().OpenApp(AppType::Profile, "User Profile");
}

static void LaunchSettings() {
    GetDesktop().OpenApp(AppType::Settings, "Settings");
}

static void LaunchFeed() {
    GetDesktop().OpenApp(AppType::Feed, "System Feed");
}

// icon functions

void Desktop::LoadIcons() {
    // Load all icon textures
    // You can use PNG with transparency (good for dark themes)
    
    auto loadIcon = [this](const std::string& key, const std::string& path) {
        Texture2D tex = LoadTexture(path.c_str());
        if (tex.id != 0) {
            m_iconTextures[key] = tex;
            printf("[ICON] Loaded: %s\n", path.c_str());
        } else {
            printf("[ICON] Failed to load: %s\n", path.c_str());
        }
    };
    
    // Load your icons
    loadIcon("browser", "assets/icons/browser.png");
    loadIcon("terminal", "assets/icons/terminal.png");
    loadIcon("profile", "assets/icons/profile.png");
    loadIcon("settings", "assets/icons/settings.png");
    loadIcon("feed", "assets/icons/feed.png");
    loadIcon("folder", "assets/icons/folder.png");
    loadIcon("about", "assets/icons/about.png");
}

void Desktop::UnloadIcons() {
    for (auto& pair : m_iconTextures) {
        UnloadTexture(pair.second);
    }
    m_iconTextures.clear();
}

Texture2D Desktop::GetIcon(const std::string& key) {
    auto it = m_iconTextures.find(key);
    if (it != m_iconTextures.end()) {
        return it->second;
    }
    // Return empty texture if not found
    Texture2D empty = {0};
    return empty;
}

// ============================================================
// DESKTOP IMPLEMENTATION
// ============================================================

void Desktop::Init() {
    m_active = true;
    m_windows.clear();
    m_windows.reserve(8);
    m_appGridVisible = false;
    m_currentWorkspace = 0;
    
    LoadIcons();  // Load icons before creating apps
    
    m_apps.clear();
    m_apps.push_back({"Browser", "browser", "Browse the VNET", LaunchBrowser});
    m_apps.push_back({"Terminal", "terminal", "Command line", LaunchTerminal});
    m_apps.push_back({"Profile", "profile", "User info", LaunchProfile});
    m_apps.push_back({"Settings", "settings", "Preferences", LaunchSettings});
    m_apps.push_back({"Feed", "feed", "System feed", LaunchFeed});
    
    m_workspaces.clear();
    m_workspaces.emplace_back("Main");
    m_workspaces.emplace_back("Work");
    m_workspaces.emplace_back("Chat");
    
    OpenApp(AppType::Browser, "VNET Browser");
}

void Desktop::Shutdown() {
    m_windows.clear();
    m_apps.clear();
    m_workspaces.clear();
    UnloadIcons();
}

// ============================================================
// WINDOW MANAGEMENT
// ============================================================

int Desktop::OpenApp(AppType type, const char* title) {
    if (m_windows.size() >= 8) return -1;
    
    for (int i = 0; i < (int)m_windows.size(); i++) {
        if (m_windows[i].type == type && !m_windows[i].minimized) {
            FocusWindow(i);
            return i;
        }
        if (m_windows[i].type == type && m_windows[i].minimized) {
            m_windows[i].minimized = false;
            FocusWindow(i);
            return i;
        }
    }
    
    AppWindow win;
    win.type = type;
    win.title = title ? title : "Window";
    win.x = 40 + (int)m_windows.size() * 20;
    win.y = 60 + (int)m_windows.size() * 20;
    win.w = 850;
    win.h = 600;
    win.minimized = false;
    win.maximized = false;
    win.focused = true;
    win.dragging = false;
    win.dragX = 0;
    win.dragY = 0;
    
    if (win.x > 200) win.x = 80;
    if (win.y > 200) win.y = 80;
    
    if (win.x + win.w > REF_WIDTH) win.x = REF_WIDTH - win.w - 20;
    if (win.y + win.h > REF_HEIGHT - m_topBarHeight - 20) {
        win.y = (int)(REF_HEIGHT - m_topBarHeight - win.h - 20);
    }
    
    m_windows.push_back(win);
    int idx = (int)m_windows.size() - 1;
    m_workspaces[m_currentWorkspace].windows.push_back(idx);
    FocusWindow(idx);
    
    return idx;
}

void Desktop::CloseWindow(int idx) {
    if (idx < 0 || idx >= (int)m_windows.size()) return;
    
    for (auto& ws : m_workspaces) {
        for (int i = 0; i < (int)ws.windows.size(); i++) {
            if (ws.windows[i] == idx) {
                ws.windows.erase(ws.windows.begin() + i);
                break;
            }
        }
    }
    for (auto& ws : m_workspaces) {
        for (int& w : ws.windows) {
            if (w > idx) w--;
        }
    }
    
    m_windows.erase(m_windows.begin() + idx);
    if (m_focused >= idx) m_focused--;
    if (m_focused >= (int)m_windows.size()) m_focused = (int)m_windows.size() - 1;
    
    if (m_windows.empty()) {
        OpenApp(AppType::Browser, "VNET Browser");
    }
}

void Desktop::FocusWindow(int idx) {
    if (idx < 0 || idx >= (int)m_windows.size()) return;
    
    for (auto& w : m_windows) w.focused = false;
    
    m_windows[idx].focused = true;
    m_focused = idx;
    
    AppWindow win = m_windows[idx];
    m_windows.erase(m_windows.begin() + idx);
    m_windows.push_back(win);
    m_focused = (int)m_windows.size() - 1;
}

void Desktop::MinimizeWindow(int idx) {
    if (idx < 0 || idx >= (int)m_windows.size()) return;
    m_windows[idx].minimized = !m_windows[idx].minimized;
    if (m_windows[idx].minimized && m_focused == idx) {
        m_focused = -1;
        for (int i = (int)m_windows.size() - 1; i >= 0; i--) {
            if (!m_windows[i].minimized) {
                m_focused = i;
                m_windows[i].focused = true;
                break;
            }
        }
    }
}

void Desktop::MaximizeWindow(int idx) {
    if (idx < 0 || idx >= (int)m_windows.size()) return;
    m_windows[idx].maximized = !m_windows[idx].maximized;
}

// ============================================================
// WORKSPACE MANAGEMENT
// ============================================================

void Desktop::SwitchWorkspace(int idx) {
    if (idx < 0 || idx >= (int)m_workspaces.size()) return;
    m_currentWorkspace = idx;
    m_focused = -1;
    for (int w : m_workspaces[idx].windows) {
        if (!m_windows[w].minimized) {
            m_focused = w;
            m_windows[w].focused = true;
            break;
        }
    }
}

void Desktop::MoveWindowToWorkspace(int winIdx, int wsIdx) {
    if (winIdx < 0 || winIdx >= (int)m_windows.size()) return;
    if (wsIdx < 0 || wsIdx >= (int)m_workspaces.size()) return;
    
    for (auto& ws : m_workspaces) {
        for (int i = 0; i < (int)ws.windows.size(); i++) {
            if (ws.windows[i] == winIdx) {
                ws.windows.erase(ws.windows.begin() + i);
                break;
            }
        }
    }
    m_workspaces[wsIdx].windows.push_back(winIdx);
}

// ============================================================
// UPDATE - COMPLETE FIXED VERSION
// ============================================================

void Desktop::Update(float dt) {
    (void)dt;
    if (!m_active) return;
    
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    bool dragging = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    
    // ============================================================
    // APP GRID
    // ============================================================
    if (IsKeyPressed(KEY_LEFT_SUPER) || IsKeyPressed(KEY_RIGHT_SUPER)) {
        m_appGridVisible = !m_appGridVisible;
    }
    
    if (m_appGridVisible) {
        float startXg = (REF_WIDTH - 600) / 2;
        float startYg = 100;
        float iconSizeg = 80;
        float spacingg = 20;
        int colsg = 4;
        
        for (int i = 0; i < (int)m_apps.size(); i++) {
            int col = i % colsg;
            int row = i / colsg;
            float x = startXg + col * (iconSizeg + spacingg);
            float y = startYg + row * (iconSizeg + spacingg + 30);
            
            if (clicked && RefRectHover(x, y, iconSizeg, iconSizeg, refMouse)) {
                m_apps[i].onClick();
                m_appGridVisible = false;
                clicked = false;
                break;
            }
        }
        if (clicked && !RefRectHover(startXg - 20, startYg - 20, 640, 500, refMouse)) {
            m_appGridVisible = false;
        }
    }
    
    // ============================================================
    // WINDOW TITLE BAR CONTROLS (Close, Minimize, Maximize, Drag)
    // ============================================================
    for (int i = (int)m_windows.size() - 1; i >= 0; i--) {
        auto& win = m_windows[i];
        
        bool inWorkspace = false;
        for (int w : m_workspaces[m_currentWorkspace].windows) {
            if (w == i) { inWorkspace = true; break; }
        }
        if (!inWorkspace) continue;
        if (win.minimized) continue;
        
        float titleY = win.y;
        float titleH = m_windowTitleHeight;
        
        // Check if mouse is on title bar
        if (RefRectHover(win.x, titleY, win.w, titleH, refMouse)) {
            // CLOSE BUTTON (rightmost)
            if (clicked && RefRectHover(win.x + win.w - 25, win.y + 5, 20, 20, refMouse)) {
                CloseWindow(i);
                return;
            }
            // MINIMIZE BUTTON
            if (clicked && RefRectHover(win.x + win.w - 50, win.y + 5, 20, 20, refMouse)) {
                MinimizeWindow(i);
                return;
            }
            // MAXIMIZE BUTTON
            if (clicked && RefRectHover(win.x + win.w - 75, win.y + 5, 20, 20, refMouse)) {
                MaximizeWindow(i);
                return;
            }
            
            // DRAG (only if not clicking a button)
            if (clicked) {
                // Bring to front FIRST before dragging
                FocusWindow(i);
                win.dragging = true;
                win.dragX = (int)(refMouse.x - win.x);
                win.dragY = (int)(refMouse.y - titleY);
                return;
            }
        }
        
        // Handle dragging - use the current window reference
        if (win.dragging && dragging) {
            win.x = (int)(refMouse.x - win.dragX);
            win.y = (int)(refMouse.y - win.dragY);
            if (win.x < 0) win.x = 0;
            if (win.y < m_topBarHeight) win.y = (int)m_topBarHeight;
            if (win.x + win.w > REF_WIDTH) win.x = REF_WIDTH - win.w;
            if (win.y + win.h > REF_HEIGHT - 20) win.y = REF_HEIGHT - win.h - 20;
        } else {
            win.dragging = false;
        }
    }

    // ============================================================
    // WINDOW CONTENT CLICKS - Bring to front
    // ============================================================
    if (clicked) {
        for (int i = (int)m_windows.size() - 1; i >= 0; i--) {
            auto& win = m_windows[i];
            
            bool inWorkspace = false;
            for (int w : m_workspaces[m_currentWorkspace].windows) {
                if (w == i) { inWorkspace = true; break; }
            }
            if (!inWorkspace) continue;
            if (win.minimized) continue;
            
            float contentX = win.x + 4;
            float contentY = win.y + m_windowTitleHeight + 4;
            float contentW = win.w - 8;
            float contentH = win.h - m_windowTitleHeight - 8;
            
            if (RefRectHover(contentX, contentY, contentW, contentH, refMouse)) {
                FocusWindow(i);
                break;
            }
        }
    }
    
    // ============================================================
    // DESKTOP ICON CLICKS (only if no window was clicked)
    // ============================================================
    if (clicked) {
        float iconSize = 80.0f;
        float spacing = 20.0f;
        float startX = 30.0f;
        float startY = 80.0f;
        int cols = 4;
        
        for (int i = 0; i < (int)m_apps.size(); i++) {
            int col = i % cols;
            int row = i / cols;
            float x = startX + col * (iconSize + spacing);
            float y = startY + row * (iconSize + spacing + 30);
            
            if (RefRectHover(x, y, iconSize, iconSize, refMouse)) {
                m_apps[i].onClick();
                break;
            }
        }
    }
    
    // ============================================================
    // WORKSPACE SWITCHING
    // ============================================================
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
        if (IsKeyPressed(KEY_ONE)) SwitchWorkspace(0);
        if (IsKeyPressed(KEY_TWO)) SwitchWorkspace(1);
        if (IsKeyPressed(KEY_THREE)) SwitchWorkspace(2);
    }
}

// ============================================================
// DRAW FUNCTIONS (Unchanged - keep as they were)
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

void Desktop::Draw() {
    if (!m_active) return;
    
    DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, Color{18, 20, 28, 255});
    
    for (int x = 0; x < REF_WIDTH; x += 40) {
        DrawScaledLine(x, 0, x, REF_HEIGHT, Color{25, 28, 38, 60});
    }
    for (int y = 0; y < REF_HEIGHT; y += 40) {
        DrawScaledLine(0, y, REF_WIDTH, y, Color{25, 28, 38, 60});
    }
    
    DrawDesktopIcons();
    
    std::vector<int> visibleWindows;
    for (int i = 0; i < (int)m_windows.size(); i++) {
        bool inWorkspace = false;
        for (int w : m_workspaces[m_currentWorkspace].windows) {
            if (w == i) { inWorkspace = true; break; }
        }
        if (inWorkspace && !m_windows[i].minimized) {
            visibleWindows.push_back(i);
        }
    }
    
    std::sort(visibleWindows.begin(), visibleWindows.end(), [this](int a, int b) {
        bool focusedA = m_windows[a].focused;
        bool focusedB = m_windows[b].focused;
        return focusedA < focusedB;
    });
    
    for (int idx : visibleWindows) {
        DrawWindow(idx);
    }
    
    DrawTopBar();
    if (m_appGridVisible) DrawAppGrid();
}

// ============================================================
// TOP BAR, WORKSPACE INDICATOR, CLOCK, APP GRID
// ============================================================

void Desktop::DrawTopBar() {
    float barY = 0;
    
    DrawScaledRect(0, barY, REF_WIDTH, m_topBarHeight, Color{28, 30, 40, 235});
    DrawScaledLine(0, barY + m_topBarHeight, REF_WIDTH, barY + m_topBarHeight, Color{40, 45, 60, 200});
    
    float btnX = 10;
    bool actHover = RefRectHover(btnX, barY + 4, 80, m_topBarHeight - 8, GetRefMousePos());
    DrawScaledRect(btnX, barY + 4, 80, m_topBarHeight - 8, 
               actHover ? Color{50, 55, 75, 200} : Color{0,0,0,0});
    DrawScaledText("Activities", btnX + 12, barY + 14, 11, actHover ? COLOR_TOXIC : COLOR_GHOST);
    btnX += 90;
    
    bool gridHover = RefRectHover(btnX, barY + 4, 30, m_topBarHeight - 8, GetRefMousePos());
    DrawScaledText("⊞", btnX + 8, barY + 12, 14, gridHover ? COLOR_TOXIC : COLOR_GHOST);
    btnX += 40;
    
    DrawWorkspaceIndicator();
    DrawClock();
    
    char vcoin[32];
    snprintf(vcoin, sizeof(vcoin), "VCOIN: %.2f", g_player.vcoin);
    DrawScaledText(vcoin, REF_WIDTH - 200, barY + 16, 11, COLOR_TOXIC);
}

void Desktop::DrawWorkspaceIndicator() {
    float barY = 0;
    float startX = 150;
    float spacing = 50;
    
    for (int i = 0; i < (int)m_workspaces.size(); i++) {
        float x = startX + i * spacing;
        bool active = (i == m_currentWorkspace);
        
        Color bg = active ? Color{50, 55, 80, 200} : Color{30, 33, 45, 200};
        Color border = active ? COLOR_BLOOD : COLOR_BORDER;
        
        DrawScaledRect(x, barY + 8, 40, m_topBarHeight - 16, bg);
        DrawScaledRectLines(x, barY + 8, 40, m_topBarHeight - 16, border);
        
        int count = (int)m_workspaces[i].windows.size();
        char dots[8] = "";
        for (int j = 0; j < count && j < 4; j++) {
            dots[j] = '•';
        }
        dots[count] = '\0';
        DrawScaledText(dots, x + 8, barY + 16, 8, active ? COLOR_TOXIC : COLOR_GHOST);
    }
}

void Desktop::DrawClock() {
    time_t now = time(nullptr);
    struct tm* local = localtime(&now);
    
    char timeStr[16];
    strftime(timeStr, sizeof(timeStr), "%H:%M", local);
    
    float barY = 0;
    DrawScaledText(timeStr, REF_WIDTH - 110, barY + 14, 13, COLOR_CYAN);
}

void Desktop::DrawAppGrid() {
    DrawScaledRect(0, m_topBarHeight, REF_WIDTH, REF_HEIGHT - m_topBarHeight, 
                   Color{0, 0, 0, 180});
    
    float startX = (REF_WIDTH - 600) / 2;
    float startY = 120;
    float iconSize = 80;
    float spacing = 20;
    int cols = 4;
    
    DrawScaledRect(startX - 20, startY - 20, 640, 480, Color{28, 30, 42, 230});
    DrawScaledRectLines(startX - 20, startY - 20, 640, 480, COLOR_BORDER);
    
    DrawScaledText("Apps", startX + 20, startY + 10, 16, COLOR_BLOOD);
    DrawScaledLine(startX + 10, startY + 40, startX + 610, startY + 40, COLOR_BORDER);
    
        for (int i = 0; i < (int)m_apps.size(); i++) {
        int col = i % cols;
        int row = i / cols;
        float x = startX + col * (iconSize + spacing);
        float y = startY + 50 + row * (iconSize + spacing + 30);
        
        Vector2 refMouse = GetRefMousePos();
        bool hover = RefRectHover(x, y, iconSize, iconSize, refMouse);
        
        DrawScaledRect(x, y, iconSize, iconSize, 
               hover ? Color{40, 45, 65, 200} : Color{20, 22, 35, 200});
        DrawScaledRectLines(x, y, iconSize, iconSize, hover ? COLOR_BLOOD : COLOR_BORDER);
        
        // Draw icon texture
        Texture2D icon = GetIcon(m_apps[i].iconKey);
        if (icon.id != 0) {
            float iconDrawSize = 40.0f;
            float iconX = x + (iconSize - iconDrawSize) / 2.0f;
            float iconY = y + 10.0f;
            DrawTexturePro(
                icon,
                {0, 0, (float)icon.width, (float)icon.height},
                {SX(iconX), SY(iconY), iconDrawSize * g_uiScale, iconDrawSize * g_uiScale},
                {0, 0},
                0.0f,
                hover ? COLOR_TOXIC : COLOR_CYAN
            );
        } else {
            DrawScaledText(m_apps[i].iconKey.c_str(), x + (iconSize - 24) / 2, y + 10, 20, COLOR_CYAN);
        }
        
        DrawScaledText(m_apps[i].name.c_str(), x + 10, y + iconSize - 20, 10, COLOR_GHOST);
    }
}

// ============================================================
// WINDOW DRAWING
// ============================================================

void Desktop::DrawWindow(int idx) { 
    const auto& win = m_windows[idx];
    if (win.minimized) return;
    
    bool focused = win.focused;
    Color borderCol = focused ? COLOR_BLOOD : COLOR_BORDER;
    Color titleCol = focused ? COLOR_BLOOD : Color{30, 35, 50, 255};
    
    DrawScaledRect(win.x + 4, win.y + 4, win.w, win.h, Color{0, 0, 0, 120});
    
    DrawScaledRect(win.x, win.y, win.w, win.h, COLOR_PANEL);
    DrawScaledRectLines(win.x, win.y, win.w, win.h, borderCol);
    
    DrawScaledRect(win.x, win.y, win.w, m_windowTitleHeight, titleCol);
    DrawScaledLine(win.x, win.y + m_windowTitleHeight, win.x + win.w, win.y + m_windowTitleHeight, borderCol);
    DrawScaledText(win.title.c_str(), win.x + 10, win.y + 8, 11, focused ? COLOR_BLACK : COLOR_GHOST);
    
    // Close button (X)
    DrawScaledRect(win.x + win.w - 25, win.y + 5, 20, 20, COLOR_BLOOD);
    DrawScaledRectLines(win.x + win.w - 25, win.y + 5, 20, 20, COLOR_BORDER);
    DrawScaledText("✕", win.x + win.w - 20, win.y + 7, 14, COLOR_BLACK);
    
    // Minimize button (-)
    DrawScaledRect(win.x + win.w - 50, win.y + 5, 20, 20, Color{35, 40, 55, 255});
    DrawScaledRectLines(win.x + win.w - 50, win.y + 5, 20, 20, COLOR_BORDER);
    DrawScaledText("─", win.x + win.w - 45, win.y + 6, 14, COLOR_GHOST);
    
    // Maximize button (□)
    DrawScaledRect(win.x + win.w - 75, win.y + 5, 20, 20, Color{35, 40, 55, 255});
    DrawScaledRectLines(win.x + win.w - 75, win.y + 5, 20, 20, COLOR_BORDER);
    DrawScaledText("□", win.x + win.w - 70, win.y + 6, 14, COLOR_GHOST);
    
    DrawWindowContent(win);
}

void Desktop::DrawWindowContent(const AppWindow& win) {
    float cx = win.x + 4;
    float cy = win.y + m_windowTitleHeight + 4;
    float cw = win.w - 8;
    float ch = win.h - m_windowTitleHeight - 8;
    
    BeginScissorMode((int)SX(cx), (int)SY(cy), 
                     (int)(cw * g_uiScale), (int)(ch * g_uiScale));
    
    switch (win.type) {
        case AppType::Browser:  DrawBrowser(win); break;
        case AppType::Terminal: DrawTerminal(win); break;
        case AppType::Profile:  DrawProfile(win); break;
        case AppType::Settings: DrawSettings(win); break;
        case AppType::Feed:     DrawFeed(win); break;
        default: break;
    }
    
    EndScissorMode();
}

// ============================================================
// APP CONTENT RENDERERS
// ============================================================

void Desktop::DrawBrowserConnectionOverlay(float contentX, float contentY, float contentW, float contentH) {
    float t = (float)GetTime();
    float pulse = sinf(t * 8.0f) * 0.5f + 0.5f;
    
    // Center of the browser content area
    float centerX = contentX + contentW / 2.0f;
    float centerY = contentY + contentH / 2.0f;
    
    // Connection box - slightly larger
    float boxW = contentW * 0.80f;
    float boxH = contentH * 0.75f;
    float boxX = centerX - boxW / 2.0f;
    float boxY = centerY - boxH / 2.0f;
    
    // ---- NO SHAKE - just subtle CRT wobble ----
    float wobbleX = sinf(t * 0.7f) * 0.5f;
    float wobbleY = cosf(t * 0.9f + 0.5f) * 0.3f;
    
    // ---- BACKGROUND SCANLINES ----
    for (int i = 0; i < (int)(boxH / 3.0f); i++) {
        float scanY = boxY + i * 3.0f + fmodf(t * 60.0f, 3.0f);
        float scanAlpha = (i % 2 == 0) ? 12 : 4;
        DrawScaledRect(boxX + wobbleX, scanY + wobbleY, boxW, 1, 
                       {0, 0, 0, (unsigned char)scanAlpha});
    }
    
    // ---- CRT VIGNETTE ----
    for (int i = 0; i < 3; i++) {
        float size = i * 30.0f;
        DrawScaledRect(boxX + wobbleX + size, boxY + wobbleY + size, 
                       boxW - size * 2, boxH - size * 2, 
                       {0, 0, 0, (unsigned char)(5 - i * 2)});
    }
    
    // ---- CORNER RETICLES (GLOWING) ----
    float cornerSize = 20.0f;
    float glowPulse = sinf(t * 2.5f) * 0.3f + 0.7f;
    Color glowCol = {220, 20, 40, (unsigned char)(glowPulse * 200 + 55)};
    
    // Top-left
    DrawScaledRect(boxX + wobbleX - 4, boxY + wobbleY - 4, cornerSize, 3, glowCol);
    DrawScaledRect(boxX + wobbleX - 4, boxY + wobbleY - 4, 3, cornerSize, glowCol);
    // Top-right
    DrawScaledRect(boxX + boxW + wobbleX + 1, boxY + wobbleY - 4, cornerSize, 3, glowCol);
    DrawScaledRect(boxX + boxW + wobbleX - 1, boxY + wobbleY - 4, 3, cornerSize, glowCol);
    // Bottom-left
    DrawScaledRect(boxX + wobbleX - 4, boxY + boxH + wobbleY + 1, cornerSize, 3, glowCol);
    DrawScaledRect(boxX + wobbleX - 4, boxY + boxH + wobbleY - 1, 3, cornerSize, glowCol);
    // Bottom-right
    DrawScaledRect(boxX + boxW + wobbleX + 1, boxY + boxH + wobbleY + 1, cornerSize, 3, glowCol);
    DrawScaledRect(boxX + boxW + wobbleX - 1, boxY + boxH + wobbleY - 1, 3, cornerSize, glowCol);
    
    // ---- CORNER GLOW BLOOM ----
    DrawScaledRect(boxX + wobbleX - 6, boxY + wobbleY - 6, 3, 3, {220, 20, 40, 80});
    DrawScaledRect(boxX + boxW + wobbleX + 3, boxY + wobbleY - 6, 3, 3, {220, 20, 40, 80});
    DrawScaledRect(boxX + wobbleX - 6, boxY + boxH + wobbleY + 3, 3, 3, {220, 20, 40, 80});
    DrawScaledRect(boxX + boxW + wobbleX + 3, boxY + boxH + wobbleY + 3, 3, 3, {220, 20, 40, 80});
    
    // ---- REMAINING TIME ----
    float remaining = g_player.targetConnectTime - g_player.connectTimer;
    if (remaining < 0.0f) remaining = 0.0f;
    float ratio = (g_player.targetConnectTime > 0.0f) ? 
                  g_player.connectTimer / g_player.targetConnectTime : 0.0f;
    if (ratio < 0.02f) ratio = 0.02f;
    if (ratio > 1.0f) ratio = 1.0f;
    
    float headerY = boxY + 30.0f + wobbleY;
    
    // ---- ANIMATED HEADER WITH PULSE (CENTERED) ----
    Color headerCol = (pulse > 0.5f) ? COLOR_AMBER : COLOR_BLOOD;
    headerCol.a = (unsigned char)(pulse * 150 + 105);
    
    const char* headerText = "[ TOR PROXY CIRCUIT HANDSHAKE ACTIVE ]";
    float headerW = MeasureScaledTextWidth(headerText, 16);
    
    // Header background glow
    float headerGlow = sinf(t * 3.0f) * 0.3f + 0.7f;
    DrawScaledRect(centerX - headerW / 2 - 20 + wobbleX, headerY - 6, 
                   headerW + 40, 34, 
                   {220, 20, 40, (unsigned char)(headerGlow * 25)});
    
    DrawScaledText(headerText, 
                   centerX - headerW / 2 + wobbleX, 
                   headerY + 2, 16, headerCol);
    
    // ---- TARGET URL (CENTERED, LARGER) ----
    char urlStr[128];
    snprintf(urlStr, sizeof(urlStr), "RESOLVING: vnet://%s", g_player.pendingURL);
    float urlW = MeasureScaledTextWidth(urlStr, 14);
    
    DrawScaledText(urlStr, 
                   centerX - urlW / 2 + wobbleX, 
                   headerY + 42.0f + wobbleY, 
                   14, COLOR_CYAN);
    
    // URL subtle pulse underline
    float linePulse = sinf(t * 2.0f) * 0.3f + 0.7f;
    float lineY = headerY + 62.0f + wobbleY;
    DrawScaledRect(centerX - urlW / 2 - 10 + wobbleX, lineY, 
                   urlW + 20, 1, 
                   {0, 220, 240, (unsigned char)(linePulse * 80)});
    
    // ---- ROTATING CROSSHAIR (CENTERED, LARGER) ----
    float crosshairY = headerY + 85.0f + wobbleY;
    float rotOff = t * 3.5f;
    float crosshairRadius = 40.0f;
    float crosshairHeight = 24.0f;
    
    // Crosshair trail
    for (int trail = 0; trail < 4; trail++) {
        float trailOff = t * 3.5f - trail * 0.12f;
        float trailAlpha = 50 - trail * 12;
        for (int i = 0; i < 4; i++) {
            float angle = trailOff + (float)i * 1.5708f;
            float rx = centerX + cosf(angle) * (crosshairRadius - trail * 6.0f) + wobbleX;
            float ry = crosshairY + sinf(angle) * (crosshairHeight - trail * 3.5f) + wobbleY;
            DrawScaledRect(rx - 1.5f, ry - 1.5f, 3, 3, 
                          {220, 20, 40, (unsigned char)trailAlpha});
        }
    }
    
    // Main crosshair (larger)
    for (int i = 0; i < 4; i++) {
        float angle = rotOff + (float)i * 1.5708f;
        float rx = centerX + cosf(angle) * crosshairRadius + wobbleX;
        float ry = crosshairY + sinf(angle) * crosshairHeight + wobbleY;
        float size = 5.0f + sinf(t * 5.0f + i) * 1.5f;
        DrawScaledRect(rx - size/2, ry - size/2, size, size, COLOR_BLOOD);
    }
    
    // Center dot (larger, pulsing)
    float dotPulse = sinf(t * 4.0f) * 0.3f + 0.7f;
    float dotSize = 4.0f + dotPulse * 2.0f;
    DrawScaledRect(centerX - dotSize/2 + wobbleX, 
                   crosshairY - dotSize/2 + wobbleY, 
                   dotSize, dotSize, 
                   {220, 20, 40, (unsigned char)(dotPulse * 200 + 55)});
    
    // ---- LATENCY STATUS (CENTERED) ----
    char latencyStr[128];
    snprintf(latencyStr, sizeof(latencyStr), "LATENCY BUFFER: %.0fs REMAINING  •  HOPS: 3/3", remaining);
    float latW = MeasureScaledTextWidth(latencyStr, 12);
    DrawScaledText(latencyStr, 
                   centerX - latW / 2 + wobbleX, 
                   crosshairY + 45.0f + wobbleY, 
                   12, COLOR_TOXIC);
    
    // ---- PROGRESS BAR (CENTERED, LARGER) ----
    float barW = 440.0f;
    float barH = 26.0f;
    float barX = centerX - barW / 2 + wobbleX;
    float barY = crosshairY + 75.0f + wobbleY;
    
    // Bar background
    DrawScaledRect(barX, barY, barW, barH, COLOR_PANEL);
    DrawScaledRectLines(barX, barY, barW, barH, COLOR_BORDER);
    
    // Bar scanlines
    for (int i = 0; i < (int)barH; i += 2) {
        DrawScaledRect(barX, barY + i, barW, 1, {0, 0, 0, (unsigned char)(8 + i * 2)});
    }
    
    // Progress fill with gradient
    float fillW = barW * ratio;
    if (fillW < 2.0f) fillW = 2.0f;
    if (fillW > barW) fillW = barW;
    
    // Gradient fill (smoother)
    for (int x = 0; x < (int)fillW; x += 2) {
        float progress = (float)x / barW;
        unsigned char r = (unsigned char)(30 + progress * 190);
        unsigned char g = (unsigned char)(230 - progress * 140);
        unsigned char b = (unsigned char)(80 - progress * 60);
        DrawScaledRect(barX + x, barY, 2, barH, {r, g, b, 255});
    }
    
    // Glow effect on fill
    if (fillW > 10.0f) {
        DrawScaledRect(barX + fillW - 10.0f, barY - 3, 10, barH + 6, 
                       {40, 240, 100, 50});
        DrawScaledRect(barX + fillW - 4.0f, barY, 4, barH, 
                       {40, 240, 100, 130});
    }
    
    // ---- PERCENTAGE TEXT (CENTERED IN BAR) ----
    char pctStr[16];
    snprintf(pctStr, sizeof(pctStr), "%d%%", (int)(ratio * 100.0f));
    float pctW = MeasureScaledTextWidth(pctStr, 13);
    Color pctCol = (ratio > 0.5f) ? COLOR_BLACK : COLOR_CYAN;
    DrawScaledText(pctStr, 
                   centerX - pctW / 2 + wobbleX, 
                   barY + 5.0f, 13, pctCol);
    
    // ---- DATA PACKET ANIMATION (ABOVE PROGRESS BAR) ----
    float packetY = barY - 35.0f + wobbleY;
    for (int p = 0; p < 6; p++) {
        float offset = fmodf(t * 25.0f + p * 30.0f, barW - 30);
        float packetX = barX + 15 + offset;
        float packetSize = 5.0f + sinf(t * 3.5f + p) * 1.5f;
        Color packetCol = (p % 2 == 0) ? COLOR_TOXIC : COLOR_CYAN;
        packetCol.a = (unsigned char)(160 + sinf(t * 4.0f + p * 2.0f) * 50 + 50);
        DrawScaledRect(packetX, packetY + p * 2.5f, packetSize, packetSize, packetCol);
        
        // Packet trail
        for (int trail = 1; trail < 5; trail++) {
            float trailX = packetX - trail * 4.5f;
            if (trailX > barX) {
                DrawScaledRect(trailX, packetY + p * 2.5f, 2, 2, 
                               {packetCol.r, packetCol.g, packetCol.b, 
                                (unsigned char)(70 - trail * 14)});
            }
        }
    }
    
    // ---- HEX STREAM TELEMETRY (CENTERED) ----
    int hexTick = (int)fmodf(t * 18.0f, 99.0f);
    char hexStr[128];
    snprintf(hexStr, sizeof(hexStr), "0x88F9_NODE_HOP_OK  •  ENCRYPTING PACKET SUBNET SECTOR #%d", hexTick);
    float hexW = MeasureScaledTextWidth(hexStr, 10);
    DrawScaledText(hexStr, 
                   centerX - hexW / 2 + wobbleX, 
                   barY + 38.0f + wobbleY, 
                   10, COLOR_AMBER);
    
    // ---- FOOTER (CENTERED) ----
    const char* footerStr = "SPOOFING MAC ADDRESS • MIRRORING VIA ARCHIVAL.VNET";
    float footW = MeasureScaledTextWidth(footerStr, 10);
    DrawScaledText(footerStr, 
                   centerX - footW / 2 + wobbleX, 
                   barY + 58.0f + wobbleY, 
                   10, COLOR_GHOST);
    
    // ---- VERTICAL SCANLINE OVERLAY ----
    for (int x = 0; x < (int)boxW; x += 5) {
        float scanAlpha = 4 + sinf(t * 1.5f + x * 0.08f) * 3 + 3;
        DrawScaledRect(boxX + wobbleX + x, boxY + wobbleY, 1, boxH, 
                       {0, 0, 0, (unsigned char)scanAlpha});
    }
    
    // ---- CRT SCREEN FLICKER (SUBTLE) ----
    if (fmodf(t * 0.5f, 1.0f) > 0.97f) {
        DrawScaledRect(boxX + wobbleX, boxY + wobbleY, boxW, boxH, 
                       {0, 0, 0, (unsigned char)(15 + rand() % 25)});
    }
}

void Desktop::DrawBrowser(const AppWindow& win) {
    float cx = win.x + 4;
    float cy = win.y + m_windowTitleHeight + 4;
    float cw = win.w - 8;
    float ch = win.h - m_windowTitleHeight - 8;
    
    // Browser background
    DrawScaledRect(cx, cy, cw, ch, COLOR_BLACK);
    
    // ---- TOOLBAR ----
    float toolbarY = cy;
    float toolbarH = 48.0f;
    float navX = cx + 8;
    float navY = toolbarY + 8;
    float navSize = 30.0f;
    float spacing = 6.0f;
    
    // Toolbar background
    DrawScaledRect(cx, toolbarY, cw, toolbarH, Color{18, 20, 28, 230});
    DrawScaledLine(cx, toolbarY + toolbarH, cx + cw, toolbarY + toolbarH, COLOR_BORDER);
    
    // ---- NAVIGATION BUTTONS ----
    Vector2 refMouse = GetRefMousePos();
    
    // Back button
    bool backHover = RefRectHover(navX, navY, navSize, navSize, refMouse);
    DrawScaledRect(navX, navY, navSize, navSize, backHover ? Color{40, 45, 65, 255} : Color{0,0,0,0});
    DrawScaledText("◄", navX + 8, navY + 6, 14, backHover ? COLOR_TOXIC : COLOR_GHOST);
    navX += navSize + spacing;
    
    // Forward button
    bool fwdHover = RefRectHover(navX, navY, navSize, navSize, refMouse);
    DrawScaledRect(navX, navY, navSize, navSize, fwdHover ? Color{40, 45, 65, 255} : Color{0,0,0,0});
    DrawScaledText("►", navX + 8, navY + 6, 14, fwdHover ? COLOR_TOXIC : COLOR_GHOST);
    navX += navSize + spacing;
    
    // Reload button
    bool reloadHover = RefRectHover(navX, navY, navSize, navSize, refMouse);
    DrawScaledRect(navX, navY, navSize, navSize, reloadHover ? Color{40, 45, 65, 255} : Color{0,0,0,0});
    DrawScaledText("⟳", navX + 6, navY + 6, 16, reloadHover ? COLOR_TOXIC : COLOR_GHOST);
    navX += navSize + spacing;
    
    // Home button
    bool homeHover = RefRectHover(navX, navY, navSize, navSize, refMouse);
    DrawScaledRect(navX, navY, navSize, navSize, homeHover ? Color{40, 45, 65, 255} : Color{0,0,0,0});
    DrawScaledText("⌂", navX + 6, navY + 4, 16, homeHover ? COLOR_TOXIC : COLOR_GHOST);
    navX += navSize + spacing + 10;
    
    // ---- URL BAR ----
    float urlX = navX;
    float urlY = navY;
    float urlW = cw - (urlX - cx) - 20;
    float urlH = navSize;
    
    DrawScaledRect(urlX, urlY, urlW, urlH, COLOR_URLBAR);
    DrawScaledRectLines(urlX, urlY, urlW, urlH, COLOR_BORDER);
    
    // Show current or pending URL
    char urlDisplay[192];
    if (g_player.isConnecting) {
        snprintf(urlDisplay, sizeof(urlDisplay), "vnet://%s", g_player.pendingURL);
    } else {
        snprintf(urlDisplay, sizeof(urlDisplay), "vnet://%s", g_player.currentURL);
    }
    DrawScaledText(urlDisplay, urlX + 8, urlY + 8, 11, g_player.isConnecting ? COLOR_AMBER : COLOR_CYAN);
    
    // ---- PAGE CONTENT ----
    float gameX = cx + 8;
    float gameY = toolbarY + toolbarH + 4;
    float gameW = cw - 16;
    float gameH = ch - toolbarH - 8;
    
    // Page background
    DrawScaledRect(gameX, gameY, gameW, gameH, COLOR_PANEL);
    DrawScaledRectLines(gameX, gameY, gameW, gameH, COLOR_BORDER);
    
    // ============================================================
    // CONNECTION OVERLAY - Show inside browser when connecting
    // ============================================================
    if (g_player.isConnecting) {
        DrawBrowserConnectionOverlay(gameX, gameY, gameW, gameH);
    } else {
        // ---- PAGE TITLE ----
        const VNETPageData* site = GetSiteData(g_player.currentURL);
        char titleStr[128];
        if (site) {
            snprintf(titleStr, sizeof(titleStr), "// %s", site->title);
        } else {
            snprintf(titleStr, sizeof(titleStr), "// %s", g_player.currentURL);
        }
        DrawScaledText(titleStr, gameX + 12, gameY + 8, 13, COLOR_BLOOD);
        DrawScaledLine(gameX + 12, gameY + 28, gameX + gameW - 12, gameY + 28, COLOR_BORDER);
        
        // ---- PAGE CONTENT ----
        float contentX = gameX + 4;
        float contentY = gameY + 34;
        float contentW = gameW - 8;
        float contentH = gameH - 38;
        
        if (contentH > 20) {
            DrawMarkupPage(contentX, contentY, contentW, contentH);
        }
    }
    
    // ---- STATUS BAR ----
    float statusY = cy + ch - 22;
    DrawScaledRect(cx, statusY, cw, 22, Color{18, 20, 28, 220});
    DrawScaledLine(cx, statusY, cx + cw, statusY, COLOR_BORDER);
    
    char status[128];
    snprintf(status, sizeof(status), "PORT: %d | VCOIN: %.2f | TRACE: %d%% | ICE: %d/3", 
             g_player.port, g_player.vcoin, g_player.traceLevel, g_player.iceShields);
    DrawScaledText(status, cx + 12, statusY + 5, 9, COLOR_GHOST);
    
    // ---- BUTTON INTERACTIONS ----
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    if (clicked && !g_player.isConnecting) {
        if (homeHover) {
            TriggerRouteNavigation("vnet.dir");
        }
        else if (reloadHover) {
            RefreshPage();
            TriggerJitter(0.15f);
        }
        else if (backHover && strlen(g_player.prevURL) > 0) {
            TriggerRouteNavigation(g_player.prevURL);
        }
        else if (fwdHover) {
            PushCliLog("[BROWSER]: Forward navigation not implemented yet");
        }
    }
}

void Desktop::DrawTerminal(const AppWindow& win) {
    float cx = win.x + 4;
    float cy = win.y + m_windowTitleHeight + 4;
    float cw = win.w - 8;
    float ch = win.h - m_windowTitleHeight - 8;
    
    // ---- TERMINAL BACKGROUND ----
    DrawScaledRect(cx, cy, cw, ch, Color{8, 10, 14, 255});  // Deeper dark background
    
    // ---- TOP BAR WITH TABS ----
    float topBarH = 30.0f;
    DrawScaledRect(cx, cy, cw, topBarH, Color{16, 18, 24, 255});
    DrawScaledLine(cx, cy + topBarH, cx + cw, cy + topBarH, Color{30, 35, 45, 200});
    
    // Tab bar
    float tabX = cx + 12;
    float tabW = 120.0f;
    float tabH = 22.0f;
    float tabY = cy + 4;
    
    // Active tab
    DrawScaledRect(tabX, tabY, tabW, tabH, Color{22, 26, 34, 255});
    DrawScaledRectLines(tabX, tabY, tabW, tabH, Color{40, 220, 120, 150});
    DrawScaledText("● TERMINAL", tabX + 8, tabY + 4, 9, COLOR_TOXIC);
    
    // Inactive tab (darker)
    tabX += tabW + 4;
    DrawScaledRect(tabX, tabY, 100, tabH, Color{12, 14, 20, 255});
    DrawScaledRectLines(tabX, tabY, 100, tabH, Color{30, 35, 45, 100});
    DrawScaledText("○ SHELL", tabX + 8, tabY + 4, 9, COLOR_GHOST);
    
    // ---- STATUS INDICATORS (right side) ----
    float indicatorX = cx + cw - 12;
    DrawScaledText("●", indicatorX - 80, cy + 8, 8, COLOR_TOXIC);  // Green dot
    DrawScaledText("ONLINE", indicatorX - 65, cy + 6, 8, COLOR_GHOST);
    
    // Port indicator
    char portStr[32];
    snprintf(portStr, sizeof(portStr), "PORT %d", g_player.port);
    DrawScaledText(portStr, indicatorX - 20, cy + 6, 8, COLOR_CYAN);
    
    // ---- LOG AREA ----
    float logY = cy + topBarH + 4;
    float logH = ch - topBarH - 32 - 4;
    
    BeginScissorMode((int)SX(cx + 4), (int)SY(logY), 
                     (int)((cw - 8) * g_uiScale), (int)(logH * g_uiScale));
    
    int total = g_cliLogCount;
    int visibleLines = (int)(logH / 20.0f);
    int start = (total > visibleLines) ? total - visibleLines : 0;
    int scrollLines = (int)(g_player.cliScroll / 20.0f);
    start -= scrollLines;
    if (start < 0) start = 0;
    if (start > total) start = total;
    
    // Draw a subtle line number / timestamp for each log
    for (int i = start; i < total; i++) {
        int row = i - start;
        float y = logY + 4 + row * 20.0f;
        if (y > cy + ch - 30) break;
        
        const char* txt = g_cliLogs[i];
        
        // Determine color based on log type
        Color col = COLOR_GHOST;
        Color prefixCol = COLOR_GHOST;
        float prefixAlpha = 0.4f;
        
        if (strncmp(txt, "> ", 2) == 0) {
            col = COLOR_TOXIC;
            prefixCol = {40, 240, 100, 100};
        } else if (strstr(txt, "[ERROR]") || strstr(txt, "[ERR]")) {
            col = COLOR_BLOOD;
            prefixCol = {220, 20, 40, 150};
        } else if (strstr(txt, "[WARNING]") || strstr(txt, "[WARN]")) {
            col = COLOR_AMBER;
            prefixCol = {255, 150, 0, 150};
        } else if (strstr(txt, "[PAGE]")) {
            col = COLOR_CYAN;
            prefixCol = {0, 220, 240, 150};
        } else if (strstr(txt, "[SYS_INIT]")) {
            col = COLOR_TOXIC;
            prefixCol = {40, 240, 100, 150};
        } else if (strstr(txt, "[SCAN]")) {
            col = COLOR_AMBER;
            prefixCol = {255, 150, 0, 150};
        } else if (strstr(txt, "[MINER]")) {
            col = COLOR_TOXIC;
            prefixCol = {40, 240, 100, 150};
        } else if (strstr(txt, "[ICE]")) {
            col = COLOR_CYAN;
            prefixCol = {0, 220, 240, 150};
        }
        
        // Check for chat/whisper
        if (strstr(txt, "[CHAT]")) {
            col = COLOR_TOXIC;
            prefixCol = {40, 240, 100, 100};
        } else if (strstr(txt, "[WHISPER")) {
            col = COLOR_AMBER;
            prefixCol = {255, 150, 0, 150};
        } else if (strstr(txt, "[DOS") || strstr(txt, "[TRACE SPIKE]")) {
            col = COLOR_BLOOD;
            prefixCol = {220, 20, 40, 150};
        }
        
        // Render with subtle line number
        char lineNum[8];
        snprintf(lineNum, sizeof(lineNum), "%03d", i + 1);
        DrawScaledText(lineNum, cx + 8, y, 7, {80, 90, 110, 120});
        
        // Main log text with slight indent
        DrawScaledText(txt, cx + 40, y, 11, col);
        
        // Small accent bar on the left for error/warning
        if (strstr(txt, "[ERROR]") || strstr(txt, "[ERR]")) {
            DrawScaledRect(cx + 4, y + 2, 3, 14, COLOR_BLOOD);
        } else if (strstr(txt, "[WARNING]") || strstr(txt, "[WARN]")) {
            DrawScaledRect(cx + 4, y + 2, 3, 14, COLOR_AMBER);
        } else if (strncmp(txt, "> ", 2) == 0) {
            DrawScaledRect(cx + 4, y + 2, 3, 14, COLOR_TOXIC);
        }
    }
    
    EndScissorMode();
    
    // ---- INPUT AREA ----
    float inputY = cy + ch - 28;
    
    // Input area background
    DrawScaledRect(cx + 4, inputY, cw - 8, 24, Color{12, 15, 20, 220});
    DrawScaledLine(cx + 4, inputY, cx + cw - 4, inputY, Color{30, 35, 45, 180});
    
    // Input prompt with arrow
    bool focused = IsTerminalFocused();
    float t = (float)GetTime();
    bool cursorVisible = focused ? (fmodf(t, 0.8f) > 0.4f) : false;
    float fontSize = 12.0f;
    
    // Animated prompt arrow
    float arrowPulse = sinf(t * 3.0f) * 0.3f + 0.7f;
    Color arrowCol = {40, 240, 100, (unsigned char)(arrowPulse * 255)};
    DrawScaledText("➜", cx + 10, inputY + 5, 12, arrowCol);
    
    // Input text
    char prompt[300];
    snprintf(prompt, sizeof(prompt), " %s", g_player.inputBuffer);
    DrawScaledText(prompt, cx + 28, inputY + 5, fontSize, COLOR_GHOST);
    
    // Input cursor
    if (cursorVisible) {
        float textWidth = MeasureScaledTextWidth(prompt, fontSize);
        float cursorHeight = fontSize * g_fontScale;
        float cursorWidth = 6.0f;
        
        DrawScaledRect(
            cx + 28 + textWidth + 1.0f,
            inputY + 4,
            cursorWidth,
            cursorHeight,
            COLOR_TOXIC
        );
    }
    
    // ---- STATUS BAR (Bottom) ----
    float statusY = cy + ch - 4;
    DrawScaledLine(cx + 4, statusY, cx + cw - 4, statusY, Color{30, 35, 45, 100});
    
    // Status indicators
    char statusBuf[128];
    snprintf(statusBuf, sizeof(statusBuf), "VCOIN: %.2f  |  ICE: %d/3  |  TRACE: %d%%", 
             g_player.vcoin, g_player.iceShields, g_player.traceLevel);
    
    float statusW = MeasureScaledTextWidth(statusBuf, 8);
    DrawScaledText(statusBuf, cx + cw - statusW - 12, cy + ch - 12, 8, {80, 90, 110, 180});
    
    // Focus hint
    if (!focused) {
        DrawScaledText("[CLICK TO FOCUS]", cx + cw - 130, inputY + 5, 9, {255, 150, 0, 180});
    }
}

void Desktop::DrawProfile(const AppWindow& win) {
    float cx = win.x + 4;
    float cy = win.y + m_windowTitleHeight + 4;
    float cw = win.w - 8;
    float ch = win.h - m_windowTitleHeight - 8;
    
    // ---- BACKGROUND ----
    DrawScaledRect(cx, cy, cw, ch, Color{8, 10, 14, 255});
    DrawScaledRectLines(cx, cy, cw, ch, COLOR_BORDER);
    
    // ---- HEADER ----
    float headerH = 60.0f;
    DrawScaledRect(cx, cy, cw, headerH, Color{14, 18, 26, 255});
    DrawScaledLine(cx, cy + headerH, cx + cw, cy + headerH, Color{40, 220, 120, 80});
    
    // Avatar circle
    float avatarSize = 40.0f;
    float avatarX = cx + 20.0f;
    float avatarY = cy + 10.0f;
    DrawScaledRect(avatarX, avatarY, avatarSize, avatarSize, Color{20, 25, 35, 255});
    DrawScaledRectLines(avatarX, avatarY, avatarSize, avatarSize, Color{40, 220, 120, 150});
    DrawScaledText("👤", avatarX + 8, avatarY + 8, 22, COLOR_CYAN);
    
    // Handle and title
    DrawScaledText(g_player.handle, avatarX + avatarSize + 15, avatarY + 8, 14, COLOR_TOXIC);
    DrawScaledText("OPERATOR // VEKTRAOS v9.5", avatarX + avatarSize + 15, avatarY + 32, 10, COLOR_GHOST);
    
    // Port badge (right side)
    char portBadge[32];
    snprintf(portBadge, sizeof(portBadge), "PORT %d", g_player.port);
    float portW = MeasureScaledTextWidth(portBadge, 11);
    DrawScaledRect(cx + cw - portW - 30, cy + 12, portW + 20, 30, Color{20, 25, 35, 255});
    DrawScaledRectLines(cx + cw - portW - 30, cy + 12, portW + 20, 30, Color{0, 220, 240, 100});
    DrawScaledText(portBadge, cx + cw - portW - 20, cy + 22, 11, COLOR_CYAN);
    
    // ---- CONTENT AREA ----
    float contentY = cy + headerH + 10.0f;
    float contentH = ch - headerH - 10.0f;
    float leftCol = cx + 20.0f;
    float rightCol = cx + cw / 2.0f + 10.0f;
    float rowH = 36.0f;
    float rowY = contentY;
    
    // ---- LEFT COLUMN: STATS ----
    struct StatItem {
        const char* label;
        const char* value;
        Color color;
        float progress;
    };
    
    // Build stat items
    std::vector<StatItem> stats;
    
    char vcoinStr[32];
    snprintf(vcoinStr, sizeof(vcoinStr), "%.2f VCOIN", g_player.vcoin);
    stats.push_back({"VCOIN", vcoinStr, COLOR_TOXIC, g_player.vcoin / 50.0f});
    
    char traceStr[32];
    snprintf(traceStr, sizeof(traceStr), "%d%%", g_player.traceLevel);
    stats.push_back({"TRACE LEVEL", traceStr, 
                    g_player.traceLevel > 70 ? COLOR_BLOOD : COLOR_AMBER, 
                    g_player.traceLevel / 100.0f});
    
    char iceStr[32];
    snprintf(iceStr, sizeof(iceStr), "%d/3", g_player.iceShields);
    stats.push_back({"ICE SHIELDS", iceStr, COLOR_CYAN, g_player.iceShields / 3.0f});
    
    char heatStr[32];
    snprintf(heatStr, sizeof(heatStr), "%.0f°C", g_player.crtHeat);
    stats.push_back({"CRT HEAT", heatStr, 
                    g_player.crtHeat > 75.0f ? COLOR_BLOOD : COLOR_AMBER, 
                    (g_player.crtHeat - 35.0f) / 65.0f});
    
    char paranoiaStr[32];
    snprintf(paranoiaStr, sizeof(paranoiaStr), "%.0f%%", g_player.neuralParanoia);
    stats.push_back({"NEURAL PARANOIA", paranoiaStr, 
                    g_player.neuralParanoia > 60.0f ? COLOR_BLOOD : COLOR_AMBER, 
                    g_player.neuralParanoia / 100.0f});
    
    // Draw stats with progress bars
    for (int i = 0; i < (int)stats.size(); i++) {
        float y = rowY + i * (rowH + 6.0f);
        if (y > contentY + contentH - 30) break;
        
        const auto& stat = stats[i];
        
        // Stat label
        DrawScaledText(stat.label, leftCol, y + 2, 10, COLOR_GHOST);
        
        // Progress bar background
        float barX = leftCol + 95.0f;
        float barY = y + 4;
        float barW = 140.0f;
        float barH = 14.0f;
        
        DrawScaledRect(barX, barY, barW, barH, Color{12, 15, 20, 255});
        DrawScaledRectLines(barX, barY, barW, barH, Color{30, 35, 45, 150});
        
        // Progress fill
        float fillW = barW * stat.progress;
        if (fillW < 2.0f) fillW = 2.0f;
        if (fillW > barW) fillW = barW;
        DrawScaledRect(barX, barY, fillW, barH, stat.color);
        
        // Value text
        float valW = MeasureScaledTextWidth(stat.value, 10);
        DrawScaledText(stat.value, barX + barW - valW - 6.0f, barY + 2, 10, stat.color);
    }
    
    // ---- RIGHT COLUMN: SITES DISCOVERED ----
    float rightY = rowY;
    
    // Section header
    DrawScaledText("DISCOVERED SITES", rightCol, rightY, 11, COLOR_AMBER);
    DrawScaledLine(rightCol, rightY + 18, rightCol + 220, rightY + 18, Color{30, 35, 45, 150});
    rightY += 28.0f;
    
    // Site count
    char siteCountStr[64];
    snprintf(siteCountStr, sizeof(siteCountStr), "%d / 20 SITES", g_player.assignedCount);
    DrawScaledText(siteCountStr, rightCol, rightY, 10, COLOR_CYAN);
    rightY += 22.0f;
    
    // Site progress bar
    float siteBarW = 210.0f;
    float siteBarH = 10.0f;
    DrawScaledRect(rightCol, rightY, siteBarW, siteBarH, Color{12, 15, 20, 255});
    DrawScaledRectLines(rightCol, rightY, siteBarW, siteBarH, Color{30, 35, 45, 150});
    float siteFill = (g_player.assignedCount / 20.0f) * siteBarW;
    if (siteFill < 2.0f) siteFill = 2.0f;
    DrawScaledRect(rightCol, rightY, siteFill, siteBarH, COLOR_TOXIC);
    rightY += 20.0f;
    
    // List discovered sites (limit to 8 visible)
    int maxSites = (int)((contentH - (rightY - rowY)) / 20.0f);
    if (maxSites > 8) maxSites = 8;
    if (maxSites < 1) maxSites = 1;
    
    // Create a scroll offset if there are more sites than fit
    static int siteScrollOffset = 0;
    
    // Handle mouse wheel for site list scrolling (using pageScroll as a hack)
    // We'll use a separate static or member variable later
    
    int startIdx = 0;
    int endIdx = g_player.assignedCount;
    if (endIdx > maxSites) {
        // Show only maxSites sites, starting from the most recent (end)
        startIdx = endIdx - maxSites;
    }
    
    for (int i = startIdx; i < endIdx && i < g_player.assignedCount; i++) {
        float y = rightY + (i - startIdx) * 20.0f;
        if (y > contentY + contentH - 10) break;
        
        const char* site = g_player.assignedSites[i];
        
        // Check if it's a core node
        bool isCore = false;
        const char* coreNodes[] = {"market.vnet", "vault.vnet", "terminal.vnet", "crypto.vnet", "hellroom.vnet"};
        for (int c = 0; c < 5; c++) {
            if (strcmp(site, coreNodes[c]) == 0) {
                isCore = true;
                break;
            }
        }
        
        // Dot indicator
        Color dotColor = isCore ? COLOR_BLOOD : COLOR_CYAN;
        DrawScaledRect(rightCol, y + 6, 5, 5, dotColor);
        
        // Site name
        Color siteColor = isCore ? COLOR_AMBER : COLOR_GHOST;
        DrawScaledText(site, rightCol + 14, y + 2, 10, siteColor);
    }
    
    // ---- DIVIDER ----
    float dividerX = cx + cw / 2.0f;
    DrawScaledLine(dividerX, contentY, dividerX, cy + ch - 10, Color{30, 35, 45, 80});
    
    // ---- STATUS BADGES ----
    float badgeY = cy + ch - 36;
    
    // Status indicator
    DrawScaledRect(cx + 20, badgeY, 12, 12, COLOR_TOXIC);
    DrawScaledText("ONLINE", cx + 38, badgeY, 10, COLOR_TOXIC);
    
    // Runtime
    char runtimeStr[64];
    int hours = (int)(g_player.runTime / 3600.0f);
    int minutes = (int)((g_player.runTime - hours * 3600) / 60.0f);
    int seconds = (int)(g_player.runTime - hours * 3600 - minutes * 60);
    snprintf(runtimeStr, sizeof(runtimeStr), "UPTIME: %02d:%02d:%02d", hours, minutes, seconds);
    DrawScaledText(runtimeStr, cx + 110, badgeY, 10, COLOR_GHOST);
    
    // Version
    DrawScaledText("VEKTRAOS v9.5", cx + cw - 110, badgeY, 10, Color{80, 90, 110, 180});
}

void Desktop::DrawSettings(const AppWindow& win) {
    float cx = win.x + 4;
    float cy = win.y + m_windowTitleHeight + 4;
    float cw = win.w - 8;
    float ch = win.h - m_windowTitleHeight - 8;
    
    DrawScaledRect(cx, cy, cw, ch, COLOR_BLACK);
    DrawScaledRectLines(cx, cy, cw, ch, COLOR_BORDER);
    
    DrawScaledText("SETTINGS", cx + 20, cy + 20, 16, COLOR_BLOOD);
    DrawScaledLine(cx + 20, cy + 45, cx + cw - 20, cy + 45, COLOR_BORDER);
    
    DrawScaledText("🔊 Audio", cx + 20, cy + 70, 12, COLOR_CYAN);
    DrawScaledText("🎨 Theme", cx + 20, cy + 100, 12, COLOR_CYAN);
    DrawScaledText("🖥 Display", cx + 20, cy + 130, 12, COLOR_CYAN);
    DrawScaledText("🔒 Security", cx + 20, cy + 160, 12, COLOR_CYAN);
    DrawScaledText("📦 System", cx + 20, cy + 190, 12, COLOR_CYAN);
}

void Desktop::DrawFeed(const AppWindow& win) {
    float cx = win.x + 4;
    float cy = win.y + m_windowTitleHeight + 4;
    float cw = win.w - 8;
    float ch = win.h - m_windowTitleHeight - 8;
    
    DrawScaledRect(cx, cy, cw, ch, COLOR_BLACK);
    DrawScaledRectLines(cx, cy, cw, ch, COLOR_BORDER);
    
    DrawScaledText("SYSTEM FEED", cx + 20, cy + 20, 14, COLOR_AMBER);
    DrawScaledLine(cx + 20, cy + 45, cx + cw - 20, cy + 45, COLOR_BORDER);
    
    int start = g_feedLogCount - 15;
    if (start < 0) start = 0;
    
    for (int i = start; i < g_feedLogCount; i++) {
        float y = cy + 60 + (i - start) * 20.0f;
        if (y > cy + ch - 20) break;
        
        const char* txt = g_feedLogs[i];
        Color col = COLOR_GHOST;
        if (strstr(txt, "[CHAT]")) col = COLOR_TOXIC;
        if (strstr(txt, "[WHISPER")) col = COLOR_AMBER;
        if (strstr(txt, "[DOS")) col = COLOR_BLOOD;
        if (strstr(txt, "[TRACE")) col = COLOR_AMBER;
        
        DrawScaledText(txt, cx + 20, y, 10, col);
    }
}

// ============================================================
// GLOBAL ACCESS
// ============================================================

Desktop& GetDesktop() {
    return Desktop::Get();
}