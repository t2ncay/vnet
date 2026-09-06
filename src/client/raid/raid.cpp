#include "raid.h"
#include "../../shared/vnet.h"
#include "../../shared/vnet_protocol.h"
#include "../render.h"
#include "../vnet_client.h"
#include "../game.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <ctime>

extern Player g_player;

// ============================================================
// RAID TARGET FILES (for BURN minigame)
// ============================================================
const char* RAID_TARGET_FILES[] = {
    "access.log", "audit.bin", "hash.log", "config.sys", 
    "vfs_cache", "route.db", "ice_log", "crypto_mem"
};
const int RAID_TARGET_FILE_COUNT = sizeof(RAID_TARGET_FILES) / sizeof(RAID_TARGET_FILES[0]);

// ============================================================
// RAID TRIGGER PROBABILITIES
// ============================================================
static const float RAID_TRIGGER_HIGH_TRACE = 0.0012f;   // ~4.3% per minute (rare but exciting)
static const float RAID_TRIGGER_MED_TRACE  = 0.0006f;   // ~2.1% per minute
static const float RAID_TRIGGER_LOW_TRACE  = 0.00025f;  // ~0.9% per minute (very rare)
static const float RAID_TRIGGER_FLAGGED_MULT = 1.3f;

// ============================================================
// INIT
// ============================================================

void InitRaidSystem(void) {
    // Initialize raid state
    g_player.raidActive = false;
    g_player.raidTimer = 0.0f;
    g_player.raidResponseTime = 0.0f;
    g_player.raidType = RAID_EVADE;
    g_player.raidStage = RAID_STAGE_IDLE;
    g_player.raidSuccess = false;
    g_player.raidInput[0] = '\0';
    g_player.raidInputTimer = 0.0f;
    g_player.raidCount = 0;
    g_player.raidFailedCount = 0;
    g_player.isFlagged = false;
    g_player.raidCooldown = 0.0f;
    g_player.raidAlertTimer = 0.0f;
    g_player.raidIntruderVisible = false;
    
    PushCliLog("[RAID]: System initialized.");
}

void ResetRaidState(void) {
    g_player.raidActive = false;
    g_player.raidTimer = 0.0f;
    g_player.raidStage = RAID_STAGE_IDLE;
    g_player.raidSuccess = false;
    g_player.raidInput[0] = '\0';
    g_player.raidIntruderVisible = false;
}

// ============================================================
// TRIGGER RAID
// ============================================================

void TriggerFederalRaid(void) {
    // Don't trigger if already active, on cooldown, or connecting
    if (g_player.raidActive) return;
    if (g_player.raidCooldown > 0.0f) return;
    if (g_player.isConnecting) return;
    
    // Random timing
    g_player.raidActive = true;
    g_player.raidTimer = 0.0f;
    g_player.raidResponseTime = 15.0f;
    g_player.raidStage = RAID_STAGE_ALERT;
    g_player.raidSuccess = false;
    g_player.raidAlertTimer = 0.0f;
    g_player.raidInput[0] = '\0';
    
    // Choose raid type based on player status
    if (g_player.traceLevel > 70) {
        g_player.raidType = rand() % 3;  // All types
    } else if (g_player.vcoin > 10.0f) {
        g_player.raidType = RAID_BURN;   // Economic threat
    } else if (g_player.traceLevel > 40) {
        g_player.raidType = RAID_ESCAPE; // High trace = escape
    } else {
        g_player.raidType = rand() % 2;  // EVADE or ESCAPE
    }
    
    // Show IntruderDetector window
    g_player.raidIntruderVisible = true;
    
    // Trigger visual effects
    TriggerGlitch(1.0f);
    TriggerJitter(0.8f, 1.5f);
    
    // Server notification
    if (IsVNetConnected()) {
        VNetSendRaw(std::string(VNetCmd::TRACE_BUST) + ":RAID_ALERT");
    }
    
    // CLI logs
    const char* typeNames[] = {"EVADE", "ESCAPE", "BURN"};
    PushCliLog("========================================");
    PushCliLog("  ⚡ FEDERAL E-RAID DETECTED!");
    PushCliLog("  ⚠ ACTIVE UDP SWEEP ON PORT %d", g_player.port);
    PushCliLog("  ⏱ RESPONSE WINDOW: %.0fs", g_player.raidResponseTime);
    PushCliLog("========================================");
    PushCliLog("  STRATEGY SUGGESTION: %s", typeNames[g_player.raidType]);
    PushCliLog("========================================");
}

