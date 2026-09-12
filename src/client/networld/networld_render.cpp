#include "networld.h"
#include "../render.h"
#include "../../shared/vnet.h"
#include "raymath.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstring>

static void DrawGlitchCubes3D(void); 
static void DrawVoidPlane(void);
static void DrawVoidHorizon(void);
static void DrawPylons(void);
static void DrawArenaHub(void);

// ============================================================
// GLITCHED CUBE STRUCT
// ============================================================
struct GlitchCube {
    Vector3 position;
    Vector3 size;
    Color color;
    float rotationSpeed;
    float rotationAngle;
    float glitchTimer;
    float glitchOffset;
    bool active;
};

static GlitchCube g_glitchCubes[25];
static bool g_glitchCubesInit = false;

// ============================================================
// FLOATING PARTICLE / DATA-STREAM STRUCTS
// ------------------------------------------------------------
// FIX: DrawNetEffects() previously called rand() to pick a brand-new random
// x/z position for every particle and every data-stream glyph on *every
// single frame*. That meant nothing ever actually animated smoothly -
// each of the 200+ particles and 60 glyphs teleported to a new random
// spot 60 times a second, reading as flickering static rather than a
// coherent floating field. The fix follows the same "generate once, then
// animate with sin/cos offsets from a stored base position" pattern the
// GlitchCube system above already uses correctly.
// ============================================================
struct FloatingParticle {
    Vector3 basePos;
    float speed;
    float phase;
    bool useCyan;
};

struct DataStreamGlyph {
    Vector3 basePos;
    float speed;
    float phase;
    char character;
    bool useCyan;
};

static const int FLOATING_PARTICLE_COUNT = 200;
static const int DATA_STREAM_GLYPH_COUNT = 60;

static FloatingParticle g_floatingParticles[FLOATING_PARTICLE_COUNT];
static DataStreamGlyph g_dataStreamGlyphs[DATA_STREAM_GLYPH_COUNT];
static bool g_netEffectsInit = false;

// ============================================================
// INIT GLITCH CUBES
// ============================================================
static void InitGlitchCubes(void) {
    float halfSize = g_netWorld.worldSize / 2.0f;
    for (int i = 0; i < 25; i++) {
        float x = -halfSize + (rand() % 1000) / 1000.0f * g_netWorld.worldSize;
        float z = -halfSize + (rand() % 1000) / 1000.0f * g_netWorld.worldSize;
        float y = 0.5f + (rand() % 1000) / 1000.0f * 4.0f;
        
        g_glitchCubes[i].position = {x, y, z};
        g_glitchCubes[i].size = {
            0.5f + (rand() % 1000) / 1000.0f * 2.0f,
            0.5f + (rand() % 1000) / 1000.0f * 2.0f,
            0.5f + (rand() % 1000) / 1000.0f * 2.0f
        };
        g_glitchCubes[i].color = {
            (unsigned char)(50 + rand() % 200),
            (unsigned char)(50 + rand() % 200),
            (unsigned char)(50 + rand() % 200),
            255
        };
        g_glitchCubes[i].rotationSpeed = 0.5f + (rand() % 1000) / 1000.0f * 2.0f;
        g_glitchCubes[i].rotationAngle = (rand() % 1000) / 1000.0f * 6.28318f;
        g_glitchCubes[i].glitchTimer = (rand() % 1000) / 1000.0f * 10.0f;
        g_glitchCubes[i].glitchOffset = (rand() % 1000) / 1000.0f * 6.28318f;
        g_glitchCubes[i].active = true;
    }
    g_glitchCubesInit = true;
}

// ============================================================
// INIT FLOATING PARTICLES / DATA STREAM GLYPHS (once)
// ============================================================
static void InitNetEffects(void) {
    float halfSize = g_netWorld.worldSize / 2.0f;
    float size = g_netWorld.worldSize;

    for (int i = 0; i < DATA_STREAM_GLYPH_COUNT; i++) {
        float x = -halfSize + (rand() % 1000) / 1000.0f * size;
        float z = -halfSize + (rand() % 1000) / 1000.0f * size;
        float y = 1.0f + (rand() % 1000) / 1000.0f * 6.0f;

        DataStreamGlyph& g = g_dataStreamGlyphs[i];
        g.basePos = {x, y, z};
        g.speed = 0.5f + (rand() % 1000) / 1000.0f * 2.0f;
        g.phase = (rand() % 1000) / 1000.0f * 2.0f * PI;
        const char* hexChars = "0123456789ABCDEF";
        g.character = hexChars[rand() % 16];
        g.useCyan = (i % 3 == 0);
    }

    for (int i = 0; i < FLOATING_PARTICLE_COUNT; i++) {
        float x = -halfSize + (rand() % 1000) / 1000.0f * size;
        float z = -halfSize + (rand() % 1000) / 1000.0f * size;
        float y = 0.5f + (rand() % 1000) / 1000.0f * 8.0f;

        FloatingParticle& p = g_floatingParticles[i];
        p.basePos = {x, y, z};
        p.speed = 0.05f + (rand() % 1000) / 1000.0f * 0.3f;
        p.phase = (rand() % 1000) / 1000.0f * 2.0f * PI;
        p.useCyan = (i % 2 == 0);
    }

    g_netEffectsInit = true;
}

// ============================================================
// HELPERS FOR 3D TEXT
// ============================================================
static void DrawScaledText3D(const char* text, Vector3 position, float fontSize, Color color) {
    Vector2 screenPos = GetWorldToScreen(position, g_netWorld.camera);
    DrawScaledText(text, (int)screenPos.x, (int)screenPos.y, (int)fontSize, color);
}

// ============================================================
// DRAW GLITCHED CUBE (wireframe with glitch effects)
// ============================================================
static void DrawGlitchedCube(const GlitchCube& cube, float time) {
    float glitch = sinf(time * 8.0f + cube.glitchOffset) * 0.5f + 0.5f;
    float glitchIntensity = 0.1f + glitch * 0.4f;
    
    Vector3 pos = cube.position;
    Vector3 size = cube.size;
    
    // Glitch displacement
    if (glitch > 0.7f) {
        pos.x += sinf(time * 15.0f + cube.glitchOffset) * 0.3f;
        pos.z += cosf(time * 12.0f + cube.glitchOffset * 1.3f) * 0.3f;
        size.x *= 1.0f + sinf(time * 20.0f) * 0.2f;
        size.z *= 1.0f + cosf(time * 18.0f) * 0.2f;
    }
    
    // Color glitch
    Color col = cube.color;
    if (glitch > 0.8f) {
        col = (rand() % 2 == 0) ? COLOR_BLOOD : COLOR_CYAN;
        col.a = 200;
    }
    
    // Draw wireframe cube
    DrawCubeWiresV(pos, size, Fade(col, 0.3f + glitchIntensity * 0.5f));
    
    // Glitch overlay (random bars)
    if (glitch > 0.6f) {
        float barX = pos.x - size.x / 2 + (rand() % 1000) / 1000.0f * size.x;
        float barZ = pos.z - size.z / 2 + (rand() % 1000) / 1000.0f * size.z;
        DrawLine3D(
            {barX, pos.y - size.y / 2, barZ},
            {barX, pos.y + size.y / 2, barZ},
            Fade(COLOR_BLOOD, 0.5f * glitch)
        );
    }
}

