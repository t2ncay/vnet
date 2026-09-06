#pragma once
#include "raylib.h"
#include <string>

// ============================================================
// FEDERAL E-RAID SYSTEM - HEADER
// ============================================================

// Raid types
enum RaidType {
    RAID_EVADE = 0,   // Deploy decoy nodes
    RAID_ESCAPE = 1,  // Port migration
    RAID_BURN = 2     // Scorched earth purge
};

// Raid stages
enum RaidStage {
    RAID_STAGE_IDLE = -1,
    RAID_STAGE_ALERT = 0,      // Notification + choice
    RAID_STAGE_CHOICE = 1,     // Player choosing strategy
    RAID_STAGE_MINIGAME = 2,   // Playing the minigame
    RAID_STAGE_RESULT = 3,     // Success/failure displayed
    RAID_STAGE_COMPLETE = 4    // Cleanup
};

// ============================================================
// FUNCTION PROTOTYPES
// ============================================================

// Core raid functions
void InitRaidSystem(void);
void UpdateRaid(float dt);
void TriggerFederalRaid(void);
void ResetRaidState(void);

// Raid minigame
void HandleRaidChoice(const char* input);
void ProcessRaidMinigameInput(const char* input);
void ApplyRaidSuccess(void);
void ApplyRaidFailure(void);

// UI
void DrawRaidOverlay(void);
void DrawIntruderDetector(float x, float y, float w, float h);
bool IsRaidActive(void);
float GetRaidRemainingTime(void);

// Commands
void ProcessRaidCommand(const char* token, const char* args, const char* cmd);

// ============================================================
// RAID TARGET FILES (for BURN minigame)
// ============================================================
extern const char* RAID_TARGET_FILES[];
extern const int RAID_TARGET_FILE_COUNT;