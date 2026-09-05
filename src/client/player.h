#pragma once
#include "raylib.h"
#include "vnet.h"

// Player functions
void InitPlayer(void);
void UpdatePlayer(float dt);
void ShutdownPlayer(void);

// Player movement (for future 3D)
void PlayerMove(float dx, float dz);
void PlayerJump(void);
void PlayerCrouch(void);
void PlayerSprint(void);

// Player state
extern Player g_player;

// Hellroom chat UI state
struct HellroomUI {
    char handleInputBuffer[32];
    char chatInputBuffer[256];
    bool handleFocused;
    bool chatFocused;
    float chatScrollY;
    float chatFeedY;
};

extern HellroomUI g_hellroom;