// ============================================================
// DRAW NETWORLD
// ============================================================

void DrawNetWorld(void) {
    if (!g_netWorld.active) return;

    if (!g_glitchCubesInit) InitGlitchCubes();
    if (!g_netEffectsInit) InitNetEffects();

    if (g_netWorld.state == NetWorldState::ENTERING ||
        g_netWorld.state == NetWorldState::EXITING) {
        DrawTransitionOverlay();
        return;
    }

    // =======================================================================
    // PASS 1 — 3D scene -> sceneRT (full resolution)
    // =======================================================================
    BeginTextureMode(g_netWorld.sceneRT);
        ClearBackground(BLACK);
        ApplyNetWorldPBR(g_netWorld.camera);   // sets uniforms, begins shader
        BeginMode3D(g_netWorld.camera);
            DrawCyberSkybox();
            DrawNetTerrain();
            DrawDataFields(); 
            DrawGlitchCubes3D();
            DrawNetNodes();
            DrawNetPortals();
            DrawNetPlayer();
            DrawNetEffects();
            DrawDataRings();
        EndMode3D();
        EndNetWorldPBR();                      // ends material shader
    EndTextureMode();

    // =======================================================================
    // PASS 2 — Bloom (sceneRT + blurred bloom -> compositeRT)
    // =======================================================================
    ApplyBloom(g_netWorld.sceneRT, g_netWorld.compositeRT);

    // =======================================================================
    // PASS 3 — Distortion / glitch (compositeRT -> sceneRT)
    //
    // sceneRT is free to reuse here: ApplyBloom already consumed it as its
    // source and won't touch it again. Reusing it saves one full-res RT.
    // =======================================================================
    ApplyDistortion(g_netWorld.compositeRT, g_netWorld.sceneRT);

    // =======================================================================
    // PASS 4 — CRT to screen (sceneRT -> default framebuffer)
    //
    // ApplyNetWorldShader begins shader mode and sets uniforms; the blit
    // happens between Apply and End so the render code, not the shader
    // system, controls what's being sampled.
    // =======================================================================
    ApplyNetWorldShader();
        Rectangle src = {0, 0, (float)g_netWorld.sceneRT.texture.width,
                              -(float)g_netWorld.sceneRT.texture.height};
        Rectangle dst = {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()};
        DrawTexturePro(g_netWorld.sceneRT.texture, src, dst, {0, 0}, 0.0f, WHITE);
    EndNetWorldShader();

    // 2D UI is drawn AFTER the CRT pass so it stays readable. Move this
    // inside the Apply/End bracket if you want the UI to get scanlines too.
    DrawNetUI();
}

// ============================================================
// DRAW GLITCH CUBES (3D)
// ============================================================
static void DrawGlitchCubes3D(void) {
    float t = g_netWorld.time;
    for (int i = 0; i < 25; i++) {
        if (!g_glitchCubes[i].active) continue;
        
        // Update rotation
        g_glitchCubes[i].rotationAngle += g_glitchCubes[i].rotationSpeed * 1.0f / 60.0f;
        g_glitchCubes[i].glitchTimer += 1.0f / 60.0f;
        
        // Move cubes slightly
        float floatOffset = sinf(t * 0.5f + i * 0.7f) * 0.5f;
        g_glitchCubes[i].position.y += floatOffset * 0.01f;
        if (g_glitchCubes[i].position.y > 5.0f) g_glitchCubes[i].position.y = 0.5f;
        if (g_glitchCubes[i].position.y < 0.5f) g_glitchCubes[i].position.y = 5.0f;
        
        DrawGlitchedCube(g_glitchCubes[i], g_glitchCubes[i].glitchTimer);
    }
}

// ============================================================
// DRAW TERRAIN (ENHANCED)
// ============================================================

void DrawNetTerrain(void) {
    // --- Distant void scenery first, so the depth buffer sits behind it ---
    DrawVoidPlane();
    DrawVoidHorizon();
    DrawPylons();

    // --- Floor grid, using the BUILDING's tile size ---
    // The previous version used a derived gridSize that didn't match
    // building.tileSize, so the floor lines didn't align with walls.
    const float TILE = g_netWorld.building.tileSize;
    const int   GW   = g_netWorld.building.gridWidth;
    const int   GD   = g_netWorld.building.gridDepth;
    const float t    = g_netWorld.time;
    const float pulse = sinf(t * 1.5f) * 0.3f + 0.7f;

    for (int x = 0; x <= GW; x++) {
        float xPos = x * TILE;
        Color col = (x % 2 == 0) ? Fade(COLOR_CYAN, 0.15f * pulse)
                                 : Fade(COLOR_TOXIC, 0.10f * pulse);
        DrawLine3D({xPos, 0.0f, 0.0f}, {xPos, 0.0f, GD * TILE}, col);
    }
    for (int z = 0; z <= GD; z++) {
        float zPos = z * TILE;
        Color col = (z % 2 == 0) ? Fade(COLOR_CYAN, 0.15f * pulse)
                                 : Fade(COLOR_TOXIC, 0.10f * pulse);
        DrawLine3D({0.0f, 0.0f, zPos}, {GW * TILE, 0.0f, zPos}, col);
    }

    // --- Wall tiles as glowing slabs ---
    // The tile grid has been generated since day one and rendered by
    // nothing. This is the visible half of the same data that already
    // drives node placement and (once collision is wired up) walls.
    const float WALL_H = 3.0f;
    const auto& tiles = g_netWorld.building.tiles;

    for (int x = 0; x < GW; x++) {
        for (int z = 0; z < GD; z++) {
            if (tiles[x][z] != 1) continue;

            Vector3 center = {x * TILE + TILE * 0.5f, 0.0f, z * TILE + TILE * 0.5f};
            Vector3 size   = {TILE * 0.98f, WALL_H, TILE * 0.98f};

            // Body. DrawCubeWiresV centres its box at the given position,
            // so lift by half-height to sit the slab on the floor.
            DrawCubeWiresV(
                {center.x, center.y + WALL_H * 0.5f, center.z},
                size, Fade(COLOR_CYAN, 0.20f)
            );

            // Top cap as a thin filled quad. Reads as a "signal surface"
            // on top of every wall rather than as a bare wireframe.
            DrawCubeV(
                {center.x, center.y + WALL_H, center.z},
                {TILE * 0.98f, 0.02f, TILE * 0.98f},
                Fade(COLOR_TOXIC, 0.06f)
            );
        }
    }

    // --- World boundary ---
    // Drawn at the physical edge of the building rather than at worldSize.
    Color boundaryCol = Fade(COLOR_BLOOD, 0.20f * pulse);
    float bx = GW * TILE;
    float bz = GD * TILE;
    DrawLine3D({0.0f, 0.0f, 0.0f}, { bx, 0.0f, 0.0f}, boundaryCol);
    DrawLine3D({ bx,   0.0f, 0.0f}, { bx, 0.0f,  bz}, boundaryCol);
    DrawLine3D({ bx,   0.0f, bz},   {0.0f, 0.0f, bz}, boundaryCol);
    DrawLine3D({0.0f, 0.0f, bz},    {0.0f, 0.0f, 0.0f}, boundaryCol);

    // --- Arena hub ---
    DrawArenaHub();
}

