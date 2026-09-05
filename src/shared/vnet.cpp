#include "vnet.h"
#include "vnet_sites.h" 
#include "vnet_protocol.h"
#include "vnet_client.h" 
#include "render.h"
#include "raylib.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstdarg>
#include <cstring>
#include <string>
#include <vector> 

// ============================================================
// GLOBAL STATE
// ============================================================

Player g_player = {0};
VNETSystem g_vnet = {0};
char g_cliLogs[250][256] = {0};
int g_cliLogCount = 0;
char g_feedLogs[100][256] = {0};
int g_feedLogCount = 0;
char g_minedBlocks[50][64] = {0};
int g_minedCount = 0;
char g_masterKeysStr[256] = {0};

// Page content for markup renderer
std::vector<std::string> g_pageLines;

// ============================================================
// INITIALIZATION
// ============================================================

void InitVNETSystem(void) {
    srand((unsigned)time(NULL));
    
    // Init player
    memset(&g_player, 0, sizeof(Player));
    snprintf(g_player.handle, sizeof(g_player.handle), "sh4d0w%d", rand() % 1000);
    g_player.port = 8001 + (rand() % 999);
    strcpy(g_player.currentURL, "vnet.dir");
    g_player.vcoin = 1.50f;
    g_player.traceLevel = 14;
    g_player.iceShields = 1;
    g_player.crtHeat = 35.0f;
    g_player.neuralParanoia = 0.0f;
    g_player.cliOpen = false;

    g_player.isInConnectionMenu = true;   // <-- ADD THIS
    g_player.ipBoxFocused = true;          // <-- ADD THIS
    strcpy(g_player.ipInputBuffer, "127.0.0.1"); // <-- ADD THIS
    
    // Init VNET
    memset(&g_vnet, 0, sizeof(VNETSystem));
    g_vnet.siteCount = 50;
    g_vnet.currentSalt = rand() % 90 + 10;
    g_vnet.targetHash = rand() % 9000 + 1000;
    g_vnet.serverUptime = 0.0f;
    g_vnet.gameOver = false;
    
    // Generate master keys
    for (int i = 0; i < 8; i++) {
        snprintf(g_vnet.masterKeys[i], sizeof(g_vnet.masterKeys[i]), 
                "%04d", rand() % 9000 + 1000);
    }
    
    // Build master keys string
    char temp[256] = {0};
    for (int i = 0; i < 8; i++) {
        strcat(temp, g_vnet.masterKeys[i]);
        if (i < 7) strcat(temp, " ");
    }
    strcpy(g_masterKeysStr, temp);
    
    // Push initial logs
    PushCliLog("[SYS_INIT]: VNET SYSTEM LOADED");
    PushCliLog("[SYS_INIT]: PLAYER HANDLE: %s", g_player.handle);
    PushCliLog("[SYS_INIT]: ACTIVE PORT: %d", g_player.port);
    PushCliLog("[SYS_INIT]: CONNECTED TO vnet.dir");
    PushCliLog("[SYS_INIT]: TYPE 'help' FOR AVAILABLE COMMANDS");
    PushCliLog("--------------------------------------------------");
    
    PushFeedLog("[FEED_INIT]: VNET BROADCAST HUB ONLINE");
    PushFeedLog("[FEED]: MONITORING P2P EXPLOIT & CHAT...");

    InitAllSites(g_vnet);
}

void ShutdownVNETSystem(void) {
    // Cleanup
}

// ============================================================
// UPDATE
// ============================================================

void UpdateVNET(float dt) {
    g_vnet.serverUptime += dt;
    g_player.runTime += dt;
    g_player.heartbeatTimer += dt;
    
    // ============================================================
    // SEND HEARTBEAT TO SERVER
    // ============================================================
    if (IsVNetConnected() && g_player.heartbeatTimer >= 3.0f) {
        g_player.heartbeatTimer = 0.0f;
        std::string ping = std::string(VNetCmd::PING) + ":" + g_player.handle + ":" + g_player.currentURL;
        VNetSendRaw(ping);
    }
    
    // Cool down CRT heat
    g_player.crtHeat -= dt * 0.5f;
    if (g_player.crtHeat < 35.0f) g_player.crtHeat = 35.0f;
    
    // Update cooldowns
    if (g_player.cdDOS > 0.0f) g_player.cdDOS -= dt;
    if (g_player.cdProbe > 0.0f) g_player.cdProbe -= dt;
    if (g_player.cdSpike > 0.0f) g_player.cdSpike -= dt;
    if (g_player.cdSnoop > 0.0f) g_player.cdSnoop -= dt;
    if (g_player.cdMine > 0.0f) g_player.cdMine -= dt;
    if (g_player.cdPatch > 0.0f) g_player.cdPatch -= dt;
    if (g_player.cdScan > 0.0f) g_player.cdScan -= dt;
    if (g_player.cdDecoy > 0.0f) g_player.cdDecoy -= dt;
    if (g_player.cdProxy > 0.0f) g_player.cdProxy -= dt;
    if (g_player.cdOverload > 0.0f) g_player.cdOverload -= dt;
    if (g_player.cdSatscan > 0.0f) g_player.cdSatscan -= dt;
    if (g_player.cdIon > 0.0f) g_player.cdIon -= dt;
    
    // DOS timer
    if (g_player.dosTimer > 0.0f) {
        g_player.dosTimer -= dt;
        g_player.glitchTrigger = 0.5f;
    }
    
    // Connection animation
    if (g_player.isConnecting) {
        g_player.connectTimer += dt;
        if (g_player.connectTimer >= g_player.targetConnectTime) {
            g_player.isConnecting = false;
            strcpy(g_player.currentURL, g_player.pendingURL);
            RefreshPage();
            PushCliLog("[TOR_ROUTE]: CONNECTED TO %s", g_player.currentURL);
        }
    }
    
    // Glitch decay
    if (g_player.glitchTrigger > 0.0f) {
        g_player.glitchTrigger -= dt * 2.0f;
        if (g_player.glitchTrigger < 0.0f) g_player.glitchTrigger = 0.0f;
    }
}

