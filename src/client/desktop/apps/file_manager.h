#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

struct FileEntry {
    std::string name;
    bool isDirectory;
    uint64_t size;
    std::string modified;
};

class VFSManager {
public:
    void SetRoot(const std::string& path);
    std::vector<FileEntry> ListDirectory(const std::string& relPath) const;
    bool IsDirectory(const std::string& relPath) const;
    std::string ReadFile(const std::string& relPath) const;
    std::string GetAbsolutePath(const std::string& relPath) const;

    // Stat a single path (file or directory) without listing its parent.
    // Returns false if the path doesn't exist.
    bool GetEntry(const std::string& relPath, FileEntry& out) const;

private:
    std::string m_rootPath;
};

// Forward declarations – no #include "desktop.h"
struct AppWindow;
class Desktop;

// UI drawing function
void DrawFileManagerUI(const AppWindow& win, Desktop& desktop);