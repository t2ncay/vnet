#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

// ============================================================
// VNET SITE DATA
// ============================================================

struct VNETSite {
    // ---- POINTERS / LARGE (8 bytes on 64-bit) ----
    char* content[50];      // 50 * 8 = 400 bytes (pointers)
    
    // ---- FLOATS (4 bytes) ----
    float mapX;             // 4 bytes
    float mapY;             // 4 bytes
    float hackDifficulty;   // 4 bytes
    
    // ---- INTEGERS (4 bytes) ----
    int contentCount;       // 4 bytes
    
    // ---- BOOLS (1 byte, grouped together) ----
    bool hasKey;            // 1 byte
    // 3 bytes padding here
    
    // ---- CHAR ARRAYS (aligned to 1 byte, but placed last) ----
    char id[64];            // 64 bytes
    char title[128];        // 128 bytes
    char category[32];      // 32 bytes
};
// Total: ~648 bytes (was ~652 bytes, saved 4 bytes)

// ============================================================
// PLAYER STRUCT (from your original code)
// ============================================================

struct Player {
    // ============================================================
    // 8-BYTE ALIGNMENT (Pointers, doubles, long long)
    // ============================================================
    // (None currently, but keep for future)
    
    // ============================================================
    // 4-BYTE ALIGNMENT (floats, ints, uint32_t)
    // ============================================================
    // ---- FLOATS (4 bytes) ----
    float vcoin;                    // 4
    float crtHeat;                  // 4
    float neuralParanoia;           // 4
    float runTime;                  // 4
    float heartbeatTimer;           // 4
    float dosTimer;                 // 4
    float glitchTrigger;            // 4
    float cliScroll;                // 4
    float feedScroll;               // 4
    float pageScroll;               // 4
    float connectTimer;             // 4
    float targetConnectTime;        // 4
    float raidTimer;                // 4
    float raidResponseTime;         // 4
    float raidInputTimer;           // 4
    float raidCooldown;             // 4
    float raidAlertTimer;           // 4
    float raidSeqTimer;             // 4
    float raidSeqStageDuration;     // 4
    float raidSeqDialogueTimer;     // 4
    float siteOverloadTimer;        // 4
    float siteOverloadTotal;        // 4
    float overloadScanlineOffset;   // 4
    float overloadGlitchIntensity;  // 4
    float cdDOS;                    // 4
    float cdProbe;                  // 4
    float cdSpike;                  // 4
    float cdSnoop;                  // 4
    float cdMine;                   // 4
    float cdPatch;                  // 4
    float cdScan;                   // 4
    float cdDecoy;                  // 4
    float cdProxy;                  // 4
    float cdOverload;               // 4
    float cdSatscan;                // 4
    float cdIon;                    // 4
    float cdRedirect;               // 4
    // ---- INTEGERS (4 bytes) ----
    uint32_t port;                  // 4
    int traceLevel;                 // 4
    int iceShields;                 // 4
    int keysFound[8];               // 8 * 4 = 32
    int keysCount;                  // 4
    int assignedCount;              // 4
    int minedBlockCount;            // 4
    int sniffCount;                 // 4
    int snifferMode;                // 4
    int bitShiftOffset;             // 4
    int raidType;                   // 4
    int raidStage;                  // 4
    int raidCount;                  // 4
    int raidFailedCount;            // 4
    int raidSeqStage;               // 4
    
    // ============================================================
    // 1-BYTE ALIGNMENT (bools) - GROUPED TOGETHER
    // ============================================================
    bool cliOpen;                   // 1
    bool urlFocused;                // 1
    bool chatFocused;               // 1
    bool handleFocused;             // 1
    bool isConnecting;              // 1
    bool gameOver;                  // 1
    bool hexStreamLocked;           // 1
    bool raidActive;                // 1
    bool raidSuccess;               // 1
    bool isFlagged;                 // 1
    bool raidIntruderVisible;       // 1
    bool raidSeqLockDesktop;        // 1
    bool raidSeqShowOperator;       // 1
    bool isInConnectionMenu;        // 1
    bool ipBoxFocused;              // 1
    bool siteOverloaded;            // 1
    // 7 bytes padding after bools (to align next 8-byte boundary)
    
    // ============================================================
    // CHARACTER ARRAYS (1-byte alignment, placed last)
    // ============================================================
    char handle[32];                // 32
    char ip[16];                    // 16
    char currentURL[64];            // 64
    char prevURL[64];               // 64
    char inputBuffer[256];          // 256
    char chatBuffer[256];           // 256
    char assignedSites[54][64];     // 54 * 64 = 3456
    char minedBlockIds[50][32];     // 50 * 32 = 1600
    float sniffFreqs[10];           // 10 * 4 = 40 (floats, already in 4-byte section)
    char pendingURL[64];            // 64
    char winMode[32];               // 32
    char raidInput[64];             // 64
    char raidSeqOperatorDialogue[256]; // 256
    char overloadedSite[64];        // 64
    char ipInputBuffer[64];         // 64
};

// Total size: ~ (floats 44*4=176 + ints ~ 200 + bools 16 + char arrays ~ 5376)
// ≈ 5768 bytes (was ~5900+, saved ~130+ bytes)

// ============================================================
// VNET SYSTEM
// ============================================================

struct VNETSystem {
    // ---- 8-BYTE ALIGNMENT ----
    // (Pointers, if any)
    
    // ---- 4-BYTE ALIGNMENT ----
    int siteCount;                  // 4
    int currentSalt;                // 4
    int targetHash;                 // 4
    float serverUptime;             // 4
    
    // ---- 1-BYTE ALIGNMENT (bools) ----
    bool gameOver;                  // 1
    // 3 bytes padding
    
    // ---- CHARACTER ARRAYS ----
    char masterKeys[8][32];         // 8 * 32 = 256
    char keyLocations[8][64];       // 8 * 64 = 512
    char hashedSites[50][64];       // 50 * 64 = 3200
    char winnerHandle[32];          // 32
    VNETSite sites[50];             // 50 * 648 = 32400
};
// Total: ~36408 bytes (was ~36420, saved ~12 bytes)

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
void StartMining(const char* blockId = nullptr);
void UpdateMining(float dt);