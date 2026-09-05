#include "../shared/vnet.h"
#include "../shared/vnet_protocol.h"
#include "../shared/vnet_sites.h"
#include "../lib/vnet_lib.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdarg>
#include <algorithm>
#include <atomic>
#include <csignal>

// ============================================================
// GLOBAL SERVER STATE
// ============================================================

static SocketHandle g_serverSock = INVALID_SOCKET_HANDLE;
static int g_serverPort = 8000;
static bool g_gameOver = false;

// Peer management
static std::vector<int> g_activePorts;
static std::vector<std::string> g_activeIPs;
static std::vector<std::string> g_activeURLs;
static std::vector<float> g_activeLastSeen;
static std::vector<int> g_activeDOSHits;
static std::vector<std::string> g_activeDirs;
static std::vector<std::string> g_activeHandles;

// Server state
static float g_serverUptime = 0.0f;
static int g_targetHash = 0;
static int g_currentSalt = 0;
static std::vector<std::string> g_allSites;

// Key system
static int g_k1, g_k2, g_k3, g_k4, g_k5, g_k6, g_k7, g_k8;
static int g_shift1, g_shift2, g_shift3, g_shift4, g_shift5, g_shift6, g_shift7, g_shift8;
static int g_scrambled1, g_scrambled2, g_scrambled3, g_scrambled4, g_scrambled5, g_scrambled6, g_scrambled7, g_scrambled8;
static std::string g_masterKeysStr;
static std::vector<std::string> g_keyLocations;
static std::vector<int> g_usedKeyIndices;

// Mining
static std::vector<std::string> g_activeBlocks;
static float g_mineBlockTimer = 0.0f;

// Decoys
static std::vector<int> g_decoyTargets;
static std::vector<int> g_decoyOwners;
static std::vector<std::string> g_decoyURLs;

// Proxy chains
static std::vector<int> g_proxySources;
static std::vector<std::string> g_proxyNodes;
static std::vector<std::string> g_proxyTargets;

// VFS
static std::vector<std::string> g_vfsPaths;
static std::vector<std::string> g_vfsData;

// Overloaded sites
static std::vector<std::string> g_overloadedURLs;
static std::vector<float> g_overloadedTimers;

// ============================================================
// HELPER FUNCTIONS
// ============================================================

static std::string CleanStr(const std::string& raw) {
    std::string out = raw;
    // Trim leading
    while (!out.empty() && (out[0] == ' ' || out[0] == '\"' || out[0] == '\'' || out[0] == '\t' || out[0] == '\r' || out[0] == '\n')) {
        out.erase(0, 1);
    }
    // Trim trailing
    while (!out.empty() && (out.back() == ' ' || out.back() == '\"' || out.back() == '\'' || out.back() == '\t' || out.back() == '\r' || out.back() == '\n')) {
        out.pop_back();
    }
    return out;
}

static std::string MaskPort(int port) {
    std::string p = std::to_string(port);
    if (p.length() >= 4) {
        p[p.length() - 1] = 'X';
    }
    return p;
}

static void BroadcastFeed(const std::string& msg) {
    for (size_t i = 0; i < g_activePorts.size(); i++) {
        VNetLib::SendTo(g_serverSock, g_activeIPs[i], g_activePorts[i], 
                       std::string(VNetResp::FEED_EVENT) + msg);
    }
}

static void BroadcastRaw(const std::string& msg) {
    for (size_t i = 0; i < g_activePorts.size(); i++) {
        VNetLib::SendTo(g_serverSock, g_activeIPs[i], g_activePorts[i], msg);
    }
}

static void UnregisterPeer(int port) {
    for (size_t i = 0; i < g_activePorts.size(); i++) {
        if (g_activePorts[i] == port) {
            g_activePorts.erase(g_activePorts.begin() + i);
            g_activeIPs.erase(g_activeIPs.begin() + i);
            g_activeURLs.erase(g_activeURLs.begin() + i);
            g_activeLastSeen.erase(g_activeLastSeen.begin() + i);
            g_activeDOSHits.erase(g_activeDOSHits.begin() + i);
            g_activeDirs.erase(g_activeDirs.begin() + i);
            g_activeHandles.erase(g_activeHandles.begin() + i);
            break;
        }
    }
    printf("[REGISTRY] Purged port %d from active peers\n", port);
}

static void PruneInactivePeers(float now) {
    for (int i = (int)g_activePorts.size() - 1; i >= 0; i--) {
        if (now - g_activeLastSeen[i] > 10.0f) {
            int deadPort = g_activePorts[i];
            UnregisterPeer(deadPort);
            BroadcastFeed("[TIMEOUT] Node port " + MaskPort(deadPort) + " timed out");
        }
    }
}

static std::string RefreshMiningBlocks() {
    g_activeBlocks.clear();
    std::string payload = "NEW_BLOCKS:";
    for (int i = 0; i < 8; i++) {
        int blockID = rand() % 9000 + 1000;
        float payout = (rand() % 46 + 5) / 100.0f;
        std::string entry = std::to_string(blockID) + ":" + std::to_string(payout);
        g_activeBlocks.push_back(entry);
        payload += entry + (i < 7 ? ";" : "");
    }
    return payload;
}

