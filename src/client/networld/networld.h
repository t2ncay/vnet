#pragma once
#include "raylib.h"
#include <string>
#include <vector>

// ============================================================
// NETWORLD ENUMS
// ============================================================

enum class NetWorldState {
    IDLE,           // Not in NetWorld
    ENTERING,       // Transition effect (glitch)
    ACTIVE,         // Fully in 3D cyberspace
    EXITING         // Leaving with glitch effect
};

enum class NodeType {
    PORTAL,         // Entry point to a VNET site
    DATA_NODE,      // Information/collectible
    ENEMY,          // Glitch enemy / bot
    CORE            // Central node (hub)
};

// ============================================================
// NETWORLD NODE STRUCT
// ============================================================

struct NetNode {
    NodeType type;
    std::string id;              // Corresponds to VNET site URL
    std::string label;
    Vector3 position;
    Vector3 color;               // RGB float for glow
    float radius;
    bool active;
    bool discovered;
    float pulsePhase;
    float rotationAngle;
};

// ============================================================
// NETWORLD PLAYER STATE
// ============================================================

struct NetPlayer {
    Vector3 position;
    Vector3 velocity;
    Vector3 forward;
    Vector3 right;
    Vector3 up;
    float yaw;                   // Horizontal rotation (radians)
    float pitch;                 // Vertical rotation (radians)
    float speed;
    float jumpVelocity;
    bool onGround;
    bool moving;
    float walkCycle;
};

// ============================================================
// MAIN NETWORLD STATE
// ============================================================

struct NetWorld {
    bool active;
    bool useShader; // For CRT shader effect
    NetWorldState state;
    float stateTimer;
    float transitionDuration;    // 2.0 seconds for enter/exit
    
    // 3D camera
    Camera3D camera;
    NetPlayer player;
    
    // World
    std::vector<NetNode> nodes;
    float worldSize;            // 100.0f units
    int gridSize;              // 20x20 grid
    float terrainHeight;
    
    // Visual state
    float glitchIntensity;
    float scanlineOffset;
    float time;
    bool showMinimap;
    bool showNodeLabels;
    
    // Selected node
    int selectedNodeIndex;
    float interactionCooldown;
    
    // Glitch transition
    float transitionGlitchTimer;
};

// ============================================================
// GLOBAL NETWORLD INSTANCE
// ============================================================

extern NetWorld g_netWorld;

// ============================================================
// FUNCTION PROTOTYPES
// ============================================================

// Core
void InitNetWorld(void);
void UpdateNetWorld(float dt);
void DrawNetWorld(void);
bool IsInNetWorld(void);
void EnterNetWorld(void);
void ExitNetWorld(void);
void UpdateNetSelection(void);

// Physics / Movement
void UpdateNetPlayer(float dt);
void HandleNetInput(void);
void CheckNetCollisions(float dt);
void InitNetWorldPhysics(void);
void ApplyNetPhysics(float dt);
bool IsPlayerGrounded(void);
bool IsPlayerDashing(void);
int GetDashCount(void);
float GetPlayerSpeed(void);

// PBR Rendering
void LoadNetWorldPBR(void);
void UnloadNetWorldPBR(void);
void ApplyNetWorldPBR(Camera3D camera);
void EndNetWorldPBR(void);

// Rendering
void DrawNetWorldScene(void);
void DrawNetTerrain(void);
void DrawNetNodes(void);
void DrawNetPortals(void);
void DrawNetPlayer(void);
void DrawNetEffects(void);
void DrawDataRings(void);
void DrawWireSphere(Vector3 position, float radius, Color color, int rings = 12, int segments = 12);
void DrawNetUI(void);
void DrawNetMinimap(void);
void LoadNetWorldShader(void);
void UnloadNetWorldShader(void);
void ApplyNetWorldShader(void);
void EndNetWorldShader(void);
void DisableNetWorldShader(void);

// World Generation
void GenerateNetWorld(void);
void SpawnNetNode(const std::string& siteId, Vector3 pos, NodeType type);
NetNode* GetNetNode(const std::string& siteId);
void InteractWithNode(const std::string& siteId);

// Commands (called from vnet.cpp)
void NetWorldCommand(const char* args);

// Debug
void DrawNetWorldDebug(void);