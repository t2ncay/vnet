#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <functional>

// ============================================================
// GNOME-STYLE DESKTOP
// ============================================================

enum class AppType {
    None,
    Browser,
    Terminal,
    Profile,
    Settings,
    Feed,
    FileManager,
    About
};

struct AppWindow {
    AppType type;
    std::string title;
    int x, y, w, h;
    bool minimized;
    bool maximized;
    bool focused;
    bool dragging;
    int dragX, dragY;
};

struct AppIcon {
    std::string name;
    std::string icon;
    std::string description;
    std::function<void()> onClick;
};

struct Workspace {
    std::vector<int> windows;
    std::string name;
    
    Workspace() = default;
    Workspace(const std::string& n) : name(n) {}
};

class Desktop {
public:
    static Desktop& Get() {
        static Desktop instance;
        return instance;
    }

    void Init();
    void Shutdown();
    void Update(float dt);
    void Draw();
    
    bool IsActive() const { return m_active; }
    void Toggle() { m_active = !m_active; }
    
    // Window management
    int OpenApp(AppType type, const char* title = nullptr);
    void CloseWindow(int idx);
    void FocusWindow(int idx);
    void MinimizeWindow(int idx);
    void MaximizeWindow(int idx);

    // Workspace management
    void SwitchWorkspace(int idx);
    void MoveWindowToWorkspace(int winIdx, int wsIdx);

    // Check if desktop is blocking input
    bool IsDesktopBlocking() const { return m_active && !m_appGridVisible; }
    
    // NEW: Check if the browser window is focused (for input forwarding)
    bool IsBrowserFocused() const {
        for (const auto& win : m_windows) {
            if (win.type == AppType::Browser && win.focused && !win.minimized) {
                return true;
            }
        }
        return false;
    }
    
    // NEW: Get the browser window's content area for mouse hit testing
    bool GetBrowserRect(int& x, int& y, int& w, int& h) const {
        for (const auto& win : m_windows) {
            if (win.type == AppType::Browser && !win.minimized) {
                x = win.x + 4;
                y = win.y + m_windowTitleHeight + 4 + 50; // URL bar offset
                w = win.w - 8;
                h = win.h - m_windowTitleHeight - 8 - 60;
                return true;
            }
        }
        return false;
    }

    bool GetTerminalRect(int& x, int& y, int& w, int& h) const {
        for (const auto& win : m_windows) {
            if (win.type == AppType::Terminal && !win.minimized) {
                x = win.x + 4;
                y = win.y + m_windowTitleHeight + 4;
                w = win.w - 8;
                h = win.h - m_windowTitleHeight - 8;
                return true;
            }
        }
        return false;
    }

    bool IsTerminalFocused() const {
        for (const auto& win : m_windows) {
            if (win.type == AppType::Terminal && win.focused && !win.minimized) {
                return true;
            }
        }
        return false;
    }

private:
    Desktop() = default;
    ~Desktop() = default;
    Desktop(const Desktop&) = delete;
    Desktop& operator=(const Desktop&) = delete;

    void DrawTopBar();
    void DrawAppGrid();
    void DrawWindow(int idx);
    void DrawWindowContent(const AppWindow& win);
    void DrawWorkspaceIndicator();
    void DrawClock();
    void DrawDesktopIcons();
    
    // App content renderers
    void DrawBrowser(const AppWindow& win);
    void DrawTerminal(const AppWindow& win);
    void DrawProfile(const AppWindow& win);
    void DrawSettings(const AppWindow& win);
    void DrawFeed(const AppWindow& win);

    std::vector<AppWindow> m_windows;
    std::vector<AppIcon> m_apps;
    std::vector<Workspace> m_workspaces;
    int m_currentWorkspace = 0;
    bool m_active = false;
    int m_focused = -1;
    bool m_appGridVisible = false;
    
    // Layout constants
    float m_topBarHeight = 44.0f;
    float m_windowTitleHeight = 30.0f;
};

Desktop& GetDesktop();