// ============================================================
// UPDATE RAID
// ============================================================

void UpdateRaid(float dt) {
    // ============================================================
    // RAID TRIGGER CHECK
    // ============================================================
    if (!g_player.raidActive) {
        // Cooldown
        if (g_player.raidCooldown > 0.0f) {
            g_player.raidCooldown -= dt;
        }
        
        // Random trigger based on trace level
        if (g_player.raidCooldown <= 0.0f && !g_player.isConnecting) {
            float triggerChance = 0.0f;
            if (g_player.traceLevel > 70) {
                triggerChance = RAID_TRIGGER_HIGH_TRACE;
            } else if (g_player.traceLevel > 50) {
                triggerChance = RAID_TRIGGER_MED_TRACE;
            } else if (g_player.traceLevel > 30) {
                triggerChance = RAID_TRIGGER_LOW_TRACE;
            }
            
            // Flagged players get higher chance
            if (g_player.isFlagged) {
                triggerChance *= RAID_TRIGGER_FLAGGED_MULT;
            }
            
            // Mining activity increases chance (tracked via cdMine)
            if (g_player.cdMine > 0.0f && g_player.cdMine < 5.0f) {
                triggerChance *= 1.5f;
            }
            
            // Random roll
            if ((rand() % 10000) < (int)(triggerChance * 10000)) {
                TriggerFederalRaid();
            }
        }
        return;
    }
    
    // ============================================================
    // ACTIVE RAID UPDATE
    // ============================================================
    g_player.raidTimer += dt;
    
    // --- STAGE: ALERT ---
    if (g_player.raidStage == RAID_STAGE_ALERT) {
        g_player.raidAlertTimer += dt;
        // Auto-advance after 3 seconds if player hasn't responded
        if (g_player.raidAlertTimer > 3.0f) {
            g_player.raidStage = RAID_STAGE_CHOICE;
            g_player.raidTimer = 0.0f;
            g_player.raidResponseTime = 8.0f + ((float)(rand() % 40) / 10.0f); // 8-11s for minigame
            
            const char* typeNames[] = {"EVADE", "ESCAPE", "BURN"};
            PushCliLog("[FEDERAL RAID]: STRATEGY SUGGESTION: %s", typeNames[g_player.raidType]);
            PushCliLog("[FEDERAL RAID]: TYPE 'raid 1', 'raid 2', or 'raid 3' TO RESPOND!");
            PushCliLog("[FEDERAL RAID]: OR USE TERMINAL COMMANDS: decoy, patch, purge");
            PushCliLog("[FEDERAL RAID]: YOU HAVE %.0fs TO RESPOND!", g_player.raidResponseTime);
        }
        return;
    }
    
    // --- STAGE: CHOICE (waiting for player to pick) ---
    if (g_player.raidStage == RAID_STAGE_CHOICE) {
        if (g_player.raidTimer > g_player.raidResponseTime) {
            PushCliLog("[FEDERAL RAID]: ⚠ NO RESPONSE! FEDERAL SCANNERS LOCKING ON...");
            ApplyRaidFailure();
            return;
        }
        return;
    }
    
    // --- STAGE: MINIGAME ---
    if (g_player.raidStage == RAID_STAGE_MINIGAME) {
        if (g_player.raidTimer > g_player.raidResponseTime) {
            if (!g_player.raidSuccess) {
                PushCliLog("[FEDERAL RAID]: ⚠ TIME'S UP! FEDERAL AGENTS BREACHING...");
                ApplyRaidFailure();
            }
        }
        return;
    }
    
    // --- STAGE: RESULT (show for 2 seconds) ---
    if (g_player.raidStage == RAID_STAGE_RESULT) {
        if (g_player.raidTimer > 2.0f) {
            g_player.raidStage = RAID_STAGE_COMPLETE;
            g_player.raidTimer = 0.0f;
            
            // Hide IntruderDetector after result
            g_player.raidIntruderVisible = false;
        }
        return;
    }
    
    // --- STAGE: COMPLETE (cleanup) ---
    if (g_player.raidStage == RAID_STAGE_COMPLETE) {
        g_player.raidActive = false;
        g_player.raidStage = RAID_STAGE_IDLE;
        g_player.raidIntruderVisible = false;
    }
}

