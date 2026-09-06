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

static void LaunchIntruderDetector() {
    GetDesktop().OpenApp(AppType::IntruderDetector, "INTRUDER DETECTOR v3.2");
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
    
    // ============================================================
    // FIX: Always create a new terminal if terminal is requested
    // ============================================================
    if (type == AppType::Terminal) {
        // Check if terminal exists but is minimized - restore it
        for (int i = 0; i < (int)m_windows.size(); i++) {
            if (m_windows[i].type == AppType::Terminal) {
                if (m_windows[i].minimized) {
                    m_windows[i].minimized = false;
                    FocusWindow(i);
                    return i;
                }
                // Already open and visible - just focus it
                FocusWindow(i);
                return i;
            }
        }
        // No terminal exists - create one
    } else {
        // For other apps, check if already open
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
    }
    
    // Create new window
    AppWindow win;
    win.type = type;
    win.title = title ? title : "Window";
    win.x = 40 + (int)m_windows.size() * 20;
    win.y = 60 + (int)m_windows.size() * 20;
    win.w = 1000;
    win.h = 720;
    win.minimized = false;
    win.maximized = false;
    win.focused = true;
    win.dragging = false;
    win.dragX = 0;
    win.dragY = 0;
    
    // Clamp position
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
    // AUTO-OPEN INTRUDER DETECTOR ON RAID
    // ============================================================
    if (g_player.raidIntruderVisible) {
        bool found = false;
        for (const auto& win : m_windows) {
            if (win.type == AppType::IntruderDetector && !win.minimized) {
                found = true;
                break;
            }
        }
        if (!found) {
            // Open the IntruderDetector window
            int idx = OpenApp(AppType::IntruderDetector, "🚨 INTRUDER DETECTOR");
            if (idx >= 0) {
                // Position it prominently on screen
                m_windows[idx].x = REF_WIDTH / 2 - 300;
                m_windows[idx].y = REF_HEIGHT / 2 - 200;
                m_windows[idx].w = 600;
                m_windows[idx].h = 420;
                FocusWindow(idx);
            }
        }
        g_player.raidIntruderVisible = false;  // Reset flag, window stays open
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

void Desktop::Draw() {
    if (!m_active) return;

    if (g_player.raidSeqLockDesktop) {
        DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, BLACK);
    } else {
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
    
    // Window title - left aligned
    DrawScaledText(win.title.c_str(), win.x + 14, win.y + 8, 11, focused ? COLOR_BLACK : COLOR_GHOST);
    
    // ============================================================
    // macOS-STYLE WINDOW CONTROLS (Circles on the RIGHT)
    // Order: GREEN (left) → YELLOW (middle) → RED (right)
    // ============================================================
    float btnSize = 14.0f;
    float spacing = 8.0f;
    float startX = win.x + win.w - 14.0f - btnSize - (btnSize + spacing) * 2;
    float startY = win.y + 7.5f;
    
    // ---- MAXIMIZE BUTTON (Green) - LEFT ----
    float maxX = startX;
    Color maxColor = focused ? Color{100, 210, 80, 255} : Color{80, 80, 85, 150};
    DrawScaledCircle(maxX + btnSize/2, startY + btnSize/2, btnSize/2, maxColor);
    if (focused) {
        DrawScaledCircleLines(maxX + btnSize/2, startY + btnSize/2, btnSize/2, Color{60, 170, 40, 180});
    }
    
    // ---- MINIMIZE BUTTON (Yellow) - MIDDLE ----
    float minX = startX + btnSize + spacing;
    Color minColor = focused ? Color{255, 200, 60, 255} : Color{80, 80, 85, 150};
    DrawScaledCircle(minX + btnSize/2, startY + btnSize/2, btnSize/2, minColor);
    if (focused) {
        DrawScaledCircleLines(minX + btnSize/2, startY + btnSize/2, btnSize/2, Color{200, 160, 30, 180});
    }
    
    // ---- CLOSE BUTTON (Red) - RIGHT ----
    float closeX = minX + btnSize + spacing;
    Color closeColor = focused ? Color{255, 95, 87, 255} : Color{80, 80, 85, 150};
    DrawScaledCircle(closeX + btnSize/2, startY + btnSize/2, btnSize/2, closeColor);
    if (focused) {
        DrawScaledCircleLines(closeX + btnSize/2, startY + btnSize/2, btnSize/2, Color{200, 60, 50, 180});
    }
    
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
        case AppType::IntruderDetector: DrawIntruderDetector(win); break;
        default: break;
    }
    
    EndScissorMode();
}

// ============================================================
// APP CONTENT RENDERERS
// ============================================================

// ============================================================
// SETTINGS CATEGORY DRAWERS
// ============================================================

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

void Desktop::SyncVDECKeys() {
    // Clear existing VDEC keys
    for (int i = 0; i < 8; i++) {
        m_vdec.keysFound[i] = false;
        m_vdec.keys[i][0] = '\0';
    }
    m_vdec.keyCount = 0;
    
    // Sync from g_vnet.masterKeys
    for (int i = 0; i < 8; i++) {
        if (strlen(g_vnet.masterKeys[i]) > 0) {
            strcpy(m_vdec.keys[i], g_vnet.masterKeys[i]);
            m_vdec.keysFound[i] = true;
            m_vdec.keyCount++;
        }
    }
    
    // Also check g_vnet.keyLocations for any keys (if masterKeys is empty)
    if (m_vdec.keyCount == 0) {
        for (int i = 0; i < 8; i++) {
            if (strlen(g_vnet.keyLocations[i]) > 0 && strlen(g_vnet.keyLocations[i]) < 32) {
                strcpy(m_vdec.keys[i], g_vnet.keyLocations[i]);
                m_vdec.keysFound[i] = true;
                m_vdec.keyCount++;
            }
        }
    }
}

Desktop& GetDesktop() {
    return Desktop::Get();
}