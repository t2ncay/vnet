#include "desktop.h"
#include "render.h"
#include "vnet.h"
#include "game.h"
#include "vnet_client.h"
#include "vnet_sites.h"
#include "wallpaper.h"
#include "vnet_protocol.h" 

#include <cstdio>
#include <cstring>
#include <ctime>
#include <cmath>
#include <algorithm>

// ============================================================
// FORWARD DECLARATIONS - Settings Category Drawers
// ============================================================
void DrawSettingsTheme(float x, float y, float w, float h);
void DrawSettingsAudio(float x, float y, float w, float h);
void DrawSettingsDisplay(float x, float y, float w, float h);
void DrawSettingsSecurity(float x, float y, float w, float h);
void DrawSettingsSystem(float x, float y, float w, float h);

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

static void LaunchHellroom() {
    GetDesktop().OpenApp(AppType::Hellroom, "Hellroom IRC");
}

static void LaunchVDEC() {
    GetDesktop().OpenApp(AppType::VDEC, "VDEC - VNET Decryption Toolkit");
}

// icon functions

void Desktop::LoadIcons() {
    auto loadIcon = [this](const std::string& key, const std::string& path) {
        Texture2D tex = LoadTexture(path.c_str());
        if (tex.id != 0) {
            m_iconTextures[key] = tex;
            printf("[ICON] Loaded: %s\n", path.c_str());
        } else {
            printf("[ICON] Failed to load: %s\n", path.c_str());
        }
    };
    
    loadIcon("browser", "assets/icons/browser.png");
    loadIcon("terminal", "assets/icons/terminal.png");
    loadIcon("profile", "assets/icons/profile.png");
    loadIcon("settings", "assets/icons/settings.png");
    loadIcon("feed", "assets/icons/feed.png");
    loadIcon("folder", "assets/icons/folder.png");
    loadIcon("about", "assets/icons/about.png");
    loadIcon("hellroom", "assets/icons/hellroom.png");
    loadIcon("vdec", "assets/icons/vdec.png");
    
    // Add music player icons
    loadIcon("play", "assets/icons/play.png");
    loadIcon("pause", "assets/icons/pause.png");
    loadIcon("next", "assets/icons/next.png");
    loadIcon("prev", "assets/icons/prev.png");
    loadIcon("shuffle", "assets/icons/shuffle.png");
    loadIcon("repeat", "assets/icons/repeat.png");
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
    m_apps.push_back({"Hellroom", "hellroom", "IRC Chatroom", LaunchHellroom});
    m_apps.push_back({"VDEC", "vdec", "Decryption Toolkit", LaunchVDEC});
    
    m_workspaces.clear();
    m_workspaces.emplace_back("Main");
    m_workspaces.emplace_back("Work");
    m_workspaces.emplace_back("Chat");

    memset(&m_hellroom, 0, sizeof(m_hellroom));
    strcpy(m_hellroom.currentNick, g_player.handle);
    strcpy(m_hellroom.nickBuffer, g_player.handle);
    m_hellroom.inputFocused = true;
    
    PushHellroomMessage("[SERVER] Welcome to Hellroom IRC!");
    PushHellroomMessage("[SERVER] Type /nick <new_nick> to change your name.");
    PushHellroomMessage("[SERVER] Type /help for available commands.");
    PushHellroomMessage("[SERVER] Connected to vnet://hellroom.vnet");

    // Init VDEC state
    memset(&m_vdec, 0, sizeof(m_vdec));
    m_vdec.selectedTab = 0;
    m_vdec.bitShiftOffset = 0;
    m_vdec.minigameActive = false;
    
    for (int i = 0; i < 8; i++) {
        if (strlen(g_vnet.masterKeys[i]) > 0) {
            strcpy(m_vdec.keys[i], g_vnet.masterKeys[i]);
            m_vdec.keysFound[i] = true;
            m_vdec.keyCount++;
        }
    }
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
    win.w = 1000;  // Was 850
    win.h = 720;   // Was 600
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
        
        if (RefRectHover(win.x, titleY, win.w, titleH, refMouse)) {
            if (clicked && RefRectHover(win.x + win.w - 25, win.y + 5, 20, 20, refMouse)) {
                CloseWindow(i);
                return;
            }
            if (clicked && RefRectHover(win.x + win.w - 50, win.y + 5, 20, 20, refMouse)) {
                MinimizeWindow(i);
                return;
            }
            if (clicked && RefRectHover(win.x + win.w - 75, win.y + 5, 20, 20, refMouse)) {
                MaximizeWindow(i);
                return;
            }
            
            if (clicked) {
                FocusWindow(i);
                win.dragging = true;
                win.dragX = (int)(refMouse.x - win.x);
                win.dragY = (int)(refMouse.y - titleY);
                return;
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
    // DESKTOP ICON CLICKS
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
    // MOUSE WHEEL SCROLLING - WITH FEED SUPPORT
    // ============================================================
    bool isTerminalFocused = IsTerminalFocused();
    float wheel = GetMouseWheelMove();
    
    if (wheel != 0.0f) {
        // ---- CHECK IF MOUSE IS OVER THE FEED WINDOW ----
        bool overFeed = false;
        float winCh = 0.0f;
        
        for (auto& win : m_windows) {
            if (win.type == AppType::Feed && !win.minimized) {
                float cx = win.x + 4;
                float cy = win.y + m_windowTitleHeight + 4;
                float cw = win.w - 8;
                float ch = win.h - m_windowTitleHeight - 8;
                // Check if mouse is over the log panel area (left panel)
                if (RefRectHover(cx + 6, cy + 48, cw * 0.70f - 4, ch - 60, refMouse)) {
                    overFeed = true;
                    winCh = ch;
                    break;
                }
            }
        }
        
        if (overFeed) {
            // Scroll the feed
            g_player.feedScroll += wheel * 20.0f;
            if (g_player.feedScroll < 0.0f) g_player.feedScroll = 0.0f;
            
            // Calculate max scroll
            int total = g_feedLogCount;
            int visibleLines = (int)((winCh - 60 - 16) / 20.0f);
            if (visibleLines < 1) visibleLines = 1;
            int maxScroll = total - visibleLines;
            if (maxScroll < 0) maxScroll = 0;
            float maxScrollPx = maxScroll * 20.0f;
            if (g_player.feedScroll > maxScrollPx) g_player.feedScroll = maxScrollPx;
        }
        // ---- TERMINAL SCROLLING ----
        else if (g_player.cliOpen || isTerminalFocused) {
            g_player.cliScroll -= wheel * 20.0f;
            if (g_player.cliScroll < 0.0f) g_player.cliScroll = 0.0f;
        } 
        // ---- BROWSER SCROLLING ----
        else {
            g_player.pageScroll -= wheel * 28.0f;
            if (g_player.pageScroll < 0.0f) g_player.pageScroll = 0.0f;
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
    DrawVektraWallpaper(REF_WIDTH, REF_HEIGHT);
    
    for (int x = 0; x < REF_WIDTH; x += 40) {
        DrawScaledLine(x, 0, x, REF_HEIGHT, Color{25, 28, 38, 30});  // Reduced alpha
    }
    for (int y = 0; y < REF_HEIGHT; y += 40) {
        DrawScaledLine(0, y, REF_WIDTH, y, Color{25, 28, 38, 30});  // Reduced alpha
    }
    
    DrawDesktopIcons();

    if (m_musicWidgetVisible) {
        m_musicWidgetX = REF_WIDTH - m_musicWidgetW - 20.0f;
        m_musicWidgetY = REF_HEIGHT - m_musicWidgetH - 60.0f;
        
        GetMusicPlayer().Draw(m_musicWidgetX, m_musicWidgetY, 
                              m_musicWidgetW, m_musicWidgetH);
    }
    
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
    
    // DEPRECATED DrawTopBar();
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
    DrawScaledText(vcoin, REF_WIDTH - 220, barY + 16, 11, COLOR_TOXIC);
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
            dots[j] = '*';
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
        case AppType::Hellroom: DrawHellroom(win); break;
        case AppType::VDEC:     DrawVDEC(win); break;
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

void Desktop::DrawFeed(const AppWindow& win) {
    float cx = win.x + 4;
    float cy = win.y + m_windowTitleHeight + 4;
    float cw = win.w - 8;
    float ch = win.h - m_windowTitleHeight - 8;
    
    float t = (float)GetTime();
    float pulse = sinf(t * 3.0f) * 0.5f + 0.5f;
    
    // ---- BACKGROUND ----
    DrawScaledRect(cx, cy, cw, ch, Color{6, 8, 14, 255});
    DrawScaledRectLines(cx, cy, cw, ch, COLOR_BORDER);
    
    // ---- HEADER WITH LIVE INDICATOR ----
    float headerH = 48.0f;
    DrawScaledRect(cx, cy, cw, headerH, Color{14, 18, 28, 255});
    DrawScaledLine(cx, cy + headerH, cx + cw, cy + headerH, Color{30, 35, 50, 150});
    
    // Title
    DrawScaledText("◈ SYSTEM FEED", cx + 16, cy + 14, 14, COLOR_AMBER);
    
    // LIVE indicator with pulse
    float livePulse = sinf(t * 4.0f) * 0.3f + 0.7f;
    Color liveCol = {40, 240, 100, (unsigned char)(livePulse * 200 + 55)};
    DrawScaledRect(cx + cw - 70, cy + 14, 8, 8, liveCol);
    DrawScaledText("LIVE", cx + cw - 56, cy + 13, 10, COLOR_TOXIC);
    
    // Feed counter
    char counterStr[32];
    snprintf(counterStr, sizeof(counterStr), "MSG: %d", g_feedLogCount);
    DrawScaledText(counterStr, cx + cw - 130, cy + 13, 10, COLOR_GHOST);
    
    // ---- CONTENT LAYOUT ----
    float contentY = cy + headerH + 6.0f;
    float contentH = ch - headerH - 10.0f;
    float leftPanelW = cw * 0.70f;
    float rightPanelW = cw - leftPanelW - 12.0f;
    
    // ---- LEFT PANEL: FEED LOGS ----
    float logX = cx + 6;
    float logY = contentY;
    float logW = leftPanelW - 4;
    float logH = contentH - 4;
    
    // Panel background with subtle scanlines
    DrawScaledRect(logX, logY, logW, logH, Color{8, 10, 18, 220});
    DrawScaledRectLines(logX, logY, logW, logH, Color{30, 35, 50, 100});
    
    // Scanline overlay on log panel
    for (int i = 0; i < (int)logH; i += 4) {
        float scanY = logY + i + fmodf(t * 30.0f, 4.0f);
        DrawScaledRect(logX, scanY, logW, 1, {0, 0, 0, 4});
    }
    
    // ---- FEED LOGS WITH SCROLL ----
    BeginScissorMode((int)SX(logX + 4), (int)SY(logY + 4), 
                     (int)((logW - 8) * g_uiScale), (int)((logH - 8) * g_uiScale));
    
    int total = g_feedLogCount;
    int visibleLines = (int)((logH - 16) / 20.0f);
    int startIdx = total - visibleLines;
    if (startIdx < 0) startIdx = 0;
    
    // Reverse scroll: positive scroll moves up
    int scrollLines = (int)(g_player.feedScroll / 20.0f);
    startIdx += scrollLines;
    if (startIdx < 0) startIdx = 0;
    if (startIdx > total) startIdx = total;
    
    for (int i = startIdx; i < total; i++) {
        int row = i - startIdx;
        float y = logY + 8 + row * 20.0f;
        if (y > logY + logH - 12) break;
        
        const char* txt = g_feedLogs[i];
        
        // Truncate long messages
        char truncated[192];
        strncpy(truncated, txt, 120);
        truncated[120] = '\0';
        
        // Determine color based on message type
        Color col = COLOR_GHOST;
        Color barCol = {0, 0, 0, 0};
        bool hasBar = false;
        
        if (strstr(txt, "[CHAT]")) {
            col = COLOR_TOXIC;
            barCol = {40, 240, 100, 80};
            hasBar = true;
        } else if (strstr(txt, "[WHISPER FROM")) {
            col = COLOR_AMBER;
            barCol = {255, 150, 0, 80};
            hasBar = true;
        } else if (strstr(txt, "[WHISPER TO")) {
            col = COLOR_CYAN;
            barCol = {0, 220, 240, 80};
            hasBar = true;
        } else if (strstr(txt, "[DOS ATTACK]") || strstr(txt, "[TRACE SPIKE]")) {
            col = COLOR_BLOOD;
            barCol = {220, 20, 40, 100};
            hasBar = true;
        } else if (strstr(txt, "[OVERLOAD]")) {
            col = COLOR_BLOOD;
            barCol = {220, 20, 40, 120};
            hasBar = true;
        } else if (strstr(txt, "[GRID BLACKOUT]") || strstr(txt, "[SYSTEM OVERRIDE]")) {
            col = COLOR_AMBER;
            barCol = {255, 150, 0, 150};
            hasBar = true;
        } else if (strstr(txt, "[FEDERAL E-RAID]")) {
            col = COLOR_BLOOD;
            barCol = {255, 0, 50, 150};
            hasBar = true;
        } else if (strstr(txt, "[MINER]") || strstr(txt, "[WHALE ALERT]")) {
            col = COLOR_TOXIC;
            barCol = {40, 240, 100, 60};
            hasBar = true;
        } else if (strstr(txt, "[FEED_INIT]") || strstr(txt, "[FEED]")) {
            col = COLOR_CYAN;
            barCol = {0, 220, 240, 40};
            hasBar = true;
        }
        
        // Accent bar on the left
        if (hasBar) {
            DrawScaledRect(logX + 4, y - 2, 3, 16, barCol);
        }
        
        // Message text
        float textX = hasBar ? logX + 12 : logX + 8;
        DrawScaledText(truncated, textX, y, 10, col);
        
        // Time stamp (fake)
        char timeStamp[12];
        int hours = (i * 7 + 13) % 24;
        int mins = (i * 13 + 42) % 60;
        snprintf(timeStamp, sizeof(timeStamp), "%02d:%02d", hours, mins);
        float timeW = MeasureScaledTextWidth(timeStamp, 8);
        DrawScaledText(timeStamp, logX + logW - timeW - 8, y + 1, 8, {80, 90, 110, 120});
    }
    
    EndScissorMode();
    
    // ---- SCROLL INDICATOR ----
    if (total > visibleLines) {
        float scrollRatio = (float)startIdx / (float)(total - visibleLines);
        float indicatorY = logY + 8 + scrollRatio * (logH - 24);
        DrawScaledRect(logX + logW - 6, indicatorY, 3, 16, {80, 90, 110, 150});
    }
    
    // ---- RIGHT PANEL: THREAT RADAR & STATS ----
    float rightX = cx + leftPanelW + 8;
    float rightY = contentY;
    float rightW = rightPanelW - 4;
    float rightH = contentH - 4;
    
    // Panel background
    DrawScaledRect(rightX, rightY, rightW, rightH, Color{8, 10, 18, 220});
    DrawScaledRectLines(rightX, rightY, rightW, rightH, Color{30, 35, 50, 100});
    
    float panelY = rightY + 8;
    float panelX = rightX + 8;
    float panelW = rightW - 16;
    
    // ---- SECTION: THREAT RADAR ----
    DrawScaledText("⚡ THREAT RADAR", panelX, panelY, 11, COLOR_BLOOD);
    DrawScaledLine(panelX, panelY + 16, panelX + panelW, panelY + 16, Color{30, 35, 50, 100});
    panelY += 24;
    
    // Trace level gauge
    DrawScaledText("TRACE", panelX, panelY + 2, 9, COLOR_CYAN);
    float traceBarX = panelX + 50;
    float traceBarY = panelY + 2;
    float traceBarW = panelW - 54;
    float traceBarH = 12.0f;
    
    DrawScaledRect(traceBarX, traceBarY, traceBarW, traceBarH, Color{12, 15, 20, 255});
    DrawScaledRectLines(traceBarX, traceBarY, traceBarW, traceBarH, Color{30, 35, 50, 100});
    
    float traceFill = (g_player.traceLevel / 100.0f) * traceBarW;
    if (traceFill < 2.0f) traceFill = 2.0f;
    Color traceCol = g_player.traceLevel > 70 ? COLOR_BLOOD : COLOR_AMBER;
    DrawScaledRect(traceBarX, traceBarY, traceFill, traceBarH, traceCol);
    
    // Trace percentage
    char tracePct[16];
    snprintf(tracePct, sizeof(tracePct), "%d%%", g_player.traceLevel);
    float pctW = MeasureScaledTextWidth(tracePct, 9);
    DrawScaledText(tracePct, traceBarX + traceBarW - pctW - 4, traceBarY + 1, 9, 
                   g_player.traceLevel > 50 ? COLOR_BLACK : COLOR_GHOST);
    
    panelY += 20;
    
    // ICE Shields
    DrawScaledText("ICE", panelX, panelY + 2, 9, COLOR_CYAN);
    float iceBarX = panelX + 30;
    float iceBarY = panelY + 2;
    float iceBarW = panelW - 34;
    float iceBarH = 12.0f;
    
    DrawScaledRect(iceBarX, iceBarY, iceBarW, iceBarH, Color{12, 15, 20, 255});
    DrawScaledRectLines(iceBarX, iceBarY, iceBarW, iceBarH, Color{30, 35, 50, 100});
    
    float iceFill = (g_player.iceShields / 3.0f) * iceBarW;
    if (iceFill < 2.0f) iceFill = 2.0f;
    DrawScaledRect(iceBarX, iceBarY, iceFill, iceBarH, COLOR_CYAN);
    
    char iceText[16];
    snprintf(iceText, sizeof(iceText), "%d/3", g_player.iceShields);
    float iceW = MeasureScaledTextWidth(iceText, 9);
    DrawScaledText(iceText, iceBarX + iceBarW - iceW - 4, iceBarY + 1, 9, COLOR_BLACK);
    
    panelY += 20;
    
    // CRT Heat
    DrawScaledText("HEAT", panelX, panelY + 2, 9, COLOR_CYAN);
    float heatBarX = panelX + 38;
    float heatBarY = panelY + 2;
    float heatBarW = panelW - 42;
    float heatBarH = 12.0f;
    
    DrawScaledRect(heatBarX, heatBarY, heatBarW, heatBarH, Color{12, 15, 20, 255});
    DrawScaledRectLines(heatBarX, heatBarY, heatBarW, heatBarH, Color{30, 35, 50, 100});
    
    float heatFill = ((g_player.crtHeat - 35.0f) / 65.0f) * heatBarW;
    if (heatFill < 0.0f) heatFill = 0.0f;
    if (heatFill < 2.0f && g_player.crtHeat > 35.0f) heatFill = 2.0f;
    Color heatCol = g_player.crtHeat > 75.0f ? COLOR_BLOOD : COLOR_AMBER;
    DrawScaledRect(heatBarX, heatBarY, heatFill, heatBarH, heatCol);
    
    char heatText[16];
    snprintf(heatText, sizeof(heatText), "%.0f°C", g_player.crtHeat);
    float heatW = MeasureScaledTextWidth(heatText, 9);
    DrawScaledText(heatText, heatBarX + heatBarW - heatW - 4, heatBarY + 1, 9, 
                   g_player.crtHeat > 60.0f ? COLOR_BLACK : COLOR_GHOST);
    
    panelY += 24;
    DrawScaledLine(panelX, panelY, panelX + panelW, panelY, Color{30, 35, 50, 80});
    panelY += 8;
    
    // ---- SECTION: VCOIN WALLET ----
    DrawScaledText("💰 WALLET", panelX, panelY, 11, COLOR_TOXIC);
    DrawScaledLine(panelX, panelY + 16, panelX + panelW, panelY + 16, Color{30, 35, 50, 100});
    panelY += 24;
    
    char vcoinDisplay[32];
    snprintf(vcoinDisplay, sizeof(vcoinDisplay), "%.2f VCOIN", g_player.vcoin);
    float vcoinW = MeasureScaledTextWidth(vcoinDisplay, 16);
    DrawScaledText(vcoinDisplay, panelX + (panelW - vcoinW) / 2, panelY, 16, COLOR_TOXIC);
    panelY += 28;
    
    // VCOIN progress to 25 (victory threshold)
    float vcoinGoal = g_player.vcoin / 25.0f;
    if (vcoinGoal > 1.0f) vcoinGoal = 1.0f;
    float goalBarX = panelX;
    float goalBarY = panelY;
    float goalBarW = panelW;
    float goalBarH = 6.0f;
    
    DrawScaledRect(goalBarX, goalBarY, goalBarW, goalBarH, Color{12, 15, 20, 255});
    DrawScaledRectLines(goalBarX, goalBarY, goalBarW, goalBarH, Color{30, 35, 50, 100});
    
    float goalFill = vcoinGoal * goalBarW;
    if (goalFill < 2.0f) goalFill = 2.0f;
    DrawScaledRect(goalBarX, goalBarY, goalFill, goalBarH, COLOR_TOXIC);
    
    char goalText[32];
    snprintf(goalText, sizeof(goalText), "GOAL: %.1f/25.0 VCOIN", g_player.vcoin);
    float goalW = MeasureScaledTextWidth(goalText, 8);
    DrawScaledText(goalText, panelX + (panelW - goalW) / 2, goalBarY + 10, 8, COLOR_GHOST);
    
    panelY += 32;
    DrawScaledLine(panelX, panelY, panelX + panelW, panelY, Color{30, 35, 50, 80});
    panelY += 8;
    
    // ---- SECTION: ACTIVE NODES ----
    DrawScaledText("🔗 ACTIVE NODES", panelX, panelY, 11, COLOR_CYAN);
    DrawScaledLine(panelX, panelY + 16, panelX + panelW, panelY + 16, Color{30, 35, 50, 100});
    panelY += 24;
    
    int nodeCount = g_player.assignedCount;
    char nodeStr[32];
    snprintf(nodeStr, sizeof(nodeStr), "%d / 20 SITES", nodeCount);
    float nodeW = MeasureScaledTextWidth(nodeStr, 12);
    DrawScaledText(nodeStr, panelX + (panelW - nodeW) / 2, panelY, 12, 
                   nodeCount >= 20 ? COLOR_TOXIC : COLOR_AMBER);
    panelY += 22;
    
    // Node progress bar
    float nodeBarX = panelX;
    float nodeBarY = panelY;
    float nodeBarW = panelW;
    float nodeBarH = 8.0f;
    
    DrawScaledRect(nodeBarX, nodeBarY, nodeBarW, nodeBarH, Color{12, 15, 20, 255});
    DrawScaledRectLines(nodeBarX, nodeBarY, nodeBarW, nodeBarH, Color{30, 35, 50, 100});
    
    float nodeFill = (nodeCount / 20.0f) * nodeBarW;
    if (nodeFill < 2.0f) nodeFill = 2.0f;
    DrawScaledRect(nodeBarX, nodeBarY, nodeFill, nodeBarH, 
                   nodeCount >= 20 ? COLOR_TOXIC : COLOR_CYAN);
    
    panelY += 16;
    
    // ---- SECTION: RECENT ACTIVITY TICKER ----
    panelY += 8;
    DrawScaledLine(panelX, panelY, panelX + panelW, panelY, Color{30, 35, 50, 80});
    panelY += 8;
    
    DrawScaledText("📡 ACTIVITY", panelX, panelY, 10, COLOR_AMBER);
    panelY += 18;
    
    // Show last 3 feed entries as quick preview
    int previewStart = g_feedLogCount - 3;
    if (previewStart < 0) previewStart = 0;
    for (int i = previewStart; i < g_feedLogCount; i++) {
        float y = panelY + (i - previewStart) * 16.0f;
        if (y > rightY + rightH - 12) break;
        
        const char* txt = g_feedLogs[i];
        char preview[80];
        strncpy(preview, txt, 50);
        preview[50] = '\0';
        
        // Shorten long messages
        if (strlen(preview) > 45) {
            preview[44] = '.';
            preview[45] = '.';
            preview[46] = '\0';
        }
        
        Color previewCol = COLOR_GHOST;
        if (strstr(txt, "[CHAT]")) previewCol = COLOR_TOXIC;
        else if (strstr(txt, "[WHISPER")) previewCol = COLOR_AMBER;
        else if (strstr(txt, "[DOS") || strstr(txt, "[TRACE")) previewCol = COLOR_BLOOD;
        else if (strstr(txt, "[OVERLOAD]")) previewCol = COLOR_BLOOD;
        
        DrawScaledText(preview, panelX, y, 8, previewCol);
    }
    
    // ---- BOTTOM STATUS BAR ----
    float statusY = cy + ch - 22;
    DrawScaledRect(cx, statusY, cw, 22, Color{14, 18, 28, 220});
    DrawScaledLine(cx, statusY, cx + cw, statusY, Color{30, 35, 50, 100});
    
    char statusText[128];
    snprintf(statusText, sizeof(statusText), "UPTIME: %.0fs | FEED: %d | TRACE: %d%% | ICE: %d/3 | VCOIN: %.2f",
             g_player.runTime, g_feedLogCount, g_player.traceLevel, g_player.iceShields, g_player.vcoin);
    DrawScaledText(statusText, cx + 12, statusY + 5, 8, COLOR_GHOST);
}

void Desktop::DrawHellroom(const AppWindow& win) {
    float cx = win.x + 4;
    float cy = win.y + m_windowTitleHeight + 4;
    float cw = win.w - 8;
    float ch = win.h - m_windowTitleHeight - 8;
    
    float t = (float)GetTime();
    float pulse = sinf(t * 3.0f) * 0.3f + 0.7f;
    
    // ---- BACKGROUND ----
    DrawScaledRect(cx, cy, cw, ch, Color{6, 8, 14, 255});
    DrawScaledRectLines(cx, cy, cw, ch, COLOR_BORDER);
    
    // ---- HEADER ----
    float headerH = 44.0f;
    DrawScaledRect(cx, cy, cw, headerH, Color{14, 18, 28, 255});
    DrawScaledLine(cx, cy + headerH, cx + cw, cy + headerH, Color{30, 35, 50, 150});
    
    // Title with pulsing live indicator
    DrawScaledText("◈ HELLROOM IRC", cx + 16, cy + 12, 14, COLOR_BLOOD);
    
    float livePulse = sinf(t * 4.0f) * 0.3f + 0.7f;
    Color liveCol = {40, 240, 100, (unsigned char)(livePulse * 200 + 55)};
    DrawScaledRect(cx + cw - 70, cy + 12, 8, 8, liveCol);
    DrawScaledText("LIVE", cx + cw - 56, cy + 11, 10, COLOR_TOXIC);
    
    // User count
    char userCountStr[32];
    snprintf(userCountStr, sizeof(userCountStr), "USERS: 1");
    DrawScaledText(userCountStr, cx + cw - 140, cy + 11, 10, COLOR_GHOST);
    
    // ---- LAYOUT: 3 columns (Chat | Users) ----
    float chatX = cx + 6;
    float chatY = cy + headerH + 6;
    float chatW = cw - 180 - 12;  // Leave space for user list
    float chatH = ch - headerH - 80;  // Leave space for input area
    
    float userX = cx + cw - 170;
    float userY = cy + headerH + 6;
    float userW = 164;
    float userH = ch - headerH - 80;
    
    // ---- CHAT LOG AREA ----
    DrawScaledRect(chatX, chatY, chatW, chatH, Color{8, 10, 18, 220});
    DrawScaledRectLines(chatX, chatY, chatW, chatH, Color{30, 35, 50, 100});
    
    // Subtle scanlines
    for (int i = 0; i < (int)chatH; i += 4) {
        float scanY = chatY + i + fmodf(t * 30.0f, 4.0f);
        DrawScaledRect(chatX, scanY, chatW, 1, {0, 0, 0, 4});
    }
    
    // ---- CHAT MESSAGES WITH SCROLL ----
    BeginScissorMode((int)SX(chatX + 4), (int)SY(chatY + 4), 
                 (int)((chatW - 8) * g_uiScale), (int)((chatH - 8) * g_uiScale));

    int total = g_feedLogCount;  // Use global feed logs
    int visibleLines = (int)((chatH - 16) / 20.0f);
    int startIdx = total - visibleLines;
    if (startIdx < 0) startIdx = 0;

    // Apply scroll
    int scrollLines = (int)(m_hellroom.scrollOffset / 20.0f);
    startIdx -= scrollLines;
    if (startIdx < 0) startIdx = 0;
    if (startIdx > total) startIdx = total;

    for (int i = startIdx; i < total; i++) {
        int row = i - startIdx;
        float y = chatY + 8 + row * 20.0f;
        if (y > chatY + chatH - 12) break;
        
        const char* msg = g_feedLogs[i];  // Use global feed logs
        
        // Parse message type for coloring
        Color col = COLOR_GHOST;
        Color prefixCol = COLOR_GHOST;
        bool isSystem = false;
        bool isWhisper = false;
        bool isAction = false;
        
        if (strncmp(msg, "[SERVER]", 8) == 0) {
            col = COLOR_CYAN;
            prefixCol = {0, 220, 240, 150};
            isSystem = true;
        } else if (strncmp(msg, "[WHISPER", 8) == 0) {
            col = COLOR_AMBER;
            prefixCol = {255, 150, 0, 150};
            isWhisper = true;
        } else if (strncmp(msg, "[ACTION]", 8) == 0) {
            col = COLOR_TOXIC;
            prefixCol = {40, 240, 100, 100};
            isAction = true;
        } else if (strncmp(msg, "[JOIN]", 6) == 0 || strncmp(msg, "[PART]", 6) == 0) {
            col = COLOR_AMBER;
            prefixCol = {255, 150, 0, 100};
            isSystem = true;
        } else if (strncmp(msg, "[CHAT]", 5) == 0) {
            // Remove [CHAT] prefix for display
            const char* chatMsg = msg + 7;  // Skip "[CHAT] "
            col = COLOR_TOXIC;
            prefixCol = {40, 240, 100, 100};
            // Draw with custom formatting
            DrawScaledRect(chatX + 4, y - 2, 3, 16, prefixCol);
            DrawScaledText(chatMsg, chatX + 12, y, 10, col);
            
            // Timestamp
            char timeStamp[12];
            int hours = (i * 7 + 13) % 24;
            int mins = (i * 13 + 42) % 60;
            snprintf(timeStamp, sizeof(timeStamp), "%02d:%02d", hours, mins);
            float timeW = MeasureScaledTextWidth(timeStamp, 7);
            DrawScaledText(timeStamp, chatX + chatW - timeW - 8, y, 7, {60, 70, 85, 120});
            continue;
        } else if (strncmp(msg, "[FEED", 5) == 0) {
            col = COLOR_CYAN;
            prefixCol = {0, 220, 240, 80};
            isSystem = true;
        } else {
            // Regular chat - find the first colon
            const char* colon = strchr(msg, ':');
            if (colon) {
                int nickLen = colon - msg;
                char nick[64];
                strncpy(nick, msg, nickLen);
                nick[nickLen] = '\0';
                
                bool isMe = (strcmp(nick, m_hellroom.currentNick) == 0);
                col = isMe ? COLOR_TOXIC : COLOR_GHOST;
                prefixCol = isMe ? COLOR_TOXIC : Color{160, 170, 185, 150};
            }
        }
        
        // Draw accent bar for system messages
        if (isSystem || isWhisper || isAction) {
            DrawScaledRect(chatX + 4, y - 2, 3, 16, prefixCol);
        }
        
        // Truncate long messages
        char truncated[256];
        strncpy(truncated, msg, 220);
        truncated[220] = '\0';
        
        float textX = (isSystem || isWhisper || isAction) ? chatX + 12 : chatX + 8;
        DrawScaledText(truncated, textX, y, 10, col);
        
        // Timestamp
        char timeStamp[12];
        int hours = (i * 7 + 13) % 24;
        int mins = (i * 13 + 42) % 60;
        snprintf(timeStamp, sizeof(timeStamp), "%02d:%02d", hours, mins);
        float timeW = MeasureScaledTextWidth(timeStamp, 7);
        DrawScaledText(timeStamp, chatX + chatW - timeW - 8, y, 7, {60, 70, 85, 120});
    }

    EndScissorMode();
    
    // ---- SCROLL INDICATOR ----
    if (total > visibleLines) {
        float scrollRatio = (float)startIdx / (float)(total - visibleLines);
        float indicatorY = chatY + 8 + scrollRatio * (chatH - 24);
        DrawScaledRect(chatX + chatW - 6, indicatorY, 3, 16, {80, 90, 110, 150});
    }
    
    // ---- USER LIST (Right Panel) ----
    DrawScaledRect(userX, userY, userW, userH, Color{8, 10, 18, 220});
    DrawScaledRectLines(userX, userY, userW, userH, Color{30, 35, 50, 100});
    
    // User list header
    DrawScaledText("║ USERS ONLINE", userX + 8, userY + 6, 9, COLOR_AMBER);
    DrawScaledLine(userX + 4, userY + 22, userX + userW - 4, userY + 22, Color{30, 35, 50, 100});
    
    // Show users (including current user and some fake ones for immersion)
    const char* fakeUsers[] = {
        "sh4d0w_net",
        "ghost_0x99",
        "void_walker",
        "neon_byte",
        "cipher_404"
    };
    int fakeUserCount = sizeof(fakeUsers) / sizeof(fakeUsers[0]);
    
    float userListY = userY + 26;
    int visibleUsers = (int)((userH - 30) / 20.0f);
    
    // Show current user first
    bool isCurrent = true;
    char userDisplay[64];
    snprintf(userDisplay, sizeof(userDisplay), "▶ %s", m_hellroom.currentNick);
    DrawScaledText(userDisplay, userX + 8, userListY, 9, COLOR_TOXIC);
    DrawScaledRect(userX + userW - 16, userListY + 4, 8, 8, COLOR_TOXIC);
    userListY += 20;
    
    // Show fake users
    for (int i = 0; i < fakeUserCount && i < visibleUsers - 1; i++) {
        float y = userX + 8;
        DrawScaledText(fakeUsers[i], userX + 8, userListY, 9, COLOR_GHOST);
        // Random online status dot
        bool online = (rand() % 10) > 2;  // 80% online
        if (online) {
            DrawScaledRect(userX + userW - 16, userListY + 4, 6, 6, {40, 240, 100, 150});
        }
        userListY += 20;
    }
    
    // ---- INPUT AREA ----
    float inputY = cy + ch - 42;
    float inputH = 36;
    
    // Input area background
    DrawScaledRect(cx + 6, inputY, cw - 12, inputH, Color{10, 12, 20, 220});
    DrawScaledLine(cx + 6, inputY, cx + cw - 6, inputY, Color{30, 35, 50, 100});
    
    // ---- NICKNAME INPUT ----
    float nickX = cx + 12;
    float nickY = inputY + 4;
    float nickW = 140;
    float nickH = 28;
    
    bool nickHover = RefRectHover(nickX, nickY, nickW, nickH, GetRefMousePos());
    
    DrawScaledRect(nickX, nickY, nickW, nickH, m_hellroom.nickFocused ? Color{16, 20, 30, 255} : Color{8, 10, 18, 255});
    DrawScaledRectLines(nickX, nickY, nickW, nickH, 
                        m_hellroom.nickFocused ? COLOR_BLOOD : Color{30, 35, 50, 100});
    
    char nickDisplay[64];
    if (m_hellroom.nickFocused) {
        const char* cursor = (fmodf(t * 2.0f, 1.0f) > 0.5f) ? "_" : "";
        snprintf(nickDisplay, sizeof(nickDisplay), "NICK: %s%s", m_hellroom.nickBuffer, cursor);
    } else {
        snprintf(nickDisplay, sizeof(nickDisplay), "NICK: %s", m_hellroom.nickBuffer);
    }
    DrawScaledText(nickDisplay, nickX + 6, nickY + 7, 10, 
                   m_hellroom.nickFocused ? COLOR_TOXIC : COLOR_GHOST);
    
    // ---- MESSAGE INPUT ----
    float msgX = nickX + nickW + 8;
    float msgY = inputY + 4;
    float msgW = cw - 12 - nickW - 8 - 90 - 8;  // Leave room for send button
    float msgH = 28;
    
    bool msgHover = RefRectHover(msgX, msgY, msgW, msgH, GetRefMousePos());
    
    DrawScaledRect(msgX, msgY, msgW, msgH, m_hellroom.inputFocused ? Color{16, 20, 30, 255} : Color{8, 10, 18, 255});
    DrawScaledRectLines(msgX, msgY, msgW, msgH, 
                        m_hellroom.inputFocused ? COLOR_BLOOD : Color{30, 35, 50, 100});
    
    char msgDisplay[256];
    if (m_hellroom.inputFocused) {
        const char* cursor = (fmodf(t * 2.0f, 1.0f) > 0.5f) ? "_" : "";
        snprintf(msgDisplay, sizeof(msgDisplay), "%s%s", m_hellroom.inputBuffer, cursor);
    } else {
        snprintf(msgDisplay, sizeof(msgDisplay), "%s", m_hellroom.inputBuffer);
    }
    if (strlen(msgDisplay) == 0 && !m_hellroom.inputFocused) {
        strcpy(msgDisplay, "Type a message...");
        DrawScaledText(msgDisplay, msgX + 6, msgY + 7, 10, {60, 70, 85, 150});
    } else {
        DrawScaledText(msgDisplay, msgX + 6, msgY + 7, 10, COLOR_CYAN);
    }
    
    // ---- SEND BUTTON ----
    float btnX = msgX + msgW + 6;
    float btnY = inputY + 4;
    float btnW = 80;
    float btnH = 28;
    
    bool btnHover = RefRectHover(btnX, btnY, btnW, btnH, GetRefMousePos());
    
    DrawScaledRect(btnX, btnY, btnW, btnH, btnHover ? COLOR_BLOOD : Color{30, 35, 55, 200});
    DrawScaledRectLines(btnX, btnY, btnW, btnH, btnHover ? COLOR_BLOOD : Color{30, 35, 50, 100});
    
    float btnTextW = MeasureScaledTextWidth("SEND", 10);
    DrawScaledText("SEND", btnX + (btnW - btnTextW) / 2.0f, btnY + 7, 10, 
                   btnHover ? COLOR_BLACK : COLOR_TOXIC);
    
    // ---- MOUSE INTERACTIONS ----
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    if (clicked) {
        if (RefRectHover(nickX, nickY, nickW, nickH, refMouse)) {
            m_hellroom.nickFocused = true;
            m_hellroom.inputFocused = false;
        } else if (RefRectHover(msgX, msgY, msgW, msgH, refMouse)) {
            m_hellroom.inputFocused = true;
            m_hellroom.nickFocused = false;
        } else if (RefRectHover(btnX, btnY, btnW, btnH, refMouse)) {
            SendHellroomMessage();
        }
    }
    
    // ---- KEYBOARD INPUT ----
    if (m_hellroom.inputFocused) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126) {
                size_t len = strlen(m_hellroom.inputBuffer);
                if (len < sizeof(m_hellroom.inputBuffer) - 1) {
                    m_hellroom.inputBuffer[len] = (char)key;
                    m_hellroom.inputBuffer[len + 1] = '\0';
                }
            }
            key = GetCharPressed();
        }
        
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = (int)strlen(m_hellroom.inputBuffer);
            if (len > 0) m_hellroom.inputBuffer[len - 1] = '\0';
        }
        
        if (IsKeyPressed(KEY_ENTER)) {
            SendHellroomMessage();
        }
    }
    
    if (m_hellroom.nickFocused) {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32 && key <= 126) && strlen(m_hellroom.nickBuffer) < 31) {
                size_t len = strlen(m_hellroom.nickBuffer);
                m_hellroom.nickBuffer[len] = (char)key;
                m_hellroom.nickBuffer[len + 1] = '\0';
            }
            key = GetCharPressed();
        }
        
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = (int)strlen(m_hellroom.nickBuffer);
            if (len > 0) m_hellroom.nickBuffer[len - 1] = '\0';
        }
        
        if (IsKeyPressed(KEY_ENTER)) {
            if (strlen(m_hellroom.nickBuffer) > 0) {
                strcpy(m_hellroom.currentNick, m_hellroom.nickBuffer);
                strcpy(g_player.handle, m_hellroom.nickBuffer);
                PushHellroomMessage("[SERVER] You are now known as %s", m_hellroom.currentNick);
                PushCliLog("[HELLROOM] Nick changed to %s", m_hellroom.currentNick);
            }
            m_hellroom.nickFocused = false;
            m_hellroom.inputFocused = true;
        }
    }
    
    // ---- MOUSE WHEEL SCROLLING ----
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        // Check if mouse is over chat area
        if (RefRectHover(chatX, chatY, chatW, chatH, refMouse)) {
            m_hellroom.scrollOffset += wheel * 20.0f;
            if (m_hellroom.scrollOffset < 0.0f) m_hellroom.scrollOffset = 0.0f;
            
            int maxScroll = (total - visibleLines) * 20;
            if (maxScroll < 0) maxScroll = 0;
            if (m_hellroom.scrollOffset > maxScroll) m_hellroom.scrollOffset = maxScroll;
        }
    }
}

