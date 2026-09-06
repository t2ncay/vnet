#pragma once
#include "vnet.h"
#include "vex_parser.h"
#include <vector>
#include <string>
#include <unordered_map>

// ============================================================
// VNET SITES - NOW USING .vex FILES
// ============================================================

// Global site manager
class SiteManager {
public:
    static SiteManager& Get() {
        static SiteManager instance;
        return instance;
    }

    bool Initialize(const std::string& sitesDirectory);
    void Shutdown();

    // Get site data by URL
    const VEXSiteData* GetSiteData(const std::string& url) const;
    const VNETSite* GetVNETSite(const std::string& url) const;

    // Get all sites
    const std::vector<VEXSiteData>& GetAllSites() const { return m_vexSites; }
    const std::vector<VNETSite>& GetAllVNETSites() const { return m_vnetSites; }

    // Load page content into vector (vnet.dir is handled here too)
    void LoadPageContent(const std::string& url, std::vector<std::string>& pageLines, 
                         Player& player, VNETSystem& vnet);

    // Conversion function (exposed for backward compatibility)
    static void ConvertSite(const VEXSiteData& vexData, VNETSite& outSite);

private:
    SiteManager() = default;
    ~SiteManager() = default;
    SiteManager(const SiteManager&) = delete;
    SiteManager& operator=(const SiteManager&) = delete;

    VEXParser m_parser;
    std::vector<VEXSiteData> m_vexSites;
    std::vector<VNETSite> m_vnetSites;
    std::unordered_map<std::string, int> m_siteIndex;
    bool m_initialized = false;
};

// ============================================================
// LEGACY FUNCTIONS (for backward compatibility)
// ============================================================

void InitAllSites(VNETSystem& vnet);
void LoadPageContent(const char* url, std::vector<std::string>& pageLines, 
                     Player& player, VNETSystem& vnet);