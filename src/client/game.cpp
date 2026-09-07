#include "game.h"
#include "../shared/vnet.h"
#include "player.h"
#include "render.h"
#include "./desktop/widgets/music_player.h"
#include "./raid/raid.h"
#include "vnet_client.h"
#include "../shared/vnet_protocol.h" 
#include "./desktop/desktop.h"
#include "connection/login_screen.h"
#include "raid/raid.h"

#include <cstdio>
#include <cmath>

GameState g_game = {0};
extern Player g_player;
LoginScreen g_loginScreen;

// ============================================================
// NEW: Loading completion timer
// ============================================================
static float g_loadingCompletionTimer = 0.0f;
static bool g_loadingWaitingToFinish = false;

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
    g_game.showFPS   = false;
    g_game.showDebug = false;
    
    // Initialize loading timer
    g_loadingCompletionTimer = 0.0f;
    g_loadingWaitingToFinish = false;

    RecalcScale();

    InitVNETSystem();
    InitPlayer();
    LoadAssets();

    InitLoginScreen(g_loginScreen);

    GetDesktop().Init();
    GetMusicPlayer().Init(); 

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

    UpdateThemeTransition(dt);

    UpdateVNET(dt);
    UpdatePlayer(dt);
    UpdateRaid(dt);

    GetDesktop().Update(dt); 
    GetMusicPlayer().Update(dt); 

    g_game.fpsTimer += dt;
    if (g_game.fpsTimer >= 0.5f) {
        g_game.currentFPS = GetFPS();
        g_game.fpsTimer = 0.0f;
    }
    
    // ============================================================
    // NEW: Handle loading completion delay
    // ============================================================
    if (g_loadingWaitingToFinish) {
        g_loadingCompletionTimer += dt;
        if (g_loadingCompletionTimer >= 1.5f) {  // Show loading for 1.5 seconds
            g_loadingWaitingToFinish = false;
            g_loginScreen.isActive = false;
            g_loginScreen.isLoading = false;
        }
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
    GetMusicPlayer().Shutdown(); 
}

void HandleInput(void) {
    // ============================================================
    // LOGIN SCREEN INPUT
    // ============================================================
    if (g_loginScreen.isActive) {
        bool shouldConnect = HandleLoginInput(g_loginScreen);
        
        if (shouldConnect) {
            // Try to connect (loading animation already started)
            if (InitVNetClient(GetLoginIP(g_loginScreen), 8000)) {
                // ============================================================
                // FIXED: Don't hide loading screen instantly!
                // ============================================================
                
                // Connection successful - let the loading animation complete
                // The loading screen will stay visible for ~1.5 seconds
                g_loginScreen.loadingProgress = 1.0f;
                g_loginScreen.loadingComplete = true;
                strcpy(g_loginScreen.loadingStatus, "CONNECTED! INITIALIZING...");
                
                // Store the handle and IP for later use
                strcpy(g_player.handle, GetLoginHandle(g_loginScreen));
                PushCliLog("[CONNECT]: Connected to %s as %s", 
                          GetLoginIP(g_loginScreen), g_player.handle);
                LoadPage("vnet.dir");
                
                std::string pingMsg = std::string(VNetCmd::PING) + ":" + 
                                     g_player.handle + ":" + g_player.currentURL;
                VNetSendRaw(pingMsg);
                
                // ============================================================
                // START THE DELAY TIMER - DON'T HIDE YET!
                // ============================================================
                g_loadingCompletionTimer = 0.0f;
                g_loadingWaitingToFinish = true;
                // g_loginScreen.isActive stays TRUE - loading screen remains visible
                
            } else {
                // Connection failed - show error and reset
                PushCliLog("[ERROR]: Failed to connect to %s", GetLoginIP(g_loginScreen));
                g_loginScreen.isLoading = false;
                g_loginScreen.loadingProgress = 0.0f;
                strcpy(g_loginScreen.loadingStatus, "CONNECTION FAILED - RETRY");
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

        static float backspaceTimer = 0.0f;
        static bool backspaceHeld = false;

        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = (int)strlen(g_player.inputBuffer);
            if (len > 0) {
                g_player.inputBuffer[len - 1] = '\0';
            }
            backspaceTimer = 0.0f;
            backspaceHeld = true;
        }
        
        if (IsKeyDown(KEY_BACKSPACE) && backspaceHeld) {
            backspaceTimer += GetFrameTime();
            if (backspaceTimer > 0.5f) {
                float repeatInterval = 0.08f;
                while (backspaceTimer > 0.5f + repeatInterval) {
                    int len = (int)strlen(g_player.inputBuffer);
                    if (len > 0) {
                        g_player.inputBuffer[len - 1] = '\0';
                    } else {
                        break;
                    }
                    backspaceTimer -= repeatInterval;
                }
            }
        }
        
        if (IsKeyReleased(KEY_BACKSPACE)) {
            backspaceHeld = false;
            backspaceTimer = 0.0f;
        }

        if (IsKeyPressed(KEY_ENTER)) {
            if (strlen(g_player.inputBuffer) > 0) {
                // ============================================================
                // HANDLE RAID CLI INPUT FIRST
                // ============================================================
                HandleRaidCLIInput(g_player.inputBuffer);
                
                // If raid handled it, it cleared the buffer
                // If not, process normally
                if (g_player.inputBuffer[0] != '\0') {
                    ProcessCommand(g_player.inputBuffer);
                    g_player.inputBuffer[0] = '\0';
                }
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