void Desktop::DrawVDEC(const AppWindow& win) {
    float cx = win.x + 4;
    float cy = win.y + m_windowTitleHeight + 4;
    float cw = win.w - 8;
    float ch = win.h - m_windowTitleHeight - 8;
    
    float t = (float)GetTime();
    float pulse = sinf(t * 3.0f) * 0.3f + 0.7f;
    
    // ---- BACKGROUND ----
    DrawScaledRect(cx, cy, cw, ch, Color{4, 6, 14, 255});
    DrawScaledRectLines(cx, cy, cw, ch, COLOR_BORDER);
    
    // ---- HEADER ----
    float headerH = 50.0f;
    DrawScaledRect(cx, cy, cw, headerH, Color{10, 14, 24, 255});
    DrawScaledLine(cx, cy + headerH, cx + cw, cy + headerH, Color{30, 35, 50, 150});
    
    // Animated VDEC logo
    float glowPulse = sinf(t * 2.0f) * 0.3f + 0.7f;
    DrawScaledText("🔐 VDEC v2.0 // VEKTRA DECRYPTION ENGINE", cx + 16, cy + 14, 14, 
                   Color{0, 220, 240, (unsigned char)(glowPulse * 200 + 55)});
    
    // Status indicator
    Color statusCol = {40, 240, 100, (unsigned char)(pulse * 200 + 55)};
    DrawScaledRect(cx + cw - 120, cy + 14, 8, 8, statusCol);
    DrawScaledText("ACTIVE", cx + cw - 105, cy + 13, 10, COLOR_TOXIC);
    
    // ---- TAB BAR ----
    float tabY = cy + headerH;
    float tabH = 36.0f;
    const char* tabs[] = {"🔑 Key Ring", "🔓 Decrypt", "🔒 Encrypt", "🔢 Hash", "🎯 Minigame"};
    int tabCount = 5;
    float tabW = cw / tabCount;
    
    DrawScaledRect(cx, tabY, cw, tabH, Color{6, 8, 16, 255});
    DrawScaledLine(cx, tabY + tabH, cx + cw, tabY + tabH, Color{30, 35, 50, 100});
    
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    for (int i = 0; i < tabCount; i++) {
        float tx = cx + i * tabW;
        bool hover = RefRectHover(tx, tabY, tabW, tabH, refMouse);
        bool active = (i == m_vdec.selectedTab);
        
        Color bg = active ? Color{20, 25, 45, 200} : (hover ? Color{15, 18, 35, 150} : Color{0,0,0,0});
        DrawScaledRect(tx, tabY, tabW, tabH, bg);
        
        if (active) {
            DrawScaledRect(tx, tabY + tabH - 3, tabW, 3, COLOR_CYAN);
        }
        
        DrawScaledText(tabs[i], tx + 8, tabY + 10, 11, 
                      active ? COLOR_CYAN : (hover ? COLOR_TOXIC : COLOR_GHOST));
        
        if (clicked && hover) {
            m_vdec.selectedTab = i;
            m_vdec.minigameActive = false;
        }
    }
    
    // ---- CONTENT AREA ----
    float contentX = cx + 12;
    float contentY = tabY + tabH + 8;
    float contentW = cw - 24;
    float contentH = ch - headerH - tabH - 16;
    
    // Content background with subtle scanlines
    DrawScaledRect(contentX, contentY, contentW, contentH, Color{6, 8, 16, 200});
    DrawScaledRectLines(contentX, contentY, contentW, contentH, Color{30, 35, 50, 80});
    
    // Scanline overlay
    for (int i = 0; i < (int)contentH; i += 4) {
        float scanY = contentY + i + fmodf(t * 30.0f, 4.0f);
        DrawScaledRect(contentX, scanY, contentW, 1, {0, 0, 0, 4});
    }
    
    // ---- DRAW TAB CONTENT ----
    switch (m_vdec.selectedTab) {
        case 0: DrawVDECKeyRing(contentX, contentY, contentW, contentH); break;
        case 1: DrawVDECDecrypt(contentX, contentY, contentW, contentH); break;
        case 2: DrawVDECEncrypt(contentX, contentY, contentW, contentH); break;
        case 3: DrawVDECHash(contentX, contentY, contentW, contentH); break;
        case 4: DrawVDECMinigame(contentX, contentY, contentW, contentH); break;
    }
}