// ============================================================
// SITE MANAGEMENT
// ============================================================

VNETSite* GetSite(const char* url) {
    for (int i = 0; i < g_vnet.siteCount; i++) {
        if (strcmp(g_vnet.sites[i].id, url) == 0) {
            return &g_vnet.sites[i];
        }
    }
    return nullptr;
}

void LoadPage(const char* url) {
    // Clean the URL
    char cleanURL[64];
    strncpy(cleanURL, url, sizeof(cleanURL) - 1);
    cleanURL[sizeof(cleanURL) - 1] = '\0';

    TriggerJitter(0.2f + (rand() % 100) / 500.0f);
    
    // Remove vnet:// prefix if present
    if (strncmp(cleanURL, "vnet://", 7) == 0) {
        memmove(cleanURL, cleanURL + 7, strlen(cleanURL) - 6);
    }
    
    // Update current URL
    strcpy(g_player.currentURL, cleanURL);
    
    // Load the page content using our site data
    LoadPageContent(cleanURL, g_pageLines, g_player, g_vnet);
    
    // Push log
    PushCliLog("[PAGE]: LOADED %s", cleanURL);
}

void RefreshPage(void) {
    // Reload current page content
    if (strlen(g_player.currentURL) > 0) {
        LoadPageContent(g_player.currentURL, g_pageLines, g_player, g_vnet);
        PushCliLog("[PAGE]: REFRESHED %s", g_player.currentURL);
    }
}

// ============================================================
// COMMAND SYSTEM - ENHANCED
// ============================================================

