#include "vnet_sites.h"
#include "vnet.h"
#include <cstring>
#include <cstdio>
#include <filesystem>

// ============================================================
// SITE MANAGER IMPLEMENTATION
// ============================================================

bool SiteManager::Initialize(const std::string& sitesDirectory) {
    if (m_initialized) return true;

    // Check if directory exists
    if (!std::filesystem::exists(sitesDirectory)) {
        printf("[SiteManager] Directory not found: %s\n", sitesDirectory.c_str());
        return false;
    }

    // Load all .vex files
    if (!m_parser.LoadAllSites(sitesDirectory, m_vexSites)) {
        printf("[SiteManager] Failed to load sites from: %s\n", sitesDirectory.c_str());
        return false;
    }

    // Convert to VNETSite format
    m_vnetSites.clear();
    m_vnetSites.reserve(m_vexSites.size());
    m_siteIndex.clear();

    for (int i = 0; i < (int)m_vexSites.size(); i++) {
        VNETSite site;
        ConvertSite(m_vexSites[i], site);
        m_vnetSites.push_back(site);
        m_siteIndex[m_vexSites[i].id] = i;
    }

    m_initialized = true;
    printf("[SiteManager] Initialized with %zu sites\n", m_vnetSites.size());
    return true;
}

void SiteManager::Shutdown() {
    // Clean up allocated content in VNETSite
    for (auto& site : m_vnetSites) {
        for (int i = 0; i < site.contentCount; i++) {
            if (site.content[i]) {
                free(site.content[i]);
                site.content[i] = nullptr;
            }
        }
        site.contentCount = 0;
    }
    m_vnetSites.clear();
    m_vexSites.clear();
    m_siteIndex.clear();
    m_initialized = false;
}

void SiteManager::ConvertSite(const VEXSiteData& vexData, VNETSite& outSite) {
    VEXParser::ConvertToVNETSite(vexData, outSite);
}

const VEXSiteData* SiteManager::GetSiteData(const std::string& url) const {
    return m_parser.GetSiteData(url);
}

const VNETSite* SiteManager::GetVNETSite(const std::string& url) const {
    auto it = m_siteIndex.find(url);
    if (it != m_siteIndex.end()) {
        return &m_vnetSites[it->second];
    }
    return nullptr;
}

void SiteManager::LoadPageContent(const std::string& url, std::vector<std::string>& pageLines,
                                   Player& player, VNETSystem& vnet) {
    pageLines.clear();

    // ============================================================
    // SPECIAL HANDLING FOR vnet.dir (HOME PAGE)
    // ============================================================
    if (url == "vnet.dir" || url == "vnet://vnet.dir") {
        // Build vnet.dir dynamically - it's NOT in .vex files
        // This is handled in LoadPageContent function below
        // We'll keep the existing logic in the legacy function
        return;
    }

    const VEXSiteData* site = GetSiteData(url);
    if (!site) {
        // 404 page
        pageLines.push_back("[TITLE] 404 // ROUTE_CORRUPTED - SECTOR NULL");
        pageLines.push_back("[HR]");
        pageLines.push_back("[BADGE:NODE VOID:BLOOD] [BADGE:SIGNAL LOST:AMBER]");
        pageLines.push_back("[BOX] +-----------------------------------------------------------------+");
        pageLines.push_back("[BOX] | ERROR CODE : 0x404_VFS_SEGFAULT_UNALLOCATED_ADDRESS_SPACE       |");
        pageLines.push_back("[BOX] | TARGET URL : vnet://" + url + "                     |");
        pageLines.push_back("[BOX] +-----------------------------------------------------------------+");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[GAUGE:100:SIGNAL_ENTROPY_DECAY]");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[SUBTITLE] DYNAMIC ROUTE RECOVERY & PACKET RE-INJECTION:");
        pageLines.push_back("[BLOOD] [CRITICAL WARNING]: UNALLOCATED SOCKET DETECTED");
        pageLines.push_back("[HR]");
        pageLines.push_back("[LINK:vnet.dir] >> ESCAPE TO MAIN DIRECTORY");
        pageLines.push_back("[HR]");
        return;
    }

    // Copy content lines from the .vex file
    for (const std::string& line : site->content) {
        std::string processed = line;

        // Replace dynamic placeholders
        if (processed.find("ICE_COUNT") != std::string::npos) {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "%d/3", player.iceShields);
            size_t pos = processed.find("ICE_COUNT");
            processed.replace(pos, 9, buffer);
        }

        if (processed.find("VCOIN_BALANCE") != std::string::npos) {
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%.2f", player.vcoin);
            size_t pos = processed.find("VCOIN_BALANCE");
            processed.replace(pos, 13, buffer);
        }

        if (processed.find("TRACE_LEVEL") != std::string::npos) {
            char buffer[16];
            snprintf(buffer, sizeof(buffer), "%d", player.traceLevel);
            size_t pos = processed.find("TRACE_LEVEL");
            processed.replace(pos, 11, buffer);
        }

        if (processed.find("HANDLE_NAME") != std::string::npos) {
            size_t pos = processed.find("HANDLE_NAME");
            processed.replace(pos, 11, player.handle);
        }

        if (processed.find("PORT_NUMBER") != std::string::npos) {
            char buffer[16];
            snprintf(buffer, sizeof(buffer), "%d", player.port);
            size_t pos = processed.find("PORT_NUMBER");
            processed.replace(pos, 11, buffer);
        }

        pageLines.push_back(processed);
    }
}