// ============================================================
// DRAW NODES (ENHANCED)
// ============================================================

void DrawNetNodes(void) {
    float t = g_netWorld.time;
    
    for (const auto& node : g_netWorld.nodes) {
        float pulse = sinf(node.pulsePhase) * 0.3f + 0.7f;
        float radius = node.radius * (0.7f + pulse * 0.3f);
        Vector3 pos = node.position;
        pos.y += sinf(node.pulsePhase * 2.0f) * 0.3f;
        
        Color col = {
            (unsigned char)(node.color.x * 255),
            (unsigned char)(node.color.y * 255),
            (unsigned char)(node.color.z * 255),
            255
        };
        
        // ---- WIREFRAME SPHERE ----
        DrawWireSphere(pos, radius, col, 8, 10);
        
        // ---- OUTER GLOW RINGS ----
        for (int ring = 0; ring < 2; ring++) {
            float ringRadius = radius * (1.2f + ring * 0.4f);
            float ringAlpha = 0.05f / (ring + 1) * pulse;
            float ringAngle = node.rotationAngle + t * (0.5f + ring * 0.3f);
            DrawCircle3D(pos, ringRadius, {0, 1, 0}, ringAngle, Fade(COLOR_CYAN, ringAlpha));
        }
        
        // ---- ROTATING DATA RING ----
        DrawCircle3D(pos, radius * 1.1f, {0, 1, 0}, node.rotationAngle + t * 0.7f, Fade(COLOR_CYAN, 0.15f * pulse));
        
        // ---- LABEL ----
        if (g_netWorld.showNodeLabels) {
            char label[64];
            snprintf(label, sizeof(label), "◈ %s", node.label.c_str());
            Vector3 labelPos = {pos.x, pos.y + radius * 2.6f, pos.z};
            DrawScaledText3D(label, labelPos, 0.6f, Fade(COLOR_GHOST, 0.5f + 0.2f * pulse));
            
            DrawLine3D({pos.x, pos.y + radius * 1.8f, pos.z}, 
                      {pos.x, pos.y + radius * 2.3f, pos.z}, 
                      Fade(COLOR_CYAN, 0.1f * pulse));
        }
        
        // ---- SELECTION HIGHLIGHT ----
        if (g_netWorld.selectedNodeIndex >= 0 && 
            g_netWorld.nodes[g_netWorld.selectedNodeIndex].id == node.id) {
            
            float selPulse = sinf(t * 4.0f) * 0.3f + 0.7f;
            Color selCol = {255, 255, 255, (unsigned char)(selPulse * 150 + 55)};
            DrawCircle3D(pos, radius * 2.0f, {0, 1, 0}, t * 2.0f, Fade(selCol, 0.2f * selPulse));
            DrawCircle3D(pos, radius * 2.0f, {1, 0, 0}, -t * 2.0f, Fade(selCol, 0.15f * selPulse));
        }

        if (node.scanRevealTimer > 0) {
            DrawCircle3D(pos, radius * 2.5f, {0,1,0}, t*2, Fade(GREEN, 0.3f));
        }
    }
}

// ============================================================
// DRAW PORTALS (ENHANCED)
// ============================================================

void DrawNetPortals(void) {
    float t = g_netWorld.time;
    
    for (const auto& node : g_netWorld.nodes) {
        if (node.type != NodeType::PORTAL) continue;
        
        float pulse = sinf(node.pulsePhase) * 0.3f + 0.7f;
        Vector3 pos = node.position;
        pos.y += 0.5f;
        
        float radius = node.radius * 1.8f;
        Color col = {
            (unsigned char)(node.color.x * 255),
            (unsigned char)(node.color.y * 255),
            (unsigned char)(node.color.z * 255),
            255
        };
        
        // ---- WIREFRAME SPHERE CORE ----
        DrawWireSphere(pos, radius * 0.5f, col, 6, 8);
        
        // ---- PORTAL RINGS ----
        for (int r = 0; r < 4; r++) {
            float ringRadius = radius * (0.5f + r * 0.2f);
            float ringAlpha = 0.15f * pulse / (r + 1);
            float ringAngle = t * (0.5f + r * 0.4f);
            DrawCircle3D(pos, ringRadius, {0, 1, 0}, ringAngle, Fade(col, ringAlpha));
        }
        
        // ---- VERTICAL RINGS ----
        DrawCircle3D(pos, radius * 0.7f, {1, 0, 0}, t * 0.8f, Fade(COLOR_CYAN, 0.15f * pulse));
        DrawCircle3D(pos, radius * 0.7f, {0, 0, 1}, t * 0.6f, Fade(COLOR_AMBER, 0.1f * pulse));
        
        // ---- CORE GLOW ----
        DrawSphere(pos, radius * 0.2f, Fade(col, 0.3f * pulse));
        DrawSphere(pos, radius * 0.1f, Fade(COLOR_AMBER, 0.2f * pulse));
        
        // ---- SPIRAL PARTICLES ----
        int particleCount = 40;
        for (int i = 0; i < particleCount; i++) {
            float angle = t * 3.0f + i * (2.0f * PI / particleCount);
            float r = radius * (0.2f + 0.7f * sinf(t * 1.5f + i * 0.3f));
            Vector3 p = {
                pos.x + cosf(angle) * r,
                pos.y + sinf(t * 2.5f + i * 0.5f) * 0.8f,
                pos.z + sinf(angle) * r
            };
            
            float alpha = 0.2f + 0.6f * (1.0f - (i / (float)particleCount));
            Color pCol = (i % 2 == 0) ? col : COLOR_CYAN;
            DrawSphere(p, 0.05f, Fade(pCol, alpha * 0.6f * pulse));
        }
        
        // ---- LABEL ----
        char label[64];
        snprintf(label, sizeof(label), "⏣ PORTAL: %s", node.label.c_str());
        Vector3 labelPos = {pos.x, pos.y + radius * 2.2f, pos.z};
        DrawScaledText3D(label, labelPos, 0.5f, Fade(COLOR_AMBER, 0.5f + 0.3f * pulse));
    }
}