static void SendKeySync(int port, const std::string& ip, const std::string& dirPayload) {
    std::string sync = std::string(VNetResp::KEY_SYNC) +
        std::to_string(g_scrambled1) + ":" + std::to_string(g_scrambled2) + ":" +
        std::to_string(g_scrambled3) + ":" + std::to_string(g_scrambled4) + ":" +
        std::to_string(g_scrambled5) + ":" + std::to_string(g_scrambled6) + ":" +
        std::to_string(g_scrambled7) + ":" + std::to_string(g_scrambled8) + ":" +
        g_keyLocations[0] + ":" + g_keyLocations[1] + ":" +
        g_keyLocations[2] + ":" + g_keyLocations[3] + ":" +
        g_keyLocations[4] + ":" + g_keyLocations[5] + ":" +
        g_keyLocations[6] + ":" + g_keyLocations[7] + ":" +
        dirPayload;
    
    VNetLib::SendTo(g_serverSock, ip, port, sync);
}

static void CheckAndTripDecoy(const std::string& target, int attackerPort) {
    for (size_t i = 0; i < g_decoyURLs.size(); i++) {
        if (g_decoyURLs[i] == target && g_decoyOwners[i] != attackerPort) {
            // Trip the decoy
            int ownerPort = g_decoyOwners[i];
            std::string ownerIP = "127.0.0.1";
            for (size_t p = 0; p < g_activePorts.size(); p++) {
                if (g_activePorts[p] == ownerPort) {
                    ownerIP = g_activeIPs[p];
                    break;
                }
            }
            VNetLib::SendTo(g_serverSock, ownerIP, ownerPort,
                          std::string(VNetResp::DECOY_TRIPPED) + "Port " + std::to_string(attackerPort) + " ambushed at " + target);
            BroadcastFeed("[DECOY TRAP] Port " + MaskPort(attackerPort) + " tripped a decoy at " + target);
            
            g_decoyTargets.erase(g_decoyTargets.begin() + i);
            g_decoyOwners.erase(g_decoyOwners.begin() + i);
            g_decoyURLs.erase(g_decoyURLs.begin() + i);
            break;
        }
    }
}

// ============================================================
// GENERATE KEYS
// ============================================================

static void GenerateKeys() {
    const int MULTIPLIER = 37;
    const int MODULUS = 100000;
    
    g_k1 = rand() % 9000 + 1000;
    g_k2 = rand() % 9000 + 1000;
    g_k3 = rand() % 9000 + 1000;
    g_k4 = rand() % 9000 + 1000;
    g_k5 = rand() % 9000 + 1000;
    g_k6 = rand() % 9000 + 1000;
    g_k7 = rand() % 9000 + 1000;
    g_k8 = rand() % 9000 + 1000;
    
    g_shift1 = rand() % 7 + 1;
    g_shift2 = rand() % 7 + 1;
    g_shift3 = rand() % 7 + 1;
    g_shift4 = rand() % 7 + 1;
    g_shift5 = rand() % 7 + 1;
    g_shift6 = rand() % 7 + 1;
    g_shift7 = rand() % 7 + 1;
    g_shift8 = rand() % 7 + 1;
    
    g_scrambled1 = (g_k1 * MULTIPLIER + g_shift1) % MODULUS;
    g_scrambled2 = (g_k2 * MULTIPLIER + g_shift2) % MODULUS;
    g_scrambled3 = (g_k3 * MULTIPLIER + g_shift3) % MODULUS;
    g_scrambled4 = (g_k4 * MULTIPLIER + g_shift4) % MODULUS;
    g_scrambled5 = (g_k5 * MULTIPLIER + g_shift5) % MODULUS;
    g_scrambled6 = (g_k6 * MULTIPLIER + g_shift6) % MODULUS;
    g_scrambled7 = (g_k7 * MULTIPLIER + g_shift7) % MODULUS;
    g_scrambled8 = (g_k8 * MULTIPLIER + g_shift8) % MODULUS;
    
    g_masterKeysStr = std::to_string(g_k1) + " " + std::to_string(g_k2) + " " +
                      std::to_string(g_k3) + " " + std::to_string(g_k4) + " " +
                      std::to_string(g_k5) + " " + std::to_string(g_k6) + " " +
                      std::to_string(g_k7) + " " + std::to_string(g_k8);
    
    // Key locations
    g_keyLocations.clear();
    g_usedKeyIndices.clear();
    for (int i = 0; i < 8; i++) {
        int idx = rand() % g_allSites.size();
        while (std::find(g_usedKeyIndices.begin(), g_usedKeyIndices.end(), idx) != g_usedKeyIndices.end()) {
            idx = (idx + 1) % g_allSites.size();
        }
        g_usedKeyIndices.push_back(idx);
        g_keyLocations.push_back(g_allSites[idx]);
    }
}

// ============================================================
// SERVER INIT
// ============================================================