void Desktop::DrawVDECDecrypt(float x, float y, float w, float h) {
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    float t = (float)GetTime();
    
    DrawScaledText("🔓 DECRYPTION ENGINE", x, y, 14, COLOR_CYAN);
    DrawScaledLine(x, y + 22, x + w, y + 22, Color{30, 35, 50, 80});
    
    float rowY = y + 34;
    
    // Input label
    DrawScaledText("ENCRYPTED TEXT:", x, rowY, 11, COLOR_GHOST);
    rowY += 20;
    
    // Input box
    float inputX = x;
    float inputY = rowY;
    float inputW = w;
    float inputH = 80;
    
    bool inputHover = RefRectHover(inputX, inputY, inputW, inputH, refMouse);
    
    DrawScaledRect(inputX, inputY, inputW, inputH, Color{8, 10, 18, 255});
    DrawScaledRectLines(inputX, inputY, inputW, inputH, 
                        m_vdec.inputFocused ? COLOR_CYAN : Color{30, 35, 50, 100});
    
    // Show input text with cursor
    char display[1024];
    if (m_vdec.inputFocused) {
        const char* cursor = (fmodf(t * 2.0f, 1.0f) > 0.5f) ? "_" : "";
        snprintf(display, sizeof(display), "%s%s", m_vdec.inputBuffer, cursor);
    } else {
        snprintf(display, sizeof(display), "%s", m_vdec.inputBuffer);
    }
    if (strlen(display) == 0 && !m_vdec.inputFocused) {
        strcpy(display, "Enter encrypted text to decrypt...");
        DrawScaledText(display, inputX + 8, inputY + 8, 11, {60, 70, 90, 150});
    } else {
        DrawScaledText(display, inputX + 8, inputY + 8, 11, COLOR_CYAN);
    }
    
    rowY += inputH + 12;
    
    // Decrypt button
    float btnX = x + w - 120;
    float btnY = rowY;
    float btnW = 110;
    float btnH = 32;
    
    bool btnHover = RefRectHover(btnX, btnY, btnW, btnH, refMouse);
    DrawScaledRect(btnX, btnY, btnW, btnH, btnHover ? COLOR_CYAN : Color{20, 25, 45, 200});
    DrawScaledRectLines(btnX, btnY, btnW, btnH, COLOR_CYAN);
    DrawScaledText("DECRYPT", btnX + 16, btnY + 8, 11, btnHover ? COLOR_BLACK : COLOR_CYAN);
    
    if (clicked && btnHover && strlen(m_vdec.inputBuffer) > 0) {
        // Send decrypt request to server
        std::string payload = std::string(VNetCmd::VDEC_DECRYPT) + ":" + m_vdec.inputBuffer;
        if (IsVNetConnected()) {
            VNetSendRaw(payload);
            PushCliLog("[VDEC] Decrypt request sent");
        } else {
            // Offline fallback
            char decrypted[1024];
            int shift = m_vdec.bitShiftOffset % 26;
            for (int i = 0; m_vdec.inputBuffer[i] && i < 1023; i++) {
                char c = m_vdec.inputBuffer[i];
                if (c >= 'A' && c <= 'Z') {
                    decrypted[i] = (char)(((c - 'A' - shift + 26) % 26) + 'A');
                } else if (c >= 'a' && c <= 'z') {
                    decrypted[i] = (char)(((c - 'a' - shift + 26) % 26) + 'a');
                } else {
                    decrypted[i] = c;
                }
            }
            decrypted[strlen(m_vdec.inputBuffer)] = '\0';
            strcpy(m_vdec.outputBuffer, decrypted);
            PushCliLog("[VDEC] Decrypted (offline): %s", decrypted);
        }
    }
    
    rowY += btnH + 12;
    DrawScaledLine(x, rowY, x + w, rowY, Color{30, 35, 50, 80});
    rowY += 8;
    
    // Output label
    DrawScaledText("DECRYPTED TEXT:", x, rowY, 11, COLOR_TOXIC);
    rowY += 20;
    
    // Output box
    float outX = x;
    float outY = rowY;
    float outW = w;
    float outH = 80;
    
    DrawScaledRect(outX, outY, outW, outH, Color{8, 10, 18, 255});
    DrawScaledRectLines(outX, outY, outW, outH, Color{30, 35, 50, 100});
    
    if (strlen(m_vdec.outputBuffer) > 0) {
        DrawScaledText(m_vdec.outputBuffer, outX + 8, outY + 8, 11, COLOR_TOXIC);
    } else {
        DrawScaledText("Decrypted output will appear here...", outX + 8, outY + 8, 11, {60, 70, 90, 150});
    }
    
    // ---- INPUT HANDLING ----
    if (clicked) {
        if (inputHover) {
            m_vdec.inputFocused = true;
            m_vdec.outputFocused = false;
        } else if (!RefRectHover(btnX, btnY, btnW, btnH, refMouse)) {
            m_vdec.inputFocused = false;
        }
    }
    
    if (m_vdec.inputFocused) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126) {
                size_t len = strlen(m_vdec.inputBuffer);
                if (len < sizeof(m_vdec.inputBuffer) - 1) {
                    m_vdec.inputBuffer[len] = (char)key;
                    m_vdec.inputBuffer[len + 1] = '\0';
                }
            }
            key = GetCharPressed();
        }
        
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = (int)strlen(m_vdec.inputBuffer);
            if (len > 0) m_vdec.inputBuffer[len - 1] = '\0';
        }
        
        if (IsKeyPressed(KEY_ENTER)) {
            // Trigger decrypt
            if (strlen(m_vdec.inputBuffer) > 0) {
                std::string payload = std::string(VNetCmd::VDEC_DECRYPT) + ":" + m_vdec.inputBuffer;
                if (IsVNetConnected()) {
                    VNetSendRaw(payload);
                    PushCliLog("[VDEC] Decrypt request sent");
                }
            }
        }
    }
}

