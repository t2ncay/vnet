#include "networld.h"
#include "../../shared/vnet.h"
#include "../../shared/vnet_sites.h"
#include "../vnet_client.h"
#include "../render.h"
#include "../desktop/desktop.h"
#include "raymath.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstring>

// ============================================================
// GENERATE NETWORLD
// ============================================================

void GenerateNetWorld(void) {
    g_netWorld.nodes.clear();
    
    float halfSize = g_netWorld.worldSize / 2.0f;
    float spacing = g_netWorld.worldSize / 6.0f;
    
    // ---- CORE NODE (center) ----
    NetNode core;
    core.type = NodeType::CORE;
    core.id = "core";
    core.label = "VEKTRA CORE";
    core.position = {0.0f, 1.0f, 0.0f};
    core.spawnPosition = core.position;
    core.color = {1.0f, 0.6f, 0.0f};
    core.radius = 2.5f;
    core.active = true;
    core.discovered = true;
    core.pulsePhase = 0.0f;
    core.rotationAngle = 0.0f;
    g_netWorld.nodes.push_back(core);
    
    // ---- PORTAL NODES (for VNET sites) ----
    std::vector<std::string> siteIds;
    for (int i = 0; i < g_player.assignedCount && i < 20; i++) {
        siteIds.push_back(g_player.assignedSites[i]);
    }
    
    // If no sites assigned, use default ones
    if (siteIds.empty()) {
        const char* defaultSites[] = {
            "market.vnet", "vault.vnet", "terminal.vnet", "crypto.vnet", "hellroom.vnet",
            "forum.vnet", "redroom.vnet", "void.vnet", "ghost.vnet", "orbital.vnet"
        };
        for (int i = 0; i < 10; i++) {
            siteIds.push_back(defaultSites[i]);
        }
    }
    
    // Place portals in a ring around the core
    int portalCount = (int)siteIds.size();
    for (int i = 0; i < portalCount && i < 20; i++) {
        float angle = (i / (float)portalCount) * 2.0f * PI;
        float radius = 15.0f + (i % 3) * 5.0f;
        
        float x = cosf(angle) * radius;
        float z = sinf(angle) * radius;
        
        NetNode portal;
        portal.type = NodeType::PORTAL;
        portal.id = siteIds[i];
        portal.label = siteIds[i];
        portal.position = {x, 0.8f, z};
        portal.spawnPosition = portal.position;
        portal.color = {0.0f, 0.8f, 1.0f};
        portal.radius = 1.2f;
        portal.active = true;
        portal.discovered = true;
        portal.pulsePhase = (float)i * 1.5f;
        portal.rotationAngle = 0.0f;
        g_netWorld.nodes.push_back(portal);
    }
    
    // ---- DATA NODES (scattered) ----
    const char* dataLabels[] = {
        "ENCRYPTED LOG",
        "VFS CACHE",
        "ROUTE TABLE",
        "ICE CONFIG",
        "SESSION DATA",
        "KEY FRAGMENT",
        "NETMAP",
        "BGP ROUTES"
    };
    
    for (int i = 0; i < 8; i++) {
        float angle = (i * 1.7f) + 0.5f;
        float radius = 25.0f + (rand() % 100) / 100.0f * 15.0f;
        
        float x = cosf(angle) * radius;
        float z = sinf(angle) * radius;
        
        NetNode dataNode;
        dataNode.type = NodeType::DATA_NODE;
        dataNode.id = "data_" + std::to_string(i);
        dataNode.label = dataLabels[i % 8];
        dataNode.position = {x, 0.6f, z};
        dataNode.spawnPosition = dataNode.position;
        dataNode.color = {0.6f, 0.6f, 0.8f};
        dataNode.radius = 0.6f;
        dataNode.active = true;
        dataNode.discovered = false;
        dataNode.pulsePhase = (float)i * 2.3f;
        dataNode.rotationAngle = 0.0f;
        g_netWorld.nodes.push_back(dataNode);
    }
    
    // ---- ENEMY NODES (glitch bots) ----
    for (int i = 0; i < 3; i++) {
        float angle = (i * 2.1f) + 1.2f;
        float radius = 20.0f + (rand() % 100) / 100.0f * 10.0f;
        
        float x = cosf(angle) * radius;
        float z = sinf(angle) * radius;
        
        NetNode enemy;
        enemy.type = NodeType::ENEMY;
        enemy.id = "enemy_" + std::to_string(i);
        enemy.label = "GLITCH_BOT";
        enemy.position = {x, 0.6f, z};
        // Anchor used by the enemy-orbit patrol motion in
        // UpdateNetWorld() (networld_core.cpp) so bots circle a fixed
        // point rather than drifting.
        enemy.spawnPosition = enemy.position;
        enemy.color = {1.0f, 0.2f, 0.2f};
        enemy.radius = 0.8f;
        enemy.active = true;
        enemy.discovered = false;
        enemy.pulsePhase = (float)i * 3.7f;
        enemy.rotationAngle = 0.0f;
        g_netWorld.nodes.push_back(enemy);
    }
    
    printf("[NETWORLD] Generated %zu nodes\n", g_netWorld.nodes.size());
}

// ============================================================
// INTERACT WITH NODE
// ============================================================