void ProcessCommand(const char* cmd) {
    char copy[256];
    strncpy(copy, cmd, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';
    
    char* token = strtok(copy, " ");
    if (!token) return;
    
    char* args = strtok(nullptr, "");
    PushCliLog("> %s", cmd);
    
    // ============================================================
    // NAVIGATION COMMANDS
    // ============================================================
    if (strcmp(token, "connect") == 0 || strcmp(token, "goto") == 0) {
        if (!args) {
            PushCliLog("[ERROR]: Usage: connect <url>");
        } else {
            TriggerRouteNavigation(args);
        }
    }
    else if (strcmp(token, "home") == 0) {
        TriggerRouteNavigation("vnet.dir");
    }
    else if (strcmp(token, "back") == 0) {
        // Go back to previous page (store in player)
        if (strlen(g_player.prevURL) > 0) {
            TriggerRouteNavigation(g_player.prevURL);
        } else {
            PushCliLog("[ERROR]: No previous page in history");
        }
    }
    
    // ============================================================
    // COMMUNICATION COMMANDS
    // ============================================================
    else if (strcmp(token, "chat") == 0 || strcmp(token, "msg") == 0) {
        if (!args) {
            PushCliLog("[ERROR]: Usage: chat <message>");
        } else {
            char msg[256];
            snprintf(msg, sizeof(msg), "[CHAT] <%s>: %s", g_player.handle, args);
            PushFeedLog(msg);
            PushCliLog(msg);
        }
    }
    else if (strcmp(token, "whisper") == 0 || strcmp(token, "pm") == 0) {
        if (!args) {
            PushCliLog("[ERROR]: Usage: whisper <handle> <message>");
        } else {
            char target[64] = {0};
            char msg[256] = {0};
            sscanf(args, "%63s %255[^\n]", target, msg);
            
            if (strlen(target) == 0 || strlen(msg) == 0) {
                PushCliLog("[ERROR]: Usage: whisper <handle> <message>");
            } else {
                char buffer[256];
                snprintf(buffer, sizeof(buffer), "[WHISPER TO <%s>]: %s", target, msg);
                PushCliLog(buffer);
                PushFeedLog(buffer);
            }
        }
    }
    else if (strcmp(token, "say") == 0) {
        if (!args) {
            PushCliLog("[ERROR]: Usage: say <message>");
        } else {
            char msg[256];
            snprintf(msg, sizeof(msg), "[SAY] %s", args);
            PushCliLog(msg);
        }
    }
    
    // ============================================================
    // HELP & INFO
    // ============================================================
    else if (strcmp(token, "help") == 0) {
        PushCliLog("========== VNET COMMAND REFERENCE ==========");
        PushCliLog("");
        PushCliLog("  THREE VICTORY GOALS:");
        PushCliLog("  1. Root Breach    : win <k1> ... <k8> at terminal.vnet");
        PushCliLog("  2. Grid Blackout  : overload 5 mutual core nodes");
        PushCliLog("  3. Economic Control: Reach 25.0 VCOIN and type 'takeover'");
        PushCliLog("");
        PushCliLog("  NAVIGATION:");
        PushCliLog("  connect <url>     - Navigate to VNET site");
        PushCliLog("  home              - Return to vnet.dir");
        PushCliLog("  back              - Go to previous page");
        PushCliLog("  bounty            - Jump to bounty contract board");
        PushCliLog("");
        PushCliLog("  COMMUNICATION:");
        PushCliLog("  chat <msg>        - Broadcast to all");
        PushCliLog("  whisper <h> <msg> - Private message");
        PushCliLog("  say <msg>         - Local chat");
        PushCliLog("");
        PushCliLog("  ECONOMY:");
        PushCliLog("  mine              - Mine VCOIN at crypto.vnet");
        PushCliLog("  wallet            - Show balance & stats");
        PushCliLog("  buy ice           - Buy ICE shield (0.30 VCOIN)");
        PushCliLog("  vektrapay         - Escrow/transfer VCOIN (at vektrapay.vnet)");
        PushCliLog("");
        PushCliLog("  EXPLOITS:");
        PushCliLog("  dos <port>        - DOS attack (0.25 VCOIN)");
        PushCliLog("  spike <port>      - Trace spike (0.20 VCOIN)");
        PushCliLog("  overload <url>    - Overload site (1.50 VCOIN)");
        PushCliLog("  redirect <p> <u>  - BGP hijack (0.15 VCOIN)");
        PushCliLog("  snoop <port>      - Interrogate target (0.05 VCOIN)");
        PushCliLog("  probe <port>      - Recon pulse (0.20 VCOIN)");
        PushCliLog("  crack <val>       - Send decryption challenge vector");
        PushCliLog("  ion <target>      - Orbital ion cannon (2.00 VCOIN at orbital.vnet)");
        PushCliLog("  0day <LOT>        - Zero-day exploit (at zeroday.vnet)");
        PushCliLog("");
        PushCliLog("  CYBERWARFARE:");
        PushCliLog("  sniffer           - Toggle UDP packet sniffer (0.05 VCOIN)");
        PushCliLog("  satscan           - Panopticon sweep (0.55 VCOIN at watchtower.vnet)");
        PushCliLog("  patch             - Emergency port rebind (0.70 VCOIN)");
        PushCliLog("  decoy <url> <p>   - Deploy trap node (0.10 VCOIN)");
        PushCliLog("  proxy <u> <n>     - Route through proxy (0.40 VCOIN)");
        PushCliLog("  netscan           - Discover active peers on current URL");
        PushCliLog("");
        PushCliLog("  UTILITY:");
        PushCliLog("  scan              - Deep subnet sweep (240s CD)");
        PushCliLog("  history           - Show discovered sites");
        PushCliLog("  clear             - Clear terminal");
        PushCliLog("  theme <name|list> - Change color theme (15 options)");
        PushCliLog("  status            - Show network status");
        PushCliLog("  info              - Show detailed system info");
        PushCliLog("  trace             - Show current trace level");
        PushCliLog("  flush             - Reduce trace by 30%% (0.10 VCOIN)");
        PushCliLog("  calm / meds       - Reduce paranoia (0.15 VCOIN)");
        PushCliLog("  inspect <url>     - Dump raw page source");
        PushCliLog("");
        PushCliLog("  VFS & DECRYPTION:");
        PushCliLog("  shift <0-15>      - Tune bit-shift offset for VDEC");
        PushCliLog("  vdec <offset>     - Decode active carrier key");
        PushCliLog("  cat <path>        - Read VFS file (0.05 VCOIN)");
        PushCliLog("  ls / dir <path>   - List VFS directory (0.02 VCOIN)");
        PushCliLog("");
        PushCliLog("  VICTORY:");
        PushCliLog("  win <k1>...<k8>   - Submit keys at terminal.vnet");
        PushCliLog("  takeover          - Economic victory (25.0 VCOIN)");
        PushCliLog("================================================");
    }
    else if (strcmp(token, "info") == 0) {
        PushCliLog("========== SYSTEM INFO ==========");
        PushCliLog("  HANDLE    : %s", g_player.handle);
        PushCliLog("  PORT      : %d", g_player.port);
        PushCliLog("  URL       : vnet://%s", g_player.currentURL);
        PushCliLog("  VCOIN     : %.2f", g_player.vcoin);
        PushCliLog("  TRACE     : %d%%", g_player.traceLevel);
        PushCliLog("  ICE       : %d/3", g_player.iceShields);
        PushCliLog("  CRT HEAT  : %.0f°C", g_player.crtHeat);
        PushCliLog("  PARANOIA  : %.0f%%", g_player.neuralParanoia);
        PushCliLog("  SITES     : %d discovered", g_player.assignedCount);
        PushCliLog("=================================");
    }
    
    // ============================================================
    // ECONOMY COMMANDS
    // ============================================================
    else if (strcmp(token, "mine") == 0) {
        StartMining();
    }
    else if (strcmp(token, "wallet") == 0) {
        PushCliLog("========== WALLET & DEFENSE ==========");
        PushCliLog("  VCOIN BALANCE  : %.2f VCOIN", g_player.vcoin);
        PushCliLog("  TRACE THREAT   : %d%%", g_player.traceLevel);
        PushCliLog("  ICE SHIELDS    : [%d/3] LAYERS", g_player.iceShields);
        PushCliLog("  CRT HEAT       : %.0f C", g_player.crtHeat);
        PushCliLog("  NEURAL PARANOIA: %.0f%%", g_player.neuralParanoia);
        PushCliLog("====================================");
    }
    else if (strcmp(token, "buy") == 0) {
        if (args && strcmp(args, "ice") == 0) {
            if (g_player.iceShields >= 3) {
                PushCliLog("[ERROR]: MAX ICE SHIELDS REACHED (3/3)");
            } else if (g_player.vcoin < 0.30f) {
                PushCliLog("[ERROR]: INSUFFICIENT VCOIN (NEEDS 0.30 VCOIN)");
            } else {
                g_player.vcoin -= 0.30f;
                g_player.iceShields++;
                TriggerJitter(0.2f);
                PushCliLog("[ICE]: ICE SHIELD PURCHASED! ACTIVE: %d/3", g_player.iceShields);
                PushFeedLog("[ICE]: Player purchased ICE shield");
            }
        } else {
            PushCliLog("[ERROR]: Usage: buy ice");
        }
    }
    
    // ============================================================
    // EXPLOIT COMMANDS
    // ============================================================
    else if (strcmp(token, "dos") == 0) {
        if (!args) {
            PushCliLog("[ERROR]: Usage: dos <port>");
        } else if (g_player.cdDOS > 0.0f) {
            PushCliLog("[ERROR]: DOS COOLDOWN (%.0fs)", g_player.cdDOS);
        } else if (g_player.vcoin < 0.25f) {
            PushCliLog("[ERROR]: NEED 0.25 VCOIN (CURRENT: %.2f)", g_player.vcoin);
        } else {
            g_player.vcoin -= 0.25f;
            g_player.cdDOS = 15.0f;
            TriggerJitter(0.6f);
            PushCliLog("[DOS]: DOS ATTACK SENT TO PORT %s", args);
            PushFeedLog("[DOS ATTACK]: Player froze port %s", args);
            g_player.glitchTrigger = 0.6f;
        }
    }
    else if (strcmp(token, "spike") == 0) {
        if (!args) {
            PushCliLog("[ERROR]: Usage: spike <port>");
        } else if (g_player.cdSpike > 0.0f) {
            PushCliLog("[ERROR]: SPIKE COOLDOWN (%.0fs)", g_player.cdSpike);
        } else if (g_player.vcoin < 0.20f) {
            PushCliLog("[ERROR]: NEED 0.20 VCOIN (CURRENT: %.2f)", g_player.vcoin);
        } else {
            g_player.vcoin -= 0.20f;
            g_player.cdSpike = 12.0f;
            TriggerJitter(0.5f);
            PushCliLog("[TRACE SPIKE]: SENT TO PORT %s", args);
            PushFeedLog("[TRACE SPIKE]: Player spiked port %s", args);
            g_player.glitchTrigger = 0.4f;
        }
    }
    else if (strcmp(token, "overload") == 0) {
        if (!args) {
            PushCliLog("[ERROR]: Usage: overload <url>");
        } else if (g_player.cdOverload > 0.0f) {
            PushCliLog("[ERROR]: OVERLOAD COOLDOWN (%.0fs)", g_player.cdOverload);
        } else if (g_player.vcoin < 1.50f) {
            PushCliLog("[ERROR]: NEED 1.50 VCOIN (CURRENT: %.2f)", g_player.vcoin);
        } else {
            g_player.vcoin -= 1.50f;
            g_player.cdOverload = 25.0f;
            g_player.crtHeat += 15.0f;
            TriggerJitter(0.8f);
            PushCliLog("[OVERLOAD]: DISPATCHED CASCADE TO %s", args);
            PushFeedLog("[OVERLOAD]: Player fried %s", args);
            g_player.glitchTrigger = 0.8f;
        }
    }
    else if (strcmp(token, "redirect") == 0) {
        if (!args) {
            PushCliLog("[ERROR]: Usage: redirect <target_port> <destination_url>");
        } else {
            char port[16] = {0};
            char url[64] = {0};
            sscanf(args, "%15s %63s", port, url);
            if (strlen(port) == 0 || strlen(url) == 0) {
                PushCliLog("[ERROR]: Usage: redirect <target_port> <destination_url>");
            } else if (g_player.cdRedirect > 0.0f) {
                PushCliLog("[ERROR]: REDIRECT COOLDOWN (%.0fs)", g_player.cdRedirect);
            } else if (g_player.vcoin < 0.15f) {
                PushCliLog("[ERROR]: NEED 0.15 VCOIN (CURRENT: %.2f)", g_player.vcoin);
            } else {
                g_player.vcoin -= 0.15f;
                g_player.cdRedirect = 10.0f;
                TriggerJitter(0.4f);
                PushCliLog("[REDIRECT]: BGP HIJACK SENT TO PORT %s -> %s", port, url);
                PushFeedLog("[BGP HIJACK]: Player redirected port %s", port);
            }
        }
    }
    else if (strcmp(token, "theme") == 0) {
        if (!args || strcmp(args, "list") == 0) {
            PushCliLog("========== AVAILABLE THEMES ==========");
            PushCliLog("  classic    - Original VNET dark theme");
            PushCliLog("  tokyo      - Tokyo night cyber");
            PushCliLog("  redroom    - Crimson blood red");
            PushCliLog("  amber      - Warm amber phosphor");
            PushCliLog("  matrix     - Green matrix style");
            PushCliLog("  cyberpunk  - Neon pink/blue");
            PushCliLog("  nord       - Cool frost blue");
            PushCliLog("  dracula    - Gothic dark purple");
            PushCliLog("  synthwave  - Retro outrun");
            PushCliLog("  cobalt     - Deep blue");
            PushCliLog("  monokai    - Editor dark");
            PushCliLog("  gruvbox    - Warm retro");
            PushCliLog("  abyss      - Deep ocean dark");
            PushCliLog("  solaris    - Orange flare");
            PushCliLog("  ghost      - Soft gray/blue");
            PushCliLog("======================================");
            PushCliLog("Usage: theme <theme_name>");
        } else {
            // Check if theme exists by trying to set it
            const char* themeNames[] = {
                "classic", "tokyo", "redroom", "amber", "matrix",
                "cyberpunk", "nord", "dracula", "synthwave", "cobalt",
                "monokai", "gruvbox", "abyss", "solaris", "ghost"
            };
            bool found = false;
            for (int i = 0; i < 15; i++) {
                if (strcmp(args, themeNames[i]) == 0) {
                    found = true;
                    break;
                }
            }
            
            if (found) {
                SetActiveTheme(args);
                TriggerJitter(0.15f);
                PushCliLog("[THEME]: Switched to '%s'", args);
            } else {
                PushCliLog("[ERROR]: Unknown theme '%s'", args);
                PushCliLog("Type 'theme list' to see all available themes.");
            }
        }
    }
    else if (strcmp(token, "snoop") == 0) {
        if (!args) {
            PushCliLog("[ERROR]: Usage: snoop <port>");
        } else if (g_player.cdSnoop > 0.0f) {
            PushCliLog("[ERROR]: SNOOP COOLDOWN (%.0fs)", g_player.cdSnoop);
        } else if (g_player.vcoin < 0.05f) {
            PushCliLog("[ERROR]: NEED 0.05 VCOIN (CURRENT: %.2f)", g_player.vcoin);
        } else {
            g_player.vcoin -= 0.05f;
            g_player.cdSnoop = 5.0f;
            PushCliLog("[SNOOP]: INTERROGATING PORT %s", args);
            PushFeedLog("[SNOOP]: Player snooped port %s", args);
            g_player.glitchTrigger = 0.3f;
        }
    }
    else if (strcmp(token, "probe") == 0) {
        if (!args) {
            PushCliLog("[ERROR]: Usage: probe <port>");
        } else if (g_player.cdProbe > 0.0f) {
            PushCliLog("[ERROR]: PROBE COOLDOWN (%.0fs)", g_player.cdProbe);
        } else if (g_player.vcoin < 0.20f) {
            PushCliLog("[ERROR]: NEED 0.20 VCOIN (CURRENT: %.2f)", g_player.vcoin);
        } else {
            g_player.vcoin -= 0.20f;
            g_player.cdProbe = 10.0f;
            TriggerJitter(0.3f);
            PushCliLog("[PROBE]: RECON PULSE TO PORT %s", args);
            g_player.glitchTrigger = 0.3f;
        }
    }
    
    // ============================================================
    // UTILITY COMMANDS
    // ============================================================
    else if (strcmp(token, "scan") == 0) {
        if (g_player.cdScan > 0.0f) {
            PushCliLog("[ERROR]: SCAN COOLDOWN (%.0fs)", g_player.cdScan);
        } else {
            g_player.cdScan = 240.0f;
            TriggerJitter(0.2f);
            PushCliLog("[SCAN]: INITIATING DEEP SUBNET FREQUENCY SWEEP...");
            
            int newSites = rand() % 3 + 1;
            for (int i = 0; i < newSites && g_player.assignedCount < 20; i++) {
                const char* names[] = {
                    "market", "vault", "terminal", "crypto", "hellroom",
                    "forum", "redroom", "morgue", "cult", "void",
                    "ghost", "silence", "watchtower", "orbital", "eye",
                    "blackbank", "silkroad", "weaponry", "passports", "darkdrop"
                };
                int idx = rand() % 20;
                char site[64];
                snprintf(site, sizeof(site), "%s.vnet", names[idx]);
                
                bool found = false;
                for (int j = 0; j < g_player.assignedCount; j++) {
                    if (strcmp(g_player.assignedSites[j], site) == 0) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    strcpy(g_player.assignedSites[g_player.assignedCount], site);
                    g_player.assignedCount++;
                    PushCliLog("[SCAN]: DISCOVERED %s", site);
                    // Refresh vnet.dir if we're on it
                    if (strcmp(g_player.currentURL, "vnet.dir") == 0) {
                        RefreshPage();
                    }
                }
            }
            if (newSites == 0) {
                PushCliLog("[SCAN]: NO NEW SITES FOUND");
            }
        }
    }
    else if (strcmp(token, "history") == 0) {
        PushCliLog("========== DISCOVERED SUBNET DIRECTORY ==========");
        if (g_player.assignedCount > 0) {
            for (int i = 0; i < g_player.assignedCount; i++) {
                PushCliLog("  -> vnet://%s", g_player.assignedSites[i]);
            }
            PushCliLog("  Total: %d sites", g_player.assignedCount);
        } else {
            PushCliLog("  [NO SITES DISCOVERED YET]");
            PushCliLog("  Type 'scan' to discover nodes.");
        }
        PushCliLog("==================================================");
    }
    else if (strcmp(token, "clear") == 0) {
        g_cliLogCount = 0;
        PushCliLog("[SYS]: TERMINAL CLEARED");
    }
    
    // ============================================================
    // WIN / VICTORY COMMANDS
    // ============================================================
    else if (strcmp(token, "win") == 0) {
        if (strstr(g_player.currentURL, "terminal.vnet")) {
            if (args && strstr(args, g_masterKeysStr)) {
                g_player.gameOver = true;
                strcpy(g_player.winMode, "KEYS");
                TriggerJitter(0.9f);
                PushCliLog("[SYSTEM OVERRIDE]: VICTORY! ROOT BREACH SUCCESSFUL!");
                PushFeedLog("[SYSTEM OVERRIDE]: PLAYER HAS BREACHED ROOT VAULT!");
            } else {
                PushCliLog("[ERROR]: INVALID KEY COMBINATION");
                PushCliLog("[INFO]: KEYS: %s", g_masterKeysStr);
            }
        } else {
            PushCliLog("[ERROR]: WIN ONLY AVAILABLE AT terminal.vnet");
        }
    }
    else if (strcmp(token, "takeover") == 0) {
        if (g_player.vcoin >= 25.0f) {
            g_player.gameOver = true;
            strcpy(g_player.winMode, "ECONOMIC");
            TriggerJitter(0.9f);
            PushCliLog("[ECONOMIC TAKEOVER]: VICTORY! YOU OWN VNET!");
            PushFeedLog("[ECONOMIC TAKEOVER]: PLAYER HAS BOUGHT OUT VNET!");
        } else {
            PushCliLog("[ERROR]: NEED 25.0 VCOIN (CURRENT: %.2f)", g_player.vcoin);
        }
    }
    
    // ============================================================
    // STATUS / STATS COMMANDS
    // ============================================================
    else if (strcmp(token, "status") == 0) {
        PushCliLog("========== NETWORK STATUS ==========");
        PushCliLog("  Current URL  : vnet://%s", g_player.currentURL);
        PushCliLog("  Active Port  : %d", g_player.port);
        PushCliLog("  VCOIN        : %.2f", g_player.vcoin);
        PushCliLog("  Trace Level  : %d%%", g_player.traceLevel);
        PushCliLog("  ICE Shields  : %d/3", g_player.iceShields);
        PushCliLog("  CRT Heat     : %.0f°C", g_player.crtHeat);
        PushCliLog("====================================");
    }
    else if (strcmp(token, "trace") == 0) {
        PushCliLog("[TRACE]: Current threat level: %d%%", g_player.traceLevel);
        if (g_player.traceLevel > 70) {
            PushCliLog("[WARNING]: High trace level! Consider using 'flush' or buying ICE.");
        }
    }
    
    // ============================================================
    // SPECIAL COMMANDS
    // ============================================================
    else if (strcmp(token, "flush") == 0) {
        if (g_player.vcoin < 0.10f) {
            PushCliLog("[ERROR]: NEED 0.10 VCOIN (CURRENT: %.2f)", g_player.vcoin);
        } else {
            g_player.vcoin -= 0.10f;
            g_player.traceLevel = (int)(g_player.traceLevel * 0.7f);
            if (g_player.traceLevel < 0) g_player.traceLevel = 0;
            g_player.neuralParanoia *= 0.85f;
            TriggerJitter(0.3f);
            PushCliLog("[FLUSH]: ROUTE PURGED! TRACE REDUCED BY 30%%");
        }
    }
    else if (strcmp(token, "calm") == 0 || strcmp(token, "meds") == 0) {
        if (g_player.vcoin < 0.15f) {
            PushCliLog("[ERROR]: NEED 0.15 VCOIN (CURRENT: %.2f)", g_player.vcoin);
        } else {
            g_player.vcoin -= 0.15f;
            g_player.neuralParanoia *= 0.6f;
            if (g_player.neuralParanoia < 0) g_player.neuralParanoia = 0;
            TriggerJitter(0.2f);
            PushCliLog("[CALM]: SYNAPTIC STABILIZER DEPLOYED. PARANOIA REDUCED.");
        }
    }
    // ============================================================
    // MISSING COMMANDS TO ADD TO ProcessCommand()
    // ============================================================

    else if (strcmp(token, "sniffer") == 0) {
        // Toggle global UDP packet sniffer
        if (g_player.snifferMode == 0 && g_player.vcoin < 0.05f) {
            PushCliLog("[ERROR]: NEED 0.05 VCOIN TO ENABLE SNIFFER");
        } else {
            g_player.snifferMode = (g_player.snifferMode == 0) ? 1 : 0;
            PushCliLog("[SNIFFER]: %s", g_player.snifferMode ? "ENABLED" : "DISABLED");
        }
    }

    else if (strcmp(token, "satscan") == 0) {
        // Requires watchtower.vnet
        if (strcmp(g_player.currentURL, "watchtower.vnet") != 0) {
            PushCliLog("[ERROR]: SATSCAN ONLY AT watchtower.vnet");
        } else if (g_player.cdSatscan > 0.0f) {
            PushCliLog("[ERROR]: SATSCAN COOLDOWN (%.0fs)", g_player.cdSatscan);
        } else if (g_player.vcoin < 0.55f) {
            PushCliLog("[ERROR]: NEED 0.55 VCOIN");
        } else {
            g_player.vcoin -= 0.55f;
            g_player.cdSatscan = 60.0f;
            PushCliLog("[SATSCAN]: INITIATING PANOPTICON SWEEP...");
            // You'd need to implement the network message
        }
    }

    else if (strcmp(token, "patch") == 0) {
        // Emergency port rebind
        if (g_player.cdPatch > 0.0f) {
            PushCliLog("[ERROR]: PATCH COOLDOWN (%.0fs)", g_player.cdPatch);
        } else if (g_player.vcoin < 0.70f) {
            PushCliLog("[ERROR]: NEED 0.70 VCOIN");
        } else {
            g_player.vcoin -= 0.70f;
            g_player.cdPatch = 120.0f;
            PushCliLog("[PATCH]: REQUESTING EMERGENCY PORT REBIND...");
        }
    }

    else if (strcmp(token, "decoy") == 0) {
        // Deploy trap node
        if (!args) {
            PushCliLog("[ERROR]: Usage: decoy <url> <dummy_port>");
        } else {
            char url[64] = {0}, port[16] = {0};
            sscanf(args, "%63s %15s", url, port);
            if (strlen(url) == 0 || strlen(port) == 0) {
                PushCliLog("[ERROR]: Usage: decoy <url> <dummy_port>");
            } else if (g_player.cdDecoy > 0.0f) {
                PushCliLog("[ERROR]: DECOY COOLDOWN (%.0fs)", g_player.cdDecoy);
            } else if (g_player.vcoin < 0.10f) {
                PushCliLog("[ERROR]: NEED 0.10 VCOIN");
            } else {
                g_player.vcoin -= 0.10f;
                g_player.cdDecoy = 5.0f;
                PushCliLog("[DECOY]: DEPLOYED TRAP NODE AT %s:%s", url, port);
            }
        }
    }

    else if (strcmp(token, "proxy") == 0) {
        // Route through intermediate proxy
        if (!args) {
            PushCliLog("[ERROR]: Usage: proxy <target_url> <proxy_node>");
        } else {
            char target[64] = {0}, proxyNode[64] = {0};
            sscanf(args, "%63s %63s", target, proxyNode);
            if (strlen(target) == 0 || strlen(proxyNode) == 0) {
                PushCliLog("[ERROR]: Usage: proxy <target_url> <proxy_node>");
            } else if (g_player.cdProxy > 0.0f) {
                PushCliLog("[ERROR]: PROXY COOLDOWN (%.0fs)", g_player.cdProxy);
            } else if (g_player.vcoin < 0.40f) {
                PushCliLog("[ERROR]: NEED 0.40 VCOIN");
            } else {
                g_player.vcoin -= 0.40f;
                g_player.cdProxy = 120.0f;
                PushCliLog("[PROXY]: ROUTING VIA %s TO %s", proxyNode, target);
                TriggerRouteNavigation(target);
            }
        }
    }

    else if (strcmp(token, "bounty") == 0) {
        TriggerRouteNavigation("bounty.vnet");
        PushCliLog("[BOUNTY]: ROUTING TO CONTRACT DIRECTORY...");
    }

    else if (strcmp(token, "crack") == 0) {
        if (!args) {
            PushCliLog("[ERROR]: Usage: crack <value>");
        } else {
            PushCliLog("[CRACK]: SENDING CHALLENGE VECTOR: %s", args);
            // Send to network
        }
    }

    else if (strcmp(token, "inspect") == 0) {
        char target[64];
        strncpy(target, args ? args : g_player.currentURL, 63);
        target[63] = '\0';
        PushCliLog("========== RAW SOURCE: vnet://%s ==========", target);
        // Load and dump raw page lines
        std::vector<std::string> rawLines;
        LoadPageContent(target, rawLines, g_player, g_vnet);
        for (const auto& line : rawLines) {
            PushCliLog("  [SRC]: %s", line.c_str());
        }
        PushCliLog("===========================================");
    }

    else if (strcmp(token, "shift") == 0) {
        if (!args) {
            PushCliLog("[ERROR]: Usage: shift <0-15>");
        } else {
            int offset = atoi(args);
            if (offset < 0 || offset > 15) {
                PushCliLog("[ERROR]: Shift must be 0-15");
            } else {
                g_player.bitShiftOffset = offset;
                PushCliLog("[SHIFT]: Bit-shift offset set to %d", offset);
            }
        }
    }

    else if (strcmp(token, "vdec") == 0 || strcmp(token, "decode") == 0) {
        // Decode active carrier key
        if (!args) {
            PushCliLog("[ERROR]: Usage: vdec <offset>");
        } else {
            int offset = atoi(args);
            PushCliLog("[VDEC]: DECRYPTING CARRIER WITH OFFSET %d...", offset);
            // You'd need to implement the decode logic
        }
    }

    else if (strcmp(token, "cat") == 0) {
        if (!args) {
            PushCliLog("[ERROR]: Usage: cat <vfs_path>");
        } else if (g_player.vcoin < 0.05f) {
            PushCliLog("[ERROR]: NEED 0.05 VCOIN");
        } else {
            g_player.vcoin -= 0.05f;
            PushCliLog("[VFS]: READING %s...", args);
            // Send to network
        }
    }

    else if (strcmp(token, "ls") == 0 || strcmp(token, "dir") == 0) {
        char path[64] = "/";
        if (args) strncpy(path, args, 63);
        if (g_player.vcoin < 0.02f) {
            PushCliLog("[ERROR]: NEED 0.02 VCOIN");
        } else {
            g_player.vcoin -= 0.02f;
            PushCliLog("[VFS]: SCANNING %s...", path);
            // Send to network
        }
    }

    else if (strcmp(token, "vektrapay") == 0) {
        if (strcmp(g_player.currentURL, "vektrapay.vnet") != 0) {
            PushCliLog("[ERROR]: VEKTRAPAY ONLY AT vektrapay.vnet");
        } else if (!args) {
            PushCliLog("========== VEKTRAPAY COMMANDS ==========");
            PushCliLog("  vektrapay create <amount>  - Lock VCOIN into escrow");
            PushCliLog("  vektrapay claim <key>      - Redeem escrow key");
            PushCliLog("========================================");
        } else {
            char action[16] = {0}, val[32] = {0};
            sscanf(args, "%15s %31s", action, val);
            if (strcmp(action, "create") == 0) {
                float amt = atof(val);
                if (amt <= 0 || g_player.vcoin < amt) {
                    PushCliLog("[ERROR]: INSUFFICIENT VCOIN");
                } else {
                    g_player.vcoin -= amt;
                    int key = rand() % 90000 + 10000;
                    PushCliLog("[VEKTRAPAY]: CREATED ESCROW KEY %d FOR %.2f VCOIN", key, amt);
                }
            } else if (strcmp(action, "claim") == 0) {
                PushCliLog("[VEKTRAPAY]: CLAIMING KEY %s...", val);
            }
        }
    }

    else if (strcmp(token, "netscan") == 0) {
        PushCliLog("[NETSCAN]: SCANNING ACTIVE PEERS ON %s...", g_player.currentURL);
        // Implement peer scanning
    }

    else if (strcmp(token, "ion") == 0 || strcmp(token, "ioncannon") == 0) {
        if (!args) {
            PushCliLog("[ERROR]: Usage: ion <target_port_or_url>");
        } else if (strcmp(g_player.currentURL, "orbital.vnet") != 0) {
            PushCliLog("[ERROR]: ION CANNON ONLY AT orbital.vnet");
        } else if (g_player.cdIon > 0.0f) {
            PushCliLog("[ERROR]: ION COOLDOWN (%.0fs)", g_player.cdIon);
        } else if (g_player.vcoin < 2.00f) {
            PushCliLog("[ERROR]: NEED 2.00 VCOIN");
        } else {
            g_player.vcoin -= 2.00f;
            g_player.cdIon = 180.0f;
            TriggerJitter(1.0f);
            PushCliLog("[ION CANNON]: FIRING ON %s...", args);
        }
    }

    else if (strcmp(token, "0day") == 0 || strcmp(token, "zeroday") == 0) {
        // Zero-day exploit synthesis
        if (strcmp(g_player.currentURL, "zeroday.vnet") != 0) {
            PushCliLog("[ERROR]: ZERO-DAY ONLY AT zeroday.vnet");
        } else if (!args) {
            PushCliLog("[ERROR]: Usage: 0day <LOT_CODE>");
            PushCliLog("Available: LOT_0D1 (overclock), LOT_0D2 (purge), LOT_0D3 (nuke)");
        } else {
            PushCliLog("[0-DAY]: INJECTING PAYLOAD %s...", args);
            // Implement based on LOT code
        }
    }
    
    // ============================================================
    // UNKNOWN COMMAND
    // ============================================================
    else {
        PushCliLog("[ERR]: UNKNOWN COMMAND '%s' (type 'help')", token);
    }
}

void ProcessChatCommand(const char* msg) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "[CHAT] <%s>: %s", g_player.handle, msg);
    PushFeedLog(buffer);
    PushCliLog(buffer);
}

