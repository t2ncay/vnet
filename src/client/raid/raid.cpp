#include "raid.h"
#include "../../shared/vnet.h"
#include "../../shared/vnet_protocol.h"
#include "../render.h"
#include "../vnet_client.h"
#include "../game.h"
#include "../desktop/desktop.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <ctime>

// fwd decls
void DrawNetworkVisualization(float x, float y, float w, float h);
void UpdateRaidSequence(float dt);
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

    // ============================================================
    // CLEAR THE TERMINAL BEFORE RAID STARTS
    // ============================================================
    // Use the existing clear command to properly clear the terminal

    g_cliLogCount = 0;                          // Reset log count
    memset(g_cliLogs, 0, sizeof(g_cliLogs));    // Wipe log buffer
    g_player.cliScroll = 0.0f;                  // Reset scroll offset
    
    // Then add the raid header
    PushCliLog("========================================");
    PushCliLog("  ⚡ FEDERAL E-RAID DETECTED!");
    PushCliLog("  ⚠ SYSTEM LOCKDOWN INITIATED...");
    PushCliLog("========================================");

    // ============================================================
    // START THE SEQUENCE
    // ============================================================
    g_player.raidSeqStage = RAID_SEQ_GLITCH;
    g_player.raidSeqTimer = 0.0f;
    g_player.raidSeqStageDuration = 6.0f;   // 6 seconds of glitch
    g_player.raidSeqLockDesktop = false;
    g_player.raidSeqShowOperator = false;
    strcpy(g_player.raidSeqOperatorDialogue, "INTRUSION DETECTED. IDENTIFY YOURSELF.");
    g_player.raidIntruderVisible = false;
    
    // ============================================================
    // SET RAID ACTIVE
    // ============================================================
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

    UpdateRaidSequence(dt);

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

    if (g_player.raidSeqStage == RAID_SEQ_GLITCH || 
        g_player.raidSeqStage == RAID_SEQ_FLASH || 
        g_player.raidSeqStage == RAID_SEQ_HEX || 
        g_player.raidSeqStage == RAID_SEQ_BLACKOUT) {
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
            g_player.raidResponseTime = 15.0f; // 8-11s for minigame
            
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
                // Strip .vnet from node if present
                char cleanNode[32];
                strcpy(cleanNode, node);
                char* dot = strstr(cleanNode, ".vnet");
                if (dot) *dot = '\0';
                
                // Check if node exists in assigned sites
                bool nodeExists = false;
                for (int i = 0; i < g_player.assignedCount; i++) {
                    char siteCopy[64];
                    strcpy(siteCopy, g_player.assignedSites[i]);
                    char* dot2 = strstr(siteCopy, ".vnet");
                    if (dot2) *dot2 = '\0';
                    
                    if (strcmp(siteCopy, cleanNode) == 0) {
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

    g_player.raidSeqStage = RAID_SEQ_SUCCESS;
    g_player.raidSeqTimer = 0.0f;
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
    PushCliLog("║  💀 FEDERAL RAID BREACH SUCCESSFUL!                ║");
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

    g_player.raidSeqStage = RAID_SEQ_FAILURE;
    g_player.raidSeqTimer = 0.0f;
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

void HandleRaidCLIInput(char* buffer) {
    if (!g_player.raidActive) return;
    if (g_player.raidSeqStage != RAID_SEQ_ACTIVE) return;
    
    if (buffer[0] == '\0') return;
    
    // Process the command
    ProcessCommand(buffer);
    
    // Clear input
    buffer[0] = '\0';
}

void UpdateRaidSequence(float dt) {
    if (!g_player.raidActive) return;

    g_player.raidSeqTimer += dt;

    switch (g_player.raidSeqStage) {
        case RAID_SEQ_GLITCH:
            if (g_player.raidSeqTimer >= g_player.raidSeqStageDuration) {
                g_player.raidSeqStage = RAID_SEQ_FLASH;
                g_player.raidSeqTimer = 0.0f;
                g_player.raidSeqStageDuration = 0.5f;  // 0.5s flash
                
            }
            break;

        case RAID_SEQ_FLASH:
            if (g_player.raidSeqTimer >= g_player.raidSeqStageDuration) {
                g_player.raidSeqStage = RAID_SEQ_HEX;
                g_player.raidSeqTimer = 0.0f;
                g_player.raidSeqStageDuration = 2.0f;  // 2s hex flood
            }
            break;

        case RAID_SEQ_HEX:
            if (g_player.raidSeqTimer >= g_player.raidSeqStageDuration) {
                g_player.raidSeqStage = RAID_SEQ_BLACKOUT;
                g_player.raidSeqTimer = 0.0f;
                g_player.raidSeqStageDuration = 0.8f;  // 0.8s blackout
            }
            break;

        case RAID_SEQ_BLACKOUT:
            if (g_player.raidSeqTimer >= g_player.raidSeqStageDuration) {
                g_player.raidSeqStage = RAID_SEQ_ACTIVE;
                g_player.raidSeqTimer = 0.0f;
                g_player.raidSeqLockDesktop = true;
                g_player.raidSeqShowOperator = true;
                g_player.raidIntruderVisible = true;
                
                // ============================================================
                // FORCE TERMINAL OPEN AND FOCUSED - FIXED
                // ============================================================
                Desktop& desktop = GetDesktop();
                
                // Close any existing terminal first to avoid duplication
                // (we'll just open a new one)
                int termIdx = desktop.OpenApp(AppType::Terminal, "Terminal");
                
                // If terminal was already open, focus it
                if (termIdx >= 0) {
                    desktop.FocusWindow(termIdx);
                }
                
                // Also ensure CLI is open
                g_player.cliOpen = true;
                
                strcpy(g_player.raidSeqOperatorDialogue, "FEDERAL E-RAID IN PROGRESS. RESPOND OR PERISH.");
            }
            break;

        case RAID_SEQ_ACTIVE:
            g_player.raidSeqDialogueTimer += dt;
            if (g_player.raidSeqDialogueTimer > 3.0f) {
                g_player.raidSeqDialogueTimer = 0.0f;
                const char* messages[] = {
                    "FEDERAL E-RAID IN PROGRESS. RESPOND OR PERISH.",
                    "I SEE YOUR CRT GLARE. YOU CAN'T HIDE.",
                    "PORT 8001 IS COMPROMISED. MAKE YOUR MOVE.",
                    "THE NETWORK IS ALIVE. IT IS DRINKING YOUR HEAT."
                };
                int idx = rand() % 4;
                strcpy(g_player.raidSeqOperatorDialogue, messages[idx]);
            }
            break;

        case RAID_SEQ_SUCCESS:
            // Display success for 2 seconds then reset
            if (g_player.raidSeqTimer >= 2.0f) {
                ResetRaidState();
                g_player.raidSeqStage = RAID_SEQ_IDLE;
                g_player.raidSeqLockDesktop = false;
                g_player.raidSeqShowOperator = false;
                g_player.raidActive = false;
                g_player.raidIntruderVisible = false;
                g_player.cliOpen = false; // maybe close terminal
            }
            break;

        case RAID_SEQ_FAILURE:
            if (g_player.raidSeqTimer >= 2.0f) {
                ResetRaidState();
                g_player.raidSeqStage = RAID_SEQ_IDLE;
                g_player.raidSeqLockDesktop = false;
                g_player.raidSeqShowOperator = false;
                g_player.raidActive = false;
                g_player.raidIntruderVisible = false;
                g_player.cliOpen = false;
            }
            break;
    }
}


void DrawRaidSequenceOverlay() {
    if (!g_player.raidActive) return;

    float t = g_player.raidSeqTimer;
    float pulse = sinf(GetTime() * 8.0f) * 0.5f + 0.5f;

    switch (g_player.raidSeqStage) {
        case RAID_SEQ_GLITCH:
            {
                float t = g_player.raidSeqTimer;
                float duration = g_player.raidSeqStageDuration;
                float progress = t / duration;  // 0 → 1
                float intensity = sinf(progress * 3.14159f) * 0.8f + 0.2f; // Peaks mid-way
                
                // ---- 1. SCREEN TEARING (horizontal displacement) ----
                int tearCount = (int)(5 + intensity * 15);
                for (int i = 0; i < tearCount; i++) {
                    float tearY = rand() % REF_HEIGHT;
                    float tearH = 2 + rand() % (int)(6 + intensity * 10);
                    float tearX = (rand() % (int)(REF_WIDTH * 0.3f)) - REF_WIDTH * 0.15f;
                    float tearW = REF_WIDTH + abs(tearX) * 2;
                    
                    Color tearColor = (i % 3 == 0) ? Fade(COLOR_BLOOD, 0.15f * intensity) :
                                    (i % 3 == 1) ? Fade(COLOR_TOXIC, 0.1f * intensity) :
                                    Fade(COLOR_CYAN, 0.08f * intensity);
                    DrawScaledRect(tearX, tearY, tearW, tearH, tearColor);
                }
                
                // ---- 2. CHROMATIC ABERRATION (color channel offset) ----
                if (intensity > 0.4f) {
                    float offset = (intensity - 0.4f) * 4.0f; // max 2.4px
                    float rOffset = (rand() % 100 / 100.0f) * offset;
                    float bOffset = (rand() % 100 / 100.0f) * offset;
                    // We'll apply via drawing red/blue overlay rectangles with blend
                    // Simulate with semi-transparent colored bars
                    for (int i = 0; i < (int)(5 + intensity * 20); i++) {
                        float x = rand() % REF_WIDTH;
                        float y = rand() % REF_HEIGHT;
                        float w = 20 + rand() % 100;
                        float h = 2 + rand() % 6;
                        if (i % 2 == 0)
                            DrawScaledRect(x + rOffset, y, w, h, Fade(COLOR_BLOOD, 0.08f * intensity));
                        else
                            DrawScaledRect(x - bOffset, y, w, h, Fade(COLOR_CYAN, 0.08f * intensity));
                    }
                }
                
                // ---- 3. DATA CORRUPTION BLOCKS (random hex characters) ----
                int blockCount = (int)(3 + intensity * 12);
                for (int i = 0; i < blockCount; i++) {
                    float bx = rand() % REF_WIDTH;
                    float by = rand() % REF_HEIGHT;
                    float bw = 20 + rand() % 60;
                    float bh = 12 + rand() % 24;
                    
                    // Background block
                    Color bgColor = (i % 3 == 0) ? Fade(COLOR_BLOOD, 0.1f * intensity) :
                                    (i % 3 == 1) ? Fade(COLOR_TOXIC, 0.08f * intensity) :
                                    Fade(COLOR_BLACK, 0.2f * intensity);
                    DrawScaledRect(bx, by, bw, bh, bgColor);
                    
                    // Hex characters inside block
                    int hexCount = 3 + rand() % 6;
                    for (int j = 0; j < hexCount; j++) {
                        char hexChars[] = "0123456789ABCDEF";
                        char c = hexChars[rand() % 16];
                        float hx = bx + 4 + (rand() % (int)(bw - 8));
                        float hy = by + 2 + (rand() % (int)(bh - 6));
                        Color col = (i % 2 == 0) ? Fade(COLOR_BLOOD, 0.5f * intensity) :
                                                Fade(COLOR_TOXIC, 0.4f * intensity);
                        DrawScaledText(&c, hx, hy, 10 + rand() % 6, col);
                    }
                }
                
                // ---- 4. AGGRESSIVE JITTER & SCANLINES ----
                float jitterIntensity = 0.5f + intensity * 1.5f;
                TriggerJitter(jitterIntensity * 0.2f, 0.02f + intensity * 0.08f);
                
                // Enhanced scanlines with varying frequency
                float scanlineFreq = 60.0f + intensity * 120.0f;
                float scanlineAlpha = 0.1f + intensity * 0.4f;
                DrawScanlineOverlay(0, 0, REF_WIDTH, REF_HEIGHT, 
                                    Fade(COLOR_BLOOD, scanlineAlpha * 0.5f), 
                                    scanlineFreq, 2.0f + intensity * 4.0f);
                
                // ---- 5. FLICKERING WHITE FLASHES (random) ----
                if (rand() % 100 < (int)(intensity * 15)) {
                    float flashAlpha = (rand() % 100 / 100.0f) * 0.2f * intensity;
                    DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, Fade(WHITE, flashAlpha));
                }
                
                // ---- 6. HORIZONTAL BARS (classic glitch) ----
                int barCount = (int)(3 + intensity * 20);
                for (int i = 0; i < barCount; i++) {
                    float x = rand() % REF_WIDTH;
                    float y = rand() % REF_HEIGHT;
                    float w = 10 + rand() % (int)(40 + intensity * 60);
                    float h = 2 + rand() % (int)(2 + intensity * 6);
                    Color barColor = (i % 4 == 0) ? Fade(COLOR_BLOOD, 0.15f * intensity) :
                                    (i % 4 == 1) ? Fade(COLOR_TOXIC, 0.1f * intensity) :
                                    (i % 4 == 2) ? Fade(COLOR_CYAN, 0.1f * intensity) :
                                    Fade(COLOR_GHOST, 0.1f * intensity);
                    DrawScaledRect(x, y, w, h, barColor);
                }
                
                // ---- 7. VERTICAL LINES (data corruption) ----
                int vLineCount = (int)(2 + intensity * 8);
                for (int i = 0; i < vLineCount; i++) {
                    float x = rand() % REF_WIDTH;
                    float y = rand() % REF_HEIGHT;
                    float w = 1 + rand() % 3;
                    float h = 10 + rand() % (int)(20 + intensity * 40);
                    DrawScaledRect(x, y, w, h, Fade(COLOR_BLOOD, 0.1f * intensity));
                }
                
                break;
            }

        case RAID_SEQ_FLASH:
            // Full white flash with extreme jitter
            {
                float flashAlpha = 1.0f - (t / g_player.raidSeqStageDuration);
                DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, Fade(WHITE, flashAlpha));
                TriggerJitter(1.0f, 0.05f);
                // Add chromatic aberration: offset red/blue channels?
                // For simplicity, we'll just use a bright flash.
            }
            break;

        case RAID_SEQ_HEX:
            {
                float t = g_player.raidSeqTimer;
                float duration = g_player.raidSeqStageDuration;
                float progress = t / duration;  // 0 → 1
                float intensity = sinf(progress * 3.14159f) * 0.8f + 0.2f;
                
                // ---- 1. MAIN DATA STREAM (multiple layers) ----
                int streamCount = 25 + (int)(intensity * 20);
                float baseSpeed = 120.0f + intensity * 200.0f;
                
                // Pre-compute stream data for consistency
                static float streamOffsets[50] = {0};
                static float streamSpeeds[50] = {0};
                static int streamLengths[50] = {0};
                static float streamX[50] = {0};
                
                // Initialize streams (only once per stage)
                static bool initialized = false;
                if (!initialized) {
                    for (int i = 0; i < 50; i++) {
                        streamOffsets[i] = (float)(rand() % 1000) / 1000.0f * 2.0f;
                        streamSpeeds[i] = 0.3f + (rand() % 100 / 100.0f) * 0.7f;
                        streamLengths[i] = 5 + rand() % 20;
                        streamX[i] = (rand() % (int)(REF_WIDTH * 0.9f)) + REF_WIDTH * 0.05f;
                    }
                    initialized = true;
                }
                
                // Update and draw streams
                float hexW = 18.0f;
                float hexH = 20.0f;
                char hexChars[] = "0123456789ABCDEF";
                
                // Draw multiple streams
                for (int s = 0; s < streamCount; s++) {
                    float speed = streamSpeeds[s] * baseSpeed;
                    float offset = streamOffsets[s];
                    float progress2 = fmodf(t * speed + offset, 2.0f);
                    if (progress2 > 1.0f) continue;
                    
                    float alpha = (1.0f - progress2) * 0.7f + 0.2f;
                    float yPos = (1.0f - progress2) * REF_HEIGHT;
                    float xPos = streamX[s];
                    
                    // Determine color
                    Color col;
                    int colorType = rand() % 3;
                    if (colorType == 0) col = Fade(COLOR_TOXIC, alpha);
                    else if (colorType == 1) col = Fade(COLOR_CYAN, alpha * 0.7f);
                    else col = Fade(COLOR_GHOST, alpha * 0.5f);
                    
                    // Draw stream characters with varying size
                    int len = streamLengths[s];
                    for (int j = 0; j < len; j++) {
                        char c = hexChars[(rand() % 16)];
                        float yOffset = j * hexH;
                        float yDraw = yPos + yOffset;
                        if (yDraw < 0 || yDraw > REF_HEIGHT) continue;
                        
                        // Random size variation per character
                        float size = 14.0f + (rand() % 12);
                        if (j % 3 == 0) size *= 1.2f;  // Some characters larger
                        
                        // Random color variation per character
                        Color charCol = col;
                        if (rand() % 5 == 0) charCol = Fade(COLOR_BLOOD, alpha * 0.6f);
                        
                        DrawScaledText(&c, xPos, yDraw, size, charCol);
                    }
                }
                
                // ---- 2. GLITCHY HEX BLOCKS (overlay) ----
                int blockCount = (int)(3 + intensity * 15);
                for (int i = 0; i < blockCount; i++) {
                    float bx = rand() % REF_WIDTH;
                    float by = rand() % REF_HEIGHT;
                    float bw = 30 + rand() % 80;
                    float bh = 15 + rand() % 40;
                    
                    // Random semitransparent block
                    Color bg = (i % 2 == 0) ? Fade(COLOR_BLOOD, 0.05f * intensity) :
                                            Fade(COLOR_TOXIC, 0.04f * intensity);
                    DrawScaledRect(bx, by, bw, bh, bg);
                    
                    // Random hex inside block
                    int inner = 2 + rand() % 5;
                    for (int j = 0; j < inner; j++) {
                        char c = hexChars[rand() % 16];
                        float hx = bx + 4 + (rand() % (int)(bw - 8));
                        float hy = by + 2 + (rand() % (int)(bh - 6));
                        DrawScaledText(&c, hx, hy, 12 + rand() % 8, 
                                    Fade(COLOR_TOXIC, 0.6f * intensity));
                    }
                }
                
                // ---- 3. DATA STREAKS (fast moving lines) ----
                int streakCount = (int)(2 + intensity * 6);
                for (int i = 0; i < streakCount; i++) {
                    float x = rand() % REF_WIDTH;
                    float y = rand() % REF_HEIGHT;
                    float w = 80 + rand() % 150;
                    float h = 1 + rand() % 3;
                    float speed = 0.2f + rand() % 100 / 100.0f;
                    float pos = fmodf(t * speed * 200.0f + i * 50.0f, REF_HEIGHT + 100.0f) - 50.0f;
                    DrawScaledRect(x, pos, w, h, Fade(COLOR_CYAN, 0.2f * intensity));
                }
                
                // ---- 4. SCREEN FLICKER (random white/color flashes) ----
                if (rand() % 100 < (int)(intensity * 20)) {
                    float flashAlpha = (rand() % 100 / 100.0f) * 0.15f * intensity;
                    Color flashColor = (rand() % 2 == 0) ? Fade(WHITE, flashAlpha) : Fade(COLOR_CYAN, flashAlpha * 0.8f);
                    DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, flashColor);
                }
                
                // ---- 5. SCANLINE OVERLAY (aggressive) ----
                float scanFreq = 40.0f + intensity * 100.0f;
                float scanAlpha = 0.1f + intensity * 0.3f;
                DrawScanlineOverlay(0, 0, REF_WIDTH, REF_HEIGHT, 
                                    Fade(COLOR_BLOOD, scanAlpha * 0.3f), 
                                    scanFreq, 2.0f + intensity * 3.0f);
                
                // ---- 6. DARKENING OVERLAY (pulsing) ----
                float darkAlpha = 0.2f + intensity * 0.2f;
                DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, Fade(BLACK, darkAlpha));
                
                break;
            }

        case RAID_SEQ_BLACKOUT:
            // Pure black
            DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, BLACK);
            break;

        case RAID_SEQ_ACTIVE: {
            // Desktop is locked, background is black
            float t = (float)GetTime();
            float pulse = sinf(t * 3.0f) * 0.3f + 0.7f;
            
            // ---- SPLIT LAYOUT: LEFT = HEX STREAM, RIGHT = TERMINAL ----
            float leftX = 0;
            float leftW = REF_WIDTH * 0.55f;
            float rightX = leftW;
            float rightW = REF_WIDTH * 0.45f;
            
            // ============================================================
            // LEFT PANEL: MODERN DASHBOARD (Hex Sidebar, Radar, Sniffer)
            // ============================================================
            {
                // 1. Clean Background (No chaotic lines, just a dark slate)
                DrawScaledRect(leftX, 0, leftW, REF_HEIGHT, Color{4, 6, 10, 255});
                
                // Subtle dot-matrix grid instead of heavy lines
                for (int x = 10; x < leftW; x += 20) {
                    for (int y = 10; y < REF_HEIGHT; y += 20) {
                        DrawScaledRect(leftX + x, y, 1, 1, Fade(COLOR_CYAN, 0.05f));
                    }
                }

                // ---- LAYOUT DEFINITIONS ----
                // Sidebar for Hex Dump
                float sideW = 60.0f;
                float sideX = leftX + 10.0f;
                
                // Main content area math
                float pad = 10.0f;
                float mainX = sideX + sideW + pad;
                float mainW = leftW - sideW - (pad * 3);
                
                float radarY = pad;
                float radarH = REF_HEIGHT * 0.35f;
                
                float snifferY = radarY + radarH + pad;
                float snifferH = REF_HEIGHT * 0.45f;
                
                float opY = snifferY + snifferH + pad;
                float opH = REF_HEIGHT - opY - pad;

                // ------------------------------------------------------------
                // MODULE 1: HEX DUMP SIDEBAR (Constrained, doesn't ruin readability)
                // ------------------------------------------------------------
                DrawScaledRect(sideX, pad, sideW, REF_HEIGHT - (pad*2), Color{6, 8, 14, 180});
                DrawScaledRectLines(sideX, pad, sideW, REF_HEIGHT - (pad*2), Fade(COLOR_CYAN, 0.1f));
                DrawScaledText("MEM", sideX + 20, pad + 5, 8, Fade(COLOR_CYAN, 0.5f));
                
                int hexCols = 4; // Down from 30, much cleaner
                int hexRows = 35;
                float hexW = sideW / hexCols;
                float hexH = (REF_HEIGHT - (pad*2)) / hexRows;
                
                for (int col = 0; col < hexCols; col++) {
                    for (int row = 2; row < hexRows - 1; row++) {
                        char hexChars[] = "0123456789ABCDEF";
                        char c = hexChars[rand() % 16];
                        
                        float speed = 0.8f + (col % 2) * 0.2f;
                        float phase = col * 0.5f + row * 0.1f;
                        float offset = fmodf(t * speed + phase, 2.0f);
                        
                        if (offset < 1.0f && (rand() % 100) < 60) { // Sparser density
                            float alpha = (1.0f - offset) * 0.6f;
                            Color colColor = (col % 2 == 0) ? Fade(COLOR_TOXIC, alpha) : Fade(COLOR_GHOST, alpha * 0.5f);
                            
                            float xPos = sideX + col * hexW + 6.0f;
                            float yPos = pad + row * hexH - offset * hexH;
                            DrawScaledText(&c, xPos, yPos, hexH * 0.8f, colColor);
                        }
                    }
                }

                // ------------------------------------------------------------
                // MODULE 2: RADAR & TARGET ACQUISITION
                // ------------------------------------------------------------
                DrawScaledRect(mainX, radarY, mainW, radarH, Color{6, 8, 14, 180});
                DrawScaledRectLines(mainX, radarY, mainW, radarH, Fade(COLOR_CYAN, 0.15f));
                
                // Tech Header
                DrawScaledRect(mainX, radarY, mainW, 18, Fade(COLOR_CYAN, 0.1f));
                DrawScaledText(":: SIGNAL GEO-LOCATION ::", mainX + 8, radarY + 4, 9, COLOR_CYAN);
                
                // Modern Radar Drawing
                float rX = mainX + (mainW / 2.0f);
                float rY = radarY + (radarH / 2.0f) + 8.0f;
                float maxR = radarH * 0.35f;
                
                // Crosshairs
                DrawScaledLine(mainX + 20, rY, mainX + mainW - 20, rY, Fade(COLOR_CYAN, 0.1f));
                DrawScaledLine(rX, radarY + 25, rX, radarY + radarH - 10, Fade(COLOR_CYAN, 0.1f));
                
                DrawScaledCircleLines(rX, rY, maxR, Fade(COLOR_CYAN, 0.3f));
                DrawScaledCircleLines(rX, rY, maxR * 0.66f, Fade(COLOR_CYAN, 0.15f));
                DrawScaledCircleLines(rX, rY, maxR * 0.33f, Fade(COLOR_CYAN, 0.05f));
                
                // Smooth Scanner Sweep
                float angle = t * 2.0f;
                DrawScaledLine(rX, rY, rX + cosf(angle) * maxR, rY + sinf(angle) * maxR, Fade(COLOR_TOXIC, 0.6f));
                
                // Blips
                for (int i = 0; i < 3; i++) {
                    float a = t * 0.2f + i * 2.1f;
                    float r = maxR * (0.4f + 0.4f * sinf(i * 1.5f));
                    float bx = rX + cosf(a) * r;
                    float by = rY + sinf(a) * r;
                    float blipAlpha = sinf(t * 3.0f - angle + a) > 0.8f ? 1.0f : 0.2f; // Flash when scanner passes
                    DrawScaledCircle(bx, by, 3.0f, Fade(COLOR_BLOOD, blipAlpha));
                    if (blipAlpha > 0.5f) {
                        DrawScaledRectLines(bx - 6, by - 6, 12, 12, Fade(COLOR_TOXIC, blipAlpha));
                    }
                }

                // ------------------------------------------------------------
                // MODULE 3: NETWORK SNIFFER (Tabular & Clean)
                // ------------------------------------------------------------
                DrawScaledRect(mainX, snifferY, mainW, snifferH, Color{6, 8, 14, 180});
                DrawScaledRectLines(mainX, snifferY, mainW, snifferH, Fade(COLOR_CYAN, 0.15f));
                
                DrawScaledRect(mainX, snifferY, mainW, 18, Fade(COLOR_CYAN, 0.1f));
                DrawScaledText(":: PACKET CAPTURE (PROMISCUOUS MODE) ::", mainX + 8, snifferY + 4, 9, COLOR_CYAN);
                
                // ---- COLUMN X POSITIONS (Relative to panel width) ----
                float colTime = mainX + 8;
                float colSrc  = mainX + mainW * 0.15f;
                float colDst  = mainX + mainW * 0.48f;
                float colType = mainX + mainW * 0.80f;
                float colSize = mainX + mainW * 0.90f;

                // Column Headers
                float colY = snifferY + 22;
                DrawScaledText("TIME", colTime, colY, 8, Fade(COLOR_GHOST, 0.6f));
                DrawScaledText("SRC IP:PORT", colSrc, colY, 8, Fade(COLOR_GHOST, 0.6f));
                DrawScaledText("DST IP:PORT", colDst, colY, 8, Fade(COLOR_GHOST, 0.6f));
                DrawScaledText("TYPE", colType, colY, 8, Fade(COLOR_GHOST, 0.6f));
                DrawScaledText("SIZE", colSize, colY, 8, Fade(COLOR_GHOST, 0.6f));
                DrawScaledLine(mainX + 5, colY + 12, mainX + mainW - 5, colY + 12, Fade(COLOR_CYAN, 0.2f));

                // Packet List (Scrolling cleanly upwards)
                float packetH = 14.0f;
                int maxPackets = (int)((snifferH - 50) / packetH);
                
                for (int i = 0; i < maxPackets; i++) {
                    float yPos = colY + 16 + (i * packetH);
                    
                    // Generate stable pseudo-random data per row based on time
                    int seed = (int)(t * 2.0f) + i; 
                    srand(seed); 
                    
                    char timeStr[16], srcStr[32], dstStr[32], typeStr[8], sizeStr[16];
                    snprintf(timeStr, sizeof(timeStr), "%04d", (int)(t * 10 + i) % 9999);
                    snprintf(srcStr, sizeof(srcStr), "%d.%d.%d.%d:%d", 192, 168, 1, rand()%255, 8000 + rand()%999);
                    snprintf(dstStr, sizeof(dstStr), "%d.%d.%d.%d:%d", rand()%255, rand()%255, rand()%255, rand()%255, 80);
                    
                    int type = rand() % 3;
                    strcpy(typeStr, type == 0 ? "TCP" : (type == 1 ? "UDP" : "HTTP"));
                    snprintf(sizeStr, sizeof(sizeStr), "%db", 64 + rand()%1400);
                    
                    // Row alternating color
                    Color rowCol = (i % 2 == 0) ? Fade(COLOR_GHOST, 0.8f) : Fade(COLOR_CYAN, 0.6f);
                    if (type == 2) rowCol = COLOR_TOXIC; // Highlight HTTP
                    
                    DrawScaledText(timeStr, colTime, yPos, 8, Fade(COLOR_GHOST, 0.4f));
                    DrawScaledText(srcStr, colSrc, yPos, 8, rowCol);
                    DrawScaledText(dstStr, colDst, yPos, 8, rowCol);
                    DrawScaledText(typeStr, colType, yPos, 8, (type==2) ? COLOR_TOXIC : Fade(COLOR_GHOST, 0.5f));
                    DrawScaledText(sizeStr, colSize, yPos, 8, Fade(COLOR_GHOST, 0.4f));
                }
                
                // Sniffer Stats Footer
                float statsY = snifferY + snifferH - 20;
                DrawScaledLine(mainX + 5, statsY, mainX + mainW - 5, statsY, Fade(COLOR_CYAN, 0.2f));
                char statsLine[128];
                snprintf(statsLine, sizeof(statsLine), "PKT/S: %d  |  TOTAL: %d  |  DROPS: 0", (int)(t * 34.7f) % 1000 + 200, (int)(t * 128.3f) % 99999 + 10000);
                DrawScaledText(statsLine, mainX + 8, statsY + 6, 8, Fade(COLOR_TOXIC, 0.7f));

                // ------------------------------------------------------------
                // MODULE 4: OPERATOR DIALOGUE
                // ------------------------------------------------------------
                DrawScaledRect(mainX, opY, mainW, opH, Color{10, 2, 2, 180}); // Slight red tint
                DrawScaledRectLines(mainX, opY, mainW, opH, Fade(COLOR_BLOOD, 0.4f));
                
                DrawScaledRect(mainX, opY, mainW, 16, Fade(COLOR_BLOOD, 0.2f));
                DrawScaledText(":: COMMS LINK ::", mainX + 8, opY + 4, 8, COLOR_BLOOD);
                
                // Blinking prompt
                if ((int)(t * 2.0f) % 2 == 0) {
                    DrawScaledRect(mainX + 8, opY + 24, 6, 12, COLOR_TOXIC);
                }
                
                DrawScaledText(g_player.raidSeqOperatorDialogue, mainX + 20, opY + 25, 11, COLOR_TOXIC);
            }
            
            // ============================================================
            // RIGHT PANEL: TERMINAL WINDOW
            // ============================================================
            {
                float winX = rightX + 8;
                float winY = 40;
                float winW = rightW - 16;
                float winH = REF_HEIGHT - 80;
                
                // ---- TERMINAL WINDOW FRAME ----
                DrawScaledRect(winX + 4, winY + 4, winW, winH, Color{0, 0, 0, 80});
                DrawScaledRect(winX, winY, winW, winH, COLOR_PANEL);
                DrawScaledRectLines(winX, winY, winW, winH, Fade(COLOR_CYAN, 0.25f));
                
                // ---- TITLE BAR ----
                float titleH = 28.0f;
                DrawScaledRect(winX, winY, winW, titleH, Color{16, 18, 28, 220});
                DrawScaledLine(winX, winY + titleH, winX + winW, winY + titleH, Fade(COLOR_CYAN, 0.15f));
                
                DrawScaledText("█ RAID TERMINAL", winX + 12, winY + 6, 10, COLOR_TOXIC);
                
                // ---- TERMINAL CONTENT ----
                float contentX = winX + 6;
                float contentY = winY + titleH + 6;
                float contentW = winW - 12;
                float contentH = winH - titleH - 12;
                
                DrawScaledRect(contentX, contentY, contentW, contentH, COLOR_CLI_BG);
                DrawScaledRectLines(contentX, contentY, contentW, contentH, Fade(COLOR_CYAN, 0.05f));
                
                // ---- CLI LOGS (compact) ----
                float lineY = contentY + 4;
                float fontSize = 9.0f;
                float lineHeight = 16.0f;
                int maxLines = (int)(contentH / lineHeight) - 2;
                int totalLines = g_cliLogCount;
                int startLine = totalLines - maxLines;
                if (startLine < 0) startLine = 0;
                
                if (g_player.cliScroll > 0.0f) {
                    int scrollLines = (int)(g_player.cliScroll / lineHeight);
                    startLine = totalLines - maxLines - scrollLines;
                    if (startLine < 0) startLine = 0;
                }
                
                for (int i = startLine; i < g_cliLogCount && i < startLine + maxLines; i++) {
                    if (i < 0) continue;
                    const char* log = g_cliLogs[i];
                    
                    Color logColor = COLOR_GHOST;
                    if (strstr(log, "[FEDERAL RAID]") || strstr(log, "[RAID]")) logColor = COLOR_BLOOD;
                    else if (strstr(log, "[ERROR]") || strstr(log, "[ERR]")) logColor = COLOR_BLOOD;
                    else if (strstr(log, "[WARNING]") || strstr(log, "[WARN]")) logColor = COLOR_AMBER;
                    else if (strstr(log, "[SUCCESS]") || strstr(log, "[MINER]")) logColor = COLOR_TOXIC;
                    else if (strstr(log, "[SCAN]") || strstr(log, "[PAGE]")) logColor = COLOR_CYAN;
                    
                    DrawScaledText(log, contentX + 4, lineY, fontSize, logColor);
                    lineY += lineHeight;
                }
                
                // ---- INPUT LINE ----
                float inputY = contentY + contentH - 24;
                DrawScaledRect(contentX + 2, inputY, contentW - 4, 20, Color{8, 10, 14, 220});
                DrawScaledRectLines(contentX + 2, inputY, contentW - 4, 20, Fade(COLOR_CYAN, 0.15f));
                
                DrawScaledText(">", contentX + 8, inputY + 4, fontSize, COLOR_TOXIC);
                DrawScaledText(g_player.inputBuffer, contentX + 20, inputY + 4, fontSize, COLOR_GHOST);
                
                if ((int)(GetTime() * 2.0f) % 2 == 0) {
                    float cursorX = contentX + 20 + MeasureScaledTextWidth(g_player.inputBuffer, fontSize);
                    DrawScaledRect(cursorX, inputY + 2, 5, 14, COLOR_TOXIC);
                }
                
                // ---- SCROLLBAR ----
                if (totalLines > maxLines) {
                    float scrollbarX = contentX + contentW - 6;
                    float scrollbarH = contentH - 4;
                    float visibleRatio = (float)maxLines / totalLines;
                    float thumbH = scrollbarH * visibleRatio;
                    if (thumbH < 12.0f) thumbH = 12.0f;
                    
                    float maxScrollPx = (totalLines - maxLines) * lineHeight;
                    float scrollRatio = 0.0f;
                    if (maxScrollPx > 0.0f) {
                        scrollRatio = g_player.cliScroll / maxScrollPx;
                        if (scrollRatio > 1.0f) scrollRatio = 1.0f;
                    }
                    float thumbY = contentY + 2 + (scrollbarH - thumbH) * scrollRatio;
                    
                    DrawScaledRect(scrollbarX, contentY + 2, 4, scrollbarH, Color{30, 35, 50, 80});
                    DrawScaledRect(scrollbarX, thumbY, 4, thumbH, Fade(COLOR_CYAN, 0.3f));
                }
                
                // ---- HELP TEXT ----
                DrawScaledText("[ENTER] send  |  [TAB] focus  |  [↑↓] scroll", 
                            contentX + 4, contentY + contentH - 46, 7, Fade(COLOR_GHOST, 0.3f));
            }
            
            // ---- DIVIDER LINE ----
            DrawScaledLine(leftW, 0, leftW, REF_HEIGHT, Fade(COLOR_CYAN, 0.05f));
            
            break;
        }
        case RAID_SEQ_SUCCESS:
            // Green flash with "RAID BYPASSED"
            {
                float alpha = 1.0f - (t / 2.0f);
                DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, Fade(GREEN, alpha * 0.6f));
                const char* msg = "⚡ RAID BYPASSED ⚡";
                float w = MeasureScaledTextWidth(msg, 48);
                // Shadow
                DrawScaledText(msg, (REF_WIDTH - w) / 2 + 2, REF_HEIGHT / 2 - 18 + 2, 48, Fade(BLACK, alpha * 0.5f));
                DrawScaledText(msg, (REF_WIDTH - w) / 2, REF_HEIGHT / 2 - 20, 48, Fade(COLOR_TOXIC, alpha));
            }
            break;

        case RAID_SEQ_FAILURE:
            // Red flash with "RAID SUCCESS"
            {
                float alpha = 1.0f - (t / 2.0f);
                DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, Fade(RED, alpha * 0.6f));
                const char* msg = "💀 RAID SUCCESS 💀";
                float w = MeasureScaledTextWidth(msg, 48);
                // Shadow
                DrawScaledText(msg, (REF_WIDTH - w) / 2 + 2, REF_HEIGHT / 2 - 18 + 2, 48, Fade(BLACK, alpha * 0.5f));
                DrawScaledText(msg, (REF_WIDTH - w) / 2, REF_HEIGHT / 2 - 20, 48, Fade(COLOR_BLOOD, alpha));
            }
            break;
    }
}