void Desktop::DrawVDECEncrypt(float x, float y, float w, float h) {
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    float t = (float)GetTime();
    
    DrawScaledText("🔒 ENCRYPTION ENGINE", x, y, 14, COLOR_AMBER);
    DrawScaledLine(x, y + 22, x + w, y + 22, Color{30, 35, 50, 80});
    
    float rowY = y + 34;
    
    DrawScaledText("PLAINTEXT:", x, rowY, 11, COLOR_GHOST);
    rowY += 20;
    
    float inputX = x;
    float inputY = rowY;
    float inputW = w;
    float inputH = 80;
    
    bool inputHover = RefRectHover(inputX, inputY, inputW, inputH, refMouse);
    
    DrawScaledRect(inputX, inputY, inputW, inputH, Color{8, 10, 18, 255});
    DrawScaledRectLines(inputX, inputY, inputW, inputH, 
                        m_vdec.inputFocused ? COLOR_AMBER : Color{30, 35, 50, 100});
    
    char display[1024];
    if (m_vdec.inputFocused) {
        const char* cursor = (fmodf(t * 2.0f, 1.0f) > 0.5f) ? "_" : "";
        snprintf(display, sizeof(display), "%s%s", m_vdec.inputBuffer, cursor);
    } else {
        snprintf(display, sizeof(display), "%s", m_vdec.inputBuffer);
    }
    if (strlen(display) == 0 && !m_vdec.inputFocused) {
        strcpy(display, "Enter text to encrypt...");
        DrawScaledText(display, inputX + 8, inputY + 8, 11, {60, 70, 90, 150});
    } else {
        DrawScaledText(display, inputX + 8, inputY + 8, 11, COLOR_AMBER);
    }
    
    rowY += inputH + 12;
    
    float btnX = x + w - 120;
    float btnY = rowY;
    float btnW = 110;
    float btnH = 32;
    
    bool btnHover = RefRectHover(btnX, btnY, btnW, btnH, refMouse);
    DrawScaledRect(btnX, btnY, btnW, btnH, btnHover ? COLOR_AMBER : Color{20, 25, 45, 200});
    DrawScaledRectLines(btnX, btnY, btnW, btnH, COLOR_AMBER);
    DrawScaledText("ENCRYPT", btnX + 16, btnY + 8, 11, btnHover ? COLOR_BLACK : COLOR_AMBER);
    
    if (clicked && btnHover && strlen(m_vdec.inputBuffer) > 0) {
        std::string payload = std::string(VNetCmd::VDEC_ENCRYPT) + ":" + m_vdec.inputBuffer;
        if (IsVNetConnected()) {
            VNetSendRaw(payload);
            PushCliLog("[VDEC] Encrypt request sent");
        } else {
            char encrypted[1024];
            int shift = m_vdec.bitShiftOffset % 26;
            for (int i = 0; m_vdec.inputBuffer[i] && i < 1023; i++) {
                char c = m_vdec.inputBuffer[i];
                if (c >= 'A' && c <= 'Z') {
                    encrypted[i] = (char)(((c - 'A' + shift) % 26) + 'A');
                } else if (c >= 'a' && c <= 'z') {
                    encrypted[i] = (char)(((c - 'a' + shift) % 26) + 'a');
                } else {
                    encrypted[i] = c;
                }
            }
            encrypted[strlen(m_vdec.inputBuffer)] = '\0';
            strcpy(m_vdec.outputBuffer, encrypted);
            PushCliLog("[VDEC] Encrypted (offline): %s", encrypted);
        }
    }
    
    rowY += btnH + 12;
    DrawScaledLine(x, rowY, x + w, rowY, Color{30, 35, 50, 80});
    rowY += 8;
    
    DrawScaledText("ENCRYPTED TEXT:", x, rowY, 11, COLOR_AMBER);
    rowY += 20;
    
    float outX = x;
    float outY = rowY;
    float outW = w;
    float outH = 80;
    
    DrawScaledRect(outX, outY, outW, outH, Color{8, 10, 18, 255});
    DrawScaledRectLines(outX, outY, outW, outH, Color{30, 35, 50, 100});
    
    if (strlen(m_vdec.outputBuffer) > 0) {
        DrawScaledText(m_vdec.outputBuffer, outX + 8, outY + 8, 11, COLOR_AMBER);
    } else {
        DrawScaledText("Encrypted output will appear here...", outX + 8, outY + 8, 11, {60, 70, 90, 150});
    }
    
    if (clicked) {
        if (inputHover) {
            m_vdec.inputFocused = true;
        } else if (!RefRectHover(btnX, btnY, btnW, btnH, refMouse)) {
            m_vdec.inputFocused = false;
        }
    }
    
    if (m_vdec.inputFocused) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126) {
                size_t len = strlen(m_vdec.inputBuffer);
                if (len < sizeof(m_vdec.inputBuffer) - 1) {
                    m_vdec.inputBuffer[len] = (char)key;
                    m_vdec.inputBuffer[len + 1] = '\0';
                }
            }
            key = GetCharPressed();
        }
        
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = (int)strlen(m_vdec.inputBuffer);
            if (len > 0) m_vdec.inputBuffer[len - 1] = '\0';
        }
    }
}

