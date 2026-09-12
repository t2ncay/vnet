#include "networld.h"
#include "../render.h"
#include "../../shared/vnet.h"
#include "../../shared/vnet_sites.h"
#include "../game.h"
#include "../desktop/desktop.h"
#include "raymath.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstring>

NetWorld g_netWorld = {0};

static const float NETWORLD_BASE_FOV = 60.0f;
static const float NETWORLD_DASH_FOV = 75.0f;

// ============================================================
// INIT
// ============================================================

void InitNetWorld(void) {
    InitNetWorldPhysics();

    memset(&g_netWorld, 0, sizeof(NetWorld));
    g_netWorld.active = false;
    g_netWorld.state = NetWorldState::IDLE;
    g_netWorld.transitionDuration = 2.0f;
    g_netWorld.worldSize = 100.0f;
    g_netWorld.gridSize = 20;
    g_netWorld.terrainHeight = 0.0f;
    g_netWorld.arenaCenter = {0.0f, 0.0f, 0.0f};  // set properly in GenerateNetWorld
    g_netWorld.arenaRadius = 10.0f;
    g_netWorld.showMinimap = true;
    g_netWorld.showNodeLabels = true;
    g_netWorld.selectedNodeIndex = -1;
    g_netWorld.interactionCooldown = 0.0f;
    g_netWorld.bloomIntensity = 0.3f;
    g_netWorld.dynamicLightCount = 0; // will be set later

    // Player
    g_netWorld.player.position = {0.0f, 2.0f, 0.0f};
    g_netWorld.player.speed = 12.0f;
    g_netWorld.player.jumpVelocity = 8.0f;
    g_netWorld.player.yaw = 0.0f;
    g_netWorld.player.pitch = 0.0f;
    g_netWorld.player.onGround = true;
    g_netWorld.player.health = 100;
    g_netWorld.player.maxHealth = 100;
    g_netWorld.player.shield = 50;
    g_netWorld.player.maxShield = 50;
    g_netWorld.player.shieldRegenTimer = 0.0f;
    g_netWorld.player.isScanning = false;
    g_netWorld.player.scanRange = 30.0f;

    // Camera
    g_netWorld.camera.position = {0.0f, 8.0f, 12.0f};
    g_netWorld.camera.target = {0.0f, 0.0f, 0.0f};
    g_netWorld.camera.up = {0.0f, 1.0f, 0.0f};
    g_netWorld.camera.fovy = NETWORLD_BASE_FOV;
    g_netWorld.camera.projection = CAMERA_PERSPECTIVE;

    // ==== ASSET LOAD: SHADERS ====
    LoadNetWorldShader();
    LoadNetWorldPBR();
    // ==== END ASSET LOAD ====

    // ==== ASSET LOAD: RENDER TEXTURES ====
    int w = GetScreenWidth();
    int h = GetScreenHeight();
    g_netWorld.sceneRT   = LoadRenderTexture(w, h);
    g_netWorld.bloomRT   = LoadRenderTexture(w/2, h/2);
    g_netWorld.blurTemp  = LoadRenderTexture(w/2, h/2);
    g_netWorld.compositeRT = LoadRenderTexture(w, h);
    // g_netWorld.fogRT can be allocated if needed
    // ==== END ASSET LOAD ====

    printf("[NETWORLD] System initialized.\n");
}

// ============================================================
// CLEANUP
// ============================================================

void UnloadNetWorldResources(void) {
    // ==== ASSET UNLOAD: RENDER TEXTURES ====
    UnloadRenderTexture(g_netWorld.sceneRT);
    UnloadRenderTexture(g_netWorld.bloomRT);
    UnloadRenderTexture(g_netWorld.blurTemp);
    UnloadRenderTexture(g_netWorld.compositeRT);
    // ==== END ASSET UNLOAD ====
    UnloadNetWorldShader();
    UnloadNetWorldPBR();
}

// ============================================================
// DYNAMIC LIGHTS
// ============================================================

void InitDynamicLights(void) {
    g_netWorld.dynamicLightCount = 8;
    for (int i = 0; i < g_netWorld.dynamicLightCount; i++) {
        DynamicLight& l = g_netWorld.dynamicLights[i];
        l.active = true;
        l.color = {(i % 3 == 0) ? 1.0f : 0.2f, 
                   (i % 3 == 1) ? 1.0f : 0.2f, 
                   (i % 3 == 2) ? 1.0f : 0.2f};
        l.intensity = 20.0f + i * 5.0f;
        l.radius = 15.0f + i * 4.0f;
        l.orbitSpeed = 0.2f + i * 0.08f;
        l.phase = i * 1.5f;
    }
}

