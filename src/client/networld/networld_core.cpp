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

// Base FOV used outside of dash and the amount to punch it out to while
// dashing. Smoothed toward its target each frame in UpdateNetWorld().
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
    g_netWorld.showMinimap = true;
    g_netWorld.showNodeLabels = true;
    g_netWorld.selectedNodeIndex = -1;
    g_netWorld.interactionCooldown = 0.0f;
    
    // Initialize player
    g_netWorld.player.position = {0.0f, 2.0f, 0.0f};
    g_netWorld.player.speed = 12.0f;
    g_netWorld.player.jumpVelocity = 8.0f;
    g_netWorld.player.yaw = 0.0f;
    g_netWorld.player.pitch = 0.0f;
    g_netWorld.player.onGround = true;
    
    // Initialize camera
    g_netWorld.camera.position = {0.0f, 8.0f, 12.0f};
    g_netWorld.camera.target = {0.0f, 0.0f, 0.0f};
    g_netWorld.camera.up = {0.0f, 1.0f, 0.0f};
    g_netWorld.camera.fovy = NETWORLD_BASE_FOV;
    g_netWorld.camera.projection = CAMERA_PERSPECTIVE;

    // ==== ASSET LOAD: NETWORLD GPU RESOURCES ====
    // Both of these compile/link shader programs and query uniform
    // locations from GPU-backed Shader objects. They must be paired with
    // UnloadNetWorldShader() / UnloadNetWorldPBR() (see networld_shader.cpp
    // and networld_pbr.cpp) before the render context is torn down, or on
    // hot-reload, to avoid leaking GPU shader handles.
    LoadNetWorldShader();
    LoadNetWorldPBR();
    // ==== END ASSET LOAD ====
    
    printf("[NETWORLD] System initialized.\n");
}

// ============================================================
// ENTER / EXIT (with cursor control)
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
    // Close CLI overlay too
    g_player.cliOpen = false;
    
    if (g_netWorld.nodes.empty()) {
        GenerateNetWorld();
    }
    
    g_netWorld.active = true;
    g_netWorld.state = NetWorldState::ENTERING;
    g_netWorld.stateTimer = 0.0f;
    g_netWorld.transitionGlitchTimer = 0.0f;
    
    g_netWorld.player.position = {0.0f, 3.0f, 0.0f};
    g_netWorld.player.yaw = 0.0f;
    g_netWorld.player.pitch = 0.0f;
    g_netWorld.player.velocity = {0.0f, 0.0f, 0.0f};
    g_netWorld.player.onGround = true;
    g_netWorld.camera.fovy = NETWORLD_BASE_FOV;
    
    // ---- DISABLE CURSOR ----
    // Also puts the cursor into raylib's locked/relative-motion mode, which
    // is what makes GetMouseDelta() usable for FPS-style look in
    // HandleNetInput() (networld_physics.cpp).
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

    // ---- ENABLE CURSOR ----
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
            g_netWorld.transitionGlitchTimer += dt;
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
            HandleNetInput();   // Mouse look (see networld_physics.cpp)
            UpdateNetPlayer(dt);
            
            for (auto& node : g_netWorld.nodes) {
                node.pulsePhase += dt * 1.5f;
                node.rotationAngle += dt * 0.5f;

                // ---- ENEMY PATROL ----
                // GLITCH_BOTs previously never moved after spawning, so
                // they read as static scenery rather than threats. They now
                // orbit their fixed spawnPosition anchor (set once in
                // GenerateNetWorld()) instead of drifting, so the motion is
                // stable and repeatable rather than an accumulating error.
                if (node.type == NodeType::ENEMY && node.active) {
                    const float orbitRadius = 3.0f;
                    const float orbitSpeed = 0.4f; // radians/sec
                    float angle = g_netWorld.time * orbitSpeed + node.pulsePhase;
                    node.position.x = node.spawnPosition.x + cosf(angle) * orbitRadius;
                    node.position.z = node.spawnPosition.z + sinf(angle) * orbitRadius;
                }
            }
            
            UpdateNetSelection();
            
            // ---- HEAD BOB ----
            // Subtle vertical bob synced to the existing walkCycle so
            // grounded movement has some tactile weight instead of a
            // perfectly static eye height.
            float bobOffset = 0.0f;
            if (g_netWorld.player.onGround && g_netWorld.player.moving) {
                bobOffset = sinf(g_netWorld.player.walkCycle * 2.0f) * 0.08f;
            }
            Vector3 eyePos = Vector3Add(g_netWorld.player.position, Vector3{0.0f, 3.0f + bobOffset, 0.0f});
    
            // The look direction uses yaw and pitch from the player
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

            // ---- DASH FOV KICK ----
            // Widens the FOV briefly during a dash for a sense of speed,
            // then eases back to the resting FOV.
            float targetFov = IsPlayerDashing() ? NETWORLD_DASH_FOV : NETWORLD_BASE_FOV;
            g_netWorld.camera.fovy += (targetFov - g_netWorld.camera.fovy) * fminf(dt * 8.0f, 1.0f);
            
            break;
        }
        
        case NetWorldState::EXITING: {
            g_netWorld.stateTimer += dt;
            g_netWorld.transitionGlitchTimer += dt;
            float progress = g_netWorld.stateTimer / g_netWorld.transitionDuration;
            
            g_netWorld.glitchIntensity = progress;
            if (g_netWorld.glitchIntensity > 1.0f) g_netWorld.glitchIntensity = 1.0f;
            
            TriggerGlitch(g_netWorld.glitchIntensity * 0.5f);
            
            if (g_netWorld.stateTimer >= g_netWorld.transitionDuration) {
                g_netWorld.active = false;
                g_netWorld.state = NetWorldState::IDLE;
                g_netWorld.glitchIntensity = 0.0f;
                
                // ---- ENSURE SHADER IS DISABLED ----
                DisableNetWorldShader();
                
                PushCliLog("[NETWORLD] Exited cyberspace.");
            }
            return;
        }
        
        default:
            break;
    }
}

// ============================================================
// UPDATE NET SELECTION
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