// ============================================================
// DRAW PLAYER (ENHANCED)
// ============================================================

void DrawNetPlayer(void) {
    Vector3 pos = g_netWorld.player.position;
    float t = g_netWorld.time;
    float pulse = sinf(t * 2.0f) * 0.3f + 0.7f;
    
    // ---- WIREFRAME PLAYER ----
    DrawWireSphere(pos, 0.8f, COLOR_TOXIC, 6, 8);
    
    // ---- DIRECTION INDICATOR ----
    Vector3 dir = {
        -sinf(g_netWorld.player.yaw),
        0.0f,
        -cosf(g_netWorld.player.yaw)
    };
    DrawLine3D(pos, {pos.x + dir.x * 2.5f, pos.y, pos.z + dir.z * 2.5f}, 
               Fade(COLOR_TOXIC, 0.7f * pulse));
    
    // ---- GLOW ----
    DrawSphere(pos, 1.5f, Fade(COLOR_TOXIC, 0.05f * pulse));
    DrawSphere(pos, 0.8f, Fade(COLOR_TOXIC, 0.1f * pulse));
    
    // ---- TRAIL PARTICLES ----
    static Vector3 trailPositions[15] = {};
    static float trailTimer = 0.0f;
    
    trailTimer += 1.0f / 60.0f;
    if (trailTimer > 0.04f && g_netWorld.player.moving) {
        trailTimer = 0.0f;
        for (int i = 14; i > 0; i--) {
            trailPositions[i] = trailPositions[i - 1];
        }
        trailPositions[0] = pos;
    }
    
    for (int i = 0; i < 15; i++) {
        if (trailPositions[i].x == 0.0f && trailPositions[i].y == 0.0f && trailPositions[i].z == 0.0f) continue;
        float alpha = 0.15f * (1.0f - i / 15.0f) * pulse;
        DrawSphere(trailPositions[i], 0.08f, Fade(COLOR_TOXIC, alpha));
    }
}

// ============================================================
// DRAW EFFECTS (ENHANCED)
// ============================================================

void DrawNetEffects(void) {
    float t = g_netWorld.time;
    float pulse = sinf(t * 1.0f) * 0.3f + 0.7f;
    
    // ---- DATA STREAMS (falling hex) ----
    // Each glyph now animates around its own fixed basePos instead of
    // being re-rolled with rand() every frame (see InitNetEffects()).
    for (int i = 0; i < DATA_STREAM_GLYPH_COUNT; i++) {
        const DataStreamGlyph& glyph = g_dataStreamGlyphs[i];
        float yOffset = sinf(t * glyph.speed + glyph.phase) * 2.0f;
        Vector3 pos = {glyph.basePos.x, glyph.basePos.y + yOffset, glyph.basePos.z};
        
        char str[2] = {glyph.character, '\0'};
        
        float alpha = 0.08f + 0.12f * sinf(t * 1.5f + i * 0.5f);
        Color col = glyph.useCyan ? COLOR_CYAN : COLOR_TOXIC;
        DrawScaledText3D(str, pos, 0.4f + 0.2f * pulse, Fade(col, alpha));
        
        // Glow trail
        if (i % 2 == 0) {
            DrawLine3D(
                {pos.x, pos.y - 1.0f, pos.z},
                {pos.x, pos.y + 1.0f, pos.z},
                Fade(col, alpha * 0.3f)
            );
        }
    }
    
    // ---- FLOATING PARTICLES ----
    // Same fix: drift around a fixed basePos with sin/cos offsets instead
    // of teleporting to a new random position each frame.
    for (int i = 0; i < FLOATING_PARTICLE_COUNT; i++) {
        const FloatingParticle& particle = g_floatingParticles[i];
        float xOffset = sinf(t * particle.speed + particle.phase) * 1.5f;
        float zOffset = cosf(t * particle.speed * 0.7f + particle.phase * 1.3f) * 1.5f;
        
        Vector3 pos = {particle.basePos.x + xOffset, particle.basePos.y, particle.basePos.z + zOffset};
        
        float alpha = 0.05f + 0.2f * sinf(t * 0.8f + i * 0.5f);
        Color pCol = particle.useCyan ? Fade(COLOR_CYAN, alpha) : Fade(COLOR_TOXIC, alpha * 0.5f);
        DrawSphere(pos, 0.03f, pCol);
    }
    
    // ---- RANDOM GLITCH FLASHES ----
    // This one is intentionally still fully randomized each trigger (it's a
    // rare, one-shot flash rather than a persistent element), so rand() here
    // is fine as-is.
    if (fmodf(t * 1.5f, 1.0f) > 0.97f) {
        float halfSize = g_netWorld.worldSize / 2.0f;
        float flashX = -halfSize + (rand() % 1000) / 1000.0f * g_netWorld.worldSize;
        float flashZ = -halfSize + (rand() % 1000) / 1000.0f * g_netWorld.worldSize;
        DrawSphere({flashX, 3.0f, flashZ}, 0.8f, Fade(COLOR_BLOOD, 0.15f));
        DrawSphere({flashX, 3.0f, flashZ}, 1.5f, Fade(COLOR_BLOOD, 0.05f));
    }
}

// ============================================================
// DRAW UI (ENHANCED)
// ============================================================

