#pragma once
#include "raylib.h"
#include <string>
#include <vector>

// ============================================================
// DUEL ENUMS
// ============================================================

enum class DuelRole {
    ATTACKER,
    DEFENDER
};

enum class DuelState {
    IDLE,
    INITIATING,
    STARTING,
    ACTIVE,
    FLAG_CAPTURED,
    COMPLETE
};

enum class DuelObjectiveType {
    SCAN,
    PORTSCAN,
    EXPLOIT,
    EXTRACT,
    ESCALATE,
    CAPTURE,
    TRACE,
    WHOIS,
    DECOY,
    COUNTER,
    PATCH
};

// ============================================================
// DUEL OBJECTIVE STRUCT
// ============================================================

struct DuelObjective {
    DuelObjectiveType type;
    std::string name;
    std::string description;
    std::string command;          // CLI command to complete it
    float cost;                   // VCOIN cost
    float difficulty;             // 0.0 - 1.0
    bool completed;
    float progress;               // 0.0 - 1.0 (for partial attempts)
    std::vector<std::string> hints;
};

// ============================================================
// DUEL PLAYER STATE
// ============================================================

struct DuelPlayer {
    std::string handle;
    int port;
    int trace;
    int iceShields;
    float vcoin;
    std::vector<DuelObjective> objectives;
    int objectivesCompleted;
    float progress;               // 0-100
    bool flagCaptured;
    float lastActionTime;
};

// ============================================================
// MAIN DUEL STATE
// ============================================================

struct Duel {
    bool active;
    DuelState state;
    float introTimer;
    float timer;
    float maxDuration;            // 45 seconds
    DuelRole localRole;
    std::string attackerHandle;
    std::string defenderHandle;
    int attackerPort;
    int defenderPort;
    DuelPlayer attacker;
    DuelPlayer defender;
    
    // Visual state
    float hexOffset;
    float scanlineOffset;
    float glitchIntensity;
    bool showSniffer;
    bool showNetworkMap;
    bool showTerminal;
    float terminalScroll;
    
    // Action log
    std::vector<std::string> actionLog;
    
    // Flag capture
    bool flagCaptured;
    std::string flagCapturedBy;
    float flagCaptureTime;
};

// ============================================================
// GLOBAL DUEL INSTANCE
// ============================================================

extern Duel g_duel;
extern char g_duelInputBuffer[256];

// ============================================================
// FUNCTION PROTOTYPES
// ============================================================

void InitDuelSystem(void);
void UpdateDuel(float dt);
void DrawDuelIntro(float dt);
void DrawDuelScreen(void);
bool IsDuelActive(void);

// Commands (called from vnet.cpp)
void ProcessDuelCommand(const char* cmd);
void DuelHack(const char* target);
void DuelScan(void);
void DuelPortScan(const char* port);
void DuelExploit(void);
void DuelExtract(void);
void DuelEscalate(void);
void DuelCapture(void);
void DuelTrace(void);
void DuelWhois(void);
void DuelDecoy(void);
void DuelCounter(void);
void DuelPatch(void);

// Server communication
void SendDuelAction(const std::string& action, const std::string& payload = "");
void ReceiveDuelPacket(const std::string& cmd, const std::string& payload);

// Utility
void GenerateObjectives(DuelPlayer& player, DuelRole role);
bool CompleteObjective(DuelPlayer& player, DuelObjectiveType type);
void AdvanceDuelProgress(DuelPlayer& player);
void ApplyDuelRewards(bool attackerWon);