// ============================================================
// LEGACY GLOBAL FUNCTIONS
// ============================================================

void InitAllSites(VNETSystem& vnet) {
    // Initialize the site manager
    SiteManager::Get().Initialize("assets/sites");
    
    // Copy to vnet for backward compatibility
    const auto& sites = SiteManager::Get().GetAllVNETSites();
    vnet.siteCount = (int)sites.size();
    for (int i = 0; i < (int)sites.size() && i < 50; i++) {
        // Deep copy content
        strncpy(vnet.sites[i].id, sites[i].id, 63);
        vnet.sites[i].id[63] = '\0';
        strncpy(vnet.sites[i].title, sites[i].title, 127);
        vnet.sites[i].title[127] = '\0';
        strncpy(vnet.sites[i].category, sites[i].category, 31);
        vnet.sites[i].category[31] = '\0';
        // Copy content - shallow copy is fine since we don't modify it
        for (int j = 0; j < sites[i].contentCount && j < 50; j++) {
            vnet.sites[i].content[j] = sites[i].content[j];
        }
        vnet.sites[i].contentCount = sites[i].contentCount;
        vnet.sites[i].mapX = sites[i].mapX;
        vnet.sites[i].mapY = sites[i].mapY;
        vnet.sites[i].hasKey = sites[i].hasKey;
        vnet.sites[i].hackDifficulty = sites[i].hackDifficulty;
    }
}