void Desktop::DrawVDECHash(float x, float y, float w, float h) {
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    float t = (float)GetTime();
    
    DrawScaledText("🔢 HASH CALCULATOR // CRYPTOGRAPHIC DIGESTS", x, y, 14, COLOR_TOXIC);
    DrawScaledLine(x, y + 22, x + w, y + 22, Color{30, 35, 50, 80});
    
    float rowY = y + 34;
    
    DrawScaledText("INPUT TEXT:", x, rowY, 11, COLOR_GHOST);
    rowY += 20;
    
    float inputX = x;
    float inputY = rowY;
    float inputW = w;
    float inputH = 60;
    
    bool inputHover = RefRectHover(inputX, inputY, inputW, inputH, refMouse);
    
    DrawScaledRect(inputX, inputY, inputW, inputH, Color{8, 10, 18, 255});
    DrawScaledRectLines(inputX, inputY, inputW, inputH, 
                        m_vdec.inputFocused ? COLOR_TOXIC : Color{30, 35, 50, 100});
    
    char display[1024];
    if (m_vdec.inputFocused) {
        const char* cursor = (fmodf(t * 2.0f, 1.0f) > 0.5f) ? "_" : "";
        snprintf(display, sizeof(display), "%s%s", m_vdec.inputBuffer, cursor);
    } else {
        snprintf(display, sizeof(display), "%s", m_vdec.inputBuffer);
    }
    if (strlen(display) == 0 && !m_vdec.inputFocused) {
        strcpy(display, "Enter text to hash...");
        DrawScaledText(display, inputX + 8, inputY + 8, 11, {60, 70, 90, 150});
    } else {
        DrawScaledText(display, inputX + 8, inputY + 8, 11, COLOR_GHOST);
    }
    
    rowY += inputH + 12;
    
    // Hash type selector
    const char* hashTypes[] = {"MD5", "SHA1", "SHA256"};
    int hashType = 0;
    
    float typeX = x;
    float typeY = rowY;
    for (int i = 0; i < 3; i++) {
        float tx = typeX + i * 80;
        bool hover = RefRectHover(tx, typeY, 70, 28, refMouse);
        bool active = (i == hashType);
        
        DrawScaledRect(tx, typeY, 70, 28, active ? Color{40, 45, 70, 200} : (hover ? Color{20, 25, 45, 150} : Color{10, 12, 20, 200}));
        DrawScaledRectLines(tx, typeY, 70, 28, active ? COLOR_TOXIC : (hover ? COLOR_CYAN : Color{30, 35, 50, 100}));
        DrawScaledText(hashTypes[i], tx + 12, typeY + 7, 10, active ? COLOR_TOXIC : COLOR_GHOST);
        
        if (clicked && hover) {
            hashType = i;
        }
    }
    
    float btnX = x + w - 120;
    float btnY = typeY;
    float btnW = 110;
    float btnH = 28;
    
    bool btnHover = RefRectHover(btnX, btnY, btnW, btnH, refMouse);
    DrawScaledRect(btnX, btnY, btnW, btnH, btnHover ? COLOR_TOXIC : Color{20, 25, 45, 200});
    DrawScaledRectLines(btnX, btnY, btnW, btnH, COLOR_TOXIC);
    DrawScaledText("HASH", btnX + 35, btnY + 7, 11, btnHover ? COLOR_BLACK : COLOR_TOXIC);
    
    if (clicked && btnHover && strlen(m_vdec.inputBuffer) > 0) {
        std::string payload = std::string(VNetCmd::VDEC_HASH) + ":" + m_vdec.inputBuffer;
        if (IsVNetConnected()) {
            VNetSendRaw(payload);
            PushCliLog("[VDEC] Hash request sent");
        } else {
            // Simple hash
            unsigned long hash = 5381;
            for (int i = 0; m_vdec.inputBuffer[i]; i++) {
                hash = ((hash << 5) + hash) + m_vdec.inputBuffer[i];
            }
            snprintf(m_vdec.hashBuffer, sizeof(m_vdec.hashBuffer), "%08lX", hash);
            PushCliLog("[VDEC] Hash (offline): %s", m_vdec.hashBuffer);
        }
    }
    
    rowY += 36;
    DrawScaledLine(x, rowY, x + w, rowY, Color{30, 35, 50, 80});
    rowY += 8;
    
    DrawScaledText("HASH RESULT:", x, rowY, 11, COLOR_TOXIC);
    rowY += 20;
    
    float outX = x;
    float outY = rowY;
    float outW = w;
    float outH = 40;
    
    DrawScaledRect(outX, outY, outW, outH, Color{8, 10, 18, 255});
    DrawScaledRectLines(outX, outY, outW, outH, Color{30, 35, 50, 100});
    
    if (strlen(m_vdec.hashBuffer) > 0) {
        DrawScaledText(m_vdec.hashBuffer, outX + 8, outY + 12, 14, COLOR_TOXIC);
    } else {
        DrawScaledText("Hash will appear here...", outX + 8, outY + 12, 11, {60, 70, 90, 150});
    }
    
    if (clicked) {
        if (inputHover) {
            m_vdec.inputFocused = true;
        } else if (!RefRectHover(btnX, btnY, btnW, btnH, refMouse) && 
                   !RefRectHover(typeX, typeY, 240, 28, refMouse)) {
            m_vdec.inputFocused = false;
        }
    }
    
    if (m_vdec.inputFocused) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126) {
                size_t len = strlen(m_vdec.inputBuffer);
                if (len < sizeof(m_vdec.inputBuffer) - 1) {
                    m_vdec.inputBuffer[len] = (char)key;
                    m_vdec.inputBuffer[len + 1] = '\0';
                }
            }
            key = GetCharPressed();
        }
        
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = (int)strlen(m_vdec.inputBuffer);
            if (len > 0) m_vdec.inputBuffer[len - 1] = '\0';
        }
    }
}

