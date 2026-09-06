#include "raylib.h"
#include "game.h"
#include <iostream>

int main(void) {
    // Get the monitor's native resolution
    int monitor = GetCurrentMonitor();
    int screenWidth  = GetMonitorWidth(monitor);
    int screenHeight = GetMonitorHeight(monitor);
    
    // Fallback if monitor detection fails
    if (screenWidth <= 0)  screenWidth  = 1920;
    if (screenHeight <= 0) screenHeight = 1080;

    // Set window flags for fullscreen
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_FULLSCREEN_MODE);
    InitWindow(screenWidth, screenHeight, "VNET - VEKTRAOS v9.5 CYBERWARFARE ENGINE");

    // Set fullscreen (if not already)
    if (!IsWindowFullscreen()) {
        ToggleFullscreen();
    }

    SetTargetFPS(90);
    SetExitKey(KEY_NULL);

    InitAudioDevice();

    if (!InitGame()) {
        std::cerr << "Failed to initialize game!" << std::endl;
        CloseAudioDevice();
        CloseWindow();
        return 1;
    }

    while (!WindowShouldClose() && g_game.running) {
        float dt = GetFrameTime();
        UpdateGame(dt);
        DrawGame();
    }

    ShutdownGame();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}