void LoadPageContent(const char* url, std::vector<std::string>& pageLines,
                     Player& player, VNETSystem& vnet) {
    std::string urlStr = url ? url : "";
    
    // ============================================================
    // SPECIAL HANDLING FOR vnet.dir (HOME PAGE) - KEPT HERE
    // ============================================================
    if (urlStr == "vnet.dir" || urlStr == "vnet://vnet.dir") {
        pageLines.clear();
        
        pageLines.push_back("[TITLE] VNET ANONYMOUS DIRECTORY v4.09 // ROOT GATEWAY");
        pageLines.push_back("[HR]");
        pageLines.push_back("[BADGE:ROOT NODE:BLOOD] [BADGE:GLOBAL ROUTING:AMBER] [BADGE:80% COVERAGE:TOXIC]");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[GLITCH] [WARNING]: UNREGISTERED EYE CONTACT DETECTED THROUGH MONITOR GLASS.");
        pageLines.push_back("[PULSE] ALL ROUTED PACKETS ARE MIRRORED TO RESTRICTED VFS MEMORY STACKS.");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[BOX] +-----------------------------------------------------------+");
        pageLines.push_back("[BOX] | STATUS: DISCOVERING HIDDEN GATEWAYS VIA UDP TRAFFIC       |");
        pageLines.push_back("[BOX] | MISSION: SNOOP & NETSCAN TRAFFIC TO CARVE ROUTES INTO RAM |");
        
        // Show assigned count
        char assignedStr[128];
        snprintf(assignedStr, sizeof(assignedStr), "BOX] | ASSIGNED NODES : %d/20 DISCOVERED SITES               |", player.assignedCount);
        pageLines.push_back(assignedStr);
        pageLines.push_back("[BOX] +-----------------------------------------------------------+");
        pageLines.push_back("[TEXT] ");
        
        // Display assigned sites if any
        if (player.assignedCount > 0) {
            // Categorize sites
            std::vector<std::string> mutualSites;
            std::vector<std::string> randomSites;
            
            const char* mutualList[] = {
                "market.vnet", "vault.vnet", "terminal.vnet",
                "forum.vnet", "crypto.vnet", "bounty.vnet",
                "vektrapay.vnet", "hellroom.vnet", "hashbeat.vnet"
            };
            int mutualCount = sizeof(mutualList) / sizeof(mutualList[0]);
            
            for (int i = 0; i < player.assignedCount; i++) {
                bool isMutual = false;
                for (int j = 0; j < mutualCount; j++) {
                    if (strcmp(player.assignedSites[i], mutualList[j]) == 0) {
                        isMutual = true;
                        break;
                    }
                }
                if (isMutual) {
                    mutualSites.push_back(player.assignedSites[i]);
                } else {
                    randomSites.push_back(player.assignedSites[i]);
                }
            }
            
            // Mutual nodes
            pageLines.push_back("[SUBTITLE] █ MUTUAL CORE NODES [" + std::to_string(mutualSites.size()) + "/9]");
            pageLines.push_back("[TEXT] ");
            pageLines.push_back("[TEXT] These are guaranteed to every player. Overload all 5 core nodes to win.");
            pageLines.push_back("[TEXT] ");
            
            const char* coreNodes[] = {"market.vnet", "vault.vnet", "terminal.vnet", "crypto.vnet", "hellroom.vnet"};
            int coreCount = sizeof(coreNodes) / sizeof(coreNodes[0]);
            
            for (const auto& site : mutualSites) {
                bool isCore = false;
                for (int i = 0; i < coreCount; i++) {
                    if (site == coreNodes[i]) { isCore = true; break; }
                }
                std::string label = "[LINK:" + site + "] >> ";
                label += isCore ? "[CORE] " : "[MUTUAL] ";
                label += site;
                pageLines.push_back(label);
            }
            pageLines.push_back("[TEXT] ");
            pageLines.push_back("[HR]");
            
            // Random discovered
            if (!randomSites.empty()) {
                pageLines.push_back("[SUBTITLE] █ RANDOM DISCOVERED NODES [" + std::to_string(randomSites.size()) + "/11]");
                pageLines.push_back("[TEXT] ");
                for (const auto& site : randomSites) {
                    pageLines.push_back("[LINK:" + site + "] >> [DISCOVERED] " + site);
                }
                pageLines.push_back("[TEXT] ");
                pageLines.push_back("[HR]");
            } else {
                pageLines.push_back("[TEXT] No random sites discovered yet. Use 'scan' to find more.");
                pageLines.push_back("[TEXT] ");
                pageLines.push_back("[HR]");
            }
        } else {
            pageLines.push_back("[BLOOD] [WARNING]: NO SITES ASSIGNED YET!");
            pageLines.push_back("[TEXT] ");
            pageLines.push_back("[TEXT] You are connected to the server but haven't received your site directory.");
            pageLines.push_back("[TEXT] The server should send KEY_SYNC with your 20 assigned sites.");
            pageLines.push_back("[TEXT] ");
            pageLines.push_back("[TEXT] Press [TAB] and type 'scan' to discover sites manually.");
            pageLines.push_back("[TEXT] ");
            pageLines.push_back("[HR]");
        }
        
        // Player stats
        pageLines.push_back("[SUBTITLE] █ OPERATOR STATUS & SYSTEM TELEMETRY");
        pageLines.push_back("[TEXT] ");
        char stats[256];
        snprintf(stats, sizeof(stats), "HANDLE    : %s", player.handle);
        pageLines.push_back("[CODE] " + std::string(stats));
        snprintf(stats, sizeof(stats), "VCOIN     : %.2f VCOIN", player.vcoin);
        pageLines.push_back("[CODE] " + std::string(stats));
        snprintf(stats, sizeof(stats), "TRACE     : %d%%", player.traceLevel);
        pageLines.push_back("[CODE] " + std::string(stats));
        snprintf(stats, sizeof(stats), "ICE       : %d/3 LAYERS", player.iceShields);
        pageLines.push_back("[CODE] " + std::string(stats));
        snprintf(stats, sizeof(stats), "CRT HEAT  : %.0f°C", player.crtHeat);
        pageLines.push_back("[CODE] " + std::string(stats));
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[HR]");
        
        // Footer
        pageLines.push_back("[PULSE] 'THEY CAN SEE THROUGH THE CRT SCREEN... DON'T LOOK BACK.'");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[TEXT] Tip: Press [TAB] to toggle terminal overlay. Mine VCOIN at crypto.vnet.");
        pageLines.push_back("[TEXT] Type 'help' for available commands.");
        pageLines.push_back("[HR]");
        pageLines.push_back("[BLOOD] 'THE NETWORK IS ALIVE. IT IS DRINKING YOUR HEAT.'");
        return;
    }
    
    // For all other sites, use SiteManager
    SiteManager::Get().LoadPageContent(urlStr, pageLines, player, vnet);
}