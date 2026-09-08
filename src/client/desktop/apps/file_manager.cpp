#include "file_manager.h"
#include "../../render.h"
#include "../../../shared/vnet.h"
#include "../../game.h"
#include "../desktop.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <cmath>

// ============================================================
// VFSManager implementation
// ============================================================

void VFSManager::SetRoot(const std::string& path) {
    m_rootPath = path;
    if (!fs::exists(m_rootPath)) {
        fs::create_directories(m_rootPath);
    }
}

// fs::path's operator/ treats any rhs starting with '/' as absolute and
// discards the lhs entirely: fs::path("C:/game/vfs_root") / "/sub" == "/sub".
// Every relPath in this app starts with '/' (currentPath begins at "/"), so
// every single call was resolving to the real filesystem root instead of
// vfs_root. Strip the leading slash(es) before joining to stay inside the
// sandbox root that was passed to SetRoot().
static fs::path ResolveInRoot(const std::string& rootPath, const std::string& relPath) {
    std::string clean = relPath;
    while (!clean.empty() && clean.front() == '/') clean.erase(clean.begin());
    if (clean.empty()) return fs::path(rootPath);
    return fs::path(rootPath) / clean;
}

std::vector<FileEntry> VFSManager::ListDirectory(const std::string& relPath) const {
    std::vector<FileEntry> entries;
    fs::path full = ResolveInRoot(m_rootPath, relPath);
    if (!fs::exists(full) || !fs::is_directory(full)) return entries;

    try {
        for (const auto& entry : fs::directory_iterator(full)) {
            try {
                FileEntry fe;
                fe.name = entry.path().filename().string();
                fe.isDirectory = entry.is_directory();
                fe.size = fe.isDirectory ? 0 : entry.file_size();

                auto ftime = fs::last_write_time(entry);
                std::time_t tt;
                
            #if __cplusplus >= 202002L
                tt = std::chrono::system_clock::to_time_t(std::chrono::file_clock::to_sys(ftime));
            #else
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
                );
                tt = std::chrono::system_clock::to_time_t(sctp);
            #endif

                std::tm* tm_ptr = std::localtime(&tt);
                if (tm_ptr) {
                    std::ostringstream oss;
                    oss << std::put_time(tm_ptr, "%Y-%m-%d %H:%M");
                    fe.modified = oss.str();
                } else {
                    fe.modified = "Unknown";
                }

                entries.push_back(fe);
            } catch (const fs::filesystem_error&) {
                continue;
            }
        }
    } catch (const fs::filesystem_error&) {
    }

    std::sort(entries.begin(), entries.end(), [](const FileEntry& a, const FileEntry& b) {
        if (a.isDirectory != b.isDirectory) return a.isDirectory > b.isDirectory;
        return a.name < b.name;
    });
    return entries;
}

bool VFSManager::GetEntry(const std::string& relPath, FileEntry& out) const {
    fs::path full = ResolveInRoot(m_rootPath, relPath);
    if (!fs::exists(full)) return false;

    try {
        out.name = full.filename().string();
        out.isDirectory = fs::is_directory(full);
        out.size = out.isDirectory ? 0 : fs::file_size(full);

        auto ftime = fs::last_write_time(full);
        std::time_t tt;
    #if __cplusplus >= 202002L
        tt = std::chrono::system_clock::to_time_t(std::chrono::file_clock::to_sys(ftime));
    #else
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
        );
        tt = std::chrono::system_clock::to_time_t(sctp);
    #endif

        std::tm* tm_ptr = std::localtime(&tt);
        if (tm_ptr) {
            std::ostringstream oss;
            oss << std::put_time(tm_ptr, "%Y-%m-%d %H:%M");
            out.modified = oss.str();
        } else {
            out.modified = "Unknown";
        }
        return true;
    } catch (const fs::filesystem_error&) {
        return false;
    }
}

bool VFSManager::IsDirectory(const std::string& relPath) const {
    fs::path full = ResolveInRoot(m_rootPath, relPath);
    return fs::exists(full) && fs::is_directory(full);
}