void DrawNetUI(void) {
    int w = GetScreenWidth();
    int h = GetScreenHeight();
    float t = g_netWorld.time;
    
    // ---- TOP LEFT: INFO ----
    char posStr[128];
    snprintf(posStr, sizeof(posStr), "POS: %.1f, %.1f, %.1f",
             g_netWorld.player.position.x,
             g_netWorld.player.position.y,
             g_netWorld.player.position.z);
    DrawScaledText(posStr, 16, 16, 14, {0, 220, 240, 200});
    
    char nodeStr[64];
    int nodeCount = (int)g_netWorld.nodes.size();
    snprintf(nodeStr, sizeof(nodeStr), "NODES: %d", nodeCount);
    DrawScaledText(nodeStr, 16, 36, 12, {160, 170, 185, 150});
    
    // ---- TOP RIGHT: HELP ----
    DrawScaledText("[ESC] Exit  |  [WASD] Move  |  [Mouse] Look  |  [Click] Interact",
             w - 400, 16, 12, {160, 170, 185, 120});
    
    // ---- CENTER: CROSSHAIR ----
    int cx = w / 2;
    int cy = h / 2;
    float pulse = sinf(t * 3.0f) * 0.3f + 0.7f;
    Color crossCol = {40, 240, 100, (unsigned char)(180 * pulse + 75)};
    
    DrawLine(cx - 12, cy, cx - 5, cy, crossCol);
    DrawLine(cx + 5, cy, cx + 12, cy, crossCol);
    DrawLine(cx, cy - 12, cx, cy - 5, crossCol);
    DrawLine(cx, cy + 5, cx, cy + 12, crossCol);
    DrawCircle(cx, cy, 2, crossCol);
    
    // Outer ring
    DrawCircleLines(cx, cy, 16, Fade(crossCol, 0.3f));
    
    // ---- BOTTOM: SELECTED NODE INFO ----
    if (g_netWorld.selectedNodeIndex >= 0) {
        const auto& node = g_netWorld.nodes[g_netWorld.selectedNodeIndex];
        char info[256];
        const char* typeStr = "NODE";
        switch (node.type) {
            case NodeType::PORTAL: typeStr = "PORTAL"; break;
            case NodeType::DATA_NODE: typeStr = "DATA_NODE"; break;
            case NodeType::ENEMY: typeStr = "ENEMY"; break;
            case NodeType::CORE: typeStr = "CORE"; break;
        }
        snprintf(info, sizeof(info), "[%s] %s  [Click to interact]", typeStr, node.label.c_str());
        
        int tw = MeasureText(info, 14);
        int tx = (w - tw) / 2;
        int ty = h - 50;
        
        DrawRectangle(tx - 16, ty - 6, tw + 32, 30, {0, 0, 0, 180});
        DrawRectangleLines(tx - 16, ty - 6, tw + 32, 30, {0, 220, 240, 100});
        DrawScaledText(info, tx, ty + 6, 14, {0, 220, 240, 220});
    }
    
    // ---- MINIMAP ----
    if (g_netWorld.showMinimap) {
        DrawNetMinimap();
    }
    
    // ---- STATUS BAR ----
    Color statusCol = {40, 240, 100, (unsigned char)(sinf(t * 2.0f) * 0.3f + 0.7f * 200 + 55)};
    DrawCircle(16, h - 20, 6, statusCol);
    DrawScaledText("CYBERSPACE ACTIVE", 28, h - 24, 12, COLOR_TOXIC);
}

// ============================================================
// DRAW MINIMAP
// ============================================================

void DrawNetMinimap(void) {
    int w = GetScreenWidth();
    float mapSize = 150.0f;
    float mapX = w - mapSize - 20.0f;
    float mapY = 60.0f;
    
    DrawRectangle(mapX, mapY, mapSize, mapSize, {0, 0, 0, 200});
    DrawRectangleLines(mapX, mapY, mapSize, mapSize, {0, 220, 240, 80});
    
    float halfWorld = g_netWorld.worldSize / 2.0f;
    for (const auto& node : g_netWorld.nodes) {
        float nx = mapX + mapSize / 2.0f + (node.position.x / halfWorld) * (mapSize / 2.0f - 4);
        float nz = mapY + mapSize / 2.0f + (node.position.z / halfWorld) * (mapSize / 2.0f - 4);
        
        if (nx < mapX + 2 || nx > mapX + mapSize - 2 || nz < mapY + 2 || nz > mapY + mapSize - 2) continue;
        
        Color col = {
            (unsigned char)(node.color.x * 255),
            (unsigned char)(node.color.y * 255),
            (unsigned char)(node.color.z * 255),
            255
        };
        
        float size = (node.type == NodeType::PORTAL) ? 5.0f : 3.0f;
        DrawCircle((int)nx, (int)nz, size, col);
        
        if (node.type == NodeType::PORTAL) {
            DrawCircle((int)nx, (int)nz, size + 3, Fade(col, 0.3f));
        }
    }
    
    float px = mapX + mapSize / 2.0f;
    float pz = mapY + mapSize / 2.0f;
    DrawCircle((int)px, (int)pz, 4, COLOR_TOXIC);
    
    Vector3 dir = {
        -sinf(g_netWorld.player.yaw),
        0.0f,
        -cosf(g_netWorld.player.yaw)
    };
    DrawLine((int)px, (int)pz, (int)(px + dir.x * 8), (int)(pz + dir.z * 8), COLOR_TOXIC);
    
    DrawRectangleLines(mapX, mapY, mapSize, mapSize, {0, 220, 240, 60});
}

void DrawDataRings(void) {
    float t = g_netWorld.time;
    float halfSize = g_netWorld.worldSize / 2.0f;
    
    for (int ring = 0; ring < 12; ring++) {
        float angle = (ring / 12.0f) * 2.0f * PI + t * 0.1f;
        float radius = 20.0f + ring * 4.0f;
        float x = cosf(angle + t * 0.05f) * radius;
        float z = sinf(angle + t * 0.05f) * radius;
        float y = 2.0f + sinf(t * 0.5f + ring * 0.7f) * 1.0f;
        
        Color col = Fade(COLOR_CYAN, 0.08f + 0.04f * sinf(t + ring));
        
        // Draw ring
        DrawCircle3D({x, y, z}, 0.3f, {0, 1, 0}, t * 0.5f + ring, col);
        
        // Data particles on ring
        for (int p = 0; p < 6; p++) {
            float pAngle = t * 2.0f + p * 1.047f + ring * 0.5f;
            float px = x + cosf(pAngle) * 0.6f;
            float pz = z + sinf(pAngle) * 0.6f;
            float py = y + sinf(pAngle * 0.5f) * 0.3f;
            
            Color pCol = Fade(COLOR_TOXIC, 0.15f + 0.1f * sinf(t * 3.0f + p + ring));
            DrawSphere({px, py, pz}, 0.04f, pCol);
        }
    }
}

