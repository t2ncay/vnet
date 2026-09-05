#include "player.h"
#include "vnet.h"
#include <cstring>

extern Player g_player;

void InitPlayer(void) {
    // Player is already initialized in vnet.cpp
    // This just handles any additional setup
    g_player.cliScroll = 0.0f;
    g_player.feedScroll = 0.0f;
    g_player.pageScroll = 0.0f;
    g_player.urlFocused = false;
    g_player.chatFocused = false;
    g_player.handleFocused = false;
}

void UpdatePlayer(float dt) {
    // Update any player-specific timers
    // For now, most logic is in vnet.cpp
    (void)dt; // Suppress unused parameter warning
}

void ShutdownPlayer(void) {
    // Cleanup
}

void PlayerMove(float dx, float dz) {
    // For future 3D movement
    (void)dx; (void)dz;
}

void PlayerJump(void) {
    // For future 3D movement
}

void PlayerCrouch(void) {
    // For future 3D movement
    g_player.cliOpen = !g_player.cliOpen; // Toggle terminal for now
}

void PlayerSprint(void) {
    // For future 3D movement
}