static void InitServerState() {
    srand((unsigned)time(NULL));
    
    // All 54 sites - FIXED: use sizeof to get exact count
    const char* siteList[] = {
        "market.vnet", "vault.vnet", "terminal.vnet", "crypto.vnet", "hellroom.vnet",
        "redroom.vnet", "dollhouse.vnet", "morgue.vnet", "snuff.vnet", "asylum.vnet",
        "cult.vnet", "skinwalker.vnet", "corridor204863.vnet", "ghost.vnet", "schizo.vnet",
        "necro.vnet", "void.vnet", "silkroad.vnet", "zeroauction.vnet", "blackbank.vnet",
        "weaponry.vnet", "passports.vnet", "darkdrop.vnet", "vektrapay.vnet", "bounty.vnet",
        "watchtower.vnet", "orbital.vnet", "cctv-core.vnet", "eye.vnet", "substation04.vnet",
        "stasi.vnet", "deadchannel.vnet", "signal0.vnet", "deepocean.vnet", "norilsk-relay.vnet",
        "forum.vnet", "deepwiki.vnet", "pastebin.vnet", "whisper.vnet", "dump.vnet",
        "index.vnet", "project9.vnet", "echolab.vnet", "phantom.vnet", "glitch.vnet",
        "stasis.vnet", "entropy.vnet", "hive.vnet", "nexus.vnet", "hashbeat.vnet",
        "lifeleaks.vnet", "luna.vnet", "subcell.vnet", "feed99.vnet"
    };
    
    // Calculate exact number of sites
    int siteCount = sizeof(siteList) / sizeof(siteList[0]);
    printf("[DEBUG] Loading %d sites\n", siteCount);
    fflush(stdout);
    
    g_allSites.clear();
    for (int i = 0; i < siteCount; i++) {
        g_allSites.push_back(siteList[i]);
    }
    printf("[DEBUG] Loaded %zu sites\n", g_allSites.size());
    fflush(stdout);
    
    g_targetHash = rand() % 9000 + 1000;
    g_currentSalt = rand() % 90 + 10;
    printf("[DEBUG] Hash: %d, Salt: %d\n", g_targetHash, g_currentSalt);
    fflush(stdout);
    
    printf("[DEBUG] Calling GenerateKeys...\n");
    fflush(stdout);
    GenerateKeys();
    printf("[DEBUG] GenerateKeys done\n");
    fflush(stdout);
    
    printf("[DEBUG] Calling RefreshMiningBlocks...\n");
    fflush(stdout);
    RefreshMiningBlocks();
    printf("[DEBUG] RefreshMiningBlocks done\n");
    fflush(stdout);
    
    // VFS
    g_vfsPaths = {"/sys/firewall.cfg", "/sys/logs.txt", "/vault/data.key", "/sys/config.txt"};
    g_vfsData = {"PORT_80_OPEN=TRUE", "LOG_INIT_SUCCESS", "FLAG{VYNE_VNET_ROOT_ACCESS}", g_masterKeysStr};
    
    printf("[VNET SERVER] Multi-PC Cyberwarfare & Chat Gateway Online (Port %d)\n", g_serverPort);
    printf("[VNET SERVER] Target Hash: %d\n", g_targetHash);
    printf("[VNET SERVER] Master Keys: %s\n", g_masterKeysStr.c_str());
    
    // Debug output
    printf("=========================================================================\n");
    printf("[DEBUG KEYCODES & MASTER ROOM LOCATIONS]\n");
    for (int i = 0; i < 8; i++) {
        int key = 0, shift = 0, scrambled = 0;
        switch(i) {
            case 0: key = g_k1; shift = g_shift1; scrambled = g_scrambled1; break;
            case 1: key = g_k2; shift = g_shift2; scrambled = g_scrambled2; break;
            case 2: key = g_k3; shift = g_shift3; scrambled = g_scrambled3; break;
            case 3: key = g_k4; shift = g_shift4; scrambled = g_scrambled4; break;
            case 4: key = g_k5; shift = g_shift5; scrambled = g_scrambled5; break;
            case 5: key = g_k6; shift = g_shift6; scrambled = g_scrambled6; break;
            case 6: key = g_k7; shift = g_shift7; scrambled = g_scrambled7; break;
            case 7: key = g_k8; shift = g_shift8; scrambled = g_scrambled8; break;
        }
        printf("  KEY %d [%d] | Shift: %d | Scrambled: %d\n", i + 1, key, shift, scrambled);
        printf("        -> Location: %s\n", g_keyLocations[i].c_str());
        printf("-------------------------------------------------------------------------\n");
    }
    printf("=========================================================================\n");
    
    printf("[DEBUG] InitServerState COMPLETE\n");
    fflush(stdout);
}

// ============================================================
// MAIN SERVER LOOP
// ============================================================