// ============================================================
// NETWORK VISUALIZATION - Replaces 3D Operator Face
// ============================================================
void DrawNetworkVisualization(float x, float y, float w, float h) {
    float t = (float)GetTime();
    float pulse = sinf(t * 3.0f) * 0.3f + 0.7f;
    
    // ---- BACKGROUND PANEL ----
    DrawScaledRect(x, y, w, h, Color{6, 8, 14, 230});
    DrawScaledRectLines(x, y, w, h, Fade(COLOR_CYAN, 0.3f));
    
    // ---- HEADER ----
    DrawScaledText("█ PORT PROBE // NODE SWAP", x + 12, y + 8, 11, COLOR_TOXIC);
    DrawScaledLine(x + 10, y + 26, x + w - 10, y + 26, Fade(COLOR_CYAN, 0.2f));
    
    // ---- NETWORK NODES ----
    float centerX = x + w / 2.0f;
    float centerY = y + h / 2.0f + 10.0f;
    float radius = 60.0f;
    int nodeCount = 6;
    
    // Draw connection lines between nodes
    for (int i = 0; i < nodeCount; i++) {
        float angle1 = t * 0.3f + (i / (float)nodeCount) * 6.28318f;
        float angle2 = t * 0.3f + ((i + 1) / (float)nodeCount) * 6.28318f;
        float x1 = centerX + cosf(angle1) * radius;
        float y1 = centerY + sinf(angle1) * radius;
        float x2 = centerX + cosf(angle2) * radius;
        float y2 = centerY + sinf(angle2) * radius;
        
        // Data flow along the line (moving dots)
        float flowPos = fmodf(t * 0.8f + i * 0.2f, 1.0f);
        float fx = x1 + (x2 - x1) * flowPos;
        float fy = y1 + (y2 - y1) * flowPos;
        
        // Glowing connection
        DrawScaledLine(x1, y1, x2, y2, Fade(COLOR_CYAN, 0.15f + 0.1f * pulse));
        // Data packet
        DrawScaledRect(fx - 2, fy - 2, 4, 4, Fade(COLOR_TOXIC, 0.6f + 0.3f * pulse));
    }
    
    // Draw nodes with port numbers
    for (int i = 0; i < nodeCount; i++) {
        float angle = t * 0.3f + (i / (float)nodeCount) * 6.28318f;
        float nx = centerX + cosf(angle) * radius;
        float ny = centerY + sinf(angle) * radius;
        float nodePulse = sinf(t * 2.0f + i * 1.2f) * 0.3f + 0.7f;
        
        // Node glow
        DrawScaledCircle(nx, ny, 12.0f * nodePulse, Fade(COLOR_BLOOD, 0.15f));
        DrawScaledCircle(nx, ny, 8.0f, Fade(COLOR_CYAN, 0.25f));
        
        // Node core
        Color nodeColor = (i % 2 == 0) ? COLOR_CYAN : COLOR_TOXIC;
        DrawScaledRect(nx - 4, ny - 4, 8, 8, Fade(nodeColor, 0.8f));
        
        // Port label
        char portStr[8];
        int port = 8000 + i * 37 + (int)(t * 1.5f) % 100;
        snprintf(portStr, sizeof(portStr), ":%d", port % 1000 + 8000);
        DrawScaledText(portStr, nx - 12, ny + 14, 7, Fade(COLOR_GHOST, 0.6f));
    }
    
    // ---- SCANNING PULSE ----
    float scanAngle = t * 0.5f;
    float scanX = centerX + cosf(scanAngle) * (radius + 25.0f);
    float scanY = centerY + sinf(scanAngle) * (radius + 25.0f);
    DrawScaledCircle(scanX, scanY, 6.0f, Fade(COLOR_AMBER, 0.8f));
    DrawScaledCircle(scanX, scanY, 18.0f, Fade(COLOR_AMBER, 0.15f));
    
    // ---- STATS BAR ----
    float statsY = y + h - 28;
    DrawScaledLine(x + 10, statsY, x + w - 10, statsY, Fade(COLOR_CYAN, 0.15f));
    
    char stats[128];
    int packets = (int)(t * 12.3f) % 999 + 100;
    int hops = (int)(t * 0.7f) % 5 + 3;
    snprintf(stats, sizeof(stats), "PACKETS: %04d  |  HOPS: %d  |  TRACE: %d%%  |  PORT: %d",
             packets, hops, g_player.traceLevel, g_player.port);
    DrawScaledText(stats, x + 12, statsY + 6, 8, Fade(COLOR_GHOST, 0.7f));
    
    // ---- GLITCH BORDER ----
    if (pulse > 0.85f) {
        float glitchX = x + rand() % (int)w;
        DrawScaledRect(glitchX, y, 2 + rand() % 8, h, Fade(COLOR_BLOOD, 0.1f));
    }
}