void Desktop::DrawVDECMinigame(float x, float y, float w, float h) {
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    float t = (float)GetTime();
    
    DrawScaledText("🎯 DECRYPTION CHALLENGE // KAGUYA TRIAL", x, y, 14, COLOR_BLOOD);
    DrawScaledLine(x, y + 22, x + w, y + 22, Color{30, 35, 50, 80});
    
    float rowY = y + 34;
    
    // Challenge description
    DrawScaledText("Decrypt the encrypted number to reveal the key fragment!", 
                   x, rowY, 11, COLOR_GHOST);
    rowY += 24;
    
    if (!m_vdec.minigameActive) {
        // Start button
        float btnX = x + w / 2 - 80;
        float btnY = rowY;
        float btnW = 160;
        float btnH = 40;
        
        bool btnHover = RefRectHover(btnX, btnY, btnW, btnH, refMouse);
        DrawScaledRect(btnX, btnY, btnW, btnH, btnHover ? COLOR_BLOOD : Color{20, 25, 45, 200});
        DrawScaledRectLines(btnX, btnY, btnW, btnH, COLOR_BLOOD);
        DrawScaledText("INITIATE CHALLENGE", btnX + 16, btnY + 12, 12, btnHover ? COLOR_BLACK : COLOR_BLOOD);
        
        if (clicked && btnHover) {
            m_vdec.minigameActive = true;
            m_vdec.minigameTimer = 0.0f;
            m_vdec.minigameAttempts = 0;
            m_vdec.minigameSuccess = false;
            m_vdec.minigameInput[0] = '\0';
            
            // Send request to server for challenge
            if (IsVNetConnected()) {
                VNetSendRaw(std::string(VNetCmd::VDEC_MINIGAME));
                PushCliLog("[VDEC] Minigame challenge requested");
            } else {
                // Offline fallback
                m_vdec.minigameTarget = rand() % 9000 + 1000;
                char targetStr[16];
                snprintf(targetStr, sizeof(targetStr), "%d", m_vdec.minigameTarget);
                PushCliLog("[VDEC] Offline challenge target: %s", targetStr);
            }
        }
    } else {
        // Active minigame
        float pulse = sinf(t * 2.0f) * 0.3f + 0.7f;
        float centerX = x + w / 2;
        
        // Animated border
        DrawScaledRect(x + 10, rowY, w - 20, 200, Color{4, 6, 14, 200});
        DrawScaledRectLines(x + 10, rowY, w - 20, 200, 
                           Color{220, 20, 40, (unsigned char)(pulse * 150 + 55)});
        
        // Decryption matrix animation
        for (int i = 0; i < 20; i++) {
            float colX = x + 20 + i * ((w - 40) / 20.0f);
            float colY = rowY + 20 + sinf(t * 2.0f + i * 0.7f) * 10.0f;
            char hexChar = "0123456789ABCDEF"[(int)(fmodf(t * 5.0f + i * 3.0f, 16))];
            DrawScaledText(&hexChar, colX, colY, 10, 
                          Color{40, 240, 100, (unsigned char)(pulse * 100 + 55)});
        }
        
        // Target display
        char targetDisplay[64];
        if (IsVNetConnected() && m_vdec.minigameTarget == 0) {
            strcpy(targetDisplay, "AWAITING CHALLENGE...");
        } else {
            snprintf(targetDisplay, sizeof(targetDisplay), "DECRYPT: %d", m_vdec.minigameTarget);
        }
        float targetW = MeasureScaledTextWidth(targetDisplay, 16);
        DrawScaledText(targetDisplay, centerX - targetW / 2, rowY + 60, 16, 
                       m_vdec.minigameSuccess ? COLOR_TOXIC : COLOR_CYAN);
        
        // Input box
        float inputX = centerX - 80;
        float inputY = rowY + 100;
        float inputW = 160;
        float inputH = 32;
        
        bool inputHover = RefRectHover(inputX, inputY, inputW, inputH, refMouse);
        
        DrawScaledRect(inputX, inputY, inputW, inputH, Color{8, 10, 18, 255});
        DrawScaledRectLines(inputX, inputY, inputW, inputH, 
                            m_vdec.inputFocused ? COLOR_BLOOD : Color{30, 35, 50, 100});
        
        char inputDisplay[32];
        if (m_vdec.inputFocused) {
            const char* cursor = (fmodf(t * 2.0f, 1.0f) > 0.5f) ? "_" : "";
            snprintf(inputDisplay, sizeof(inputDisplay), "%s%s", m_vdec.minigameInput, cursor);
        } else {
            snprintf(inputDisplay, sizeof(inputDisplay), "%s", m_vdec.minigameInput);
        }
        DrawScaledText(inputDisplay, inputX + 8, inputY + 8, 11, COLOR_GHOST);
        
        // Submit button
        float subX = inputX + inputW + 12;
        float subY = inputY;
        float subW = 80;
        float subH = 32;
        
        bool subHover = RefRectHover(subX, subY, subW, subH, refMouse);
        DrawScaledRect(subX, subY, subW, subH, subHover ? COLOR_BLOOD : Color{20, 25, 45, 200});
        DrawScaledRectLines(subX, subY, subW, subH, COLOR_BLOOD);
        DrawScaledText("SUBMIT", subX + 14, subY + 8, 11, subHover ? COLOR_BLACK : COLOR_BLOOD);
        
        // Attempt counter
        char attemptStr[32];
        snprintf(attemptStr, sizeof(attemptStr), "ATTEMPTS: %d", m_vdec.minigameAttempts);
        float attemptW = MeasureScaledTextWidth(attemptStr, 10);
        DrawScaledText(attemptStr, centerX - attemptW / 2, rowY + 150, 10, COLOR_GHOST);
        
        // Success message
        if (m_vdec.minigameSuccess) {
            float pulse2 = sinf(t * 4.0f) * 0.3f + 0.7f;
            Color successCol = {40, 240, 100, (unsigned char)(pulse2 * 200 + 55)};
            DrawScaledText("★ DECRYPTION SUCCESSFUL! ★", centerX - 120, rowY + 175, 14, successCol);
        }
        
        // Input handling
        if (clicked) {
            if (inputHover) {
                m_vdec.inputFocused = true;
            } else if (!subHover) {
                m_vdec.inputFocused = false;
            }
        }
        
        if (clicked && subHover && !m_vdec.minigameSuccess) {
            if (strlen(m_vdec.minigameInput) > 0) {
                int attempt = atoi(m_vdec.minigameInput);
                m_vdec.minigameAttempts++;
                
                if (attempt == m_vdec.minigameTarget) {
                    m_vdec.minigameSuccess = true;
                    // Unlock a key!
                    int keyIndex = m_vdec.minigameAttempts % 8;
                    if (!m_vdec.keysFound[keyIndex]) {
                        snprintf(m_vdec.keys[keyIndex], sizeof(m_vdec.keys[0]), 
                                "KEY_%02d_%04d", keyIndex + 1, rand() % 9000 + 1000);
                        m_vdec.keysFound[keyIndex] = true;
                        m_vdec.keyCount++;
                        PushCliLog("[VDEC] ★ KEY %d UNLOCKED! ★", keyIndex + 1);
                        PushHellroomMessage("[VDEC] Player unlocked key %d!", keyIndex + 1);
                    }
                    PushCliLog("[VDEC] ✅ Challenge completed in %d attempts!", m_vdec.minigameAttempts);
                } else {
                    PushCliLog("[VDEC] ❌ Incorrect! Target: %d, Got: %d", m_vdec.minigameTarget, attempt);
                }
                m_vdec.minigameInput[0] = '\0';
            }
        }
        
        if (m_vdec.inputFocused && !m_vdec.minigameSuccess) {
            int key = GetCharPressed();
            while (key > 0) {
                if (key >= '0' && key <= '9') {
                    size_t len = strlen(m_vdec.minigameInput);
                    if (len < 15) {
                        m_vdec.minigameInput[len] = (char)key;
                        m_vdec.minigameInput[len + 1] = '\0';
                    }
                }
                key = GetCharPressed();
            }
            
            if (IsKeyPressed(KEY_BACKSPACE)) {
                int len = (int)strlen(m_vdec.minigameInput);
                if (len > 0) m_vdec.minigameInput[len - 1] = '\0';
            }
            
            if (IsKeyPressed(KEY_ENTER)) {
                if (strlen(m_vdec.minigameInput) > 0 && !m_vdec.minigameSuccess) {
                    int attempt = atoi(m_vdec.minigameInput);
                    m_vdec.minigameAttempts++;
                    
                    if (attempt == m_vdec.minigameTarget) {
                        m_vdec.minigameSuccess = true;
                        int keyIndex = m_vdec.minigameAttempts % 8;
                        if (!m_vdec.keysFound[keyIndex]) {
                            snprintf(m_vdec.keys[keyIndex], sizeof(m_vdec.keys[0]), 
                                    "KEY_%02d_%04d", keyIndex + 1, rand() % 9000 + 1000);
                            m_vdec.keysFound[keyIndex] = true;
                            m_vdec.keyCount++;
                            PushCliLog("[VDEC] ★ KEY %d UNLOCKED! ★", keyIndex + 1);
                            PushHellroomMessage("[VDEC] Player unlocked key %d!", keyIndex + 1);
                        }
                        PushCliLog("[VDEC] ✅ Challenge completed in %d attempts!", m_vdec.minigameAttempts);
                    } else {
                        PushCliLog("[VDEC] ❌ Incorrect! Target: %d, Got: %d", m_vdec.minigameTarget, attempt);
                    }
                    m_vdec.minigameInput[0] = '\0';
                }
            }
        }
    }
}

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
            DrawScaledText(m_vdec.keys[i], kx + 6, ky + 22, 14, COLOR_BLACK);
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
// ============================================================
// GLOBAL ACCESS
// ============================================================