// ============================================================
// RAID CHOICE HANDLER
// ============================================================

void HandleRaidChoice(const char* input) {
    if (!g_player.raidActive) {
        PushCliLog("[RAID]: No active federal raid detected.");
        return;
    }
    
    if (g_player.raidStage != RAID_STAGE_CHOICE) {
        PushCliLog("[RAID]: You already made your choice!");
        return;
    }
    
    int choice = atoi(input);
    
    // Also handle "evade", "escape", "burn" text input
    if (choice == 0) {
        if (strcmp(input, "evade") == 0) choice = 1;
        else if (strcmp(input, "escape") == 0) choice = 2;
        else if (strcmp(input, "burn") == 0) choice = 3;
    }
    
    if (choice < 1 || choice > 3) {
        PushCliLog("[FEDERAL RAID]: INVALID CHOICE! Use 1, 2, or 3");
        PushCliLog("[FEDERAL RAID]:   raid 1  - EVADE (deploy decoys)");
        PushCliLog("[FEDERAL RAID]:   raid 2  - ESCAPE (port migration)");
        PushCliLog("[FEDERAL RAID]:   raid 3  - BURN (scorched earth)");
        return;
    }
    
    g_player.raidType = choice - 1;
    g_player.raidStage = RAID_STAGE_MINIGAME;
    g_player.raidTimer = 0.0f;
    g_player.raidResponseTime = 15.0f;  // 15 seconds for minigame
    g_player.raidSuccess = false;
    g_player.raidInput[0] = '\0';
    
    const char* typeNames[] = {"EVADE", "ESCAPE", "BURN"};
    PushCliLog("[FEDERAL RAID]: STRATEGY SELECTED: %s", typeNames[g_player.raidType]);
    PushCliLog("[FEDERAL RAID]: INITIATING %s PROTOCOL...", typeNames[g_player.raidType]);
    PushCliLog("[FEDERAL RAID]: YOU HAVE %.0fs TO COMPLETE!", g_player.raidResponseTime);
    
    // Show minigame instructions
    switch (g_player.raidType) {
        case RAID_EVADE:
            PushCliLog("[RAID]: TYPE: decoy <node> <port>");
            PushCliLog("[RAID]: Choose a proxy node to redirect federal scanners.");
            PushCliLog("[RAID]: Example: decoy cult.vnet 8080");
            break;
        case RAID_ESCAPE:
            PushCliLog("[RAID]: TYPE: patch <port>");
            PushCliLog("[RAID]: Choose a new port to migrate to (8001-8999).");
            PushCliLog("[RAID]: Example: patch 8050");
            break;
        case RAID_BURN:
            PushCliLog("[RAID]: TYPE: purge <target>");
            PushCliLog("[RAID]: Choose a log file to purge.");
            PushCliLog("[RAID]: Available: access.log, audit.bin, hash.log, config.sys, vfs_cache, route.db, ice_log, crypto_mem");
            PushCliLog("[RAID]: Example: purge access.log");
            break;
    }
}

// ============================================================
// MINIGAME INPUT PROCESSOR
// ============================================================