std::string VFSManager::ReadFile(const std::string& relPath) const {
    fs::path full = ResolveInRoot(m_rootPath, relPath);
    if (!fs::exists(full) || fs::is_directory(full)) return "";
    std::ifstream file(full, std::ios::binary);
    if (!file.is_open()) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string VFSManager::GetAbsolutePath(const std::string& relPath) const {
    return (ResolveInRoot(m_rootPath, relPath)).string();
}

// ============================================================
// UI DRAWING FUNCTIONS - REDESIGNED
// ============================================================

static void DrawPathBar(Desktop& desktop, float x, float y, float w, float h);
static void DrawDirectoryTree(Desktop& desktop, float x, float y, float w, float h);
static void DrawFileList(Desktop& desktop, float x, float y, float w, float h);
static void DrawStatusBar(Desktop& desktop, float x, float y, float w, float h);
static void DrawFileContentViewer(Desktop& desktop, float cx, float cy, float cw, float ch);
static std::string FormatSize(uint64_t bytes);

// ============================================================
// MAIN DRAW FUNCTION
// ============================================================

void DrawFileManagerUI(const AppWindow& win, Desktop& desktop) {
    float cx = win.x + 4;
    float cy = win.y + desktop.m_windowTitleHeight + 4;
    float cw = win.w - 8;
    float ch = win.h - desktop.m_windowTitleHeight - 8;

    // ---- Background ----
    DrawScaledRect(cx, cy, cw, ch, Color{6, 8, 14, 255});
    DrawScaledRectLines(cx, cy, cw, ch, COLOR_BORDER);

    // ---- Layout constants ----
    float pathH = 40.0f;
    float statusH = 28.0f;
    float treeW = 200.0f;
    float contentY = cy + pathH + 4;
    float contentH = ch - pathH - statusH - 8;

    // ---- Draw components ----
    DrawPathBar(desktop, cx, cy, cw, pathH);
    DrawDirectoryTree(desktop, cx + 4, contentY, treeW, contentH);
    DrawFileList(desktop, cx + treeW + 8, contentY, cw - treeW - 12, contentH);
    DrawStatusBar(desktop, cx, cy + ch - statusH, cw, statusH);

    // ---- File content viewer overlay ----
    if (desktop.m_fileManager.showingFileContent) {
        DrawFileContentViewer(desktop, cx, cy, cw, ch);
    }
}

// ============================================================
// PATH BAR
// ============================================================

static void DrawPathBar(Desktop& desktop, float x, float y, float w, float h) {
    // Background
    DrawScaledRect(x, y, w, h, Color{10, 12, 20, 220});
    DrawScaledLine(x, y + h, x + w, y + h, Color{30, 35, 50, 120});

    // Navigation buttons
    float btnX = x + 8;
    float btnY = y + 6;
    float btnSize = 26.0f;
    const char* navLabels[] = {"◄", "►", "▲"};
    Vector2 refMouse = GetRefMousePos();

    for (int i = 0; i < 3; i++) {
        float bx = btnX + i * (btnSize + 6);
        bool hover = RefRectHover(bx, btnY, btnSize, btnSize, refMouse);
        
        DrawScaledRect(bx, btnY, btnSize, btnSize,
                       hover ? Color{30, 35, 55, 150} : Color{0,0,0,0});
        DrawScaledRectLines(bx, btnY, btnSize, btnSize,
                            hover ? COLOR_CYAN : Color{30, 35, 50, 60});
        DrawScaledText(navLabels[i], bx + 6, btnY + 4, 12,
                       hover ? COLOR_CYAN : COLOR_GHOST);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hover) {
            if (i == 2) { // Up
                if (desktop.m_fileManager.currentPath != "/") {
                    std::string parent = fs::path(desktop.m_fileManager.currentPath).parent_path().string();
                    if (parent.empty()) parent = "/";
                    desktop.m_fileManager.currentPath = parent;
                    desktop.m_fileManager.scrollOffset = 0;
                    desktop.m_fileManager.selectedIndex = -1;
                }
            }
        }
    }

    // Path display
    float pathX = btnX + 3 * (btnSize + 6) + 12;
    char pathDisplay[256];
    snprintf(pathDisplay, sizeof(pathDisplay), "📂 %s", desktop.m_fileManager.currentPath.c_str());
    DrawScaledText(pathDisplay, pathX, y + 12, 13, COLOR_AMBER);

    // Decorative line under path
    float lineX = pathX + MeasureScaledTextWidth(pathDisplay, 13) + 8;
    DrawScaledRect(lineX, y + h - 2, w - lineX - 20, 1, Color{30, 35, 50, 60});

    // Search icon (placeholder)
    DrawScaledText("🔍", x + w - 30, y + 10, 14, Color{60, 70, 90, 120});
}

