#include "raid.h"
#include "../../shared/vnet.h"
#include "../render.h"
#include "../desktop/desktop.h"
#include "../game.h"
#include <cstdlib>
#include <ctime>

extern Player g_player;

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