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
    g_netWorld.camera.fovy = 60.0f;
    g_netWorld.camera.projection = CAMERA_PERSPECTIVE;

    LoadNetWorldShader();
    LoadNetWorldPBR();  
    
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
    
    // ---- DISABLE CURSOR ----
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
            HandleNetInput();
            UpdateNetPlayer(dt);
            
            for (auto& node : g_netWorld.nodes) {
                node.pulsePhase += dt * 1.5f;
                node.rotationAngle += dt * 0.5f;
            }
            
            UpdateNetSelection();
            
            Vector3 eyePos = Vector3Add(g_netWorld.player.position, Vector3{0.0f, 3.0f, 0.0f});
    
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
                
                // ---- ENSURE SHADER IS DISABLED ---- 👈 ADD THIS
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