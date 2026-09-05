#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

// ============================================================
// VNET SITE DATA
// ============================================================

struct VNETSite {
    char id[64];
    char title[128];
    char category[32];
    char* content[50];  // Page lines
    int contentCount;
    float mapX;
    float mapY;
    bool hasKey;
    float hackDifficulty;
};

// ============================================================
// PLAYER STRUCT (from your original code)
// ============================================================

struct Player {
    char handle[32];
    uint32_t port;
    char ip[16];
    float vcoin;
    int traceLevel;
    int iceShields;
    char currentURL[64];
    char prevURL[64];
    char inputBuffer[256];
    char chatBuffer[256];

    // Keys
    int keysFound[8];
    int keysCount;

    // Assigned sites
    char assignedSites[54][64];
    int assignedCount;

    // Sniffer
    float sniffFreqs[10];
    int sniffCount;

    int snifferMode;
    int bitShiftOffset;
    bool hexStreamLocked;

    // Cooldowns
    float cdDOS;
    float cdProbe;
    float cdSpike;
    float cdSnoop;
    float cdMine;
    float cdPatch;
    float cdScan;
    float cdDecoy;
    float cdProxy;
    float cdOverload;
    float cdSatscan;
    float cdIon;
    float cdRedirect; 

    // UI State
    bool cliOpen;
    float cliScroll;
    float feedScroll;
    float pageScroll;
    bool urlFocused;
    bool chatFocused;
    bool handleFocused;

    // Game state
    bool isConnecting;
    float connectTimer;
    float targetConnectTime;
    char pendingURL[64];
    bool gameOver;
    char winMode[32];

    // Status
    float crtHeat;
    float neuralParanoia;
    float runTime;
    float heartbeatTimer;
    float dosTimer;
    float glitchTrigger;
    
    bool isInConnectionMenu;  // true when showing connection screen
    bool ipBoxFocused;        // true when IP input box is focused
    char ipInputBuffer[64];   // IP address string
};

// ============================================================
// VNET SYSTEM
// ============================================================

struct VNETSystem {
    VNETSite sites[50];
    int siteCount;
    char masterKeys[8][32];
    char keyLocations[8][64];
    int currentSalt;
    int targetHash;
    char hashedSites[50][64];
    float serverUptime;
    bool gameOver;
    char winnerHandle[32];
};

// ============================================================
// GLOBAL REFERENCES
// ============================================================

extern Player g_player;
extern VNETSystem g_vnet;
extern char g_cliLogs[250][256];
extern int g_cliLogCount;
extern char g_feedLogs[100][256];
extern int g_feedLogCount;
extern char g_minedBlocks[50][64];
extern int g_minedCount;

// ============================================================
// MARKUP PAGE CONTENT (rendered by the tag-based UI renderer)
// ============================================================
// This is the C++ equivalent of `page_body :: Array` in the Vyne script.
// Each entry is one raw markup line, e.g. "[TITLE] VNET DIRECTORY",
// "[LINK:market.vnet] >> BLACK MARKET", "[GAUGE:78:CONGESTION]", etc.
// LoadPage() (to be implemented in vnet.cpp / a future pass) fills this in;
// for now it can be populated with a placeholder directory page so the
// renderer has something real to draw.
extern std::vector<std::string> g_pageLines;

// ============================================================
// FUNCTION PROTOTYPES
// ============================================================

// Init/Shutdown
void InitVNETSystem(void);
void ShutdownVNETSystem(void);

// Update
void UpdateVNET(float dt);

// Site management
VNETSite* GetSite(const char* url);
void LoadPage(const char* url);
void RefreshPage(void);

// Commands
void ProcessCommand(const char* cmd);
void ProcessChatCommand(const char* msg);
void ProcessWhisperCommand(const char* target, const char* msg);

// Utility
void PushCliLog(const char* fmt, ...);
void PushFeedLog(const char* fmt, ...);
void TriggerRouteNavigation(const char* url);
void UpdatePeerSession(uint32_t port, const char* ip, const char* url);

// Mining
void StartMining(void);
void UpdateMining(float dt);