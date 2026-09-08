#include "networld.h"
#include "../render.h"
#include "../game.h"
#include "raymath.h"
#include <cmath>

// ============================================================
// PHYSICS CONSTANTS
// ============================================================

// Gravity & Movement
const float GRAVITY = -35.0f;
const float TERMINAL_VELOCITY = -50.0f;
const float PLAYER_SPEED = 14.0f;
const float PLAYER_SPRINT_SPEED = 22.0f;
const float PLAYER_ACCELERATION = 45.0f;
const float PLAYER_DECELERATION = 35.0f;
const float PLAYER_AIR_ACCELERATION = 15.0f;
// FIX: this constant existed but was never actually used anywhere (the air
// deceleration branch below hard-coded PLAYER_AIR_ACCELERATION * 0.5f
// instead), and its old value of 0.85f was far too small to read as a
// per-second deceleration rate on the same scale as the other movement
// constants. Given a real value and now wired into ApplyNetPhysics().
const float PLAYER_AIR_DECELERATION = 6.0f;

// Jumping
const float JUMP_VELOCITY = 10.0f;
const float COYOTE_TIME = 0.08f;
const float JUMP_BUFFER_TIME = 0.08f;
const int MAX_DOUBLE_JUMPS = 1;

// Dashing
const float DASH_SPEED = 35.0f;
const float DASH_DURATION = 0.2f;
const float DASH_COOLDOWN = 0.6f;
const int MAX_DASHES = 1;

// Mouse Look
// Radians of yaw/pitch per pixel of raw mouse delta. Tuned for
// DisableCursor()'s raw relative-motion mode (set in EnterNetWorld()).
const float MOUSE_SENSITIVITY = 0.0015f;
// Clamp pitch just short of +/-90 degrees so the look direction never
// flips past straight up/down (which would invert yaw controls).
const float PITCH_LIMIT = 1.45f;

// ============================================================
// PLAYER PHYSICS STATE
// ============================================================

struct PhysicsState {
    Vector3 velocity;
    bool isGrounded;
    bool wasGrounded;
    float coyoteTimer;
    float jumpBufferTimer;
    int doubleJumpsRemaining;
    bool hasJumped;
    bool isMoving;
    float walkCycle;
    
    // Dashing
    bool isDashing;
    float dashTimer;
    int dashesRemaining;
    float dashCooldownTimer;
    Vector3 dashDirection;
    
    // Input
    Vector3 moveInput;
    bool sprinting;
};

static PhysicsState g_physics = {0};

// ============================================================
// INIT
// ============================================================

void InitNetWorldPhysics(void) {
    memset(&g_physics, 0, sizeof(PhysicsState));
    g_physics.doubleJumpsRemaining = MAX_DOUBLE_JUMPS;
    g_physics.dashesRemaining = MAX_DASHES;
}

// ============================================================
// GROUND CHECK
// ============================================================

static bool CheckGround(Vector3 position, float radius, Vector3& groundNormal) {
    float rayLength = 0.3f + radius * 0.3f;
    float groundY = g_netWorld.terrainHeight + 0.1f;
    
    // Check if player is above ground
    float diff = position.y - groundY;
    if (diff < 0.5f && diff > -rayLength) {
        groundNormal = {0, 1, 0};
        return true;
    }
    
    // Check terrain pillars
    float halfSize = g_netWorld.worldSize / 2.0f;
    float gridSize = g_netWorld.worldSize / g_netWorld.gridSize;
    float height = g_netWorld.terrainHeight;
    
    for (int x = -2; x <= 2; x++) {
        for (int z = -2; z <= 2; z++) {
            float px = floorf(position.x / gridSize) * gridSize + x * gridSize;
            float pz = floorf(position.z / gridSize) * gridSize + z * gridSize;
            
            if (px < -halfSize || px > halfSize || pz < -halfSize || pz > halfSize) continue;
            
            float distX = position.x - px;
            float distZ = position.z - pz;
            float dist = sqrtf(distX * distX + distZ * distZ);
            
            if (dist > gridSize * 0.6f) continue;
            
            float pillarHeight = 0.3f + sinf(px * 0.3f + pz * 0.2f + g_netWorld.time * 0.2f) * 0.2f + 0.3f;
            float pillarY = height + pillarHeight;
            
            float diffY = position.y - pillarY;
            if (diffY < 0.5f && diffY > -rayLength) {
                groundNormal = {0, 1, 0};
                return true;
            }
        }
    }
    
    // Check nodes
    for (const auto& node : g_netWorld.nodes) {
        Vector3 diff = {
            position.x - node.position.x,
            position.y - node.position.y,
            position.z - node.position.z
        };
        float dist = sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
        float minDist = node.radius + radius;
        
        if (dist < minDist && diff.y < 0) {
            groundNormal = {0, 1, 0};
            return true;
        }
    }
    
    groundNormal = {0, 1, 0};
    return false;
}

