#include "duel.h"
#include "../../shared/vnet_protocol.h"
#include "../../shared/vnet.h"
#include "../vnet_client.h"
#include <cstring>

// ============================================================
// SEND DUEL ACTION TO SERVER
// ============================================================

void SendDuelAction(const std::string& action, const std::string& payload) {
    if (!IsVNetConnected()) return;
    std::string packet = std::string(VNetCmd::DUEL_ACTION) + ":" + action;
    if (!payload.empty()) packet += ":" + payload;
    VNetSendRaw(packet);
}

// ============================================================
// RECEIVE DUEL PACKET (called from UpdateVNET)
// ============================================================

void ReceiveDuelPacket(const std::string& cmd, const std::string& payload) {
    if (cmd == "DUEL_START") {
        // Format: attacker:defender:role
        size_t sep1 = payload.find(':');
        size_t sep2 = payload.find(':', sep1 + 1);
        if (sep1 != std::string::npos && sep2 != std::string::npos) {
            std::string attacker = payload.substr(0, sep1);
            std::string defender = payload.substr(sep1 + 1, sep2 - sep1 - 1);
            std::string roleStr = payload.substr(sep2 + 1);
            
            g_duel.active = true;
            g_duel.state = DuelState::STARTING;      // 👈 STARTING, not ACTIVE
            g_duel.introTimer = 6.0f;                // 👈 6-second intro
            g_duel.attackerHandle = attacker;
            g_duel.defenderHandle = defender;
            g_duel.localRole = (roleStr == "ATTACKER") ? DuelRole::ATTACKER : DuelRole::DEFENDER;
            g_duel.timer = 0.0f;
            g_duel.maxDuration = 45.0f;

            // ... rest of player setup ...

            g_duel.actionLog.clear();
            g_duel.actionLog.push_back("[SYSTEM] Duel started!");
            PushCliLog("[DUEL] Duel started: %s vs %s", attacker.c_str(), defender.c_str());
        }
    }
    else if (cmd == "DUEL_ACTION_RES") {
        // Result of an action: success:msg
        size_t sep = payload.find(':');
        if (sep != std::string::npos) {
            bool success = (payload.substr(0, sep) == "1");
            std::string msg = payload.substr(sep + 1);
            g_duel.actionLog.push_back(success ? "[SUCCESS] " + msg : "[FAIL] " + msg);
            PushCliLog("[DUEL] %s", msg.c_str());
            // Update progress based on result (would need to track which objective)
        }
    }
    else if (cmd == "DUEL_RESULT") {
        // Final result: winner:attacker:defender
        size_t sep1 = payload.find(':');
        size_t sep2 = payload.find(':', sep1 + 1);
        if (sep1 != std::string::npos && sep2 != std::string::npos) {
            bool attackerWon = (payload.substr(0, sep1) == "1");
            std::string attacker = payload.substr(sep1 + 1, sep2 - sep1 - 1);
            std::string defender = payload.substr(sep2 + 1);
            g_duel.state = DuelState::COMPLETE;
            g_duel.active = false;
            ApplyDuelRewards(attackerWon);
            PushCliLog("[DUEL] Duel result: %s wins!", attackerWon ? attacker.c_str() : defender.c_str());
            g_duel.state = DuelState::IDLE;
        }
    }
}

// ============================================================
// GENERATE OBJECTIVES (same for both roles, but role-specific)
// ============================================================

#include <algorithm>
#include <random>

// ============================================================
// GENERATE OBJECTIVES (FIXED)
// ============================================================

void GenerateObjectives(DuelPlayer& player, DuelRole role) {
    player.objectives.clear();
    player.objectivesCompleted = 0;
    
    std::vector<DuelObjective> pool;
    if (role == DuelRole::ATTACKER) {
        pool = {
            {DuelObjectiveType::SCAN, "Scan Target Node", "Recon the defender's node", "scan", 0.05f, 0.1f, false, 0.0f, {"Use 'scan'"}},
            {DuelObjectiveType::PORTSCAN, "Find Open Port", "Discover an open port", "portscan <port>", 0.05f, 0.2f, false, 0.0f, {"Try ports 8000-8100"}},
            {DuelObjectiveType::EXPLOIT, "Exploit Vulnerability", "Use an exploit to gain access", "exploit", 0.15f, 0.4f, false, 0.0f, {"Costs 0.15 VCOIN"}},
            {DuelObjectiveType::EXTRACT, "Extract Key Fragment", "Pull a key fragment from memory", "extract", 0.10f, 0.5f, false, 0.0f, {"Requires exploit first"}},
            {DuelObjectiveType::ESCALATE, "Escalate Privileges", "Gain higher access", "escalate", 0.25f, 0.7f, false, 0.0f, {"Costs 0.25 VCOIN"}},
            {DuelObjectiveType::CAPTURE, "Capture the Flag!", "Claim the victory flag", "capture", 0.30f, 0.9f, false, 0.0f, {"All previous objectives required"}}
        };
    } else {
        pool = {
            {DuelObjectiveType::TRACE, "Trace Attacker", "Identify attacker IP", "trace", 0.05f, 0.1f, false, 0.0f, {"Use 'trace'"}},
            {DuelObjectiveType::WHOIS, "Identify Target IP", "Resolve attacker node", "whois", 0.05f, 0.2f, false, 0.0f, {"Use 'whois'"}},
            {DuelObjectiveType::DECOY, "Deploy Decoy", "Misdirect with a decoy", "decoy", 0.10f, 0.4f, false, 0.0f, {"Costs 0.10 VCOIN"}},
            {DuelObjectiveType::COUNTER, "Counter-Hack", "Launch a counter-attack", "counter", 0.15f, 0.5f, false, 0.0f, {"Requires trace"}},
            {DuelObjectiveType::PATCH, "Patch & Lock", "Secure vulnerabilities", "patch", 0.20f, 0.7f, false, 0.0f, {"Costs 0.20 VCOIN"}},
            {DuelObjectiveType::CAPTURE, "Capture the Flag!", "Claim the victory flag", "capture", 0.30f, 0.9f, false, 0.0f, {"All previous objectives required"}}
        };
    }
    
    // Always include the first objective (scan/trace)
    player.objectives.push_back(pool[0]);
    
    // Shuffle the pool (excluding the first and last elements)
    // Use std::shuffle with a random generator
    std::random_device rd;
    std::mt19937 g(rd());
    
    // We want to shuffle elements from index 1 to size-2 (exclude capture)
    std::shuffle(pool.begin() + 1, pool.end() - 1, g);
    
    // Pick 3-4 random objectives from the shuffled pool
    int count = 3 + rand() % 3; // 3-5 total, but we already have one
    int pick = std::min(count, (int)pool.size() - 2);
    for (int i = 0; i < pick; i++) {
        player.objectives.push_back(pool[i + 1]);
    }
    
    // Ensure capture is last
    player.objectives.push_back(pool[pool.size() - 1]);
}