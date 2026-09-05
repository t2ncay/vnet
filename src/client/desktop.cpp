#include "desktop.h"
#include "render.h"
#include "vnet.h"
#include "game.h"
#include "vnet_client.h"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <cmath>

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

// ============================================================
// DESKTOP IMPLEMENTATION
// ============================================================

void Desktop::Init() {
    m_active = true;
    m_windows.clear();
    m_windows.reserve(8);
    m_appGridVisible = false;
    m_currentWorkspace = 0;
    
    // Define apps with proper icons
    m_apps.clear();
    m_apps.push_back({"Browser", "🌐", "Browse the VNET", LaunchBrowser});
    m_apps.push_back({"Terminal", "💻", "Command line", LaunchTerminal});
    m_apps.push_back({"Profile", "👤", "User info", LaunchProfile});
    m_apps.push_back({"Settings", "⚙", "Preferences", LaunchSettings});
    m_apps.push_back({"Feed", "📊", "System feed", LaunchFeed});
    
    // Workspaces
    m_workspaces.clear();
    m_workspaces.emplace_back("Main");
    m_workspaces.emplace_back("Work");
    m_workspaces.emplace_back("Chat");
    
    // Start with browser window
    OpenApp(AppType::Browser, "VNET Browser");
}

void Desktop::Shutdown() {
    m_windows.clear();
    m_apps.clear();
    m_workspaces.clear();
}

// ============================================================
// WINDOW MANAGEMENT
// ============================================================

int Desktop::OpenApp(AppType type, const char* title) {
    if (m_windows.size() >= 8) return -1;
    
    // Check if already open (only one instance per type)
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
    
    // Create new window
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
    
    // Center if too many windows
    if (win.x > 200) win.x = 80;
    if (win.y > 200) win.y = 80;
    
    // Clamp
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
    
    // Remove from workspace
    for (auto& ws : m_workspaces) {
        for (int i = 0; i < (int)ws.windows.size(); i++) {
            if (ws.windows[i] == idx) {
                ws.windows.erase(ws.windows.begin() + i);
                break;
            }
        }
    }
    // Shift indices in workspace after removal
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
    
    // Bring to front
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
    // Focus first window in workspace
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
    
    // Remove from current workspace
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
// UPDATE - Handle all input here
// ============================================================

void Desktop::Update(float dt) {
    (void)dt;
    if (!m_active) return;
    
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    bool dragging = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    
    // ============================================================
    // DESKTOP ICON CLICKS
    // ============================================================
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
        
        if (clicked && RefRectHover(x, y, iconSize, iconSize, refMouse)) {
            m_apps[i].onClick();
            break;
        }
    }
    
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
                break;
            }
        }
        if (clicked && !RefRectHover(startXg - 20, startYg - 20, 640, 500, refMouse)) {
            m_appGridVisible = false;
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
    
    // ============================================================
    // WINDOW MANAGEMENT - Only process window dragging/closing
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
        
        // Only handle title bar interactions here
        if (RefRectHover(win.x, titleY, win.w, titleH, refMouse)) {
            if (clicked) {
                FocusWindow(i);
                win.dragging = true;
                win.dragX = (int)(refMouse.x - win.x);
                win.dragY = (int)(refMouse.y - titleY);
            }
            
            if (clicked && RefRectHover(win.x + win.w - 25, win.y + 5, 20, 20, refMouse)) {
                CloseWindow(i);
                continue;
            }
            if (clicked && RefRectHover(win.x + win.w - 50, win.y + 5, 20, 20, refMouse)) {
                MinimizeWindow(i);
                continue;
            }
            if (clicked && RefRectHover(win.x + win.w - 75, win.y + 5, 20, 20, refMouse)) {
                MaximizeWindow(i);
                continue;
            }
        }
        
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

    // Click on terminal content to focus it
    for (int i = (int)m_windows.size() - 1; i >= 0; i--) {
        auto& win = m_windows[i];
        if (win.type != AppType::Terminal) continue;
        if (win.minimized) continue;
        if (!win.focused) continue; // Only check focused window
        
        // Check if click is in the terminal content area (not title bar)
        float contentX = win.x + 4;
        float contentY = win.y + m_windowTitleHeight + 4 + 28; // Header offset
        float contentW = win.w - 8;
        float contentH = win.h - m_windowTitleHeight - 8 - 60; // Input area offset
        
        if (clicked && RefRectHover(contentX, contentY, contentW, contentH, refMouse)) {
            // Focus the terminal (already focused, but this ensures it)
            FocusWindow(i);
            // Set focus flag for input
        }
    }
}

