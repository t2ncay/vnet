#include "game.h"
#include "vnet.h"
#include "player.h"
#include "render.h"
#include "vnet_client.h"
#include "vnet_protocol.h" 
#include "desktop.h"

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

    GetDesktop().Init();

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

    UpdateVNET(dt);
    UpdatePlayer(dt);

    GetDesktop().Update(dt); 

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
    GetDesktop().Shutdown();
}

void HandleInput(void) {
    // ============================================================
    // CONNECTION MENU INPUT
    // ============================================================
    if (g_player.isInConnectionMenu) {
        Vector2 refMouse = GetRefMousePos();
        bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
                
        bool ipBoxHover = RefRectHover(200, 290, 400, 40, refMouse);
        if (clicked && ipBoxHover) {
            g_player.ipBoxFocused = true;
        }
        
        bool btnHover = RefRectHover(270, 370, 260, 40, refMouse);
        if ((clicked && btnHover) || IsKeyPressed(KEY_ENTER)) {
            if (strlen(g_player.ipInputBuffer) > 0) {
                if (InitVNetClient(g_player.ipInputBuffer, 8000)) {
                    g_player.isInConnectionMenu = false;
                    PushCliLog("[CONNECT]: Connected to %s", g_player.ipInputBuffer);
                    LoadPage("vnet.dir");
                    
                    std::string pingMsg = std::string(VNetCmd::PING) + ":" + g_player.handle + ":" + g_player.currentURL;
                    VNetSendRaw(pingMsg);
                } else {
                    PushCliLog("[ERROR]: Failed to connect to %s", g_player.ipInputBuffer);
                }
            }
        }
        
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

    // ============================================================
    // DESKTOP MODE - Process CLI input
    // ============================================================
    
    bool isTerminalFocused = GetDesktop().IsTerminalFocused();
    bool isBrowserFocused = GetDesktop().IsBrowserFocused();
    
    // Process CLI input if terminal is focused or CLI is open
    if (isTerminalFocused || g_player.cliOpen) {
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
    // MOUSE WHEEL SCROLLING - SINGLE HANDLER
    // ============================================================
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        // If terminal is focused or CLI is open, scroll the terminal
        if (g_player.cliOpen || isTerminalFocused) {
            g_player.cliScroll -= wheel * 20.0f;
            if (g_player.cliScroll < 0.0f) g_player.cliScroll = 0.0f;
        } 
        // Otherwise scroll the browser
        else {
            g_player.pageScroll -= wheel * 28.0f;
            if (g_player.pageScroll < 0.0f) g_player.pageScroll = 0.0f;
        }
    }
    
    // Handle TAB to toggle CLI (duplicate removed)
    if (IsKeyPressed(KEY_TAB)) {
        g_player.cliOpen = !g_player.cliOpen;
    }

    // ============================================================
    // HELLROOM INPUT HANDLING
    // ============================================================
    if (strcmp(g_player.currentURL, "hellroom.vnet") == 0) {
        if (!g_player.cliOpen) {
            if (g_hellroom.handleFocused) {
                int key = GetCharPressed();
                while (key > 0) {
                    if (key >= 32 && key <= 126) {
                        size_t len = strlen(g_hellroom.handleInputBuffer);
                        if (len < sizeof(g_hellroom.handleInputBuffer) - 1) {
                            g_hellroom.handleInputBuffer[len] = (char)key;
                            g_hellroom.handleInputBuffer[len + 1] = '\0';
                        }
                    }
                    key = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    int len = (int)strlen(g_hellroom.handleInputBuffer);
                    if (len > 0) {
                        g_hellroom.handleInputBuffer[len - 1] = '\0';
                    }
                }
                if (IsKeyPressed(KEY_ENTER)) {
                    if (strlen(g_hellroom.handleInputBuffer) > 0) {
                        strcpy(g_player.handle, g_hellroom.handleInputBuffer);
                        PushCliLog("[HELLROOM]: Handle updated to %s", g_player.handle);
                    }
                    g_hellroom.handleFocused = false;
                    g_hellroom.chatFocused = true;
                }
            } else if (g_hellroom.chatFocused) {
                int key = GetCharPressed();
                while (key > 0) {
                    if (key >= 32 && key <= 126) {
                        size_t len = strlen(g_hellroom.chatInputBuffer);
                        if (len < sizeof(g_hellroom.chatInputBuffer) - 1) {
                            g_hellroom.chatInputBuffer[len] = (char)key;
                            g_hellroom.chatInputBuffer[len + 1] = '\0';
                        }
                    }
                    key = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    int len = (int)strlen(g_hellroom.chatInputBuffer);
                    if (len > 0) {
                        g_hellroom.chatInputBuffer[len - 1] = '\0';
                    }
                }
                if (IsKeyPressed(KEY_ENTER)) {
                    if (strlen(g_hellroom.chatInputBuffer) > 0) {
                        if (strlen(g_hellroom.chatInputBuffer) >= 3 && 
                            (strncmp(g_hellroom.chatInputBuffer, "/w ", 3) == 0 || 
                             strncmp(g_hellroom.chatInputBuffer, "/pm ", 4) == 0)) {
                            char* cmdStart = g_hellroom.chatInputBuffer;
                            if (cmdStart[0] == '/' && cmdStart[1] == 'w') {
                                cmdStart += 3;
                            } else if (cmdStart[0] == '/' && cmdStart[1] == 'p' && cmdStart[2] == 'm') {
                                cmdStart += 4;
                            }
                            while (*cmdStart == ' ') cmdStart++;
                            
                            char target[64] = {0};
                            char msg[256] = {0};
                            if (sscanf(cmdStart, "%63s %255[^\n]", target, msg) == 2) {
                                std::string payload = std::string(g_player.handle) + ":" + target + ":" + msg;
                                if (IsVNetConnected()) {
                                    VNetSendRaw(std::string(VNetCmd::WHISPER) + ":" + payload);
                                }
                                char buffer[256];
                                snprintf(buffer, sizeof(buffer), "[WHISPER TO <%s>]: %s", target, msg);
                                PushCliLog(buffer);
                                PushFeedLog(buffer);
                            }
                        } else {
                            std::string payload = std::string(g_player.handle) + ":" + g_hellroom.chatInputBuffer;
                            if (IsVNetConnected()) {
                                VNetSendRaw(std::string(VNetCmd::CHAT) + ":" + payload);
                            }
                        }
                        g_hellroom.chatInputBuffer[0] = '\0';
                        TriggerJitter(0.2f);
                    }
                }
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
}