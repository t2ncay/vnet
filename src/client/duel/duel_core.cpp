#include "duel.h"
#include "../render.h"
#include "../../shared/vnet.h"
#include "../../shared/vnet_protocol.h"
#include "../vnet_client.h"
#include "../desktop/desktop.h"
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>

Duel g_duel = {0};
char g_duelInputBuffer[256] = {0};

void InitDuelSystem(void) {
    memset(&g_duel, 0, sizeof(Duel));
    g_duel.active = false;
    g_duel.state = DuelState::IDLE;
    g_duel.maxDuration = 45.0f;
    g_duel.showSniffer = true;
    g_duel.showNetworkMap = true;
    g_duel.showTerminal = true;
    printf("[DUEL] System initialized.\n");
}

bool IsDuelActive(void) {
    return g_duel.active && g_duel.state != DuelState::COMPLETE;
}

void UpdateDuel(float dt) {
    if (!g_duel.active) return;

    // intro sequence
    if (g_duel.state == DuelState::STARTING) {
        g_duel.introTimer -= dt;
        if (g_duel.introTimer <= 0.0f) {
            g_duel.state = DuelState::ACTIVE;
            g_duel.introTimer = 0.0f;
            g_duel.actionLog.push_back("[SYSTEM] Exploit initialized. Duel active.");
            PushCliLog("[DUEL] Duel is now active!");
        }
        return;  // skip timer/glitch updates during intro
    }
    
    g_duel.timer += dt;
    g_duel.scanlineOffset += dt * 2.0f;
    g_duel.hexOffset += dt * 0.5f;
    
    // Glitch intensity increases as timer runs out
    float remaining = g_duel.maxDuration - g_duel.timer;
    if (remaining < 10.0f) {
        g_duel.glitchIntensity = 1.0f - (remaining / 10.0f);
        if (g_duel.glitchIntensity > 0.3f) {
            TriggerGlitch(g_duel.glitchIntensity * 0.4f);
        }
    } else {
        g_duel.glitchIntensity = 0.0f;
    }
    
    // Check timer expiry
    if (g_duel.timer >= g_duel.maxDuration && g_duel.state == DuelState::ACTIVE) {
        // Timeout – draw
        g_duel.state = DuelState::COMPLETE;
        PushCliLog("[DUEL] Timeout! No one captured the flag.");
        ApplyDuelRewards(false);
        // After a short delay, reset
        g_duel.active = false;
        g_duel.state = DuelState::IDLE;
        return;
    }
    
    // Check flag capture
    if (g_duel.flagCaptured) {
        g_duel.flagCaptureTime += dt;
        if (g_duel.flagCaptureTime > 3.0f) {
            g_duel.state = DuelState::COMPLETE;
            bool attackerWon = (g_duel.flagCapturedBy == g_duel.attackerHandle);
            ApplyDuelRewards(attackerWon);
            g_duel.active = false;
            g_duel.state = DuelState::IDLE;
        }
    }
}

void ApplyDuelRewards(bool attackerWon) {
    float attackerReward = 0.0f, defenderReward = 0.0f;
    if (attackerWon) {
        attackerReward = 1.50f;
        defenderReward = 0.50f;
        PushCliLog("[DUEL] Attacker %s wins! +%.2f VCOIN", g_duel.attackerHandle.c_str(), attackerReward);
        PushCliLog("[DUEL] Defender %s loses. +%.2f VCOIN consolation.", g_duel.defenderHandle.c_str(), defenderReward);
    } else if (g_duel.flagCaptured) {
        // Defender captured
        attackerReward = -0.50f;
        defenderReward = 1.50f;
        PushCliLog("[DUEL] Defender %s wins! +%.2f VCOIN", g_duel.defenderHandle.c_str(), defenderReward);
        PushCliLog("[DUEL] Attacker %s loses. -%.2f VCOIN penalty.", g_duel.attackerHandle.c_str(), -attackerReward);
    } else {
        // Timeout draw
        attackerReward = 0.25f;
        defenderReward = 0.25f;
        PushCliLog("[DUEL] Timeout – draw. Both get +0.25 VCOIN.");
    }
    
    // Apply to local player (in a real multiplayer, these would be server-side)
    // For single-player or local testing, we apply directly.
    if (g_duel.localRole == DuelRole::ATTACKER) {
        g_player.vcoin += attackerReward;
        if (attackerWon) g_player.raidCount++; // track wins
    } else {
        g_player.vcoin += defenderReward;
        if (!attackerWon && g_duel.flagCaptured) g_player.raidCount++;
    }
    // Clamp VCOIN
    if (g_player.vcoin < 0) g_player.vcoin = 0;
    
    // Broadcast result (if multiplayer)
    if (IsVNetConnected()) {
        std::string resultMsg = "DUEL_RESULT:" + std::to_string(attackerWon) + ":" +
                                g_duel.attackerHandle + ":" + g_duel.defenderHandle;
        VNetSendRaw(resultMsg);
    }
}