// ============================================================
// SETTINGS CATEGORY DRAWERS
// ============================================================

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

void DrawSettingsSystem(float x, float y, float w, float h) {
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    float t = (float)GetTime();
    float pulse = sinf(t * 2.0f) * 0.3f + 0.7f;
    
    DrawScaledText("📦 SYSTEM INFORMATION", x, y, 16, COLOR_CYAN);
    DrawScaledLine(x, y + 24, x + w, y + 24, Color{30, 35, 50, 100});
    
    float rowY = y + 44;
    
    // ---- SYSTEM STATS ----
    struct SysStat {
        const char* label;
        char value[64];
        Color color;
    };
    
    SysStat stats[8];
    
    snprintf(stats[0].value, sizeof(stats[0].value), "%s", g_player.handle);
    stats[0].label = "HANDLE";
    stats[0].color = COLOR_TOXIC;
    
    snprintf(stats[1].value, sizeof(stats[1].value), "%d", g_player.port);
    stats[1].label = "PORT";
    stats[1].color = COLOR_CYAN;
    
    snprintf(stats[2].value, sizeof(stats[2].value), "%.2f VCOIN", g_player.vcoin);
    stats[2].label = "VCOIN";
    stats[2].color = COLOR_TOXIC;
    
    snprintf(stats[3].value, sizeof(stats[3].value), "%d%%", g_player.traceLevel);
    stats[3].label = "TRACE";
    stats[3].color = g_player.traceLevel > 70 ? COLOR_BLOOD : COLOR_AMBER;
    
    snprintf(stats[4].value, sizeof(stats[4].value), "%d/3", g_player.iceShields);
    stats[4].label = "ICE";
    stats[4].color = COLOR_CYAN;
    
    snprintf(stats[5].value, sizeof(stats[5].value), "%.0f°C", g_player.crtHeat);
    stats[5].label = "CRT HEAT";
    stats[5].color = g_player.crtHeat > 75.0f ? COLOR_BLOOD : COLOR_AMBER;
    
    snprintf(stats[6].value, sizeof(stats[6].value), "%.0f%%", g_player.neuralParanoia);
    stats[6].label = "PARANOIA";
    stats[6].color = g_player.neuralParanoia > 60.0f ? COLOR_BLOOD : COLOR_AMBER;
    
    snprintf(stats[7].value, sizeof(stats[7].value), "%d sites", g_player.assignedCount);
    stats[7].label = "DISCOVERED";
    stats[7].color = COLOR_GHOST;
    
    for (int i = 0; i < 8; i++) {
        float sx = x + 10 + (i % 2) * 250.0f;
        float sy = rowY + (i / 2) * 36.0f;
        
        DrawScaledText(stats[i].label, sx, sy, 10, COLOR_GHOST);
        DrawScaledText(stats[i].value, sx + 100, sy, 11, stats[i].color);
    }
    
    rowY += 160;
    DrawScaledLine(x + 10, rowY, x + w - 10, rowY, Color{30, 35, 50, 60});
    rowY += 12;
    
    // ---- UPTIME ----
    char uptimeStr[64];
    int hours = (int)(g_player.runTime / 3600.0f);
    int minutes = (int)((g_player.runTime - hours * 3600) / 60.0f);
    int seconds = (int)(g_player.runTime - hours * 3600 - minutes * 60);
    snprintf(uptimeStr, sizeof(uptimeStr), "UPTIME: %02d:%02d:%02d", hours, minutes, seconds);
    DrawScaledText(uptimeStr, x + 10, rowY, 12, COLOR_TOXIC);
    
    rowY += 30;
    
    // ---- SYSTEM STATUS INDICATORS ----
    DrawScaledText("SYSTEM STATUS", x, rowY, 12, COLOR_GHOST);
    rowY += 22;
    
    // Status grid
    const char* statusLabels[] = {"Network", "Audio", "VFS", "ICE"};
    bool statusStates[] = {IsVNetConnected(), true, true, g_player.iceShields > 0};
    
    for (int i = 0; i < 4; i++) {
        float sx = x + 10 + i * 120.0f;
        float pulse2 = sinf(t * 2.0f + i * 1.5f) * 0.3f + 0.7f;
        Color statusCol = statusStates[i] ? 
            Color{40, 240, 100, (unsigned char)(pulse2 * 200 + 55)} : 
            COLOR_BLOOD;
        
        DrawScaledRect(sx, rowY, 8, 8, statusCol);
        DrawScaledText(statusLabels[i], sx + 14, rowY, 10, statusStates[i] ? COLOR_TOXIC : COLOR_BLOOD);
    }
    
    rowY += 30;
    
    // ---- RESET BUTTON ----
    bool hoverReset = RefRectHover(x + 10, rowY, 160, 32, refMouse);
    DrawScaledRect(x + 10, rowY, 160, 32, hoverReset ? COLOR_BLOOD : Color{30, 35, 55, 200});
    DrawScaledRectLines(x + 10, rowY, 160, 32, hoverReset ? COLOR_BLOOD : Color{50, 55, 70, 150});
    DrawScaledText("RESET SETTINGS", x + 25, rowY + 8, 11, hoverReset ? COLOR_BLACK : COLOR_GHOST);
    
    if (clicked && hoverReset) {
        PushCliLog("[SETTINGS]: Settings reset to default");
        TriggerJitter(0.3f);
        SetActiveTheme("classic");
        GetMusicPlayer().SetVolume(0.7f);
    }
}