void UpdateDynamicLights(float dt) {
    float t = g_netWorld.time;
    for (int i = 0; i < g_netWorld.dynamicLightCount; i++) {
        DynamicLight& l = g_netWorld.dynamicLights[i];
        if (!l.active) continue;
        float angle = t * l.orbitSpeed + l.phase;
        l.position.x = cosf(angle) * l.radius;
        l.position.z = sinf(angle) * l.radius;
        l.position.y = 5.0f + sinf(t * 0.5f + l.phase) * 2.0f;
    }
}

// ============================================================
// ENTER / EXIT
// ============================================================

void EnterNetWorld(void) {
    if (g_netWorld.active) return;
    if (g_netWorld.state == NetWorldState::ENTERING) return;

    auto& desktop = GetDesktop();
    for (int i = 0; i < (int)desktop.m_windows.size(); i++) {
        if (desktop.m_windows[i].type == AppType::Terminal) {
            desktop.CloseWindow(i);
            break;
        }
    }
    g_player.cliOpen = false;

    if (g_netWorld.nodes.empty()) {
        GenerateNetWorld();
    }
    // Initialize dynamic lights if not done yet
    if (g_netWorld.dynamicLightCount == 0) {
        InitDynamicLights();
    }

    g_netWorld.active = true;
    g_netWorld.state = NetWorldState::ENTERING;
    g_netWorld.stateTimer = 0.0f;
    g_netWorld.transitionGlitchTimer = 0.0f;
    g_netWorld.player.position = {
        g_netWorld.arenaCenter.x,
        0.5f,
        g_netWorld.arenaCenter.z + g_netWorld.arenaRadius + 3.0f
    };
    g_netWorld.player.yaw = 0.0f;   // forward = (0, 0, -1) = north
    g_netWorld.player.pitch = -0.05f;  // slight downward tilt to frame the dais
    g_netWorld.player.velocity = {0.0f, 0.0f, 0.0f};
    g_netWorld.player.onGround = true;
    g_netWorld.camera.fovy = NETWORLD_BASE_FOV;
    g_netWorld.player.health = g_netWorld.player.maxHealth;
    g_netWorld.player.shield = g_netWorld.player.maxShield;

    DisableCursor();

    TriggerJitter(0.6f, 1.0f);
    TriggerGlitch(0.8f);

    PushCliLog("[NETWORLD] Entering cyberspace...");
    GetDesktop().PushHellroomMessage("[NETWORLD] User jacked into cyberspace!");
}

void ExitNetWorld(void) {
    if (!g_netWorld.active) return;
    if (g_netWorld.state == NetWorldState::EXITING) return;

    g_netWorld.state = NetWorldState::EXITING;
    g_netWorld.stateTimer = 0.0f;
    g_netWorld.transitionGlitchTimer = 0.0f;

    DisableNetWorldShader();
    EnableCursor();

    TriggerJitter(0.6f, 1.0f);
    TriggerGlitch(0.8f);

    PushCliLog("[NETWORLD] Leaving cyberspace...");
}

bool IsInNetWorld(void) {
    return g_netWorld.active && g_netWorld.state != NetWorldState::IDLE;
}

// ============================================================
// UPDATE
// ============================================================