void DrawWireSphere(Vector3 position, float radius, Color color, int rings, int segments) {
    float t = g_netWorld.time;
    
    // ---- HORIZONTAL RINGS ----
    for (int i = 0; i <= rings; i++) {
        float theta = (i / (float)rings) * PI;
        float y = position.y + radius * cosf(theta);
        float ringRadius = radius * sinf(theta);
        
        if (ringRadius < 0.01f) continue;
        
        // Draw ring with slight offset for each
        for (int j = 0; j < segments; j++) {
            float phi1 = (j / (float)segments) * 2.0f * PI + t * (0.1f + i * 0.02f);
            float phi2 = ((j + 1) / (float)segments) * 2.0f * PI + t * (0.1f + i * 0.02f);
            
            Vector3 p1 = {
                position.x + ringRadius * cosf(phi1),
                y,
                position.z + ringRadius * sinf(phi1)
            };
            Vector3 p2 = {
                position.x + ringRadius * cosf(phi2),
                y,
                position.z + ringRadius * sinf(phi2)
            };
            
            float alpha = 0.3f + 0.7f * (1.0f - abs(cosf(theta)));
            DrawLine3D(p1, p2, Fade(color, alpha * 0.6f));
        }
    }
    
    // ---- VERTICAL RINGS ----
    for (int i = 0; i < segments; i++) {
        float phi = (i / (float)segments) * 2.0f * PI + t * 0.15f;
        float cosP = cosf(phi);
        float sinP = sinf(phi);
        
        for (int j = 0; j <= rings * 2; j++) {
            float theta1 = (j / (float)(rings * 2)) * PI;
            float theta2 = ((j + 1) / (float)(rings * 2)) * PI;
            
            Vector3 p1 = {
                position.x + radius * sinf(theta1) * cosP,
                position.y + radius * cosf(theta1),
                position.z + radius * sinf(theta1) * sinP
            };
            Vector3 p2 = {
                position.x + radius * sinf(theta2) * cosP,
                position.y + radius * cosf(theta2),
                position.z + radius * sinf(theta2) * sinP
            };
            
            DrawLine3D(p1, p2, Fade(color, 0.2f));
        }
    }
    
    // ---- CENTER GLOW ----
    DrawSphere(position, radius * 0.08f, color);
}

void DrawCyberSkybox(void) {
    // Simple gradient background
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {8, 8, 24, 255});
    // Draw stars as white dots (use a fixed set)
    static Vector3 stars[200];
    static bool init = false;
    if (!init) {
        for (int i = 0; i < 200; i++) {
            float theta = (rand() / (float)RAND_MAX) * 2 * PI;
            float phi = acos(2 * (rand() / (float)RAND_MAX) - 1);
            float r = 150.0f;
            stars[i] = {r * sin(phi) * cos(theta), r * cos(phi), r * sin(phi) * sin(theta)};
        }
        init = true;
    }
    for (int i = 0; i < 200; i++) {
        DrawSphere(stars[i], 0.3f, Fade(WHITE, 0.5f + 0.5f * sinf(g_netWorld.time + i)));
    }
    // Grid lines far away
    float size = 200.0f;
    float half = size / 2.0f;
    for (int i = -10; i <= 10; i++) {
        float pos = i * (size / 20.0f);
        DrawLine3D({pos, -20.0f, -half}, {pos, -20.0f, half}, Fade(COLOR_CYAN, 0.03f));
        DrawLine3D({-half, -20.0f, pos}, {half, -20.0f, pos}, Fade(COLOR_CYAN, 0.03f));
    }
}

