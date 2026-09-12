#pragma once
#include "raylib.h"
#include <string>
#include <vector>

#define MAX_DYNAMIC_LIGHTS 12
#define MAX_GLITCH_LAYERS  4
#define MAX_DATA_FIELDS    8

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

enum class DataFieldType {
    ENCRYPTION,     // Cyan. Slows movement, reduces effective scan.
    ICE,            // Amber. Drains shield, accelerates enemies inside.
    NOISE           // Magenta. Raises glitch intensity, corrupts minimap.
};

struct DynamicLight {
    Vector3 position;
    Vector3 color;
    float intensity;
    float radius;
    float orbitSpeed;
    float phase;
    bool active;
};

struct GlitchLayer {
    float intensity;
    float duration;
    float phase;
    int type; // 0=SCANLINE, 1=CHROMATIC, 2=BLOCK, 3=PIXELATE
};

// ============================================================
// DATA FIELD — diegetic haze. A localized zone of visible "data"
// (signal, encryption noise, ICE) that the player can see, enter,
// and be affected by. Deliberately not atmospheric fog: it renders
// as discrete points and glyphs on a lattice, not as a soft haze.
// ============================================================

struct DataField {
    DataFieldType type;
    Vector3 center;
    float radius;
    float intensity;    // 0..1 — scales both visuals and gameplay effects
    Vector3 color;      // primary tint
    float phase;        // animation offset so fields don't sync
    float damageTimer;  // ICE tick pacing
    float pulseTimer;   // scan-ring progress, 0..3
    bool active;
};

// ============================================================
// NETWORLD NODE STRUCT
// ============================================================

struct NetNode {
    NodeType type;
    std::string id;              // Corresponds to VNET site URL
    std::string label;
    Vector3 position;
    Vector3 spawnPosition;       // Anchor point used for patrol/orbit motion so
                                 // moving nodes (e.g. ENEMY) circle a fixed
                                 // origin instead of drifting via integration.
    Vector3 color;               // RGB float for glow
    float radius;
    bool active;
    bool discovered;
    float pulsePhase;
    float rotationAngle;

    bool isCollectible;
    bool isHostile;
    float attackCooldown;
    float scanRevealTimer; 
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
    int health;
    int maxHealth;
    int shield;
    int maxShield;
    float shieldRegenTimer;
    bool isScanning;
    float scanRange;
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
    Vector3 arenaCenter;
    float   arenaRadius;
    
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

    bool showBloom;
    float bloomIntensity;

    DynamicLight dynamicLights[MAX_DYNAMIC_LIGHTS];
    int dynamicLightCount;

    DataField dataFields[MAX_DATA_FIELDS];
    int dataFieldCount;

    float distortionStrength;
    GlitchLayer glitchLayers[MAX_GLITCH_LAYERS];
    int glitchLayerCount;

    // Render targets for post‑processing
    RenderTexture2D sceneRT;        // full resolution
    RenderTexture2D bloomRT;        // half resolution
    RenderTexture2D blurTemp;       // ping-pong
    RenderTexture2D fogRT;          // optional (unused)
    RenderTexture2D compositeRT;

    struct Building {
        int gridWidth;          // number of tiles in X
        int gridDepth;          // number of tiles in Z
        float tileSize;         // world units per tile (e.g., 4.0f)
        std::vector<std::vector<int>> tiles; // 0 = floor, 1 = wall
        std::vector<Rectangle> rooms;        // for node placement (x, y, width, depth)
        std::vector<BoundingBox> wallBoxes;
    } building;

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

// Material + post-process rendering
void LoadNetWorldPBR(void);
void UnloadNetWorldPBR(void);
void ApplyNetWorldPBR(Camera3D camera);
void EndNetWorldPBR(void);
void ApplyDistortion(RenderTexture2D source, RenderTexture2D dest);
void ApplyBloom(RenderTexture2D scene, RenderTexture2D output);

// Rendering
void DrawTransitionOverlay(void);
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
void DrawCyberSkybox(void);
void LoadNetWorldShader(void);
void UnloadNetWorldShader(void);
void ApplyNetWorldShader(void);
void EndNetWorldShader(void);
void DisableNetWorldShader(void);

// Data fields
void  InitDataFields(void);
void  UpdateDataFields(float dt);
void  UpdateDataFieldEffects(float dt);
void  DrawDataFields(void);
float GetDataFieldInfluence(Vector3 position, DataFieldType type);
void  SpawnDataField(Vector3 center, float radius, DataFieldType type);

// World Generation
void GenerateNetWorld(void);
void SpawnNetNode(const std::string& siteId, Vector3 pos, NodeType type);
NetNode* GetNetNode(const std::string& siteId);
void InteractWithNode(const std::string& siteId);

// Commands (called from vnet.cpp)
void NetWorldCommand(const char* args);

// Debug
void DrawNetWorldDebug(void);