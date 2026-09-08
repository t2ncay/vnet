#include "duel.h"
#include "../../shared/vnet.h"
#include "../../shared/vnet_protocol.h"
#include "../vnet_client.h"
#include <cstring>
#include <cstdio>

// ============================================================
// DUEL COMMANDS (called from ProcessCommand)
// ============================================================

// ============================================================
// PROCESS DUEL COMMAND (called from game.cpp)
// ============================================================
void ProcessDuelCommand(const char* cmd) {
    if (!IsDuelActive()) {
        PushCliLog("[DUEL] No active duel.");
        return;
    }

    char copy[256];
    strncpy(copy, cmd, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    char* token = strtok(copy, " ");
    if (!token) return;

    char* args = strtok(nullptr, "");

    // --- Attacker commands ---
    if (strcmp(token, "scan") == 0) {
        DuelScan();
    }
    else if (strcmp(token, "portscan") == 0) {
        if (!args) {
            g_duel.actionLog.push_back("[SYSTEM] Usage: portscan <port>");
        } else {
            DuelPortScan(args);
        }
    }
    else if (strcmp(token, "exploit") == 0) {
        DuelExploit();
    }
    else if (strcmp(token, "extract") == 0) {
        DuelExtract();
    }
    else if (strcmp(token, "escalate") == 0) {
        DuelEscalate();
    }
    else if (strcmp(token, "capture") == 0) {
        DuelCapture();
    }

    // --- Defender commands ---
    else if (strcmp(token, "trace") == 0) {
        DuelTrace();
    }
    else if (strcmp(token, "whois") == 0) {
        DuelWhois();
    }
    else if (strcmp(token, "decoy") == 0) {
        DuelDecoy();
    }
    else if (strcmp(token, "counter") == 0) {
        DuelCounter();
    }
    else if (strcmp(token, "patch") == 0) {
        DuelPatch();
    }

    // --- Help ---
    else if (strcmp(token, "help") == 0) {
        g_duel.actionLog.push_back("[SYSTEM] Available duel commands:");
        g_duel.actionLog.push_back("  Attacker: scan, portscan <port>, exploit, extract, escalate, capture");
        g_duel.actionLog.push_back("  Defender: trace, whois, decoy, counter, patch");
        PushCliLog("[DUEL] Type 'help' for available commands.");
    }

    // --- Unknown ---
    else {
        char log[128];
        snprintf(log, sizeof(log), "[SYSTEM] Unknown command: %s", token);
        g_duel.actionLog.push_back(log);
        PushCliLog("[DUEL] Unknown command: %s", token);
    }
}

void DuelHack(const char* target) {
    if (IsDuelActive()) {
        PushCliLog("[DUEL] Already in a duel.");
        return;
    }
    if (g_player.vcoin < 0.50f) {
        PushCliLog("[DUEL] Need 0.50 VCOIN to initiate a duel.");
        return;
    }
    g_player.vcoin -= 0.50f;
    PushCliLog("[DUEL] Initiating duel with %s...", target);

    // ============================================================
    // FORCE OFFLINE MOCK FOR TESTING (when target is "CPU")
    // ============================================================
    if (strcmp(target, "CPU") == 0 || !IsVNetConnected()) {
        PushCliLog("[DUEL] Offline mode – starting mock duel.");

        g_duel.active = true;
        g_duel.state = DuelState::STARTING;      // 👈 STARTING, not ACTIVE
        g_duel.introTimer = 6.0f;                // 👈 6-second intro
        g_duel.localRole = DuelRole::ATTACKER;
        g_duel.attackerHandle = g_player.handle;
        g_duel.defenderHandle = "CPU";
        g_duel.attackerPort = g_player.port;
        g_duel.defenderPort = 8002;
        g_duel.timer = 0.0f;
        g_duel.maxDuration = 45.0f;

        g_duel.attacker.handle = g_player.handle;
        g_duel.attacker.port = g_player.port;
        g_duel.attacker.trace = g_player.traceLevel;
        g_duel.attacker.iceShields = g_player.iceShields;
        g_duel.attacker.vcoin = g_player.vcoin;

        g_duel.defender.handle = "CPU";
        g_duel.defender.port = 8002;
        g_duel.defender.trace = 20;
        g_duel.defender.iceShields = 1;
        g_duel.defender.vcoin = 0.50f;

        GenerateObjectives(g_duel.attacker, DuelRole::ATTACKER);
        GenerateObjectives(g_duel.defender, DuelRole::DEFENDER);

        g_duel.actionLog.clear();
        g_duel.actionLog.push_back("[SYSTEM] Initializing exploit...");

        printf("[DEBUG] Offline mock started. Active: %d, State: STARTING\n", g_duel.active);
        return;
    }

    // ============================================================
    // ONLINE BRANCH – when server supports DUEL_INIT
    // ============================================================
    if (IsVNetConnected()) {
        std::string payload = std::string(VNetCmd::DUEL_INIT) + ":" + target;
        VNetSendRaw(payload);
        PushCliLog("[DUEL] Challenge sent to %s (waiting for server response).", target);
    }
}

// Attacker commands
void DuelScan(void) {
    if (!IsDuelActive()) return;
    if (g_duel.localRole != DuelRole::ATTACKER) return;
    DuelPlayer& player = g_duel.attacker;
    // Check objective
    for (auto& obj : player.objectives) {
        if (obj.completed) continue;
        if (obj.type == DuelObjectiveType::SCAN) {
            bool success = (rand() % 100) < 80; // 80% success
            if (success) {
                obj.completed = true;
                player.objectivesCompleted++;
                PushCliLog("[DUEL] ✅ SCAN successful! Open ports revealed.");
                g_duel.actionLog.push_back("[SUCCESS] Scan completed.");
            } else {
                PushCliLog("[DUEL] ❌ SCAN failed. Try again.");
                g_duel.actionLog.push_back("[FAIL] Scan failed.");
            }
            AdvanceDuelProgress(player);
            return;
        }
    }
    PushCliLog("[DUEL] Scan objective already complete or not available.");
}

void DuelPortScan(const char* port) {
    if (!IsDuelActive()) return;
    if (g_duel.localRole != DuelRole::ATTACKER) return;
    DuelPlayer& player = g_duel.attacker;
    // Check if we have an open port objective
    bool found = false;
    for (auto& obj : player.objectives) {
        if (obj.completed) continue;
        if (obj.type == DuelObjectiveType::PORTSCAN) {
            // Success if port is in a reasonable range
            int p = atoi(port);
            if (p >= 8000 && p <= 8999) {
                obj.completed = true;
                player.objectivesCompleted++;
                PushCliLog("[DUEL] ✅ Port %s is open!", port);
                g_duel.actionLog.push_back("[SUCCESS] Open port found.");
                found = true;
                AdvanceDuelProgress(player);
            } else {
                PushCliLog("[DUEL] ❌ Port %s is closed. Try another.", port);
                g_duel.actionLog.push_back("[FAIL] Port closed.");
            }
            break;
        }
    }
    if (!found) {
        PushCliLog("[DUEL] No port-scan objective active.");
    }
}

// Similar implementations for other commands (exploit, extract, escalate, capture, trace, etc.)
// Each checks if the corresponding objective is incomplete, attempts with success probability,
// and advances progress on success.

// Helper to advance progress
void AdvanceDuelProgress(DuelPlayer& player) {
    float total = (float)player.objectives.size();
    float completed = (float)player.objectivesCompleted;
    player.progress = (completed / total) * 100.0f;
    // Check if all objectives complete – then flag capture is ready
    if (player.objectivesCompleted == (int)player.objectives.size()) {
        PushCliLog("[DUEL] All objectives complete! Type 'capture' to claim the flag!");
        g_duel.actionLog.push_back("[SYSTEM] All objectives complete. Capture the flag!");
    }
}

// Capture command
void DuelCapture(void) {
    if (!IsDuelActive()) return;
    DuelPlayer* player = nullptr;
    if (g_duel.localRole == DuelRole::ATTACKER) player = &g_duel.attacker;
    else player = &g_duel.defender;
    if (!player) return;
    if (player->objectivesCompleted < (int)player->objectives.size()) {
        PushCliLog("[DUEL] Complete all objectives first!");
        return;
    }
    // Attempt capture (30% success)
    bool success = (rand() % 100) < 30;
    if (success) {
        g_duel.flagCaptured = true;
        g_duel.flagCapturedBy = player->handle;
        g_duel.flagCaptureTime = 0.0f;
        g_duel.state = DuelState::FLAG_CAPTURED;
        PushCliLog("[DUEL] 🏆 FLAG CAPTURED by %s!", player->handle.c_str());
        g_duel.actionLog.push_back("[FLAG] " + player->handle + " captured the flag!");
    } else {
        PushCliLog("[DUEL] ❌ Capture failed! Try again.");
        g_duel.actionLog.push_back("[FAIL] Capture failed.");
    }
}

// ============================================================
// DEFENDER COMMANDS
// ============================================================

void DuelTrace(void) {
    if (!IsDuelActive()) return;
    if (g_duel.localRole != DuelRole::DEFENDER) return;
    DuelPlayer& player = g_duel.defender;
    for (auto& obj : player.objectives) {
        if (obj.completed) continue;
        if (obj.type == DuelObjectiveType::TRACE) {
            bool success = (rand() % 100) < 80;
            if (success) {
                obj.completed = true;
                player.objectivesCompleted++;
                PushCliLog("[DUEL] ✅ TRACE successful! Attacker IP identified.");
                g_duel.actionLog.push_back("[SUCCESS] Trace completed.");
            } else {
                PushCliLog("[DUEL] ❌ TRACE failed. Try again.");
                g_duel.actionLog.push_back("[FAIL] Trace failed.");
            }
            AdvanceDuelProgress(player);
            return;
        }
    }
    PushCliLog("[DUEL] Trace objective already complete or not available.");
}

void DuelWhois(void) {
    if (!IsDuelActive()) return;
    if (g_duel.localRole != DuelRole::DEFENDER) return;
    DuelPlayer& player = g_duel.defender;
    for (auto& obj : player.objectives) {
        if (obj.completed) continue;
        if (obj.type == DuelObjectiveType::WHOIS) {
            bool success = (rand() % 100) < 80;
            if (success) {
                obj.completed = true;
                player.objectivesCompleted++;
                PushCliLog("[DUEL] ✅ WHOIS successful! Attacker node resolved.");
                g_duel.actionLog.push_back("[SUCCESS] Whois completed.");
            } else {
                PushCliLog("[DUEL] ❌ WHOIS failed. Try again.");
                g_duel.actionLog.push_back("[FAIL] Whois failed.");
            }
            AdvanceDuelProgress(player);
            return;
        }
    }
    PushCliLog("[DUEL] Whois objective already complete or not available.");
}

void DuelDecoy(void) {
    if (!IsDuelActive()) return;
    if (g_duel.localRole != DuelRole::DEFENDER) return;
    if (g_player.vcoin < 0.10f) {
        PushCliLog("[DUEL] Need 0.10 VCOIN for decoy.");
        return;
    }
    g_player.vcoin -= 0.10f;
    DuelPlayer& player = g_duel.defender;
    for (auto& obj : player.objectives) {
        if (obj.completed) continue;
        if (obj.type == DuelObjectiveType::DECOY) {
            bool success = (rand() % 100) < 60;
            if (success) {
                obj.completed = true;
                player.objectivesCompleted++;
                PushCliLog("[DUEL] ✅ DECOY deployed! Attacker misdirected.");
                g_duel.actionLog.push_back("[SUCCESS] Decoy deployed.");
            } else {
                PushCliLog("[DUEL] ❌ DECOY failed. Try again.");
                g_duel.actionLog.push_back("[FAIL] Decoy failed.");
            }
            AdvanceDuelProgress(player);
            return;
        }
    }
    PushCliLog("[DUEL] Decoy objective already complete or not available.");
}

void DuelCounter(void) {
    if (!IsDuelActive()) return;
    if (g_duel.localRole != DuelRole::DEFENDER) return;
    if (g_player.vcoin < 0.15f) {
        PushCliLog("[DUEL] Need 0.15 VCOIN for counter-hack.");
        return;
    }
    g_player.vcoin -= 0.15f;
    DuelPlayer& player = g_duel.defender;
    for (auto& obj : player.objectives) {
        if (obj.completed) continue;
        if (obj.type == DuelObjectiveType::COUNTER) {
            bool success = (rand() % 100) < 50;
            if (success) {
                obj.completed = true;
                player.objectivesCompleted++;
                PushCliLog("[DUEL] ✅ COUNTER-HACK successful! Attacker repelled.");
                g_duel.actionLog.push_back("[SUCCESS] Counter-hack completed.");
            } else {
                PushCliLog("[DUEL] ❌ COUNTER-HACK failed. Try again.");
                g_duel.actionLog.push_back("[FAIL] Counter-hack failed.");
            }
            AdvanceDuelProgress(player);
            return;
        }
    }
    PushCliLog("[DUEL] Counter-hack objective already complete or not available.");
}

void DuelPatch(void) {
    if (!IsDuelActive()) return;
    if (g_duel.localRole != DuelRole::DEFENDER) return;
    if (g_player.vcoin < 0.20f) {
        PushCliLog("[DUEL] Need 0.20 VCOIN for patch.");
        return;
    }
    g_player.vcoin -= 0.20f;
    DuelPlayer& player = g_duel.defender;
    for (auto& obj : player.objectives) {
        if (obj.completed) continue;
        if (obj.type == DuelObjectiveType::PATCH) {
            bool success = (rand() % 100) < 40;
            if (success) {
                obj.completed = true;
                player.objectivesCompleted++;
                PushCliLog("[DUEL] ✅ PATCH successful! Vulnerabilities locked.");
                g_duel.actionLog.push_back("[SUCCESS] Patch completed.");
            } else {
                PushCliLog("[DUEL] ❌ PATCH failed. Try again.");
                g_duel.actionLog.push_back("[FAIL] Patch failed.");
            }
            AdvanceDuelProgress(player);
            return;
        }
    }
    PushCliLog("[DUEL] Patch objective already complete or not available.");
}

// ============================================================
// ATTACKER COMMANDS (Missing ones)
// ============================================================

void DuelExploit(void) {
    if (!IsDuelActive()) return;
    if (g_duel.localRole != DuelRole::ATTACKER) return;
    if (g_player.vcoin < 0.15f) {
        PushCliLog("[DUEL] Need 0.15 VCOIN for exploit.");
        return;
    }
    g_player.vcoin -= 0.15f;
    DuelPlayer& player = g_duel.attacker;
    for (auto& obj : player.objectives) {
        if (obj.completed) continue;
        if (obj.type == DuelObjectiveType::EXPLOIT) {
            bool success = (rand() % 100) < 60;
            if (success) {
                obj.completed = true;
                player.objectivesCompleted++;
                PushCliLog("[DUEL] ✅ EXPLOIT successful! Vulnerability breached.");
                g_duel.actionLog.push_back("[SUCCESS] Exploit completed.");
            } else {
                PushCliLog("[DUEL] ❌ EXPLOIT failed. Try again.");
                g_duel.actionLog.push_back("[FAIL] Exploit failed.");
            }
            AdvanceDuelProgress(player);
            return;
        }
    }
    PushCliLog("[DUEL] Exploit objective already complete or not available.");
}

void DuelExtract(void) {
    if (!IsDuelActive()) return;
    if (g_duel.localRole != DuelRole::ATTACKER) return;
    if (g_player.vcoin < 0.10f) {
        PushCliLog("[DUEL] Need 0.10 VCOIN for extraction.");
        return;
    }
    g_player.vcoin -= 0.10f;
    DuelPlayer& player = g_duel.attacker;
    for (auto& obj : player.objectives) {
        if (obj.completed) continue;
        if (obj.type == DuelObjectiveType::EXTRACT) {
            bool success = (rand() % 100) < 50;
            if (success) {
                obj.completed = true;
                player.objectivesCompleted++;
                PushCliLog("[DUEL] ✅ EXTRACT successful! Key fragment retrieved.");
                g_duel.actionLog.push_back("[SUCCESS] Extract completed.");
            } else {
                PushCliLog("[DUEL] ❌ EXTRACT failed. Try again.");
                g_duel.actionLog.push_back("[FAIL] Extract failed.");
            }
            AdvanceDuelProgress(player);
            return;
        }
    }
    PushCliLog("[DUEL] Extract objective already complete or not available.");
}

void DuelEscalate(void) {
    if (!IsDuelActive()) return;
    if (g_duel.localRole != DuelRole::ATTACKER) return;
    if (g_player.vcoin < 0.25f) {
        PushCliLog("[DUEL] Need 0.25 VCOIN for escalation.");
        return;
    }
    g_player.vcoin -= 0.25f;
    DuelPlayer& player = g_duel.attacker;
    for (auto& obj : player.objectives) {
        if (obj.completed) continue;
        if (obj.type == DuelObjectiveType::ESCALATE) {
            bool success = (rand() % 100) < 40;
            if (success) {
                obj.completed = true;
                player.objectivesCompleted++;
                PushCliLog("[DUEL] ✅ ESCALATE successful! Privileges elevated.");
                g_duel.actionLog.push_back("[SUCCESS] Escalate completed.");
            } else {
                PushCliLog("[DUEL] ❌ ESCALATE failed. Try again.");
                g_duel.actionLog.push_back("[FAIL] Escalate failed.");
            }
            AdvanceDuelProgress(player);
            return;
        }
    }
    PushCliLog("[DUEL] Escalate objective already complete or not available.");
}