// ============================================================
// DIRECTORY TREE
// ============================================================

static void DrawDirectoryTree(Desktop& desktop, float x, float y, float w, float h) {
    auto& state = desktop.m_fileManager;
    
    // Panel background
    DrawScaledRect(x, y, w, h, Color{8, 10, 18, 220});
    DrawScaledRectLines(x, y, w, h, Color{30, 35, 50, 80});

    // Header
    float headerH = 28.0f;
    DrawScaledRect(x, y, w, headerH, Color{12, 14, 24, 220});
    DrawScaledLine(x, y + headerH, x + w, y + headerH, Color{30, 35, 50, 80});
    DrawScaledText("📁 FOLDERS", x + 8, y + 6, 9, COLOR_CYAN);

    // Tree content
    float contentY = y + headerH + 4;
    float contentH = h - headerH - 8;
    
    BeginScissorMode((int)SX(x + 4), (int)SY(contentY),
                     (int)((w - 8) * g_uiScale), (int)(contentH * g_uiScale));

    float tx = x + 8;
    float ty = contentY + 4;
    float lineH = 22.0f;
    int indent = 12;

    // Root
    bool isRoot = (state.currentPath == "/");
    Color rootCol = isRoot ? COLOR_TOXIC : COLOR_AMBER;
    std::string rootDisplay = isRoot ? "📁 /" : "📁 root";
    DrawScaledText(rootDisplay.c_str(), tx, ty, 10, rootCol);
    ty += lineH;

    // Breadcrumb path
    std::vector<std::string> parts;
    if (state.currentPath != "/") {
        fs::path p(state.currentPath);
        while (p != "/") {
            parts.push_back(p.filename().string());
            p = p.parent_path();
        }
        std::reverse(parts.begin(), parts.end());
    }

    for (const auto& part : parts) {
        DrawScaledText(("├── " + part).c_str(), tx + indent, ty, 10, COLOR_GHOST);
        ty += lineH;
    }

    // Subdirectories under current path
    auto entries = desktop.m_vfs.ListDirectory(state.currentPath);
    for (const auto& e : entries) {
        if (e.isDirectory && ty < contentY + contentH - lineH) {
            bool isCurrent = (state.currentPath == "/" + e.name) || 
                             (state.currentPath.find(e.name) != std::string::npos);
            Color dirCol = isCurrent ? COLOR_TOXIC : COLOR_CYAN;
            DrawScaledText(("├── 📁 " + e.name).c_str(), tx + indent * 2, ty, 10, dirCol);
            ty += lineH;
        }
    }

    // Show "empty" message
    if (ty < contentY + contentH - lineH && entries.empty()) {
        DrawScaledText("└── (empty)", tx + indent * 2, ty, 9, Color{60, 70, 90, 120});
    }

    EndScissorMode();
}

// ============================================================
// FILE LIST
// ============================================================

