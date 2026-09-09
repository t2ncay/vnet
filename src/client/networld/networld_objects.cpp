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
#include <algorithm>

static void PlaceRoom(int x, int y, int w, int d, std::vector<std::vector<int>>& tiles) {
    for (int i = x; i < x + w && i < (int)tiles.size(); i++) {
        for (int j = y; j < y + d && j < (int)tiles[0].size(); j++) {
            tiles[i][j] = 0; // floor
        }
    }
}

// ------------------------------------------------------------------
// Helper: carve a corridor between two rooms (L-shaped)
// ------------------------------------------------------------------
static void CarveCorridor(int x1, int y1, int x2, int y2, std::vector<std::vector<int>>& tiles) {
    // Move along X first, then Z
    int stepX = (x2 > x1) ? 1 : -1;
    int stepZ = (y2 > y1) ? 1 : -1;
    int curX = x1, curZ = y1;
    while (curX != x2) {
        tiles[curX][curZ] = 0;
        curX += stepX;
    }
    while (curZ != y2) {
        tiles[curX][curZ] = 0;
        curZ += stepZ;
    }
    tiles[curX][curZ] = 0; // final cell
}

// ============================================================
// GENERATE NETWORLD
// ============================================================

void GenerateNetWorld(void) {
    g_netWorld.nodes.clear();
    g_netWorld.building.wallBoxes.clear();
    g_netWorld.building.rooms.clear();

    // --- 1. Grid size ---
    const int GRID_W = 20;
    const int GRID_D = 20;
    const float TILE = 4.0f;
    g_netWorld.building.gridWidth = GRID_W;
    g_netWorld.building.gridDepth = GRID_D;
    g_netWorld.building.tileSize = TILE;

    // Initialize all tiles as walls (1)
    std::vector<std::vector<int>> tiles(GRID_W, std::vector<int>(GRID_D, 1));
    std::vector<Rectangle> rooms;

    // --- 2. Place rooms (random rectangles) ---
    int numRooms = 6 + rand() % 5; // 6-10 rooms
    for (int i = 0; i < numRooms; i++) {
        int rw = 2 + rand() % 3; // width 2-4 tiles
        int rd = 2 + rand() % 3; // depth 2-4 tiles
        int rx = rand() % (GRID_W - rw - 2) + 1; // ensure margin from edges
        int ry = rand() % (GRID_D - rd - 2) + 1;
        PlaceRoom(rx, ry, rw, rd, tiles);
        rooms.push_back({(float)rx, (float)ry, (float)rw, (float)rd});
    }

    // --- 3. Connect rooms with corridors ---
    // For simplicity, connect each room to the next in the list
    for (int i = 0; i < (int)rooms.size() - 1; i++) {
        int x1 = (int)rooms[i].x + (int)rooms[i].width / 2;
        int y1 = (int)rooms[i].y + (int)rooms[i].height / 2;
        int x2 = (int)rooms[i+1].x + (int)rooms[i+1].width / 2;
        int y2 = (int)rooms[i+1].y + (int)rooms[i+1].height / 2;
        CarveCorridor(x1, y1, x2, y2, tiles);
    }

    // Store the tile map and room list in g_netWorld
    g_netWorld.building.tiles = tiles;
    g_netWorld.building.rooms = rooms;

    // --- 4. Build wall collision boxes ---
    // For each wall tile, create a bounding box (tile size, height 3 units)
    for (int x = 0; x < GRID_W; x++) {
        for (int z = 0; z < GRID_D; z++) {
            if (tiles[x][z] == 1) {
                Vector3 pos = { x * TILE + TILE/2, 1.5f, z * TILE + TILE/2 };
                Vector3 size = { TILE, 3.0f, TILE };
                BoundingBox box = {
                    { pos.x - size.x/2, pos.y - size.y/2, pos.z - size.z/2 },
                    { pos.x + size.x/2, pos.y + size.y/2, pos.z + size.z/2 }
                };
                g_netWorld.building.wallBoxes.push_back(box);
            }
        }
    }

    // --- 5. Place nodes (portals, data, enemies) in rooms ---
    // Use the rooms list; place at least one portal per room, plus random nodes.

    // Core node at the first room's center
    Rectangle firstRoom = rooms[0];
    Vector3 corePos = {
        (firstRoom.x + firstRoom.width/2) * TILE,
        1.0f,
        (firstRoom.y + firstRoom.height/2) * TILE
    };
    NetNode core;
    core.type = NodeType::CORE;
    core.id = "core";
    core.label = "VEKTRA CORE";
    core.position = corePos;
    core.spawnPosition = corePos;
    core.color = {1.0f, 0.6f, 0.0f};
    core.radius = 2.5f;
    core.active = true;
    core.discovered = true;
    core.pulsePhase = 0.0f;
    core.rotationAngle = 0.0f;
    g_netWorld.nodes.push_back(core);

    // Place portals in each room (skip the first room if already core)
    for (int i = 1; i < (int)rooms.size(); i++) {
        Rectangle r = rooms[i];
        Vector3 pos = {
            (r.x + r.width/2) * TILE,
            0.8f,
            (r.y + r.height/2) * TILE
        };
        // Choose a site ID from assigned or default
        std::string siteId;
        if (i < g_player.assignedCount && i < 20) {
            siteId = g_player.assignedSites[i];
        } else {
            const char* defaultSites[] = {
                "market.vnet", "vault.vnet", "terminal.vnet", "crypto.vnet", "hellroom.vnet",
                "forum.vnet", "redroom.vnet", "void.vnet", "ghost.vnet", "orbital.vnet"
            };
            siteId = defaultSites[i % 10];
        }
        NetNode portal;
        portal.type = NodeType::PORTAL;
        portal.id = siteId;
        portal.label = siteId;
        portal.position = pos;
        portal.spawnPosition = pos;
        portal.color = {0.0f, 0.8f, 1.0f};
        portal.radius = 1.2f;
        portal.active = true;
        portal.discovered = true;
        portal.pulsePhase = (float)i * 1.5f;
        portal.rotationAngle = 0.0f;
        g_netWorld.nodes.push_back(portal);
    }

    // Place data nodes and enemies in random rooms (some may share)
    const char* dataLabels[] = {
        "ENCRYPTED LOG", "VFS CACHE", "ROUTE TABLE", "ICE CONFIG",
        "SESSION DATA", "KEY FRAGMENT", "NETMAP", "BGP ROUTES"
    };
    for (int i = 0; i < 8; i++) {
        int roomIdx = rand() % rooms.size();
        Rectangle r = rooms[roomIdx];
        // Slight random offset within room
        float offX = (rand() % 100) / 100.0f * 0.8f - 0.4f;
        float offZ = (rand() % 100) / 100.0f * 0.8f - 0.4f;
        Vector3 pos = {
            (r.x + r.width/2 + offX) * TILE,
            0.6f,
            (r.y + r.height/2 + offZ) * TILE
        };
        NetNode data;
        data.type = NodeType::DATA_NODE;
        data.id = "data_" + std::to_string(i);
        data.label = dataLabels[i % 8];
        data.position = pos;
        data.spawnPosition = pos;
        data.color = {0.6f, 0.6f, 0.8f};
        data.radius = 0.6f;
        data.active = true;
        data.discovered = false;
        data.pulsePhase = (float)i * 2.3f;
        data.rotationAngle = 0.0f;
        g_netWorld.nodes.push_back(data);
    }

    // Enemies (glitch bots) in corridors or rooms
    for (int i = 0; i < 5; i++) {
        // Place in a corridor: find a floor tile that is not a room
        // Simpler: pick a random floor tile that is not in any room (corridor)
        // We'll just place them in rooms for simplicity
        int roomIdx = rand() % rooms.size();
        Rectangle r = rooms[roomIdx];
        float offX = (rand() % 100) / 100.0f * 0.8f - 0.4f;
        float offZ = (rand() % 100) / 100.0f * 0.8f - 0.4f;
        Vector3 pos = {
            (r.x + r.width/2 + offX) * TILE,
            0.6f,
            (r.y + r.height/2 + offZ) * TILE
        };
        NetNode enemy;
        enemy.type = NodeType::ENEMY;
        enemy.id = "enemy_" + std::to_string(i);
        enemy.label = "GLITCH_BOT";
        enemy.position = pos;
        enemy.spawnPosition = pos;
        enemy.color = {1.0f, 0.2f, 0.2f};
        enemy.radius = 0.8f;
        enemy.active = true;
        enemy.discovered = false;
        enemy.pulsePhase = (float)i * 3.7f;
        enemy.rotationAngle = 0.0f;
        enemy.isHostile = true;
        g_netWorld.nodes.push_back(enemy);
    }

    Vector3 spawnPos = {
        (firstRoom.x + firstRoom.width/2) * TILE,
        0.5f,
        (firstRoom.y + firstRoom.height/2) * TILE
    };
    g_netWorld.player.position = spawnPos;
    printf("[NETWORLD] Player spawn: %.1f, %.1f, %.1f\n", spawnPos.x, spawnPos.y, spawnPos.z);
    printf("[NETWORLD] Building center: %.1f, 0, %.1f\n", GRID_W * TILE / 2, GRID_D * TILE / 2);
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