// ============================================================
// APPLY PHYSICS
// ============================================================

void ApplyNetPhysics(float dt) {
    if (g_netWorld.state != NetWorldState::ACTIVE) return;
    
    NetPlayer& player = g_netWorld.player;
    Vector3& pos = player.position;
    
    // ---- GROUND CHECK ----
    Vector3 groundNormal;
    bool wasGrounded = g_physics.isGrounded;
    g_physics.isGrounded = CheckGround(pos, 0.5f, groundNormal);
    g_physics.wasGrounded = wasGrounded;
    
    // ---- COYOTE TIME ----
    if (g_physics.isGrounded) {
        g_physics.coyoteTimer = COYOTE_TIME;
    } else {
        g_physics.coyoteTimer -= dt;
        if (g_physics.coyoteTimer < 0) g_physics.coyoteTimer = 0;
    }
    
    // ---- JUMP BUFFER ----
    if (g_physics.jumpBufferTimer > 0) {
        g_physics.jumpBufferTimer -= dt;
    }
    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP)) {
        g_physics.jumpBufferTimer = JUMP_BUFFER_TIME;
    }
    
    // ---- GET INPUT (FIXED: W = forward, S = backward) ----
    Vector3 moveInput = {0, 0, 0};
    if (IsKeyDown(KEY_W)) moveInput.z += 1.0f;  // FORWARD
    if (IsKeyDown(KEY_S)) moveInput.z -= 1.0f;  // BACKWARD
    if (IsKeyDown(KEY_A)) moveInput.x -= 1.0f;
    if (IsKeyDown(KEY_D)) moveInput.x += 1.0f;
    
    // Normalize input
    float inputLen = sqrtf(moveInput.x * moveInput.x + moveInput.z * moveInput.z);
    if (inputLen > 1.0f) {
        moveInput.x /= inputLen;
        moveInput.z /= inputLen;
    }
    
    g_physics.isMoving = inputLen > 0.01f;
    g_physics.sprinting = IsKeyDown(KEY_LEFT_SHIFT) && g_physics.isGrounded && g_physics.isMoving;
    
    // ---- TRANSFORM INPUT TO WORLD SPACE ----
    // Forward is the direction the player is looking
    Vector3 forward = {
        -sinf(player.yaw),
        0.0f,
        -cosf(player.yaw)
    };
    Vector3 right = {
        cosf(player.yaw),
        0.0f,
        -sinf(player.yaw)
    };
    
    Vector3 worldInput = {
        forward.x * moveInput.z + right.x * moveInput.x,
        0.0f,
        forward.z * moveInput.z + right.z * moveInput.x
    };
    
    // Normalize world input
    float worldLen = sqrtf(worldInput.x * worldInput.x + worldInput.z * worldInput.z);
    if (worldLen > 0.01f) {
        worldInput.x /= worldLen;
        worldInput.z /= worldLen;
    }
    
    // ---- CALCULATE TARGET SPEED ----
    float targetSpeed = g_physics.sprinting ? PLAYER_SPRINT_SPEED : PLAYER_SPEED;
    
    // ---- APPLY MOVEMENT ----
    Vector3& vel = g_physics.velocity;
    float accel = g_physics.isGrounded ? PLAYER_ACCELERATION : PLAYER_AIR_ACCELERATION;
    
    if (g_physics.isMoving) {
        float targetX = worldInput.x * targetSpeed;
        float targetZ = worldInput.z * targetSpeed;
        vel.x += (targetX - vel.x) * fminf(accel * dt, 1.0f);
        vel.z += (targetZ - vel.z) * fminf(accel * dt, 1.0f);
    } else {
        // FIX: now actually uses PLAYER_AIR_DECELERATION for the airborne
        // case instead of silently reusing PLAYER_AIR_ACCELERATION * 0.5f.
        float decel = g_physics.isGrounded ? PLAYER_DECELERATION : PLAYER_AIR_DECELERATION;
        vel.x *= (1.0f - fminf(decel * dt, 1.0f));
        vel.z *= (1.0f - fminf(decel * dt, 1.0f));
        if (fabsf(vel.x) < 0.1f) vel.x = 0;
        if (fabsf(vel.z) < 0.1f) vel.z = 0;
    }
    
    // ---- GRAVITY ----
    if (!g_physics.isGrounded) {
        vel.y += GRAVITY * dt;
        if (vel.y < TERMINAL_VELOCITY) vel.y = TERMINAL_VELOCITY;
    } else {
        vel.y = 0;
    }
    
    // ---- JUMPING ----
    if (g_physics.jumpBufferTimer > 0 && g_physics.coyoteTimer > 0) {
        vel.y = JUMP_VELOCITY;
        g_physics.hasJumped = true;
        g_physics.coyoteTimer = 0;
        g_physics.jumpBufferTimer = 0;
        g_physics.isGrounded = false;
    }
    
    // ---- DOUBLE JUMP ----
    if (g_physics.jumpBufferTimer > 0 && !g_physics.isGrounded && 
        g_physics.doubleJumpsRemaining > 0 && g_physics.hasJumped) {
        vel.y = JUMP_VELOCITY;
        g_physics.doubleJumpsRemaining--;
        g_physics.jumpBufferTimer = 0;
        TriggerGlitch(0.2f);
    }
    
    // ---- RESET DOUBLE JUMP ----
    if (g_physics.isGrounded && !wasGrounded) {
        g_physics.doubleJumpsRemaining = MAX_DOUBLE_JUMPS;
        g_physics.hasJumped = false;
    }
    
    // ---- DASH ----
    if (IsKeyPressed(KEY_LEFT_CONTROL) && g_physics.dashesRemaining > 0 && !g_physics.isDashing) {
        g_physics.isDashing = true;
        g_physics.dashTimer = DASH_DURATION;
        g_physics.dashesRemaining--;
        
        if (g_physics.isMoving) {
            g_physics.dashDirection = {worldInput.x, 0, worldInput.z};
        } else {
            g_physics.dashDirection = forward;
        }
        
        float len = sqrtf(g_physics.dashDirection.x * g_physics.dashDirection.x + 
                         g_physics.dashDirection.z * g_physics.dashDirection.z);
        if (len > 0.01f) {
            g_physics.dashDirection.x /= len;
            g_physics.dashDirection.z /= len;
        }
        
        vel.x = g_physics.dashDirection.x * DASH_SPEED;
        vel.z = g_physics.dashDirection.z * DASH_SPEED;
        vel.y = 0;
        
        TriggerGlitch(0.3f);
        TriggerJitter(0.2f, 0.15f);
    }
    
    // ---- DASH UPDATE ----
    if (g_physics.isDashing) {
        g_physics.dashTimer -= dt;
        if (g_physics.dashTimer <= 0) {
            g_physics.isDashing = false;
            vel.x *= 0.5f;
            vel.z *= 0.5f;
        }
    }
    
    // ---- DASH COOLDOWN ----
    if (g_physics.dashCooldownTimer > 0) {
        g_physics.dashCooldownTimer -= dt;
        if (g_physics.dashCooldownTimer <= 0) {
            g_physics.dashesRemaining = fminf(g_physics.dashesRemaining + 1, MAX_DASHES);
        }
    }
    if (g_physics.isDashing && g_physics.dashTimer < 0.05f) {
        g_physics.dashCooldownTimer = DASH_COOLDOWN;
    }
    
    // ---- APPLY VELOCITY ----
    pos.x += vel.x * dt;
    pos.y += vel.y * dt;
    pos.z += vel.z * dt;
    
    // ---- GROUND COLLISION ----
    float groundY = g_netWorld.terrainHeight + 0.5f;
    if (pos.y < groundY) {
        pos.y = groundY;
        vel.y = 0;
    }
    
    // ---- NODE COLLISION ----
    for (auto& node : g_netWorld.nodes) {
        Vector3 diff = {
            pos.x - node.position.x,
            0.0f,
            pos.z - node.position.z
        };
        float dist = sqrtf(diff.x * diff.x + diff.z * diff.z);
        float minDist = node.radius + 0.6f;
        
        if (dist < minDist && dist > 0.01f) {
            Vector3 normal = {diff.x / dist, 0, diff.z / dist};
            float overlap = minDist - dist;
            
            pos.x += normal.x * overlap * 1.1f;
            pos.z += normal.z * overlap * 1.1f;
            
            float velDot = vel.x * normal.x + vel.z * normal.z;
            if (velDot < 0) {
                vel.x -= 1.0f * velDot * normal.x;
                vel.z -= 1.0f * velDot * normal.z;
            }
        }
    }

    if (g_physics.isDashing) {
        for (auto& node : g_netWorld.nodes) {
            if (node.type == NodeType::ENEMY && node.active) {
                float d = Vector3Distance(pos, node.position);
                if (d < node.radius + 0.8f) {
                    node.active = false;
                    node.color = {0.2f,0.2f,0.2f};
                    g_player.vcoin += 0.5f;
                    TriggerGlitch(0.5f);
                    // Bounce back
                    vel.x = -vel.x * 0.5f;
                    vel.z = -vel.z * 0.5f;
                    // Give brief invincibility? Not implemented.
                }
            }
        }
    }
    
    // ---- BOUNDARY COLLISION ----
    float halfSize = g_netWorld.worldSize / 2.0f - 0.5f;
    if (pos.x < -halfSize) { pos.x = -halfSize; vel.x = 0; }
    if (pos.x > halfSize) { pos.x = halfSize; vel.x = 0; }
    if (pos.z < -halfSize) { pos.z = -halfSize; vel.z = 0; }
    if (pos.z > halfSize) { pos.z = halfSize; vel.z = 0; }
    
    // ---- WALK CYCLE ----
    float speed = sqrtf(vel.x * vel.x + vel.z * vel.z);
    if (g_physics.isGrounded && speed > 0.5f) {
        g_physics.walkCycle += dt * speed * 0.3f;
    }
    
    // ---- UPDATE PLAYER ----
    player.velocity = vel;
    player.onGround = g_physics.isGrounded;
    player.moving = g_physics.isMoving;
    player.walkCycle = g_physics.walkCycle;
}