// ============================================================
// CINEMATIC TRANSITION OVERLAY
// ============================================================
// ============================================================
// CINEMATIC TRANSITION OVERLAY – KERNEL/SNIFFER STYLE
// ============================================================
void DrawTransitionOverlay(void) {
    float intensity = g_netWorld.glitchIntensity;
    float progress = g_netWorld.stateTimer / g_netWorld.transitionDuration;
    bool isEntering = (g_netWorld.state == NetWorldState::ENTERING);
    float t = g_netWorld.time;
    int w = GetScreenWidth();
    int h = GetScreenHeight();

    // ---- 1. BACKGROUND (dark gradient) ----
    for (int y = 0; y < h; y++) {
        float alpha = 1.0f - (float)y / h;
        Color col = {0, (unsigned char)(10 * alpha), (unsigned char)(20 * alpha), 255};
        DrawPixel(0, y, col);
    }

    // ---- 2. SUBTLE MATRIX RAIN (muted) ----
    static const int RAIN_COUNT = 40;
    static struct RainDrop {
        float x, y, speed;
        char ch;
    } rain[RAIN_COUNT];
    static bool rainInit = false;
    if (!rainInit) {
        for (int i = 0; i < RAIN_COUNT; i++) {
            rain[i].x = (rand() % w);
            rain[i].y = (rand() % h) - h;
            rain[i].speed = 30.0f + rand() % 80;
            rain[i].ch = "0123456789ABCDEF"[rand() % 16];
        }
        rainInit = true;
    }
    for (int i = 0; i < RAIN_COUNT; i++) {
        rain[i].y += rain[i].speed * GetFrameTime();
        if (rain[i].y > h) {
            rain[i].y = -20.0f;
            rain[i].x = rand() % w;
            rain[i].ch = "0123456789ABCDEF"[rand() % 16];
        }
        float alpha = 0.08f * (1.0f - intensity * 0.5f) * (1.0f - fabs(rain[i].y - h/2) / (h/2));
        DrawScaledText(TextFormat("%c", rain[i].ch), (int)rain[i].x, (int)rain[i].y, 12, Fade(COLOR_CYAN, alpha));
    }

    // ---- 3. WINDOW SYSTEM ----
    struct TransitionWindow {
        Rectangle rect;
        std::string title;
        std::vector<std::string> lines;
        float alpha;
        float targetAlpha;
        float lifetime;
        float phase;
        int type; // 0=log, 1=hex, 2=packet
    };
    static std::vector<TransitionWindow> windows;
    static bool windowsInit = false;

    if (!windowsInit) {
        windows.clear();
        // Generate 8-12 windows with random positions, sizes, and contents
        const char* titles[] = {
            "KERNEL_LOAD", "PKT_SNIFF", "NET_INIT", "ROUTE_TABLE",
            "MODULE", "SYSLOG", "FIREWALL", "ARP_CACHE", "DNS_RESOLV",
            "TCP_STACK", "UDP_SOCKET", "ICMP_ECHO"
        };
        int numWindows = 8 + rand() % 5;
        for (int i = 0; i < numWindows; i++) {
            TransitionWindow win;
            int winW = 180 + rand() % 220;
            int winH = 80 + rand() % 100;
            win.rect = {
                (float)(rand() % (w - winW)),
                (float)(rand() % (h - winH)),
                (float)winW,
                (float)winH
            };
            win.title = titles[rand() % 12];
            win.alpha = 0.0f;
            win.targetAlpha = 0.6f + (rand() % 100) / 200.0f; // 0.6-1.0
            win.lifetime = 2.0f + (rand() % 100) / 50.0f;     // 2-4 seconds
            win.phase = (rand() % 1000) / 1000.0f * 2.0f * PI;
            win.type = rand() % 3;

            // Generate content lines based on type
            for (int j = 0; j < 4 + rand() % 4; j++) {
                std::string line;
                switch (win.type) {
                    case 0: { // system log
                        const char* msgs[] = {
                            "loading module...", "init complete", "allocated buffer",
                            "handshake OK", "auth success", "packet queued",
                            "sync lost", "retry...", "connection established"
                        };
                        line = msgs[rand() % 9];
                        break;
                    }
                    case 1: { // hex dump
                        char hex[32];
                        for (int k = 0; k < 8; k++) {
                            sprintf(hex + k*3, "%02X ", rand() % 256);
                        }
                        line = hex;
                        break;
                    }
                    case 2: { // packet sniff
                        char pkt[64];
                        sprintf(pkt, "SRC %d.%d.%d.%d : DST %d.%d.%d.%d",
                                rand()%256, rand()%256, rand()%256, rand()%256,
                                rand()%256, rand()%256, rand()%256, rand()%256);
                        line = pkt;
                        break;
                    }
                }
                win.lines.push_back(line);
            }
            windows.push_back(win);
        }
        windowsInit = true;
    }

    // Update window alphas (fade in/out over time)
    float dt = GetFrameTime();
    for (auto& win : windows) {
        // Slight random movement
        win.rect.x += sinf(t * 0.2f + win.phase) * 0.1f;
        win.rect.y += cosf(t * 0.15f + win.phase * 1.3f) * 0.1f;
        // Fade target: windows disappear/reappear randomly
        if (rand() % 100 < 2) {
            win.targetAlpha = (win.targetAlpha > 0.5f) ? 0.2f : 0.7f + (rand()%60)/100.0f;
        }
        win.alpha += (win.targetAlpha - win.alpha) * dt * 2.0f;
        win.alpha = fmaxf(0.0f, fminf(1.0f, win.alpha));
    }

    // Draw windows
    for (const auto& win : windows) {
        if (win.alpha < 0.01f) continue;
        Color winBg = {10, 14, 28, (unsigned char)(180 * win.alpha)};
        Color titleBg = {20, 40, 80, (unsigned char)(220 * win.alpha)};
        Color borderCol = {80, 180, 255, (unsigned char)(150 * win.alpha)};
        Color textCol = {180, 220, 255, (unsigned char)(200 * win.alpha)};

        // Window body
        DrawRectangleRec(win.rect, winBg);
        DrawRectangleLinesEx(win.rect, 1, borderCol);

        // Title bar
        Rectangle titleRect = {win.rect.x, win.rect.y, win.rect.width, 20};
        DrawRectangleRec(titleRect, titleBg);
        DrawScaledText(win.title.c_str(), (int)win.rect.x + 6, (int)win.rect.y + 4, 12, Fade(COLOR_CYAN, win.alpha));

        // Close/minimize buttons (fake)
        DrawRectangle((int)(win.rect.x + win.rect.width - 18), (int)win.rect.y + 4, 12, 12, {200,40,40, (unsigned char)(150*win.alpha)});
        DrawRectangle((int)(win.rect.x + win.rect.width - 34), (int)win.rect.y + 4, 12, 12, {40,200,40, (unsigned char)(150*win.alpha)});

        // Content lines
        float lineY = win.rect.y + 24;
        int fontSize = 10;
        for (const auto& line : win.lines) {
            if (lineY + fontSize > win.rect.y + win.rect.height - 4) break;
            // Add a little random offset for a "jitter" effect
            int xOff = (rand() % 3) - 1;
            DrawScaledText(line.c_str(), (int)win.rect.x + 6 + xOff, (int)lineY, fontSize, textCol);
            lineY += fontSize + 2;
        }
    }

    // ---- 4. PROGRESS INDICATOR (optional, subtle) ----
    int cx = w / 2;
    int cy = h / 2 - 20;
    int radius = 60;
    float progressAngle = (isEntering ? progress : 1.0f - progress) * 360.0f;
    DrawCircleSector({(float)cx, (float)cy}, radius, -90, -90 + progressAngle, 36, Fade(COLOR_TOXIC, 0.4f));
    DrawCircleLines(cx, cy, radius + 4, Fade(COLOR_CYAN, 0.2f));

    // ---- 5. SCANLINES (keep) ----
    for (int y = 0; y < h; y += 3) {
        float scanY = y + fmodf(t * 60.0f + g_netWorld.scanlineOffset, 3.0f);
        unsigned char alpha = (unsigned char)(intensity * 20 + 8);
        DrawRectangle(0, (int)scanY, w, 1, {0, 0, 0, alpha});
    }

    // ---- 6. VIGNETTE (dark edges) ----
    DrawRectangle(0, 0, w, 4, {0, 0, 0, 80});
    DrawRectangle(0, h - 4, w, 4, {0, 0, 0, 80});
    DrawRectangle(0, 0, 4, h, {0, 0, 0, 80});
    DrawRectangle(w - 4, 0, 4, h, {0, 0, 0, 80});
}

// ============================================================
// VOID SCENERY
//
// Everything that establishes "we are floating in a data void."
// None of this is gameplay geometry — it exists so the eye has
// three distinct scales to read (near platforms, mid pylons,
// far horizon) instead of one flat grid against black.
// ============================================================

static void DrawVoidPlane(void) {
    const Vector3 c = g_netWorld.arenaCenter;
    const float Y = -8.0f;
    const float extent = 70.0f;
    const float step = 8.0f;
    Color col = Fade(COLOR_CYAN, 0.035f);

    for (float d = -extent; d <= extent; d += step) {
        DrawLine3D({c.x + d, Y, c.z - extent}, {c.x + d, Y, c.z + extent}, col);
        DrawLine3D({c.x - extent, Y, c.z + d}, {c.x + extent, Y, c.z + d}, col);
    }
    // Concentric markers give the plane a "scanned region" feel.
    DrawCircle3D({c.x, Y, c.z}, extent,       {0, 1, 0}, 0, Fade(COLOR_TOXIC, 0.06f));
    DrawCircle3D({c.x, Y, c.z}, extent * 0.5f, {0, 1, 0}, 0, Fade(COLOR_CYAN,  0.04f));
}

static void DrawVoidHorizon(void) {
    const Vector3 c = g_netWorld.arenaCenter;
    const float t = g_netWorld.time;
    const float radius = 60.0f;
    const int count = 64;

    // Alternating vertical bars, height-modulated so the horizon breathes.
    // This is what tells the player "the void extends past what you can
    // walk on" without any actual geometry out there.
    for (int i = 0; i < count; i++) {
        float angle = (i / (float)count) * 2.0f * PI;
        float x = c.x + cosf(angle) * radius;
        float z = c.z + sinf(angle) * radius;
        float h = 10.0f + 6.0f * sinf(t * 0.15f + i * 0.2f);
        float a = 0.05f + 0.03f * sinf(t * 0.5f + i * 0.4f);
        DrawLine3D({x, 0.0f, z}, {x, h, z}, Fade(COLOR_CYAN, a));
    }
}

