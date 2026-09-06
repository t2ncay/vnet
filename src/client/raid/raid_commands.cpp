#include "raid.h"
#include "../../shared/vnet.h"
#include "../render.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>

extern Player g_player;

// ============================================================
// RAID COMMAND PROCESSOR
// ============================================================

void ProcessRaidCommand(const char* token, const char* args, const char* cmd) {
    // === RAID STATUS ===
    if (strcmp(token, "raidstatus") == 0 || strcmp(token, "raid") == 0) {
        if (!g_player.raidActive) {
            PushCliLog("[RAID]: No active federal raid detected.");
            PushCliLog("[RAID]: Current trace: %d%% | Flagged: %s", 
                       g_player.traceLevel, g_player.isFlagged ? "YES" : "NO");
            PushCliLog("[RAID]: Raids survived: %d | Failed: %d", 
                       g_player.raidCount, g_player.raidFailedCount);
            if (g_player.raidCooldown > 0.0f) {
                PushCliLog("[RAID]: Cooldown: %.0fs remaining", g_player.raidCooldown);
            }
            return;
        }
        
        // Active raid - show status
        const char* stageNames[] = {"ALERT", "CHOICE", "MINIGAME", "RESULT", "COMPLETE"};
        const char* typeNames[] = {"EVADE", "ESCAPE", "BURN"};
        
        PushCliLog("[RAID]: ⚡ ACTIVE FEDERAL E-RAID!");
        PushCliLog("[RAID]: Stage: %s", stageNames[g_player.raidStage + 1]);
        PushCliLog("[RAID]: Type: %s", typeNames[g_player.raidType]);
        PushCliLog("[RAID]: Time remaining: %.1fs", GetRaidRemainingTime());
        
        if (g_player.raidStage == RAID_STAGE_CHOICE) {
            PushCliLog("[RAID]: SELECT EVASION STRATEGY:");
            PushCliLog("  raid 1  - EVADE (deploy decoys)");
            PushCliLog("  raid 2  - ESCAPE (port migration)");
            PushCliLog("  raid 3  - BURN (scorched earth)");
        } else if (g_player.raidStage == RAID_STAGE_MINIGAME) {
            switch (g_player.raidType) {
                case RAID_EVADE:
                    PushCliLog("[RAID]: TYPE: decoy <node> <port>");
                    break;
                case RAID_ESCAPE:
                    PushCliLog("[RAID]: TYPE: patch <port>");
                    break;
                case RAID_BURN:
                    PushCliLog("[RAID]: TYPE: purge <target>");
                    break;
            }
        }
        return;
    }
    
    // === RAID CHOICE ===
    if (strcmp(token, "raidchoice") == 0) {
        if (!args) {
            PushCliLog("[RAID]: Usage: raidchoice <1|2|3>");
            return;
        }
        HandleRaidChoice(args);
        return;
    }
    
    // === DECOY (override for raid) ===
    if (strcmp(token, "decoy") == 0) {
        // Check if this is a raid minigame
        if (g_player.raidActive && g_player.raidStage == RAID_STAGE_MINIGAME && 
            g_player.raidType == RAID_EVADE) {
            ProcessRaidMinigameInput(cmd);
            return;
        }
        // Otherwise, let existing decoy logic handle it
        // (fall through to existing command handler)
    }
    
    // === PATCH (override for raid) ===
    if (strcmp(token, "patch") == 0) {
        if (g_player.raidActive && g_player.raidStage == RAID_STAGE_MINIGAME && 
            g_player.raidType == RAID_ESCAPE) {
            ProcessRaidMinigameInput(cmd);
            return;
        }
        // Otherwise, let existing patch logic handle it
    }
    
    // === PURGE (override for raid) ===
    if (strcmp(token, "purge") == 0) {
        if (g_player.raidActive && g_player.raidStage == RAID_STAGE_MINIGAME && 
            g_player.raidType == RAID_BURN) {
            ProcessRaidMinigameInput(cmd);
            return;
        }
        // Otherwise, let existing purge logic handle it
    }
}