// ============================================================
// DRAW DESKTOP ICONS
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
        
        // Icon background
        DrawScaledRect(x, y, iconSize, iconSize, 
                       hover ? Color{40, 45, 65, 200} : Color{20, 22, 35, 150});
        DrawScaledRectLines(x, y, iconSize, iconSize, 
                            hover ? COLOR_BLOOD : Color{30, 35, 50, 100});
        
        // Icon emoji
        DrawScaledText(m_apps[i].icon.c_str(), 
                       x + (iconSize - 24) / 2, y + 8, 28, COLOR_CYAN);
        
        // Icon label
        DrawScaledText(m_apps[i].name.c_str(), 
                       x + 10, y + iconSize - 18, 10, hover ? COLOR_TOXIC : COLOR_GHOST);
    }
}

// ============================================================
// DRAW
// ============================================================

void Desktop::Draw() {
    if (!m_active) return;
    
    // Desktop background
    DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, Color{18, 20, 28, 255});
    
    // Subtle grid pattern
    for (int x = 0; x < REF_WIDTH; x += 40) {
        DrawScaledLine(x, 0, x, REF_HEIGHT, Color{25, 28, 38, 60});
    }
    for (int y = 0; y < REF_HEIGHT; y += 40) {
        DrawScaledLine(0, y, REF_WIDTH, y, Color{25, 28, 38, 60});
    }
    
    // Draw desktop icons
    DrawDesktopIcons();
    
    // Draw windows (only current workspace)
    for (int i = 0; i < (int)m_windows.size(); i++) {
        bool inWorkspace = false;
        for (int w : m_workspaces[m_currentWorkspace].windows) {
            if (w == i) { inWorkspace = true; break; }
        }
        if (!inWorkspace) continue;
        DrawWindow(i);
    }
    
    // Top bar
    DrawTopBar();
    
    // App grid overlay
    if (m_appGridVisible) DrawAppGrid();
}

// ============================================================
// TOP BAR
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
    DrawScaledText(vcoin, REF_WIDTH - 180, barY + 14, 11, COLOR_TOXIC);
}

// ============================================================
// WORKSPACE INDICATOR
// ============================================================

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
            dots[j] = '.';
        }
        dots[count] = '\0';
        DrawScaledText(dots, x + 8, barY + 16, 8, active ? COLOR_TOXIC : COLOR_GHOST);
    }
}

// ============================================================
// CLOCK
// ============================================================

void Desktop::DrawClock() {
    time_t now = time(nullptr);
    struct tm* local = localtime(&now);
    
    char timeStr[16];
    strftime(timeStr, sizeof(timeStr), "%H:%M", local);
    
    float barY = 0;
    DrawScaledText(timeStr, REF_WIDTH - 110, barY + 14, 13, COLOR_CYAN);
}

