#include "game.h"
#include "vnet.h"
#include "player.h"
#include "render.h"
#include "vnet_client.h"
#include "vnet_protocol.h" 
#include <cstdio>
#include <cmath>

GameState g_game = {0};
extern Player g_player;

// ============================================================
// SCALE / LETTERBOX CALCULATION
// ============================================================
static void RecalcScale(void) {
    g_game.screenWidth  = GetScreenWidth();
    g_game.screenHeight = GetScreenHeight();

    float scaleX = (float)g_game.screenWidth  / (float)REF_WIDTH;
    float scaleY = (float)g_game.screenHeight / (float)REF_HEIGHT;
    
    g_game.uiScale = (scaleX < scaleY) ? scaleX : scaleY;
    
    if (g_game.uiScale < 0.6f) g_game.uiScale = 0.6f;
    if (g_game.uiScale > 2.5f) g_game.uiScale = 2.5f;

    float scaledW = REF_WIDTH  * g_game.uiScale;
    float scaledH = REF_HEIGHT * g_game.uiScale;

    g_game.offsetX = (g_game.screenWidth  - scaledW) * 0.5f;
    g_game.offsetY = (g_game.screenHeight - scaledH) * 0.5f;

    g_uiScale  = g_game.uiScale;
    g_offsetX  = g_game.offsetX;
    g_offsetY  = g_game.offsetY;
}

bool InitGame(void) {
    g_game.running   = true;
    g_game.runTime   = 0.0f;
    g_game.showFPS   = true;
    g_game.showDebug = false;

    RecalcScale();

    InitVNETSystem();
    InitPlayer();
    LoadAssets();

    // Placeholder starting page
    LoadPage("vnet.dir");

    return true;
}

void HandleResize(void) {
    int newWidth  = GetScreenWidth();
    int newHeight = GetScreenHeight();
    if (newWidth != g_game.screenWidth || newHeight != g_game.screenHeight) {
        RecalcScale();
    }
}