// ============================================================
// GET PHYSICS STATE
// ============================================================

bool IsPlayerGrounded(void) {
    return g_physics.isGrounded;
}

bool IsPlayerDashing(void) {
    return g_physics.isDashing;
}

int GetDashCount(void) {
    return g_physics.dashesRemaining;
}

float GetPlayerSpeed(void) {
    return sqrtf(g_physics.velocity.x * g_physics.velocity.x + 
                 g_physics.velocity.z * g_physics.velocity.z +
                 g_physics.velocity.y * g_physics.velocity.y);
}

// ============================================================
// MOUSE LOOK  (previously missing entirely — player.yaw / player.pitch
// were declared and consumed by the camera in networld_core.cpp, but
// nothing ever wrote to them, so the camera could never actually turn)
// ============================================================

void HandleNetInput(void) {
    if (g_netWorld.state != NetWorldState::ACTIVE) return;

    // DisableCursor() is called in EnterNetWorld(), which puts raylib's
    // cursor into locked/relative mode — GetMouseDelta() then reports raw
    // frame-to-frame look movement, which is what we want for an FPS-style
    // camera instead of clamped screen-space cursor position.
    Vector2 mouseDelta = GetMouseDelta();

    NetPlayer& player = g_netWorld.player;
    player.yaw   -= mouseDelta.x * MOUSE_SENSITIVITY;
    player.pitch -= mouseDelta.y * MOUSE_SENSITIVITY;

    // Keep yaw bounded so it doesn't grow without limit over a long session
    const float TWO_PI = 6.28318530718f;
    if (player.yaw > TWO_PI)  player.yaw -= TWO_PI;
    if (player.yaw < -TWO_PI) player.yaw += TWO_PI;

    // Clamp pitch so the look direction can't flip past straight up/down
    if (player.pitch > PITCH_LIMIT)  player.pitch = PITCH_LIMIT;
    if (player.pitch < -PITCH_LIMIT) player.pitch = -PITCH_LIMIT;
}

void UpdateNetPlayer(float dt) {
    ApplyNetPhysics(dt);
}

void CheckNetCollisions(float dt) {
    // Now handled inside ApplyNetPhysics
}