// ============================================================
// APP GRID
// ============================================================

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
        
        DrawScaledText(m_apps[i].icon.c_str(), x + (iconSize - 24) / 2, y + 10, 24, COLOR_CYAN);
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
    
    DrawScaledRect(win.x + win.w - 25, win.y + 5, 20, 20, COLOR_BLOOD);
    DrawScaledRectLines(win.x + win.w - 25, win.y + 5, 20, 20, COLOR_BORDER);
    DrawScaledText("✕", win.x + win.w - 20, win.y + 7, 14, COLOR_BLACK);
    
    DrawScaledRect(win.x + win.w - 50, win.y + 5, 20, 20, Color{35, 40, 55, 255});
    DrawScaledRectLines(win.x + win.w - 50, win.y + 5, 20, 20, COLOR_BORDER);
    DrawScaledText("─", win.x + win.w - 45, win.y + 6, 14, COLOR_GHOST);
    
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

void Desktop::DrawBrowser(const AppWindow& win) {
    float cx = win.x + 4;
    float cy = win.y + m_windowTitleHeight + 4;
    float cw = win.w - 8;
    float ch = win.h - m_windowTitleHeight - 8;
    
    // Draw the ENTIRE game UI inside the browser window
    // This is the same rendering as the fullscreen mode
    
    // Save current UI scale and offset to render inside the window
    float savedScale = g_uiScale;
    float savedOffsetX = g_offsetX;
    float savedOffsetY = g_offsetY;
    
    // Temporarily set offset to render within the window
    float windowScale = (cw / REF_WIDTH) * savedScale;
    if (windowScale < 0.5f) windowScale = 0.5f;
    if (windowScale > 1.5f) windowScale = 1.5f;
    
    // Calculate offset to center the game in the window
    float contentX = cx;
    float contentY = cy + 40; // URL bar offset
    float contentW = cw;
    float contentH = ch - 40;
    
    // Draw the game content inside the window
    // For simplicity, we'll draw a scaled version of the browser content
    
    // Dark background for browser
    DrawScaledRect(cx, cy, cw, ch, COLOR_BLACK);
    
    // URL bar
    DrawScaledRect(cx + 10, cy + 10, cw - 20, 30, COLOR_URLBAR);
    DrawScaledRectLines(cx + 10, cy + 10, cw - 20, 30, COLOR_BORDER);
    DrawScaledText("vnet://vnet.dir", cx + 20, cy + 18, 11, COLOR_CYAN);
    
    // Draw the game content as a nested UI
    float gameX = cx + 10;
    float gameY = cy + 50;
    float gameW = cw - 20;
    float gameH = ch - 60;
    
    DrawScaledRect(gameX, gameY, gameW, gameH, COLOR_PANEL);
    DrawScaledRectLines(gameX, gameY, gameW, gameH, COLOR_BORDER);
    
    DrawMarkupPage(gameX, gameY, gameW, gameH);
    
    // Status bar at bottom
    char status[64];
    snprintf(status, sizeof(status), "PORT: %d | VCOIN: %.2f", g_player.port, g_player.vcoin);
    DrawScaledText(status, gameX + 20, gameY + gameH - 25, 9, COLOR_GHOST);
}

