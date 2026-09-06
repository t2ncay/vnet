#pragma once
#include "vnet.h"
#include <string>
#include <vector>
#include <unordered_map>

// ============================================================
// VEX FILE PARSER - .vex Site File Format
// ============================================================

struct VEXSiteData {
    std::string id;
    std::string title;
    std::string category;
    std::vector<std::string> content;
    float mapX;
    float mapY;
    bool hasKey;
    float hackDifficulty;
};

class VEXParser {
public:
    VEXParser();
    ~VEXParser();

    // Load a single .vex file
    bool LoadSiteFile(const std::string& path, VEXSiteData& outData);

    // Load all .vex files from a directory
    bool LoadAllSites(const std::string& directory, std::vector<VEXSiteData>& outSites);

    // Convert VEXSiteData to VNETSite (for game use)
    static void ConvertToVNETSite(const VEXSiteData& vexData, VNETSite& outSite);

    // Get the site data for a URL
    const VEXSiteData* GetSiteData(const std::string& url) const;

    // Get all site data
    const std::vector<VEXSiteData>& GetAllSites() const { return m_sites; }

    // Clear all loaded sites
    void Clear();

private:
    std::vector<VEXSiteData> m_sites;
    std::unordered_map<std::string, int> m_siteIndex;  // url -> index

    // Helper parsing functions
    bool ParseLine(const std::string& line, VEXSiteData& data, bool& inContent);
    void TrimString(std::string& str);
    bool StartsWith(const std::string& str, const std::string& prefix);
};