void ProcessWhisperCommand(const char* target, const char* msg) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "[WHISPER TO <%s>]: %s", target, msg);
    PushCliLog(buffer);
    PushFeedLog(buffer);
}

// ============================================================
// UTILITY FUNCTIONS
// ============================================================

void PushCliLog(const char* fmt, ...) {
    if (g_cliLogCount >= 250) {
        // Shift logs
        for (int i = 0; i < 249; i++) {
            strcpy(g_cliLogs[i], g_cliLogs[i + 1]);
        }
        g_cliLogCount = 249;
    }
    
    va_list args;
    va_start(args, fmt);
    vsnprintf(g_cliLogs[g_cliLogCount], sizeof(g_cliLogs[0]), fmt, args);
    va_end(args);
    
    g_cliLogCount++;
}

void PushFeedLog(const char* fmt, ...) {
    if (g_feedLogCount >= 100) {
        for (int i = 0; i < 99; i++) {
            strcpy(g_feedLogs[i], g_feedLogs[i + 1]);
        }
        g_feedLogCount = 99;
    }
    
    va_list args;
    va_start(args, fmt);
    vsnprintf(g_feedLogs[g_feedLogCount], sizeof(g_feedLogs[0]), fmt, args);
    va_end(args);
    
    g_feedLogCount++;
}

