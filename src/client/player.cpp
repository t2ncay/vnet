#include "player.h"
#include "vnet.h"
#include <cstring>

extern Player g_player;

// Hellroom UI state
HellroomUI g_hellroom = {0};

void InitPlayer(void) {
    // Player is already initialized in vnet.cpp
    g_player.cliScroll = 0.0f;
    g_player.feedScroll = 0.0f;
    g_player.pageScroll = 0.0f;
    g_player.urlFocused = false;
    g_player.chatFocused = false;
    g_player.handleFocused = false;

    // Init Hellroom UI state
    memset(&g_hellroom, 0, sizeof(HellroomUI));
    g_hellroom.handleFocused = false;
    g_hellroom.chatFocused = false;
    g_hellroom.chatScrollY = 0.0f;
    g_hellroom.chatFeedY = 710.0f;
    strcpy(g_hellroom.handleInputBuffer, g_player.handle);
}

void UpdatePlayer(float dt) {
    (void)dt;
}

void ShutdownPlayer(void) {
    // Cleanup
}

void PlayerMove(float dx, float dz) {
    (void)dx; (void)dz;
}

void PlayerJump(void) {
}

void PlayerCrouch(void) {
    g_player.cliOpen = !g_player.cliOpen;
}

void PlayerSprint(void) {
}