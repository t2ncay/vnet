#pragma once
#include "raylib.h"
#include "music_player.h"

#include <cstring>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <cmath> 

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
    About,
    Hellroom,
    VDEC
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
    std::string iconKey;  // Key to look up texture
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

    struct VDECState {
        // Key ring - stores found keys
        char keys[8][32];
        bool keysFound[8];
        int keyCount;
        
        // Current operation
        char inputBuffer[1024];
        char outputBuffer[1024];
        char hashBuffer[1024];
        
        // UI state
        int selectedTab;  // 0=Keys, 1=Encrypt, 2=Hash, 3=Master
        int selectedKeyIndex;
        bool inputFocused;
        bool outputFocused;
        float scrollOffset;
        
        // Minigame state
        bool minigameActive;
        float minigameTimer;
        int minigameTarget;
        int minigameAttempts;
        char minigameInput[16];
        bool minigameSuccess;
        
        // Bit-shift
        int bitShiftOffset;
    } m_vdec;

    void Init();
    void Shutdown();
    void Update(float dt);
    void Draw();
    
    bool IsActive() const { return m_active; }
    void Toggle() {  }
    
    int OpenApp(AppType type, const char* title = nullptr);
    void CloseWindow(int idx);
    void FocusWindow(int idx);
    void MinimizeWindow(int idx);
    void MaximizeWindow(int idx);

    void SwitchWorkspace(int idx);
    void MoveWindowToWorkspace(int winIdx, int wsIdx);

    bool IsDesktopBlocking() const { return m_active && !m_appGridVisible; }
    
    bool IsBrowserFocused() const {
        for (const auto& win : m_windows) {
            if (win.type == AppType::Browser && win.focused && !win.minimized) {
                return true;
            }
        }
        return false;
    }
    
    bool GetBrowserRect(int& x, int& y, int& w, int& h) const {
        for (const auto& win : m_windows) {
            if (win.type == AppType::Browser && !win.minimized) {
                x = win.x + 4;
                y = win.y + m_windowTitleHeight + 4 + 50;
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

    void NavigateTo(const char* url);
    void NavigateBack();
    void NavigateForward();

    // VDEC helpers
    void SetVDECOutput(const char* text) { 
        strncpy(m_vdec.outputBuffer, text, sizeof(m_vdec.outputBuffer) - 1); 
    }
    void SetVDECHash(const char* text) { 
        strncpy(m_vdec.hashBuffer, text, sizeof(m_vdec.hashBuffer) - 1); 
    }
    void SetVDECMinigameTarget(int target) { 
        m_vdec.minigameTarget = target; 
    }

    void SyncVDECKeys();

private:
    Desktop() = default;
    ~Desktop() = default;
    Desktop(const Desktop&) = delete;
    Desktop& operator=(const Desktop&) = delete;

    void DrawHellroom(const AppWindow& win);
    void PushHellroomMessage(const char* fmt, ...);
    void SendHellroomMessage();

    // Add to private section
    struct HellroomState {
        char inputBuffer[512];
        char nickBuffer[32];
        char chatMessages[200][512];
        int messageCount;
        bool nickFocused;
        bool inputFocused;
        float scrollOffset;
        char currentNick[32];
    } m_hellroom;

    void DrawTopBar();
    void DrawAppGrid();
    void DrawWindow(int idx);
    void DrawWindowContent(const AppWindow& win);
    void DrawWorkspaceIndicator();
    void DrawClock();
    void DrawDesktopIcons();
    void DrawVDEC(const AppWindow& win);
    void DrawVDECDecrypt(float x, float y, float w, float h);
    void DrawVDECEncrypt(float x, float y, float w, float h);
    void DrawVDECHash(float x, float y, float w, float h);
    void DrawVDECMinigame(float x, float y, float w, float h);
    void DrawVDECKeyRing(float x, float y, float w, float h);
    
    // App content renderers
    void DrawBrowser(const AppWindow& win);
    void DrawBrowserConnectionOverlay(float contentX, float contentY, float contentW, float contentH);
    void DrawTerminal(const AppWindow& win);
    void DrawProfile(const AppWindow& win);
    void DrawSettings(const AppWindow& win);
    void DrawFeed(const AppWindow& win);

    // Icon management
    void LoadIcons();
    void UnloadIcons();
    Texture2D GetIcon(const std::string& key);

    std::vector<AppWindow> m_windows;
    std::vector<AppIcon> m_apps;
    std::vector<Workspace> m_workspaces;
    std::vector<std::string> m_navHistory;
    int m_navHistoryIndex = -1;
    int m_currentWorkspace = 0;
    bool m_active = false;
    int m_focused = -1;
    bool m_appGridVisible = false;

    float m_musicWidgetX = 0.0f;
    float m_musicWidgetY = 0.0f;
    float m_musicWidgetW = 280.0f;
    float m_musicWidgetH = 380.0f;
    bool m_musicWidgetVisible = true;
    
    // Icon textures
    std::unordered_map<std::string, Texture2D> m_iconTextures;
    
    // Layout constants
    float m_topBarHeight = 44.0f;
    float m_windowTitleHeight = 30.0f;
};

Desktop& GetDesktop();