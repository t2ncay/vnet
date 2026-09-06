#include "vex_parser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>

// ============================================================
// VEX PARSER IMPLEMENTATION
// ============================================================

VEXParser::VEXParser() {
    // Initialize
}

VEXParser::~VEXParser() {
    Clear();
}

void VEXParser::Clear() {
    m_sites.clear();
    m_siteIndex.clear();
}

void VEXParser::TrimString(std::string& str) {
    // Remove leading/trailing whitespace
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        str.clear();
        return;
    }
    size_t end = str.find_last_not_of(" \t\n\r");
    str = str.substr(start, end - start + 1);
}

bool VEXParser::StartsWith(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

bool VEXParser::ParseLine(const std::string& line, VEXSiteData& data, bool& inContent) {
    std::string trimmed = line;
    TrimString(trimmed);
    
    if (trimmed.empty()) return true;

    // --- CONTENT BLOCK ---
    if (trimmed == "[CONTENT]") {
        inContent = true;
        return true;
    }
    if (trimmed == "[/CONTENT]") {
        inContent = false;
        return true;
    }

    // If we're in content block, just store the line
    if (inContent) {
        data.content.push_back(line);  // Keep original formatting
        return true;
    }

    // --- METADATA PARSING ---
    if (StartsWith(trimmed, "[TITLE]")) {
        data.title = trimmed.substr(7);
        TrimString(data.title);
        return true;
    }

    if (StartsWith(trimmed, "[CATEGORY]")) {
        data.category = trimmed.substr(10);
        TrimString(data.category);
        return true;
    }

    if (StartsWith(trimmed, "[ID]")) {
        data.id = trimmed.substr(4);
        TrimString(data.id);
        return true;
    }

    if (StartsWith(trimmed, "[MAP_X]")) {
        std::string val = trimmed.substr(7);
        TrimString(val);
        data.mapX = std::stof(val);
        return true;
    }

    if (StartsWith(trimmed, "[MAP_Y]")) {
        std::string val = trimmed.substr(7);
        TrimString(val);
        data.mapY = std::stof(val);
        return true;
    }

    if (StartsWith(trimmed, "[HAS_KEY]")) {
        std::string val = trimmed.substr(9);
        TrimString(val);
        data.hasKey = (val == "true" || val == "1" || val == "TRUE");
        return true;
    }

    if (StartsWith(trimmed, "[HACK_DIFFICULTY]")) {
        std::string val = trimmed.substr(18);
        TrimString(val);
        data.hackDifficulty = std::stof(val);
        return true;
    }

    // Unknown tag - ignore
    return true;
}

bool VEXParser::LoadSiteFile(const std::string& path, VEXSiteData& outData) {
    std::ifstream file(path);
    if (!file.is_open()) {
        printf("[VEX] Failed to open: %s\n", path.c_str());
        return false;
    }

    VEXSiteData data;
    data.mapX = 0.0f;
    data.mapY = 0.0f;
    data.hasKey = false;
    data.hackDifficulty = 1.0f;
    
    bool inContent = false;
    std::string line;
    int lineNum = 0;

    while (std::getline(file, line)) {
        lineNum++;
        if (!ParseLine(line, data, inContent)) {
            printf("[VEX] Parse error at line %d: %s\n", lineNum, line.c_str());
            return false;
        }
    }

    file.close();

    // Validate required fields
    if (data.id.empty()) {
        // Extract filename as ID if not set
        size_t lastSlash = path.find_last_of("/\\");
        std::string filename = (lastSlash != std::string::npos) ? path.substr(lastSlash + 1) : path;
        size_t dot = filename.find_last_of('.');
        data.id = (dot != std::string::npos) ? filename.substr(0, dot) : filename;

        // Site URLs in-game are always referenced with a ".vnet" suffix
        // (e.g. "[LINK:market.vnet]"), so the derived ID must match that,
        // otherwise GetSiteData() lookups miss and every page 404s.
        if (data.id.size() < 5 || data.id.compare(data.id.size() - 5, 5, ".vnet") != 0) {
            data.id += ".vnet";
        }
    }

    if (data.title.empty()) {
        data.title = data.id;
    }

    if (data.category.empty()) {
        data.category = "hub";
    }

    outData = data;
    return true;
}

bool VEXParser::LoadAllSites(const std::string& directory, std::vector<VEXSiteData>& outSites) {
    Clear();
    outSites.clear();

    printf("[VEX] Loading sites from: %s\n", directory.c_str());

    // Use std::filesystem to iterate directory
    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".vex") {
                std::string path = entry.path().string();
                VEXSiteData data;
                if (LoadSiteFile(path, data)) {
                    m_sites.push_back(data);
                    m_siteIndex[data.id] = (int)m_sites.size() - 1;
                    outSites.push_back(data);
                    printf("[VEX] Loaded: %s (%zu lines)\n", data.id.c_str(), data.content.size());
                }
            }
        }
    } catch (const std::exception& e) {
        printf("[VEX] Error reading directory: %s\n", e.what());
        return false;
    }

    // Sort sites by ID for consistency (vnet.dir is handled separately)
    std::sort(m_sites.begin(), m_sites.end(), [](const VEXSiteData& a, const VEXSiteData& b) {
        return a.id < b.id;
    });
    std::sort(outSites.begin(), outSites.end(), [](const VEXSiteData& a, const VEXSiteData& b) {
        return a.id < b.id;
    });

    // Rebuild index
    m_siteIndex.clear();
    for (int i = 0; i < (int)m_sites.size(); i++) {
        m_siteIndex[m_sites[i].id] = i;
    }

    printf("[VEX] Loaded %zu sites\n", m_sites.size());
    return !m_sites.empty();
}

void VEXParser::ConvertToVNETSite(const VEXSiteData& vexData, VNETSite& outSite) {
    memset(&outSite, 0, sizeof(VNETSite));

    strncpy(outSite.id, vexData.id.c_str(), 63);
    outSite.id[63] = '\0';

    strncpy(outSite.title, vexData.title.c_str(), 127);
    outSite.title[127] = '\0';

    strncpy(outSite.category, vexData.category.c_str(), 31);
    outSite.category[31] = '\0';

    // Copy content lines
    int j = 0;
    for (const std::string& line : vexData.content) {
        if (j >= 50) break;
        size_t len = line.length();
        outSite.content[j] = (char*)malloc(len + 1);
        if (outSite.content[j]) {
            strcpy(outSite.content[j], line.c_str());
        }
        j++;
    }
    outSite.contentCount = j;

    outSite.mapX = vexData.mapX;
    outSite.mapY = vexData.mapY;
    outSite.hasKey = vexData.hasKey;
    outSite.hackDifficulty = vexData.hackDifficulty;
}

const VEXSiteData* VEXParser::GetSiteData(const std::string& url) const {
    auto it = m_siteIndex.find(url);
    if (it != m_siteIndex.end()) {
        return &m_sites[it->second];
    }
    return nullptr;
}