void InteractWithNode(const std::string& siteId) {
    // Find the node
    NetNode* node = GetNetNode(siteId);
    if (!node) return;
    
    switch (node->type) {
        case NodeType::PORTAL: {
            PushCliLog("[NETWORLD] Entering portal to %s", siteId.c_str());
            GetDesktop().PushHellroomMessage("[NETWORLD] User entered portal to %s", siteId.c_str());
            
            // ---- EXIT NETWORLD ----
            ExitNetWorld();
            
            // ---- NAVIGATE TO THE SITE IN BROWSER ----
            // Open browser if not already open
            bool browserOpen = false;
            for (int i = 0; i < (int)GetDesktop().m_windows.size(); i++) {
                if (GetDesktop().m_windows[i].type == AppType::Browser) {
                    browserOpen = true;
                    GetDesktop().FocusWindow(i);
                    if (GetDesktop().m_windows[i].minimized) {
                        GetDesktop().m_windows[i].minimized = false;
                    }
                    break;
                }
            }
            
            if (!browserOpen) {
                GetDesktop().OpenApp(AppType::Browser, "VNET Browser");
            }
            
            // Navigate to the site
            char navURL[128];
            snprintf(navURL, sizeof(navURL), "%s", siteId.c_str());
            TriggerRouteNavigation(navURL);
            
            break;
        }
        
        case NodeType::DATA_NODE: {
            // Collect data node
            if (!node->discovered) {
                node->discovered = true;
                float reward = 0.05f + (rand() % 20) / 100.0f;
                g_player.vcoin += reward;
                PushCliLog("[NETWORLD] Data node collected! +%.2f VCOIN", reward);
                GetDesktop().PushHellroomMessage("[NETWORLD] Data node collected! +%.2f VCOIN", reward);
                
                // Change color to indicate collected
                node->color = {0.4f, 1.0f, 0.4f};
                TriggerGlitch(0.3f);
            } else {
                PushCliLog("[NETWORLD] Data node already collected.");
            }
            break;
        }
        
        case NodeType::ENEMY: {
            // Engage enemy
            PushCliLog("[NETWORLD] Engaging GLITCH_BOT!");
            GetDesktop().PushHellroomMessage("[NETWORLD] Player engaged a glitch bot!");
            
            // Simple fight: random outcome
            int result = rand() % 3;
            if (result == 0) {
                // Win
                PushCliLog("[NETWORLD] ✅ GLITCH_BOT destroyed!");
                GetDesktop().PushHellroomMessage("[NETWORLD] Glitch bot destroyed!");
                node->active = false;
                node->color = {0.2f, 0.2f, 0.2f};
                
                float reward = 0.15f + (rand() % 20) / 100.0f;
                g_player.vcoin += reward;
                PushCliLog("[NETWORLD] +%.2f VCOIN reward!", reward);
                TriggerGlitch(0.5f);
            } else if (result == 1) {
                // Lose
                PushCliLog("[NETWORLD] ❌ GLITCH_BOT damaged you!");
                GetDesktop().PushHellroomMessage("[NETWORLD] Glitch bot damaged player!");
                g_player.traceLevel += 10;
                if (g_player.traceLevel > 100) g_player.traceLevel = 100;
                TriggerGlitch(1.0f);
                TriggerJitter(0.5f, 1.0f);
            } else {
                // Draw
                PushCliLog("[NETWORLD] ⚡ GLITCH_BOT evaded!");
                TriggerGlitch(0.2f);
            }
            break;
        }
        
        case NodeType::CORE: {
            PushCliLog("[NETWORLD] VEKTRA CORE selected.");
            GetDesktop().PushHellroomMessage("[NETWORLD] User is at the VEKTRA CORE!");
            // Show some info or stats
            break;
        }
        
        default:
            break;
    }
}

// ============================================================
// GET NODE BY ID
// ============================================================

NetNode* GetNetNode(const std::string& siteId) {
    for (auto& node : g_netWorld.nodes) {
        if (node.id == siteId) {
            return &node;
        }
    }
    return nullptr;
}

// ============================================================
// COMMAND: networld
// ============================================================

void NetWorldCommand(const char* args) {
    if (!args) {
        if (IsInNetWorld()) {
            PushCliLog("[NETWORLD] Currently in cyberspace. Type 'exit' to leave.");
            return;
        }
        PushCliLog("[NETWORLD] Usage: networld [enter|exit|status]");
        PushCliLog("[NETWORLD]   enter  - Enter cyberspace");
        PushCliLog("[NETWORLD]   exit   - Leave cyberspace");
        PushCliLog("[NETWORLD]   status - Show current state");
        return;
    }
    
    if (strcmp(args, "enter") == 0) {
        if (IsInNetWorld()) {
            PushCliLog("[NETWORLD] Already in cyberspace.");
            return;
        }
        EnterNetWorld();
    }
    else if (strcmp(args, "exit") == 0) {
        if (!IsInNetWorld()) {
            PushCliLog("[NETWORLD] Not in cyberspace.");
            return;
        }
        ExitNetWorld();
    }
    else if (strcmp(args, "status") == 0) {
        if (IsInNetWorld()) {
            const char* stateStr = "ACTIVE";
            switch (g_netWorld.state) {
                case NetWorldState::ENTERING: stateStr = "ENTERING"; break;
                case NetWorldState::ACTIVE: stateStr = "ACTIVE"; break;
                case NetWorldState::EXITING: stateStr = "EXITING"; break;
                default: stateStr = "UNKNOWN"; break;
            }
            PushCliLog("[NETWORLD] Status: %s", stateStr);
            PushCliLog("[NETWORLD] Nodes: %zu", g_netWorld.nodes.size());
            PushCliLog("[NETWORLD] Position: %.1f, %.1f, %.1f",
                      g_netWorld.player.position.x,
                      g_netWorld.player.position.y,
                      g_netWorld.player.position.z);
        } else {
            PushCliLog("[NETWORLD] Status: IDLE (not in cyberspace)");
        }
    }
    else {
        PushCliLog("[NETWORLD] Unknown subcommand: %s", args);
    }
}