void UpdateGame(float dt) {
    g_game.deltaTime = dt;
    g_game.runTime  += dt;

    HandleResize();
    HandleInput();

    // ============================================================
    // PROCESS NETWORK PACKETS FROM SERVER
    // ============================================================
    if (IsVNetConnected()) {
        auto packets = VNetReceive();
        for (const auto& packet : packets) {
            // Parse packet
            auto parsed = ParsePacket(packet);
            std::string cmd = parsed.first;
            std::string payload = parsed.second;
            
            // Handle server responses
            if (cmd == VNetResp::FEED_EVENT) {
                PushFeedLog(payload.c_str());
            }
            else if (cmd == VNetResp::KEY_SYNC) {
                // Parse key sync - format: KEY_SYNC:k1:k2:k3:k4:k5:k6:k7:k8:loc1:loc2:...:dirPayload
                // For now, just log it
                PushCliLog("[SERVER]: Received key sync");
            }
            else if (cmd == VNetResp::NEW_BLOCKS) {
                // Parse new blocks
                PushCliLog("[SERVER]: New mining blocks available!");
            }
            else if (cmd == VNetResp::WHISPER_IN) {
                // Format: WHISPER_IN:from_handle:message
                size_t sep = payload.find(':');
                if (sep != std::string::npos) {
                    std::string fromHandle = payload.substr(0, sep);
                    std::string msg = payload.substr(sep + 1);
                    char buffer[256];
                    snprintf(buffer, sizeof(buffer), "[WHISPER FROM <%s>]: %s", fromHandle.c_str(), msg.c_str());
                    PushFeedLog(buffer);
                    PushCliLog(buffer);
                }
            }
            else if (cmd == VNetResp::EXPLOIT_DOS) {
                PushCliLog("[ALERT]: INCOMING DOS ATTACK!");
                PushFeedLog("[DOS ATTACK]: You are being DOSed!");
                g_player.dosTimer = 8.0f;
            }
            else if (cmd == VNetResp::EXPLOIT_TRACE_SPIKE) {
                PushCliLog("[ALERT]: REVERSE TRACE SPIKE DETECTED!");
                if (g_player.iceShields > 0) {
                    g_player.iceShields--;
                    PushCliLog("[ICE]: ABSORBED TRACE SPIKE! (%d/3 remaining)", g_player.iceShields);
                } else {
                    g_player.traceLevel += 35;
                    if (g_player.traceLevel > 100) g_player.traceLevel = 100;
                }
            }
            else if (cmd == VNetResp::EXPLOIT_REDIRECT) {
                PushCliLog("[ALERT]: BGP HIJACK DETECTED! REDIRECTING TO %s", payload.c_str());
                LoadPage(payload.c_str());
            }
            else if (cmd == VNetResp::EXPLOIT_SITE_OVERLOADED) {
                PushCliLog("[ALERT]: SITE %s IS OVERLOADED!", payload.c_str());
            }
            else if (cmd == VNetResp::SCAN_RESULT) {
                PushCliLog("[SCAN RESULT]: Discovered %s", payload.c_str());
                // Add to discovered sites if not already there
                bool found = false;
                for (int i = 0; i < g_player.assignedCount; i++) {
                    if (strcmp(g_player.assignedSites[i], payload.c_str()) == 0) {
                        found = true;
                        break;
                    }
                }
                if (!found && g_player.assignedCount < 20) {
                    strcpy(g_player.assignedSites[g_player.assignedCount], payload.c_str());
                    g_player.assignedCount++;
                    if (strcmp(g_player.currentURL, "vnet.dir") == 0) {
                        RefreshPage();
                    }
                }
            }
            else if (cmd == VNetResp::SATSCAN_RES) {
                PushCliLog("[SATSCAN RESULTS]:");
                // Parse and display results
                std::string remaining = payload;
                size_t pos;
                while ((pos = remaining.find(';')) != std::string::npos) {
                    std::string entry = remaining.substr(0, pos);
                    remaining = remaining.substr(pos + 1);
                    // Format: site|traffic|status
                    size_t sep1 = entry.find('|');
                    if (sep1 != std::string::npos) {
                        std::string site = entry.substr(0, sep1);
                        std::string rest = entry.substr(sep1 + 1);
                        size_t sep2 = rest.find('|');
                        if (sep2 != std::string::npos) {
                            std::string traffic = rest.substr(0, sep2);
                            std::string status = rest.substr(sep2 + 1);
                            PushCliLog("  %s | %s KB/s | [%s]", site.c_str(), traffic.c_str(), status.c_str());
                        }
                    }
                }
            }
            else if (cmd == VNetResp::EXPLOIT_WINNER) {
                PushCliLog("[GAME OVER]: %s", payload.c_str());
                g_player.gameOver = true;
            }
            else {
                // Unknown packet - log it
                PushCliLog("[NET]: %s", packet.c_str());
            }
        }
    }

    UpdateVNET(dt);
    UpdatePlayer(dt);

    g_game.fpsTimer += dt;
    if (g_game.fpsTimer >= 0.5f) {
        g_game.currentFPS = GetFPS();
        g_game.fpsTimer = 0.0f;
    }
}

void DrawGame(void) {
    BeginDrawing();

    ClearBackground(COLOR_BLACK);

    DrawUI();

    if (g_game.showFPS) {
        char fpsText[32];
        snprintf(fpsText, sizeof(fpsText), "FPS: %d", g_game.currentFPS);
        DrawText(fpsText, 10, 10, 18, COLOR_TOXIC);
    }

    EndDrawing();
}

void ShutdownGame(void) {
    ShutdownVNETSystem();
    ShutdownPlayer();
    UnloadAssets();
    ShutdownVNetClient();
}