void TriggerRouteNavigation(const char* url) {
    char cleanURL[64];
    strncpy(cleanURL, url, sizeof(cleanURL) - 1);
    cleanURL[sizeof(cleanURL) - 1] = '\0';
    
    if (strncmp(cleanURL, "vnet://", 7) == 0) {
        memmove(cleanURL, cleanURL + 7, strlen(cleanURL) - 6);
    }

    // Store current URL as previous before navigating
    strcpy(g_player.prevURL, g_player.currentURL);
    
    TriggerJitter(0.3f + (rand() % 100) / 500.0f);
    
    g_player.isConnecting = true;
    g_player.connectTimer = 0.0f;
    g_player.targetConnectTime = 0.8f + (rand() % 100) / 100.0f;
    strcpy(g_player.pendingURL, cleanURL);
    
    PushCliLog("[TOR_ROUTE]: CONNECTING TO %s...", cleanURL);
}

void UpdatePeerSession(uint32_t port, const char* ip, const char* url) {
    // In single-player, just update player
    if (port == g_player.port) {
        strcpy(g_player.currentURL, url);
    }
}

// ============================================================
// MINING SYSTEM
// ============================================================

void StartMining(void) {
    // Only at crypto.vnet
    if (!strstr(g_player.currentURL, "crypto.vnet")) {
        PushCliLog("[ERROR]: MINING ONLY AT crypto.vnet");
        return;
    }
    
    // Check cooldown
    if (g_player.cdMine > 0.0f) {
        PushCliLog("[ERROR]: RIG COOLING DOWN (%.0fs)", g_player.cdMine);
        return;
    }
    
    // Generate block
    int blockID = rand() % 9000 + 1000;
    float reward = (rand() % 46 + 5) / 100.0f;
    
    char block[64];
    snprintf(block, sizeof(block), "%d:%.2f", blockID, reward);
    strcpy(g_minedBlocks[g_minedCount], block);
    g_minedCount++;
    
    g_player.vcoin += reward;
    g_player.cdMine = 5.0f;
    g_player.traceLevel += 18;
    if (g_player.traceLevel > 100) g_player.traceLevel = 100;
    g_player.crtHeat += 3.5f;
    g_player.glitchTrigger = 0.4f;
    
    PushCliLog("[MINER]: MINED BLOCK #%d! +%.2f VCOIN", blockID, reward);
    PushFeedLog("[MINER]: PLAYER MINED BLOCK #%d", blockID);
}

void UpdateMining(float dt) {
    if (g_player.cdMine > 0.0f) {
        g_player.cdMine -= dt;
    }
}