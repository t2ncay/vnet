#pragma once
#include "raylib.h"
#include "../shared/vnet.h"
#include "./player.h"
#include "render.h"
#include "../shared/utils.h"
#include "./duel/duel.h"
#include "./networld/networld.h"

// ============================================================
// REFERENCE CANVAS
// ============================================================
constexpr int REF_WIDTH  = 1920;
constexpr int REF_HEIGHT = 1080;

bool InitGame(void);
void UpdateGame(float dt);
void DrawGame(void);
void ShutdownGame(void);
void HandleInput(void);
void HandleResize(void);

// Global game state
struct GameState {
    bool running;
    float deltaTime;
    float runTime;
    bool showFPS;
    bool showDebug;
    int fpsCounter;
    float fpsTimer;
    int currentFPS;
    int screenWidth;
    int screenHeight;
    float uiScale;
    float offsetX;
    float offsetY;
};

extern GameState g_game;
extern struct LoginScreen g_loginScreen;
extern struct Duel g_duel;
extern NetWorld g_netWorld;