void ProcessRaidMinigameInput(const char* input) {
    if (!g_player.raidActive) return;
    if (g_player.raidStage != RAID_STAGE_MINIGAME) return;
    if (g_player.raidSuccess) return;
    
    strncpy(g_player.raidInput, input, 63);
    g_player.raidInput[63] = '\0';
    
    bool success = false;
    const char* resultMsg = "";
    
    switch (g_player.raidType) {
        case RAID_EVADE: {
            // decoy <node> <port>
            char node[32] = {0}, port[16] = {0};
            if (sscanf(input, "decoy %31s %15s", node, port) == 2) {
                // Check if node exists in assigned sites
                bool nodeExists = false;
                for (int i = 0; i < g_player.assignedCount; i++) {
                    if (strcmp(g_player.assignedSites[i], node) == 0) {
                        nodeExists = true;
                        break;
                    }
                }
                int portNum = atoi(port);
                if (nodeExists && portNum > 0 && portNum < 65535) {
                    success = true;
                    resultMsg = "Decoy deployed! Federal scanners redirected!";
                } else {
                    resultMsg = "Invalid node or port!";
                    if (!nodeExists) PushCliLog("[RAID]: Node '%s' not in your assigned sites", node);
                    if (portNum <= 0 || portNum >= 65535) PushCliLog("[RAID]: Invalid port (1-65534)");
                }
            } else {
                resultMsg = "Usage: decoy <node> <port>";
            }
            break;
        }
        case RAID_ESCAPE: {
            // patch <port>
            char port[16] = {0};
            if (sscanf(input, "patch %15s", port) == 1) {
                int newPort = atoi(port);
                if (newPort >= 8001 && newPort <= 8999) {
                    success = true;
                    g_player.port = newPort;
                    resultMsg = "Port migrated successfully! Trace reset!";
                } else {
                    resultMsg = "Invalid port! Use 8001-8999";
                }
            } else {
                resultMsg = "Usage: patch <port>";
            }
            break;
        }
        case RAID_BURN: {
            // purge <target>
            char target[32] = {0};
            if (sscanf(input, "purge %31s", target) == 1) {
                // Allow any of the target files
                bool validTarget = false;
                for (int i = 0; i < RAID_TARGET_FILE_COUNT; i++) {
                    if (strcmp(RAID_TARGET_FILES[i], target) == 0) {
                        validTarget = true;
                        break;
                    }
                }
                if (validTarget) {
                    success = true;
                    resultMsg = "System purged! Logs wiped, trace reduced!";
                } else {
                    resultMsg = "Invalid target file!";
                    PushCliLog("[RAID]: Available: access.log, audit.bin, hash.log, config.sys, vfs_cache, route.db, ice_log, crypto_mem");
                }
            } else {
                resultMsg = "Usage: purge <target>";
            }
            break;
        }
    }
    
    if (success) {
        g_player.raidSuccess = true;
        g_player.raidStage = RAID_STAGE_RESULT;
        g_player.raidTimer = 0.0f;
        PushCliLog("[FEDERAL RAID]: ✅ %s", resultMsg);
        ApplyRaidSuccess();
    } else {
        PushCliLog("[FEDERAL RAID]: ❌ %s", resultMsg);
        float remaining = g_player.raidResponseTime - g_player.raidTimer;
        if (remaining <= 0.0f) {
            PushCliLog("[FEDERAL RAID]: ⚠ TIME'S UP!");
            ApplyRaidFailure();
        } else {
            PushCliLog("[FEDERAL RAID]: TRY AGAIN! (%.0fs remaining)", remaining);
        }
    }
}

// ============================================================
// SUCCESS / FAILURE APPLICATION
// ============================================================