void RunVNTServer(int port) {
    printf("[DEBUG] Step 1: Entering RunVNTServer\n");
    fflush(stdout);
    
    g_serverPort = port;
    printf("[DEBUG] Step 2: Port set to %d\n", port);
    fflush(stdout);
    
    printf("[DEBUG] Step 3: Creating UDP socket...\n");
    fflush(stdout);
    g_serverSock = VNetLib::CreateUDPSocket(port);
    printf("[DEBUG] Step 4: Socket created, handle: %d\n", (int)g_serverSock);
    fflush(stdout);
    
    if (IS_INVALID_SOCKET(g_serverSock)) {
        printf("[ERROR] Failed to create server socket on port %d!\n", port);
        return;
    }
    printf("[DEBUG] Step 5: Socket is valid\n");
    fflush(stdout);
    
    printf("[DEBUG] Step 6: Initializing server state...\n");
    fflush(stdout);
    InitServerState();
    printf("[DEBUG] Step 7: Server state initialized\n");
    fflush(stdout);
    
    printf("[DEBUG] Step 8: Setting up timers...\n");
    fflush(stdout);
    
    float decoyTimer = 0.0f;
    float botTimer = 0.0f;
    float raidTimer = 0.0f;
    int botPort = 8999;
    std::string botURL = "zeroday.vnet";
    float serverOutageTimer = 0.0f;
    
    while (!g_gameOver) {
        float dt = 0.016f; // ~60 FPS
        g_serverUptime += dt;
        
        PruneInactivePeers(g_serverUptime);
        
        // Server outage simulation
        serverOutageTimer += dt;
        if (serverOutageTimer >= 30.0f) {
            serverOutageTimer = 0.0f;
            if ((rand() % 100) < 90) {
                int idx = rand() % g_allSites.size();
                std::string site = g_allSites[idx];
                bool alreadyDown = false;
                for (const auto& s : g_overloadedURLs) {
                    if (s == site) { alreadyDown = true; break; }
                }
                if (!alreadyDown) {
                    g_overloadedURLs.push_back(site);
                    g_overloadedTimers.push_back(20.0f);
                    BroadcastFeed("[NET_DESYNC] BGP router dropped subnet " + site);
                    BroadcastRaw(std::string(VNetResp::EXPLOIT_SITE_OVERLOADED) + site);
                }
            }
        }
        
        // Decay overloaded timers
        for (int i = (int)g_overloadedTimers.size() - 1; i >= 0; i--) {
            g_overloadedTimers[i] -= dt;
            if (g_overloadedTimers[i] <= 0.0f) {
                g_overloadedURLs.erase(g_overloadedURLs.begin() + i);
                g_overloadedTimers.erase(g_overloadedTimers.begin() + i);
            }
        }
        
        // Decoy cleanup
        decoyTimer += dt;
        if (decoyTimer >= 30.0f) {
            decoyTimer = 0.0f;
            if (!g_decoyURLs.empty()) {
                g_decoyTargets.clear();
                g_decoyOwners.clear();
                g_decoyURLs.clear();
                BroadcastFeed("[SYS_CLEANUP] All active decoys wiped (30s cycle)");
            }
        }
        
        // Bot stalker
        botTimer += dt;
        if (botTimer >= 120.0f) {
            botTimer = 0.0f;
            bool targetFound = false;
            if (!g_activePorts.empty()) {
                int idx = rand() % g_activePorts.size();
                if (g_activePorts[idx] != botPort && g_activeURLs[idx] != "vnet.dir") {
                    botURL = g_activeURLs[idx];
                    targetFound = true;
                }
            }
            if (!targetFound) {
                botURL = g_allSites[rand() % g_allSites.size()];
            }
            for (size_t i = 0; i < g_activePorts.size(); i++) {
                if (g_activePorts[i] != botPort && g_activeURLs[i] == botURL && g_activeURLs[i] != "vnet.dir") {
                    VNetLib::SendTo(g_serverSock, g_activeIPs[i], g_activePorts[i], VNetResp::EXPLOIT_BOT_STALK);
                    BroadcastFeed("[WARNING] SPECTRE_BOT locked onto port " + MaskPort(g_activePorts[i]) + " at " + botURL);
                }
            }
        }
        
        // Federal raid
        raidTimer += dt;
        if (raidTimer >= 120.0f) {
            raidTimer = 0.0f;
            if (!g_activePorts.empty()) {
                printf("[FEDERAL E-RAID] Shadow PMC initiating network-wide purge!\n");
                BroadcastFeed("[FEDERAL E-RAID] Shadow PMC intercept active! 30s purge window!");
                BroadcastRaw(VNetResp::EXPLOIT_FEDERAL_RAID);
            }
        }
        
        // Mining blocks refresh
        g_mineBlockTimer += dt;
        if (g_mineBlockTimer >= 30.0f) {
            g_mineBlockTimer = 0.0f;
            std::string blocks = RefreshMiningBlocks();
            BroadcastRaw(blocks);
            BroadcastFeed("[WHALE ALERT] 8 new mining blocks discovered at crypto.vnet!");
        }
        
        // ============================================================
        // PROCESS INCOMING PACKETS
        // ============================================================
        auto packet = VNetLib::RecvFrom(g_serverSock);
        
        if (packet.size() >= 3) {
            std::string msg = packet[0];
            std::string senderIP = packet[1];
            int senderPort = std::stoi(packet[2]);
            
            // Parse command
            auto parsed = ParsePacket(msg);
            std::string cmd = parsed.first;
            std::string payload = parsed.second;
            
            printf("[RX] %s:%d -> %s\n", senderIP.c_str(), senderPort, cmd.c_str());
            
            // ============================================================
            // PING
            // ============================================================
            if (cmd == "PING" || cmd == VNetCmd::PING) {
                size_t sep = payload.find(':');
                if (sep != std::string::npos) {
                    std::string handle = payload.substr(0, sep);
                    std::string url = payload.substr(sep + 1);
                    
                    bool found = false;
                    for (size_t i = 0; i < g_activePorts.size(); i++) {
                        if (g_activePorts[i] == senderPort) {
                            g_activeURLs[i] = url;
                            g_activeIPs[i] = senderIP;
                            g_activeLastSeen[i] = g_serverUptime;
                            g_activeHandles[i] = handle; // Fixed: update handle on ping
                            found = true;
                            break;
                        }
                    }
                    
                    if (!found) {
                        std::vector<std::string> assigned = {
                            "market.vnet", "vault.vnet", "terminal.vnet",
                            "forum.vnet", "crypto.vnet", "bounty.vnet",
                            "vektrapay.vnet", "hellroom.vnet", "hashbeat.vnet"
                        };
                        while (assigned.size() < 20) {
                            int idx = rand() % g_allSites.size();
                            std::string site = g_allSites[idx];
                            if (std::find(assigned.begin(), assigned.end(), site) == assigned.end()) {
                                assigned.push_back(site);
                            }
                        }
                        
                        std::string dirPayload;
                        for (size_t i = 0; i < assigned.size(); i++) {
                            dirPayload += assigned[i] + (i < assigned.size() - 1 ? ":" : "");
                        }
                        
                        g_activePorts.push_back(senderPort);
                        g_activeIPs.push_back(senderIP);
                        g_activeURLs.push_back(url);
                        g_activeLastSeen.push_back(g_serverUptime);
                        g_activeDOSHits.push_back(0);
                        g_activeDirs.push_back(dirPayload);
                        g_activeHandles.push_back(handle);
                        
                        SendKeySync(senderPort, senderIP, dirPayload);
                    }
                }
            }
            
            // ============================================================
            // CHAT
            // ============================================================
            else if (cmd == "CHAT" || cmd == VNetCmd::CHAT) {
                size_t sep = payload.find(':');
                if (sep != std::string::npos) {
                    std::string handle = payload.substr(0, sep);
                    std::string msgText = payload.substr(sep + 1);
                    BroadcastFeed("[CHAT] <" + handle + ">: " + msgText);
                }
            }
            
            // ============================================================
            // WHISPER
            // ============================================================
            else if (cmd == "WHISPER" || cmd == VNetCmd::WHISPER) {
                size_t sep1 = payload.find(':');
                if (sep1 != std::string::npos) {
                    std::string fromHandle = payload.substr(0, sep1);
                    std::string rest = payload.substr(sep1 + 1);
                    size_t sep2 = rest.find(':');
                    if (sep2 != std::string::npos) {
                        std::string toHandle = rest.substr(0, sep2);
                        std::string msgText = rest.substr(sep2 + 1);
                        
                        for (size_t i = 0; i < g_activeHandles.size(); i++) {
                            if (g_activeHandles[i] == toHandle) {
                                // Fixed: VNetResp::WHISPER_IN already contains trailing colon
                                std::string whisperPacket = std::string(VNetResp::WHISPER_IN) + fromHandle + ":" + msgText;
                                VNetLib::SendTo(g_serverSock, g_activeIPs[i], g_activePorts[i], whisperPacket);
                                break;
                            }
                        }
                    }
                }
            }
            
            // ============================================================
            // GET - Page request
            // ============================================================
            else if (cmd == VNetCmd::GET) {
                std::string cleanURL = CleanStr(payload);
                if (cleanURL.substr(0, 7) == "vnet://") {
                    cleanURL = CleanStr(cleanURL.substr(7));
                }
                
                // Check if site is overloaded
                bool isDown = false;
                for (const auto& s : g_overloadedURLs) {
                    if (s == cleanURL) {
                        isDown = true;
                        break;
                    }
                }
                
                if (isDown) {
                    VNetLib::SendTo(g_serverSock, senderIP, senderPort,
                                  std::string(VNetResp::EXPLOIT_SITE_OVERLOADED) + cleanURL);
                } else {
                    printf("[ROUTE] %d -> %s\n", senderPort, cleanURL.c_str());
                    
                    // Send blocks if crypto.vnet
                    if (cleanURL == "crypto.vnet" && !g_activeBlocks.empty()) {
                        std::string blocks = "NEW_BLOCKS:";
                        for (size_t i = 0; i < g_activeBlocks.size(); i++) {
                            blocks += g_activeBlocks[i] + (i < g_activeBlocks.size() - 1 ? ";" : "");
                        }
                        VNetLib::SendTo(g_serverSock, senderIP, senderPort, blocks);
                    }
                }
            }
            
            // ============================================================
            // DOS
            // ============================================================
            else if (cmd == VNetCmd::DOS) {
                int targetNode = std::stoi(payload);
                
                // Check decoy
                bool isDecoy = false;
                for (size_t i = 0; i < g_decoyTargets.size(); i++) {
                    if (g_decoyTargets[i] == targetNode) {
                        isDecoy = true;
                        int ownerPort = g_decoyOwners[i];
                        if (ownerPort != senderPort) {
                            std::string ownerIP = "127.0.0.1";
                            for (size_t p = 0; p < g_activePorts.size(); p++) {
                                if (g_activePorts[p] == ownerPort) {
                                    ownerIP = g_activeIPs[p];
                                    break;
                                }
                            }
                            VNetLib::SendTo(g_serverSock, ownerIP, ownerPort,
                                          std::string(VNetResp::DECOY_TRIPPED) + "Port " + std::to_string(senderPort) + " attacked decoy port " + payload);
                            BroadcastFeed("[DECOY TRAP] Port " + MaskPort(senderPort) + " tried to DOS decoy port " + payload);
                            VNetLib::SendTo(g_serverSock, senderIP, senderPort, VNetResp::EXPLOIT_DOS);
                            g_decoyTargets.erase(g_decoyTargets.begin() + i);
                            g_decoyOwners.erase(g_decoyOwners.begin() + i);
                            g_decoyURLs.erase(g_decoyURLs.begin() + i);
                        }
                        break;
                    }
                }
                
                if (!isDecoy) {
                    // Find target
                    for (size_t i = 0; i < g_activePorts.size(); i++) {
                        if (g_activePorts[i] == targetNode) {
                            VNetLib::SendTo(g_serverSock, g_activeIPs[i], targetNode, VNetResp::EXPLOIT_DOS);
                            BroadcastFeed("[DOS ATTACK] Node " + MaskPort(senderPort) + " froze node " + std::to_string(targetNode));
                            
                            g_activeDOSHits[i]++;
                            if (g_activeDOSHits[i] >= 2) {
                                // Leak some sites
                                std::string dropPayload;
                                int leakCount = 0;
                                std::string dirs = g_activeDirs[i];
                                size_t pos = 0;
                                std::vector<std::string> sites;
                                while ((pos = dirs.find(':')) != std::string::npos) {
                                    sites.push_back(dirs.substr(0, pos));
                                    dirs.erase(0, pos + 1);
                                }
                                if (!dirs.empty()) sites.push_back(dirs);
                                
                                for (const auto& site : sites) {
                                    bool isMutual = false;
                                    const char* mutual[] = {"market.vnet", "vault.vnet", "terminal.vnet", "crypto.vnet", "hellroom.vnet"};
                                    for (int m = 0; m < 5; m++) {
                                        if (site == mutual[m]) { isMutual = true; break; }
                                    }
                                    if (!isMutual && leakCount < 5) {
                                        dropPayload += site + ":";
                                        leakCount++;
                                    }
                                }
                                
                                if (!dropPayload.empty()) {
                                    VNetLib::SendTo(g_serverSock, senderIP, senderPort,
                                                  std::string(VNetResp::DOS_DROP_DIR) + dropPayload);
                                    BroadcastFeed("[DATA LEAK] Node " + std::to_string(targetNode) + " dosed 2x! 5 hidden nodes exfiltrated.");
                                }
                            }
                            break;
                        }
                    }
                }
            }
            
            // ============================================================
            // SPIKE
            // ============================================================
            else if (cmd == VNetCmd::SPIKE) {
                CheckAndTripDecoy(payload, senderPort);
                int targetNode = std::stoi(payload);
                for (size_t i = 0; i < g_activePorts.size(); i++) {
                    if (g_activePorts[i] == targetNode) {
                        VNetLib::SendTo(g_serverSock, g_activeIPs[i], targetNode, VNetResp::EXPLOIT_TRACE_SPIKE);
                        BroadcastFeed("[TRACE SPIKE] Node " + MaskPort(senderPort) + " spiked trace on node " + payload);
                        break;
                    }
                }
            }
            
            // ============================================================
            // REDIRECT
            // ============================================================
            else if (cmd == VNetCmd::REDIRECT) {
                size_t sep = payload.find(':');
                if (sep != std::string::npos) {
                    std::string targetNodeStr = payload.substr(0, sep);
                    CheckAndTripDecoy(targetNodeStr, senderPort);
                    
                    int targetNode = std::stoi(targetNodeStr);
                    std::string targetURL = payload.substr(sep + 1);
                    
                    for (size_t i = 0; i < g_activePorts.size(); i++) {
                        if (g_activePorts[i] == targetNode) {
                            std::string redirectMsg = std::string(VNetResp::EXPLOIT_REDIRECT) + targetURL;
                            VNetLib::SendTo(g_serverSock, g_activeIPs[i], targetNode, redirectMsg);
                            BroadcastFeed("[BGP HIJACK] Node " + MaskPort(senderPort) + " rerouted node " + targetNodeStr + " to " + targetURL);
                            break;
                        }
                    }
                }
            }
            
            // ============================================================
            // OVERLOAD
            // ============================================================
            else if (cmd == VNetCmd::OVERLOAD) {
                std::string targetSite = CleanStr(payload);
                std::string handle;
                
                size_t sep = payload.find(':');
                if (sep != std::string::npos) {
                    targetSite = CleanStr(payload.substr(0, sep));
                    handle = CleanStr(payload.substr(sep + 1));
                }
                
                if (targetSite.substr(0, 7) == "vnet://") {
                    targetSite = CleanStr(targetSite.substr(7));
                }
                
                bool isDown = false;
                for (const auto& s : g_overloadedURLs) {
                    if (s == targetSite) { isDown = true; break; }
                }
                
                if (!isDown) {
                    g_overloadedURLs.push_back(targetSite);
                    g_overloadedTimers.push_back(45.0f);
                    BroadcastFeed("[GRID OVERLOAD] Node " + MaskPort(senderPort) + " fried " + targetSite + "!");
                    std::string overloadedMsg = std::string(VNetResp::EXPLOIT_SITE_OVERLOADED) + targetSite;
                    BroadcastRaw(overloadedMsg);
                    
                    // Check for victory (all 5 core nodes overloaded)
                    bool mkt=false, vlt=false, trm=false, cry=false, hel=false;
                    for (const auto& s : g_overloadedURLs) {
                        if (s == "market.vnet") mkt = true;
                        if (s == "vault.vnet") vlt = true;
                        if (s == "terminal.vnet") trm = true;
                        if (s == "crypto.vnet") cry = true;
                        if (s == "hellroom.vnet") hel = true;
                    }
                    
                    if (mkt && vlt && trm && cry && hel) {
                        g_gameOver = true;
                        BroadcastFeed("[GRID BLACKOUT] Node " + MaskPort(senderPort) + " fried all 5 mutual core nodes!");
                        BroadcastRaw(std::string(VNetResp::EXPLOIT_WINNER) + std::to_string(senderPort) + ":BLACKOUT:" + handle);
                    }
                }
            }
            
            // ============================================================
            // WIN
            // ============================================================
            else if (cmd == VNetCmd::WIN) {
                std::string keys = payload;
                std::string handle;
                
                size_t sep = payload.find(':');
                if (sep != std::string::npos) {
                    keys = payload.substr(0, sep);
                    handle = payload.substr(sep + 1);
                }
                
                if (keys == g_masterKeysStr) {
                    g_gameOver = true;
                    printf("[SYSTEM OVERRIDE] Victory detected from port %d\n", senderPort);
                    BroadcastFeed("[SYSTEM OVERRIDE] Node " + std::to_string(senderPort) + " has breached root vault!");
                    std::string winnerMsg = std::string(VNetResp::EXPLOIT_WINNER) + std::to_string(senderPort) + ":BLACKOUT:" + handle;
                    BroadcastRaw(winnerMsg);
                } else {
                    VNetLib::SendTo(g_serverSock, senderIP, senderPort, "WIN:FAIL:INVALID_KEY_COMBINATION");
                }
            }
            
            // ============================================================
            // TAKEOVER
            // ============================================================
            else if (cmd == VNetCmd::TAKEOVER) {
                std::string handle = payload.empty() ? ("PORT_" + std::to_string(senderPort)) : payload;
                g_gameOver = true;
                printf("[ECONOMIC TAKEOVER] Victory detected from port %d\n", senderPort);
                BroadcastFeed("[ECONOMIC TAKEOVER] Node " + MaskPort(senderPort) + " bought out VNET network!");
                BroadcastRaw(std::string(VNetResp::EXPLOIT_WINNER) + std::to_string(senderPort) + ":ECONOMIC:" + handle);
            }
            
            // ============================================================
            // SCAN
            // ============================================================
            else if (cmd == VNetCmd::SCAN_REQ) {
                int idx = rand() % g_allSites.size();
                VNetLib::SendTo(g_serverSock, senderIP, senderPort,
                              std::string(VNetResp::SCAN_RESULT) + g_allSites[idx]);
            }
            
            // ============================================================
            // SATSCAN
            // ============================================================
            else if (cmd == VNetCmd::SATSCAN_REQ) {
                std::vector<int> indices;
                while (indices.size() < 10) {
                    int idx = rand() % g_allSites.size();
                    if (std::find(indices.begin(), indices.end(), idx) == indices.end()) {
                        indices.push_back(idx);
                    }
                }
                
                std::string result = VNetResp::SATSCAN_RES;
                for (size_t i = 0; i < indices.size(); i++) {
                    int traffic = rand() % 9680 + 120;
                    std::string status = "NOMINAL";
                    if (traffic > 7000) status = "CRITICAL";
                    else if (traffic > 3000) status = "ELEVATED";
                    
                    result += g_allSites[indices[i]] + "|" + std::to_string(traffic) + "|" + status;
                    if (i < indices.size() - 1) result += ";";
                }
                VNetLib::SendTo(g_serverSock, senderIP, senderPort, result);
            }
            
            // ============================================================
            // DECOY
            // ============================================================
            else if (cmd == VNetCmd::DECOY) {
                size_t sep = payload.find(':');
                if (sep != std::string::npos) {
                    std::string targetURL = payload.substr(0, sep);
                    int dummyPort = std::stoi(payload.substr(sep + 1));
                    
                    g_decoyTargets.push_back(dummyPort);
                    g_decoyURLs.push_back(targetURL);
                    g_decoyOwners.push_back(senderPort);
                    
                    VNetLib::SendTo(g_serverSock, senderIP, senderPort,
                                  "DECOY_SET:Decoy deployed on " + targetURL + " as port " + payload.substr(sep + 1));
                    BroadcastFeed("[NETWORK] Decoy port " + std::to_string(dummyPort) + " deployed at " + targetURL);
                }
            }
            
            // ============================================================
            // CAT - VFS read
            // ============================================================
            else if (cmd == VNetCmd::CAT) {
                bool found = false;
                for (size_t i = 0; i < g_vfsPaths.size(); i++) {
                    if (g_vfsPaths[i] == payload) {
                        VNetLib::SendTo(g_serverSock, senderIP, senderPort,
                                      std::string(VNetResp::VFS_DATA) + g_vfsPaths[i] + ":" + g_vfsData[i]);
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    VNetLib::SendTo(g_serverSock, senderIP, senderPort, "VFS_DATA:ERROR:PATH_NOT_FOUND");
                }
            }
            
            // ============================================================
            // LS - VFS list
            // ============================================================
            else if (cmd == VNetCmd::LS) {
                std::string path = CleanStr(payload);
                if (path.empty()) path = "/";
                
                std::string files;
                for (size_t i = 0; i < g_vfsPaths.size(); i++) {
                    if (path == "/" || g_vfsPaths[i].substr(0, path.length()) == path) {
                        files += g_vfsPaths[i] + " ";
                    }
                }
                
                if (!files.empty()) {
                    VNetLib::SendTo(g_serverSock, senderIP, senderPort,
                                  std::string(VNetResp::VFS_LIST) + path + ":" + files);
                } else {
                    VNetLib::SendTo(g_serverSock, senderIP, senderPort, "VFS_LIST:ERROR:NO_NODES_FOUND");
                }
            }
            
            // ============================================================
            // PATCH - Port rebind
            // ============================================================
            else if (cmd == VNetCmd::PATCH) {
                int newPort = rand() % 999 + 8001;
                
                // Ensure port is unique
                while (std::find(g_activePorts.begin(), g_activePorts.end(), newPort) != g_activePorts.end()) {
                    newPort = rand() % 999 + 8001;
                }
                
                for (size_t i = 0; i < g_activePorts.size(); i++) {
                    if (g_activePorts[i] == senderPort) {
                        VNetLib::SendTo(g_serverSock, senderIP, senderPort,
                                      std::string(VNetResp::PATCH_SUCCESS) + std::to_string(newPort));
                        g_activePorts[i] = newPort;
                        BroadcastFeed("[SOCKET MIGRATION] Node " + MaskPort(senderPort) + " rebound to new subnet");
                        break;
                    }
                }
            }
            
            // ============================================================
            // SNIFFER_ADD
            // ============================================================
            else if (cmd == VNetCmd::SNIFFER_ADD) {
                VNetLib::SendTo(g_serverSock, senderIP, senderPort,
                              std::string(VNetResp::SNIFFER_ADD_ACK) + payload);
            }
            
            // ============================================================
            // SNIFFER_STATUS
            // ============================================================
            else if (cmd == VNetCmd::SNIFFER_STATUS) {
                if (payload == "1") {
                    BroadcastFeed("[WARNING] Peer port " + MaskPort(senderPort) + " entered promiscuous sniffing mode!");
                } else {
                    BroadcastFeed("[NETWORK] Peer port " + MaskPort(senderPort) + " disabled packet sniffer.");
                }
            }
            
            // ============================================================
            // CRACK
            // ============================================================
            else if (cmd == VNetCmd::CRACK) {
                int attempt = std::stoi(payload);
                int computed = (attempt * g_currentSalt) % 9999;
                if (computed == g_targetHash) {
                    VNetLib::SendTo(g_serverSock, senderIP, senderPort, "AUTH:SUCCESS:ACCESS_GRANTED");
                    BroadcastFeed("[VAULT BREACH] Gateway firewall cracked by node " + std::to_string(senderPort));
                } else {
                    VNetLib::SendTo(g_serverSock, senderIP, senderPort, "AUTH:FAIL:INVALID_HASH");
                }
            }
            
            // ============================================================
            // ION_STRIKE
            // ============================================================
            else if (cmd == VNetCmd::ION_STRIKE) {
                std::string target = CleanStr(payload);
                bool hit = false;
                
                // Try port
                try {
                    int targetPort = std::stoi(target);
                    for (size_t i = 0; i < g_activePorts.size(); i++) {
                        if (g_activePorts[i] == targetPort) {
                            std::string brainDeadMsg = std::string(VNetResp::EXPLOIT_BRAINDEAD) + "ION_STRIKE_DIRECT_HIT";
                            VNetLib::SendTo(g_serverSock, g_activeIPs[i], targetPort, brainDeadMsg);
                            hit = true;
                            break;
                        }
                    }
                } catch (...) {
                    // Try URL
                    for (size_t i = 0; i < g_activePorts.size(); i++) {
                        if (g_activeURLs[i] == target) {
                            std::string brainDeadMsg = std::string(VNetResp::EXPLOIT_BRAINDEAD) + "ION_STRIKE_SITE_OBLITERATED:" + target;
                            VNetLib::SendTo(g_serverSock, g_activeIPs[i], g_activePorts[i], brainDeadMsg);
                            hit = true;
                            break;
                        }
                    }
                }
                
                if (hit) {
                    printf("[ION STRIKE] SAT-99 ion beam fired at %s!\n", target.c_str());
                    BroadcastFeed("[ION STRIKE] SAT-99 ion cannon fired at " + target + "!");
                }
            }
            
            // ============================================================
            // PAY_TRANSFER (VektraPay)
            // ============================================================
            else if (cmd == VNetCmd::PAY_TRANSFER) {
                // Stub - would handle escrow
                BroadcastFeed("[VEKTRAPAY] New escrow key generated");
            }
            
            // ============================================================
            // PAY_CLAIM (VektraPay)
            // ============================================================
            else if (cmd == VNetCmd::PAY_CLAIM) {
                VNetLib::SendTo(g_serverSock, senderIP, senderPort,
                              std::string(VNetResp::PAY_CLAIM_FAILED) + "INVALID_KEY");
            }
            
            // ============================================================
            // NETSCAN
            // ============================================================
            else if (cmd == VNetCmd::NETSCAN) {
                std::string peers;
                int count = 0;
                for (size_t i = 0; i < g_activePorts.size(); i++) {
                    if (g_activeURLs[i] == payload && g_activePorts[i] != senderPort) {
                        peers += "PORT_" + std::to_string(g_activePorts[i]) + " ";
                        count++;
                    }
                }
                VNetLib::SendTo(g_serverSock, senderIP, senderPort,
                              "NETSCAN: PEERS DETECTED ON " + payload + " -> [" + peers + "]");
            }
            
            // ============================================================
            // MINE_EVENT
            // ============================================================
            else if (cmd == VNetCmd::MINE_EVENT) {
                BroadcastFeed("[WHALE ALERT] Node ???? mined +0.05 VCOIN block at crypto.vnet");
            }
            
            // ============================================================
            // TRACE_BUST
            // ============================================================
            else if (cmd == VNetCmd::TRACE_BUST) {
                UnregisterPeer(senderPort);
                BroadcastFeed("[ICE LOCKOUT] Node " + std::to_string(senderPort) + " traced down & disconnected!");
            }
            
            // ============================================================
            // PROXY
            // ============================================================
            else if (cmd == VNetCmd::PROXY) {
                size_t sep = payload.find(':');
                if (sep != std::string::npos) {
                    std::string proxyNode = payload.substr(0, sep);
                    std::string targetURL = payload.substr(sep + 1);
                    g_proxySources.push_back(senderPort);
                    g_proxyNodes.push_back(proxyNode);
                    g_proxyTargets.push_back(targetURL);
                    BroadcastFeed("[SPY ROUTE] Port " + MaskPort(senderPort) + " encrypted chain through " + proxyNode + " to " + targetURL);
                }
            }
            
            // ============================================================
            // Unknown command
            // ============================================================
            else {
                printf("[WARN] Unknown command: %s\n", cmd.c_str());
            }
        }
        
        // Sleep a bit to prevent CPU spinning
        VNetLib::Sleep(16);
    }
    
    printf("[SERVER] Shutting down...\n");
    VNetLib::CloseSocket(g_serverSock);
    printf("[SERVER] Socket closed.\n");
}