static void DrawPylons(void) {
    const Vector3 c = g_netWorld.arenaCenter;
    const float t = g_netWorld.time;
    const float ringR = 42.0f;
    const int count = 8;

    // Pylons sit between the near platforms and the far horizon. They give
    // the void a middle distance and anchor the arena's cardinal directions
    // visually, so the player has a compass even without the minimap.
    for (int i = 0; i < count; i++) {
        float angle = (i / (float)count) * 2.0f * PI + PI / 8.0f;
        float x = c.x + cosf(angle) * ringR;
        float z = c.z + sinf(angle) * ringR;
        float h = 14.0f + 2.5f * sinf(t * 0.3f + i * 1.7f);
        float pulse = 0.5f + 0.5f * sinf(t * 0.8f + i * 1.3f);

        DrawLine3D({x, 0.0f, z}, {x, h, z}, Fade(COLOR_CYAN, 0.35f));

        DrawCircle3D({x, 0.05f, z}, 1.0f, {0, 1, 0}, 0, Fade(COLOR_CYAN, 0.4f));

        float mid = h * 0.5f;
        DrawLine3D({x - 0.8f, mid, z}, {x + 0.8f, mid, z}, Fade(COLOR_TOXIC, 0.20f));
        DrawLine3D({x, mid, z - 0.8f}, {x, mid, z + 0.8f}, Fade(COLOR_TOXIC, 0.20f));

        DrawSphere({x, h, z}, 0.2f, Fade(COLOR_CYAN, 0.70f * pulse));
        DrawSphere({x, h, z}, 0.5f, Fade(COLOR_CYAN, 0.15f * pulse));
    }
}

// ============================================================
// ARENA HUB
//
// The designed centerpiece. Reads as a place rather than as
// terrain: floor ring, raised dais, pillar ring, observation
// ring, and an overhead light-frame that suggests a room
// without sealing the sky.
// ============================================================

static void DrawArenaHub(void) {
    const Vector3 c = g_netWorld.arenaCenter;
    const float t = g_netWorld.time;
    const float pulse = 0.5f + 0.5f * sinf(t * 1.5f);
    const float outerR  = g_netWorld.arenaRadius;    // 10
    const float daisR   = outerR * 0.5f;             //  5
    const float pillarR = outerR * 0.7f;             //  7
    const float pillarH = 4.5f;

    // --- Floor ring ---
    DrawCircle3D({c.x, c.y + 0.03f, c.z}, outerR, {0, 1, 0}, 0, Fade(COLOR_CYAN, 0.45f));
    DrawCircle3D({c.x, c.y + 0.03f, c.z}, outerR, {0, 1, 0}, 0, Fade(COLOR_TOXIC, 0.15f + 0.15f * pulse));

    // Radial spokes from dais edge to outer ring. Without these the floor
    // reads as empty even though it's bounded — the eye needs to see the
    // edges "held" by something structural.
    for (int i = 0; i < 8; i++) {
        float a = (i / 8.0f) * 2.0f * PI;
        Vector3 p1 = {c.x + cosf(a) * daisR,  c.y + 0.03f, c.z + sinf(a) * daisR};
        Vector3 p2 = {c.x + cosf(a) * outerR, c.y + 0.03f, c.z + sinf(a) * outerR};
        DrawLine3D(p1, p2, Fade(COLOR_CYAN, 0.12f));
    }

    // --- Raised dais ---
    // A short filled cylinder for volume, then a bright rim at the top so
    // the lip reads as designed rather than as a smoothing artifact.
    DrawCylinderEx(
        {c.x, c.y,         c.z},
        {c.x, c.y + 0.6f,  c.z},
        daisR, daisR, 32, Fade(COLOR_CYAN, 0.10f)
    );
    DrawCircle3D({c.x, c.y + 0.6f, c.z}, daisR,        {0, 1, 0}, 0, Fade(COLOR_CYAN,  0.70f));
    DrawCircle3D({c.x, c.y + 0.6f, c.z}, daisR * 0.55f, {0, 1, 0}, 0, Fade(COLOR_TOXIC, 0.40f));

    // --- Pillar ring ---
    // Six slim boxy spires between the dais and the outer ring. They frame
    // the centerpiece without blocking sightlines to it.
    const float halfW = 0.35f;
    for (int i = 0; i < 6; i++) {
        float a = (i / 6.0f) * 2.0f * PI + PI / 6.0f;
        float px = c.x + cosf(a) * pillarR;
        float pz = c.z + sinf(a) * pillarR;

        Vector3 corners[4] = {
            {px - halfW, 0.0f, pz - halfW},
            {px + halfW, 0.0f, pz - halfW},
            {px + halfW, 0.0f, pz + halfW},
            {px - halfW, 0.0f, pz + halfW}
        };
        for (int j = 0; j < 4; j++) {
            DrawLine3D(corners[j], {corners[j].x, pillarH, corners[j].z}, Fade(COLOR_CYAN, 0.35f));
        }
        for (int j = 0; j < 4; j++) {
            int k = (j + 1) % 4;
            DrawLine3D({corners[j].x, pillarH, corners[j].z},
                       {corners[k].x, pillarH, corners[k].z}, Fade(COLOR_TOXIC, 0.50f));
        }
        DrawSphere({px, pillarH, pz}, 0.15f, Fade(COLOR_TOXIC, 0.60f * pulse));
    }

    // --- Observation ring ---
    DrawCircle3D({c.x, c.y + 6.0f, c.z}, outerR * 0.85f, {0, 1, 0}, 0,
                 Fade(COLOR_CYAN, 0.20f + 0.10f * pulse));
    for (int i = 0; i < 8; i++) {
        float a = (i / 8.0f) * 2.0f * PI;
        float x = c.x + cosf(a) * outerR * 0.85f;
        float z = c.z + sinf(a) * outerR * 0.85f;
        DrawLine3D({x, 0.05f, z}, {x, 6.0f, z}, Fade(COLOR_CYAN, 0.08f));
    }

    // --- Overhead light-frame ---
    // A large ring at Y=11 plus four spokes to a hub. This is what turns
    // the arena from "a floor with pillars" into "a room" — the eye reads
    // an implied ceiling without actually committing to geometry that
    // would block the void sky.
    DrawCircle3D({c.x, c.y + 11.0f, c.z}, outerR * 1.2f, {0, 1, 0}, 0, Fade(COLOR_CYAN, 0.15f));
    for (int i = 0; i < 4; i++) {
        float a = (i / 4.0f) * 2.0f * PI + PI / 4.0f;
        Vector3 p1 = {c.x + cosf(a) * outerR * 1.2f, c.y + 11.0f, c.z + sinf(a) * outerR * 1.2f};
        Vector3 p2 = {c.x, c.y + 13.0f, c.z};
        DrawLine3D(p1, p2, Fade(COLOR_CYAN, 0.08f));
    }
    DrawSphere({c.x, c.y + 13.0f, c.z}, 0.3f, Fade(COLOR_TOXIC, 0.40f * pulse));
}