void ApplyRaidSuccess(void) {
    float reward = 0.0f;
    switch (g_player.raidType) {
        case RAID_EVADE:
            reward = 0.20f;
            g_player.traceLevel = (int)(g_player.traceLevel * 0.8f);
            if (g_player.iceShields < 3) g_player.iceShields++;
            PushCliLog("[RAID]: ✅ EVADE SUCCESS! Decoys misdirected federal scanners.");
            break;
        case RAID_ESCAPE:
            reward = 0.15f;
            g_player.traceLevel = 0;
            PushCliLog("[RAID]: ✅ ESCAPE SUCCESS! Port migrated, trace reset.");
            break;
        case RAID_BURN:
            reward = 0.10f;
            g_player.traceLevel = (int)(g_player.traceLevel * 0.1f);
            g_player.neuralParanoia *= 0.85f;
            PushCliLog("[RAID]: ✅ BURN SUCCESS! Logs purged, trace minimized.");
            break;
    }
    g_player.vcoin += reward;
    g_player.raidCount++;
    g_player.raidCooldown = 60.0f;
    g_player.raidIntruderVisible = false;
    
    PushCliLog("================================================");
    PushCliLog("  🏆 RAID ESCAPED SUCCESSFULLY!");
    PushCliLog("  VCOIN BONUS: +%.2f VCOIN", reward);
    PushCliLog("  TRACE LEVEL: %d%%", g_player.traceLevel);
    PushCliLog("  ICE SHIELDS: %d/3", g_player.iceShields);
    PushCliLog("  RAIDS SURVIVED: %d", g_player.raidCount);
    PushCliLog("================================================");
    
    TriggerGlitch(0.3f);
}

void ApplyRaidFailure(void) {
    // Penalties
    g_player.traceLevel = 100;
    g_player.iceShields = 0;
    g_player.neuralParanoia += 30.0f;
    g_player.crtHeat += 15.0f;
    g_player.raidFailedCount++;
    g_player.isFlagged = true;
    
    // VCOIN confiscation (25%)
    float seized = g_player.vcoin * 0.25f;
    g_player.vcoin -= seized;
    if (g_player.vcoin < 0.0f) g_player.vcoin = 0.0f;
    
    // Burn a random site
    char burnedSite[64] = "none";
    if (g_player.assignedCount > 0) {
        int idx = rand() % g_player.assignedCount;
        strcpy(burnedSite, g_player.assignedSites[idx]);
        for (int i = idx; i < g_player.assignedCount - 1; i++) {
            strcpy(g_player.assignedSites[i], g_player.assignedSites[i + 1]);
        }
        g_player.assignedCount--;
    }
    
    g_player.raidCooldown = 30.0f;
    g_player.raidIntruderVisible = false;
    g_player.raidStage = RAID_STAGE_RESULT;
    g_player.raidTimer = 0.0f;
    
    PushCliLog("╔════════════════════════════════════════════════════╗");
    PushCliLog("║  💀 FEDERAL RAID BREACH SUCCESSFUL!               ║");
    PushCliLog("║                                                    ║");
    PushCliLog("║  ❌ PORT: %d LOCKED", g_player.port);
    PushCliLog("║  ❌ ICE SHIELDS: 0/3");
    PushCliLog("║  ❌ TRACE: 100%%");
    PushCliLog("║  ❌ VCOIN LOST: %.2f VCOIN", seized);
    PushCliLog("║  ❌ SITE BURNED: %s", burnedSite);
    PushCliLog("║                                                    ║");
    PushCliLog("║  You've been flagged by federal intelligence.     ║");
    PushCliLog("║  Future raids will be more frequent.              ║");
    PushCliLog("╚════════════════════════════════════════════════════╝");
    
    TriggerGlitch(1.5f);
    TriggerJitter(1.0f, 2.0f);
}

// ============================================================
// UI HELPERS
// ============================================================

bool IsRaidActive(void) {
    return g_player.raidActive;
}

float GetRaidRemainingTime(void) {
    if (!g_player.raidActive) return 0.0f;
    if (g_player.raidStage == RAID_STAGE_CHOICE || g_player.raidStage == RAID_STAGE_MINIGAME) {
        return g_player.raidResponseTime - g_player.raidTimer;
    }
    return 0.0f;
}