void UpdateNetWorld(float dt) {
    if (!g_netWorld.active) return;

    g_netWorld.time += dt;
    g_netWorld.scanlineOffset += dt * 2.0f;
    g_netWorld.interactionCooldown -= dt;
    if (g_netWorld.interactionCooldown < 0.0f) g_netWorld.interactionCooldown = 0.0f;

    switch (g_netWorld.state) {
        case NetWorldState::ENTERING: {
            g_netWorld.stateTimer += dt;
            float progress = g_netWorld.stateTimer / g_netWorld.transitionDuration;
            g_netWorld.glitchIntensity = 1.0f - progress;
            if (g_netWorld.glitchIntensity < 0.0f) g_netWorld.glitchIntensity = 0.0f;
            TriggerGlitch(g_netWorld.glitchIntensity * 0.5f);
            if (g_netWorld.stateTimer >= g_netWorld.transitionDuration) {
                g_netWorld.state = NetWorldState::ACTIVE;
                g_netWorld.glitchIntensity = 0.0f;
                PushCliLog("[NETWORLD] Cyberspace active.");
            }
            return;
        }

        case NetWorldState::ACTIVE: {
            // Update dynamic lights
            UpdateDynamicLights(dt);

            HandleNetInput();
            UpdateNetPlayer(dt);

            // Update nodes and enemy AI
            for (auto& node : g_netWorld.nodes) {
                node.pulsePhase += dt * 1.5f;
                node.rotationAngle += dt * 0.5f;
                if (node.scanRevealTimer > 0) node.scanRevealTimer -= dt;

                if (node.type == NodeType::ENEMY && node.active && node.isHostile) {
                    Vector3 toPlayer = Vector3Subtract(g_netWorld.player.position, node.position);
                    float dist = Vector3Length(toPlayer);
                    if (dist > 20.0f) {
                        float angle = g_netWorld.time * 0.4f + node.pulsePhase;
                        node.position.x = node.spawnPosition.x + cosf(angle) * 3.0f;
                        node.position.z = node.spawnPosition.z + sinf(angle) * 3.0f;
                    } else if (dist > 1.5f) {
                        Vector3 dir = Vector3Normalize(toPlayer);
                        node.position.x += dir.x * 2.5f * dt;
                        node.position.z += dir.z * 2.5f * dt;
                    } else {
                        g_netWorld.player.health -= 10 * dt;
                        if (g_netWorld.player.health < 0) g_netWorld.player.health = 0;
                        TriggerGlitch(0.3f);
                    }
                }
            }

            UpdateDataFields(dt);
            UpdateDataFieldEffects(dt);

            UpdateNetSelection();

            // Scan mode
            if (IsKeyPressed(KEY_E)) {
                g_netWorld.player.isScanning = !g_netWorld.player.isScanning;
            }
            if (g_netWorld.player.isScanning) {
                for (auto& node : g_netWorld.nodes) {
                    float d = Vector3Distance(g_netWorld.player.position, node.position);
                    if (d < g_netWorld.player.scanRange) {
                        node.scanRevealTimer = 0.5f;
                    }
                }
            }

            // Shield regeneration
            g_netWorld.player.shieldRegenTimer += dt;
            if (g_netWorld.player.shieldRegenTimer > 2.0f && g_netWorld.player.shield < g_netWorld.player.maxShield) {
                g_netWorld.player.shield += (int)(5 * dt);
                if (g_netWorld.player.shield > g_netWorld.player.maxShield)
                    g_netWorld.player.shield = g_netWorld.player.maxShield;
            }

            // Head bob
            float bobOffset = 0.0f;
            if (g_netWorld.player.onGround && g_netWorld.player.moving) {
                bobOffset = sinf(g_netWorld.player.walkCycle * 2.0f) * 0.08f;
            }
            Vector3 eyePos = Vector3Add(g_netWorld.player.position, Vector3{0.0f, 3.0f + bobOffset, 0.0f});

            Vector3 lookDir = {
                -sinf(g_netWorld.player.yaw) * cosf(g_netWorld.player.pitch),
                -sinf(g_netWorld.player.pitch),
                -cosf(g_netWorld.player.yaw) * cosf(g_netWorld.player.pitch)
            };
            Vector3 targetOffset = Vector3Scale(lookDir, 10.0f);
            Vector3 targetPos = Vector3Add(eyePos, targetOffset);

            g_netWorld.camera.position = eyePos;
            g_netWorld.camera.target = targetPos;
            g_netWorld.camera.up = {0.0f, 1.0f, 0.0f};

            float targetFov = IsPlayerDashing() ? NETWORLD_DASH_FOV : NETWORLD_BASE_FOV;
            g_netWorld.camera.fovy += (targetFov - g_netWorld.camera.fovy) * fminf(dt * 8.0f, 1.0f);

            break;
        }

        case NetWorldState::EXITING: {
            g_netWorld.stateTimer += dt;
            float progress = g_netWorld.stateTimer / g_netWorld.transitionDuration;
            g_netWorld.glitchIntensity = progress;
            if (g_netWorld.glitchIntensity > 1.0f) g_netWorld.glitchIntensity = 1.0f;
            TriggerGlitch(g_netWorld.glitchIntensity * 0.5f);
            if (g_netWorld.stateTimer >= g_netWorld.transitionDuration) {
                g_netWorld.active = false;
                g_netWorld.state = NetWorldState::IDLE;
                g_netWorld.glitchIntensity = 0.0f;
                DisableNetWorldShader();
                PushCliLog("[NETWORLD] Exited cyberspace.");
            }
            return;
        }

        default: break;
    }
}

// ============================================================
// SELECTION (unchanged)
// ============================================================

void UpdateNetSelection(void) {
    Vector2 mousePos = GetMousePosition();
    Ray ray = GetMouseRay(mousePos, g_netWorld.camera);

    g_netWorld.selectedNodeIndex = -1;
    float closestDist = 1000.0f;

    for (int i = 0; i < (int)g_netWorld.nodes.size(); i++) {
        const auto& node = g_netWorld.nodes[i];
        BoundingBox box = {
            {node.position.x - node.radius, node.position.y - node.radius, node.position.z - node.radius},
            {node.position.x + node.radius, node.position.y + node.radius, node.position.z + node.radius}
        };
        RayCollision hit = GetRayCollisionBox(ray, box);
        if (hit.hit && hit.distance < closestDist) {
            closestDist = hit.distance;
            g_netWorld.selectedNodeIndex = i;
        }
    }

    if (g_netWorld.selectedNodeIndex >= 0 && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        g_netWorld.interactionCooldown <= 0.0f) {
        const auto& node = g_netWorld.nodes[g_netWorld.selectedNodeIndex];
        InteractWithNode(node.id);
        g_netWorld.interactionCooldown = 0.5f;
    }
}