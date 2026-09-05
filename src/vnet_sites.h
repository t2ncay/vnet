#pragma once
#include "vnet.h"
#include <vector>
#include <string>

// ============================================================
// 50 VNET SITES - DATA STRUCTURE
// ============================================================

struct VNETPageData {
    const char* id;
    const char* title;
    const char* category;
    const char** content;
    float mapX;
    float mapY;
    bool hasKey;
    float hackDifficulty;
};

// ============================================================
// FUNCTION PROTOTYPES
// ============================================================

void InitAllSites(VNETSystem& vnet);
void LoadPageContent(const char* url, std::vector<std::string>& pageLines, Player& player, VNETSystem& vnet);
const VNETPageData* GetSiteData(const char* url);