void HandleInput(void) {

    // ============================================================
    // CONNECTION MENU INPUT
    // ============================================================
    if (g_player.isInConnectionMenu) {
        Vector2 refMouse = GetRefMousePos();
        bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
                
        // Check if click is on IP input box
        bool ipBoxHover = RefRectHover(200, 290, 400, 40, refMouse);
        if (clicked && ipBoxHover) {
            g_player.ipBoxFocused = true;
        }
        
        // Check if click is on connect button
        bool btnHover = RefRectHover(270, 370, 260, 40, refMouse);
        if ((clicked && btnHover) || IsKeyPressed(KEY_ENTER)) {
            if (strlen(g_player.ipInputBuffer) > 0) {
                // Connect to server
                if (InitVNetClient(g_player.ipInputBuffer, 8000)) {
                    g_player.isInConnectionMenu = false;
                    PushCliLog("[CONNECT]: Connected to %s", g_player.ipInputBuffer);
                    LoadPage("vnet.dir");
                    
                    // Send initial PING to register with server
                    std::string pingMsg = std::string(VNetCmd::PING) + ":" + g_player.handle + ":" + g_player.currentURL;
                    VNetSendRaw(pingMsg);
                } else {
                    PushCliLog("[ERROR]: Failed to connect to %s", g_player.ipInputBuffer);
                }
            }
        }
        
        // Handle typing in IP box
        if (g_player.ipBoxFocused) {
            int key = GetCharPressed();
            while (key > 0) {
                if (key >= 32 && key <= 126) {
                    size_t len = strlen(g_player.ipInputBuffer);
                    if (len < sizeof(g_player.ipInputBuffer) - 1) {
                        g_player.ipInputBuffer[len] = (char)key;
                        g_player.ipInputBuffer[len + 1] = '\0';
                    }
                }
                key = GetCharPressed();
            }
            
            if (IsKeyPressed(KEY_BACKSPACE)) {
                int len = (int)strlen(g_player.ipInputBuffer);
                if (len > 0) {
                    g_player.ipInputBuffer[len - 1] = '\0';
                }
            }
        }
        
        return;
    }

    // ============================================================
    // MAIN INPUT HANDLING
    // ============================================================
    
    if (IsKeyPressed(KEY_F1)) g_game.showFPS = !g_game.showFPS;
    if (IsKeyPressed(KEY_F2)) g_game.showDebug = !g_game.showDebug;

    if (IsKeyPressed(KEY_F11)) {
        ToggleFullscreen();
        RecalcScale();
    }

    if (IsKeyPressed(KEY_TAB)) {
        g_player.cliOpen = !g_player.cliOpen;
    }

    if (g_player.cliOpen) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126) {
                size_t len = strlen(g_player.inputBuffer);
                if (len < sizeof(g_player.inputBuffer) - 1) {
                    g_player.inputBuffer[len] = (char)key;
                    g_player.inputBuffer[len + 1] = '\0';
                }
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = (int)strlen(g_player.inputBuffer);
            if (len > 0) g_player.inputBuffer[len - 1] = '\0';
        }

        if (IsKeyPressed(KEY_ENTER)) {
            if (strlen(g_player.inputBuffer) > 0) {
                ProcessCommand(g_player.inputBuffer);
                g_player.inputBuffer[0] = '\0';
            }
        }
    }

    // ============================================================
    // MOUSE CLICK HANDLING
    // ============================================================
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        float intensity = 0.05f + (rand() % 100) / 250.0f;
        if (intensity > 0.4f) intensity = 0.4f;
        TriggerJitter(intensity);
    }

    // Mouse wheel scroll
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        if (g_player.cliOpen) {
            g_player.cliScroll -= wheel * 22.0f;
            if (g_player.cliScroll < 0.0f) g_player.cliScroll = 0.0f;
        } else {
            g_player.pageScroll -= wheel * 28.0f;
            if (g_player.pageScroll < 0.0f) g_player.pageScroll = 0.0f;
        }
    }
}