static void DrawFileList(Desktop& desktop, float x, float y, float w, float h) {
    auto entries = desktop.m_vfs.ListDirectory(desktop.m_fileManager.currentPath);
    float lineH = 26.0f;
    float padding = 4.0f;
    
    // Column widths
    float iconW = 28.0f;
    float nameW = 200.0f;
    float sizeW = 80.0f;
    float dateW = 130.0f;

    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    // ---- Panel background ----
    DrawScaledRect(x, y, w, h, Color{8, 10, 18, 220});
    DrawScaledRectLines(x, y, w, h, Color{30, 35, 50, 80});

    // ---- Header ----
    float headerH = 28.0f;
    DrawScaledRect(x, y, w, headerH, Color{12, 14, 24, 220});
    DrawScaledLine(x, y + headerH, x + w, y + headerH, Color{30, 35, 50, 80});

    DrawScaledText("Name", x + iconW + padding, y + 6, 9, COLOR_CYAN);
    DrawScaledText("Size", x + iconW + nameW + padding, y + 6, 9, COLOR_CYAN);
    DrawScaledText("Modified", x + iconW + nameW + sizeW + padding, y + 6, 9, COLOR_CYAN);

    // ---- File list with scissor ----
    float listY = y + headerH + 2;
    float listH = h - headerH - 4;

    BeginScissorMode((int)SX(x), (int)SY(listY),
                     (int)(w * g_uiScale), (int)(listH * g_uiScale));

    int total = (int)entries.size();
    int visible = (int)(listH / lineH);
    int start = desktop.m_fileManager.scrollOffset;
    if (start > total - visible) start = total - visible;
    if (start < 0) start = 0;

    static double lastClickTime = 0.0;

    for (int i = start; i < total && i < start + visible; i++) {
        const auto& entry = entries[i];
        float rowY = listY + 2 + (i - start) * lineH;
        if (rowY > listY + listH - lineH) break;

        bool hover = RefRectHover(x, rowY, w, lineH, refMouse);
        bool selected = (i == desktop.m_fileManager.selectedIndex);

        // Row background
        if (hover || selected) {
            DrawScaledRect(x, rowY, w, lineH,
                          selected ? Color{0, 220, 240, 30} : Color{20, 25, 45, 60});
        }

        // Icon
        std::string icon = entry.isDirectory ? "📁" : "📄";
        Color iconCol = entry.isDirectory ? COLOR_AMBER : COLOR_GHOST;
        DrawScaledText(icon.c_str(), x + padding, rowY + 3, 14, iconCol);

        // Name
        Color nameCol = entry.isDirectory ? COLOR_AMBER : COLOR_GHOST;
        if (selected) nameCol = COLOR_TOXIC;
        DrawScaledText(entry.name.c_str(), x + iconW + padding, rowY + 4, 10, nameCol);

        // Size
        if (!entry.isDirectory) {
            std::string sizeStr = FormatSize(entry.size);
            DrawScaledText(sizeStr.c_str(), x + iconW + nameW + padding, rowY + 4, 9, COLOR_GHOST);
        } else {
            DrawScaledText("<DIR>", x + iconW + nameW + padding, rowY + 4, 9, Color{60, 70, 90, 150});
        }

        // Modified date
        DrawScaledText(entry.modified.c_str(), x + iconW + nameW + sizeW + padding, rowY + 4, 9, Color{80, 90, 110, 180});

        // Handle double-click
        if (clicked && hover) {
            desktop.m_fileManager.selectedIndex = i;
            double now = GetTime();
            if (now - lastClickTime < 0.4) {
                if (entry.isDirectory) {
                    fs::path newPath = fs::path(desktop.m_fileManager.currentPath) / entry.name;
                    desktop.m_fileManager.currentPath = newPath.string();
                    desktop.m_fileManager.scrollOffset = 0;
                    desktop.m_fileManager.selectedIndex = -1;
                } else {
                    std::string content = desktop.m_vfs.ReadFile(
                        (fs::path(desktop.m_fileManager.currentPath) / entry.name).string()
                    );
                    desktop.m_fileManager.showingFileContent = true;
                    desktop.m_fileManager.currentFileContent = content;
                    desktop.m_fileManager.currentFileName = entry.name;
                }
            }
            lastClickTime = now;
        }
    }

    EndScissorMode();

    // ---- Scrollbar ----
    if (total > visible) {
        float barX = x + w - 6;
        float barY = listY + 2;
        float barH = listH - 4;
        float thumbH = (float)visible / total * barH;
        if (thumbH < 12) thumbH = 12;
        float scrollRatio = (float)start / (total - visible);
        float thumbY = barY + scrollRatio * (barH - thumbH);
        
        DrawScaledRect(barX, barY, 3, barH, Color{30, 35, 50, 100});
        DrawScaledRect(barX, thumbY, 3, thumbH, Color{0, 220, 240, 120});
    }

    // ---- Empty state ----
    if (total == 0) {
        float centerX = x + w / 2;
        float centerY = listY + listH / 2 - 20;
        DrawScaledText("📂", centerX - 16, centerY - 10, 32, Color{60, 70, 90, 100});
        DrawScaledText("This folder is empty", centerX - 70, centerY + 30, 12, Color{60, 70, 90, 120});
        DrawScaledText("Create files in vfs_root/", centerX - 70, centerY + 50, 10, Color{50, 60, 80, 100});
    }

    // ---- Mouse wheel scrolling ----
    float wheel = GetMouseWheelMove();
    if (RefRectHover(x, listY, w, listH, refMouse) && wheel != 0) {
        desktop.m_fileManager.scrollOffset -= (int)(wheel * 3);
        if (desktop.m_fileManager.scrollOffset < 0) desktop.m_fileManager.scrollOffset = 0;
        int maxScroll = total - visible;
        if (maxScroll < 0) maxScroll = 0;
        if (desktop.m_fileManager.scrollOffset > maxScroll) desktop.m_fileManager.scrollOffset = maxScroll;
    }
}