// ============================================================
// HELLROOM HELPERS
// ============================================================

void Desktop::PushHellroomMessage(const char* fmt, ...) {
    // Format the message
    char buffer[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    // Check if this message already exists in the feed to avoid duplicates
    // (optional - helps prevent duplicates from overlapping sources)
    for (int i = g_feedLogCount - 1; i >= 0 && i >= g_feedLogCount - 3; i--) {
        if (strcmp(g_feedLogs[i], buffer) == 0) {
            return;  // Already in feed, skip
        }
    }
    
    // Push to system feed logs
    PushFeedLog("%s", buffer);
    
    // Also store in hellroom's local buffer
    if (m_hellroom.messageCount >= 200) {
        for (int i = 0; i < 199; i++) {
            strcpy(m_hellroom.chatMessages[i], m_hellroom.chatMessages[i + 1]);
        }
        m_hellroom.messageCount = 199;
    }
    
    strcpy(m_hellroom.chatMessages[m_hellroom.messageCount], buffer);
    m_hellroom.messageCount++;
    
    // Auto-scroll to bottom
    int total = m_hellroom.messageCount;
    int visibleLines = 20;
    int maxScroll = (total - visibleLines) * 20;
    if (maxScroll < 0) maxScroll = 0;
    m_hellroom.scrollOffset = maxScroll;
}

void Desktop::SendHellroomMessage() {
    if (strlen(m_hellroom.inputBuffer) == 0) return;
    
    char msg[512];
    strcpy(msg, m_hellroom.inputBuffer);
    m_hellroom.inputBuffer[0] = '\0';
    
    // Check for commands (starting with /)
    if (msg[0] == '/') {
        char cmd[256];
        strcpy(cmd, msg + 1);
        
        // Handle /nick
        if (strncmp(msg + 1, "nick ", 5) == 0) {
            char newNick[32];
            strncpy(newNick, msg + 6, 31);
            newNick[31] = '\0';
            char* end = newNick + strlen(newNick) - 1;
            while (end > newNick && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) {
                end--;
            }
            end[1] = '\0';
            
            if (strlen(newNick) > 0) {
                strcpy(m_hellroom.currentNick, newNick);
                strcpy(m_hellroom.nickBuffer, newNick);
                strcpy(g_player.handle, newNick);
                // Let server broadcast the nick change
                std::string payload = std::string(VNetCmd::CHAT) + ":" + 
                                      std::string(g_player.handle) + ":" + 
                                      "*** " + std::string(newNick) + " is now known as " + std::string(newNick);
                if (IsVNetConnected()) {
                    VNetSendRaw(payload);
                }
                // Also show locally
                PushHellroomMessage("[SERVER] You are now known as %s", m_hellroom.currentNick);
            }
            return;
        }
        
        // Handle /me
        if (strncmp(msg + 1, "me ", 3) == 0) {
            char action[256];
            strcpy(action, msg + 4);
            std::string payload = std::string(VNetCmd::CHAT) + ":" + 
                                  std::string(g_player.handle) + ":" + 
                                  "* " + std::string(m_hellroom.currentNick) + " " + action;
            if (IsVNetConnected()) {
                VNetSendRaw(payload);
            }
            // DON'T add locally - server will broadcast it back
            // BUT we want to see it immediately, so add a local copy with [ACTION] tag
            PushHellroomMessage("[ACTION] * %s %s", m_hellroom.currentNick, action);
            return;
        }
        
        // Handle /clear
        if (strcmp(msg + 1, "clear") == 0) {
            // Clear local messages only
            m_hellroom.messageCount = 0;
            PushHellroomMessage("[SERVER] Chat cleared");
            return;
        }
        
        // Handle /help
        if (strcmp(msg + 1, "help") == 0) {
            PushHellroomMessage("[SERVER] Available commands:");
            PushHellroomMessage("[SERVER]   /nick <name>  - Change your nickname");
            PushHellroomMessage("[SERVER]   /me <action>  - Send an action message");
            PushHellroomMessage("[SERVER]   /w <nick> <msg> - Whisper to a user");
            PushHellroomMessage("[SERVER]   /clear        - Clear chat history");
            PushHellroomMessage("[SERVER]   /help         - Show this help");
            return;
        }
        
        // Handle /w (whisper)
        if (strncmp(msg + 1, "w ", 2) == 0 || strncmp(msg + 1, "whisper ", 8) == 0) {
            char* cmdStart = msg + 1;
            if (cmdStart[0] == 'w') {
                cmdStart += 2;
            } else if (strncmp(cmdStart, "whisper ", 8) == 0) {
                cmdStart += 8;
            }
            while (*cmdStart == ' ') cmdStart++;
            
            char target[64] = {0};
            char whisperMsg[256] = {0};
            if (sscanf(cmdStart, "%63s %255[^\n]", target, whisperMsg) == 2) {
                // Send whisper via network
                std::string payload = std::string(VNetCmd::WHISPER) + ":" + 
                                      std::string(g_player.handle) + ":" + 
                                      std::string(target) + ":" + 
                                      std::string(whisperMsg);
                if (IsVNetConnected()) {
                    VNetSendRaw(payload);
                }
                // Show locally immediately (server won't echo whispers back to sender)
                PushHellroomMessage("[WHISPER TO %s] %s", target, whisperMsg);
            } else {
                PushHellroomMessage("[SERVER] Usage: /w <nick> <message>");
            }
            return;
        }
        
        // For all other commands, pass to ProcessCommand
        ProcessCommand(cmd);
        return;
    }
    
    // ---- REGULAR CHAT MESSAGE ----
    // DO NOT add locally - server will broadcast it back via FEED_EVENT
    if (IsVNetConnected()) {
        std::string payload = std::string(VNetCmd::CHAT) + ":" + 
                              std::string(g_player.handle) + ":" + msg;
        VNetSendRaw(payload);
    } else {
        // Offline fallback - show locally
        PushHellroomMessage("%s: %s", m_hellroom.currentNick, msg);
    }
}

Desktop& GetDesktop() {
    return Desktop::Get();
}