void Desktop::DrawTerminal(const AppWindow& win) {
    float cx = win.x + 4;
    float cy = win.y + m_windowTitleHeight + 4;
    float cw = win.w - 8;
    float ch = win.h - m_windowTitleHeight - 8;
    
    // Background
    DrawScaledRect(cx, cy, cw, ch, COLOR_CLI_BG);
    DrawScaledRectLines(cx, cy, cw, ch, COLOR_BORDER);
    
    // Terminal header
    DrawScaledRect(cx, cy, cw, 22, COLOR_PANEL);
    DrawScaledLine(cx, cy + 22, cx + cw, cy + 22, COLOR_BORDER);
    DrawScaledText("TERMINAL v9.5 // SYSTEM CLI", cx + 10, cy + 5, 10, COLOR_BLOOD);
    
    // CLI Logs area
    float logY = cy + 28;
    float logH = ch - 60;
    
    BeginScissorMode((int)SX(cx), (int)SY(cy + 28), 
                     (int)(cw * g_uiScale), (int)((ch - 60) * g_uiScale));
    
    int total = g_cliLogCount;
    int visibleLines = (int)(logH / 20.0f);
    int start = (total > visibleLines) ? total - visibleLines : 0;
    int scrollLines = (int)(g_player.cliScroll / 20.0f);
    start -= scrollLines;
    if (start < 0) start = 0;
    if (start > total) start = total;
    
    for (int i = start; i < total; i++) {
        int row = i - start;
        float y = logY + row * 20.0f;
        if (y > cy + ch - 30) break;
        
        Color col = COLOR_CYAN;
        const char* txt = g_cliLogs[i];
        if (strncmp(txt, "> ", 2) == 0) col = COLOR_TOXIC;
        if (strstr(txt, "[ERROR]") || strstr(txt, "[ERR]")) col = COLOR_BLOOD;
        if (strstr(txt, "[WARNING]")) col = COLOR_AMBER;
        if (strstr(txt, "[PAGE]")) col = COLOR_CYAN;
        
        DrawScaledText(txt, cx + 10, y, 11, col);
    }
    
    EndScissorMode();
    
    // Input line at bottom
    float inputY = cy + ch - 28;
    DrawScaledLine(cx, inputY, cx + cw, inputY, COLOR_BORDER);
    
    // Check if terminal is focused for cursor blink
    bool focused = IsTerminalFocused();
    float t = (float)GetTime();
    bool cursorVisible = focused ? (fmodf(t, 0.8f) > 0.4f) : false;
    
    // Show prompt with cursor
    char prompt[300];
    snprintf(prompt, sizeof(prompt), "CMD> %s", g_player.inputBuffer);
    DrawScaledText(prompt, cx + 10, inputY + 6, 12, COLOR_TOXIC);
    
    if (cursorVisible) {
        float textWidth = MeasureScaledTextWidth(prompt, 12);
        DrawScaledRect(cx + 10 + textWidth + 2.0f, inputY + 6, 7, 14, COLOR_TOXIC);
    }
    
    // Focus indicator
    if (!focused) {
        DrawScaledText("[CLICK TO FOCUS]", cx + cw - 130, inputY + 6, 9, COLOR_AMBER);
    }
}

void Desktop::DrawProfile(const AppWindow& win) {
    float cx = win.x + 4;
    float cy = win.y + m_windowTitleHeight + 4;
    float cw = win.w - 8;
    float ch = win.h - m_windowTitleHeight - 8;
    
    DrawScaledRect(cx, cy, cw, ch, COLOR_BLACK);
    DrawScaledRectLines(cx, cy, cw, ch, COLOR_BORDER);
    
    DrawScaledText("USER PROFILE", cx + 20, cy + 20, 16, COLOR_BLOOD);
    DrawScaledLine(cx + 20, cy + 45, cx + cw - 20, cy + 45, COLOR_BORDER);
    
    char info[256];
    snprintf(info, sizeof(info), "HANDLE: %s", g_player.handle);
    DrawScaledText(info, cx + 20, cy + 70, 12, COLOR_CYAN);
    snprintf(info, sizeof(info), "PORT: %d", g_player.port);
    DrawScaledText(info, cx + 20, cy + 100, 12, COLOR_CYAN);
    snprintf(info, sizeof(info), "VCOIN: %.2f", g_player.vcoin);
    DrawScaledText(info, cx + 20, cy + 130, 12, COLOR_TOXIC);
    snprintf(info, sizeof(info), "TRACE: %d%%", g_player.traceLevel);
    DrawScaledText(info, cx + 20, cy + 160, 12, g_player.traceLevel > 70 ? COLOR_BLOOD : COLOR_AMBER);
    snprintf(info, sizeof(info), "ICE: %d/3", g_player.iceShields);
    DrawScaledText(info, cx + 20, cy + 190, 12, COLOR_TOXIC);
    snprintf(info, sizeof(info), "SITES: %d/20", g_player.assignedCount);
    DrawScaledText(info, cx + 20, cy + 220, 12, COLOR_CYAN);
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