// ============================================================
// STATUS BAR
// ============================================================

static void DrawStatusBar(Desktop& desktop, float x, float y, float w, float h) {
    auto entries = desktop.m_vfs.ListDirectory(desktop.m_fileManager.currentPath);
    
    // Background
    DrawScaledRect(x, y, w, h, Color{10, 12, 20, 220});
    DrawScaledLine(x, y, x + w, y, Color{30, 35, 50, 80});

    // Item count
    char itemStr[64];
    snprintf(itemStr, sizeof(itemStr), "📊 %zu items", entries.size());
    DrawScaledText(itemStr, x + 12, y + 6, 9, COLOR_GHOST);

    // Total size
    uint64_t totalSize = 0;
    int folderCount = 0;
    for (const auto& e : entries) {
        if (e.isDirectory) folderCount++;
        else totalSize += e.size;
    }
    char sizeStr[64];
    snprintf(sizeStr, sizeof(sizeStr), "💾 %s used", FormatSize(totalSize).c_str());
    DrawScaledText(sizeStr, x + 140, y + 6, 9, COLOR_GHOST);

    // Folder count
    char folderStr[64];
    snprintf(folderStr, sizeof(folderStr), "📁 %d folders", folderCount);
    DrawScaledText(folderStr, x + 290, y + 6, 9, COLOR_GHOST);

    // Last modified (find newest file)
    std::string newest = "N/A";
    std::string newestTime = "";
    for (const auto& e : entries) {
        if (!e.isDirectory && e.modified > newestTime) {
            newestTime = e.modified;
            newest = e.name;
        }
    }
    if (newest != "N/A") {
        char lastStr[128];
        snprintf(lastStr, sizeof(lastStr), "📅 Latest: %s (%s)", newest.c_str(), newestTime.c_str());
        float lastW = MeasureScaledTextWidth(lastStr, 9);
        DrawScaledText(lastStr, x + w - lastW - 12, y + 6, 9, Color{60, 70, 90, 150});
    }

    // VCOIN indicator (right side)
    char vcoinStr[32];
    snprintf(vcoinStr, sizeof(vcoinStr), "💰 %.2f VCOIN", g_player.vcoin);
    float vcoinW = MeasureScaledTextWidth(vcoinStr, 9);
    if (vcoinW < 100) DrawScaledText(vcoinStr, x + w - 12, y + 6, 9, COLOR_TOXIC);
}

// ============================================================
// FILE CONTENT VIEWER (Overlay)
// ============================================================

static void DrawFileContentViewer(Desktop& desktop, float cx, float cy, float cw, float ch) {
    float overlayX = cx + 30;
    float overlayY = cy + 30;
    float overlayW = cw - 60;
    float overlayH = ch - 60;

    // ---- Background with shadow ----
    DrawScaledRect(overlayX + 4, overlayY + 4, overlayW, overlayH, Color{0, 0, 0, 120});
    DrawScaledRect(overlayX, overlayY, overlayW, overlayH, Color{4, 6, 14, 240});
    DrawScaledRectLines(overlayX, overlayY, overlayW, overlayH, COLOR_BORDER);

    // ---- Title bar ----
    float titleH = 34.0f;
    DrawScaledRect(overlayX, overlayY, overlayW, titleH, Color{10, 12, 20, 220});
    DrawScaledLine(overlayX, overlayY + titleH, overlayX + overlayW, overlayY + titleH, Color{30, 35, 50, 100});

    // Title
    char title[128];
    snprintf(title, sizeof(title), "📄 %s", desktop.m_fileManager.currentFileName.c_str());
    DrawScaledText(title, overlayX + 14, overlayY + 8, 12, COLOR_TOXIC);

    // File size in title
    auto entries = desktop.m_vfs.ListDirectory(desktop.m_fileManager.currentPath);
    for (const auto& e : entries) {
        if (e.name == desktop.m_fileManager.currentFileName) {
            char sizeInfo[32];
            snprintf(sizeInfo, sizeof(sizeInfo), "(%s)", FormatSize(e.size).c_str());
            float sizeW = MeasureScaledTextWidth(sizeInfo, 9);
            DrawScaledText(sizeInfo, overlayX + overlayW - sizeW - 40, overlayY + 9, 9, Color{60, 70, 90, 150});
            break;
        }
    }

    // ---- Close button ----
    float closeX = overlayX + overlayW - 30;
    float closeY = overlayY + 6;
    Vector2 refMouse = GetRefMousePos();
    bool closeHover = RefRectHover(closeX, closeY, 22, 22, refMouse);
    
    DrawScaledRect(closeX, closeY, 22, 22, closeHover ? Color{220, 20, 40, 150} : Color{0,0,0,0});
    DrawScaledRectLines(closeX, closeY, 22, 22, closeHover ? COLOR_BLOOD : Color{30, 35, 50, 80});
    DrawScaledText("✕", closeX + 5, closeY + 3, 13, closeHover ? COLOR_BLACK : COLOR_GHOST);
    
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && closeHover) {
        desktop.m_fileManager.showingFileContent = false;
        desktop.m_fileManager.currentFileContent.clear();
    }
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !RefRectHover(overlayX, overlayY, overlayW, overlayH, refMouse)) {
        desktop.m_fileManager.showingFileContent = false;
        desktop.m_fileManager.currentFileContent.clear();
    }

    // ---- Content area ----
    float contentX = overlayX + 12;
    float contentY = overlayY + titleH + 8;
    float contentW = overlayW - 24;
    float contentH = overlayH - titleH - 16;

    BeginScissorMode((int)SX(contentX), (int)SY(contentY),
                     (int)(contentW * g_uiScale), (int)(contentH * g_uiScale));

    // Split lines
    std::vector<std::string> lines;
    std::stringstream ss(desktop.m_fileManager.currentFileContent);
    std::string line;
    while (std::getline(ss, line)) {
        lines.push_back(line);
    }

    float lineHeight = 18.0f;
    int visibleLines = (int)(contentH / lineHeight);
    static int contentScroll = 0;
    
    float wheel = GetMouseWheelMove();
    if (RefRectHover(contentX, contentY, contentW, contentH, refMouse) && wheel != 0) {
        contentScroll -= (int)(wheel * 3);
        if (contentScroll < 0) contentScroll = 0;
        int maxScroll = (int)lines.size() - visibleLines;
        if (maxScroll < 0) maxScroll = 0;
        if (contentScroll > maxScroll) contentScroll = maxScroll;
    }

    // Draw line numbers
    for (int i = contentScroll; i < (int)lines.size() && i < contentScroll + visibleLines; i++) {
        char numStr[8];
        snprintf(numStr, sizeof(numStr), "%4d", i + 1);
        DrawScaledText(numStr, contentX, contentY + (i - contentScroll) * lineHeight, 8, Color{40, 50, 70, 100});
        DrawScaledText(lines[i].c_str(), contentX + 40, contentY + (i - contentScroll) * lineHeight, 10, COLOR_GHOST);
    }

    // Scrollbar for content
    if ((int)lines.size() > visibleLines) {
        float barX = contentX + contentW - 6;
        float barY = contentY + 2;
        float barH = contentH - 4;
        float thumbH = (float)visibleLines / lines.size() * barH;
        if (thumbH < 12) thumbH = 12;
        float scrollRatio = (float)contentScroll / (lines.size() - visibleLines);
        float thumbY = barY + scrollRatio * (barH - thumbH);
        
        DrawScaledRect(barX, barY, 3, barH, Color{30, 35, 50, 100});
        DrawScaledRect(barX, thumbY, 3, thumbH, Color{0, 220, 240, 120});
    }

    EndScissorMode();

    // ---- Line count at bottom ----
    char lineCountStr[64];
    snprintf(lineCountStr, sizeof(lineCountStr), "📄 %zu lines", lines.size());
    float lcW = MeasureScaledTextWidth(lineCountStr, 8);
    DrawScaledText(lineCountStr, overlayX + overlayW - lcW - 14, overlayY + overlayH - 16, 8, Color{60, 70, 90, 120});
}

// ============================================================
// HELPER: Format file size
// ============================================================

static std::string FormatSize(uint64_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = (double)bytes;
    
    while (size >= 1024.0 && unitIndex < 4) {
        size /= 1024.0;
        unitIndex++;
    }
    
    char buffer[32];
    if (unitIndex == 0) {
        snprintf(buffer, sizeof(buffer), "%llu %s", (unsigned long long)bytes, units[unitIndex]);
    } else {
        snprintf(buffer, sizeof(buffer), "%.1f %s", size, units[unitIndex]);
    }
    return std::string(buffer);
}