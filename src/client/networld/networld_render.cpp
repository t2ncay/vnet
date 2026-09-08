#include "networld.h"
#include "../render.h"
#include "../../shared/vnet.h"
#include "raymath.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstring>

static void DrawGlitchCubes3D(void); 

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
    
    // Init glitch cubes if needed
    if (!g_glitchCubesInit) {
        InitGlitchCubes();
    }
    // Init the persistent particle / data-stream buffers if needed
    if (!g_netEffectsInit) {
        InitNetEffects();
    }
    
    // ---- GLITCH TRANSITION OVERLAY ----
    if (g_netWorld.state == NetWorldState::ENTERING || g_netWorld.state == NetWorldState::EXITING) {
        float intensity = g_netWorld.glitchIntensity;
        
        unsigned char alpha = (unsigned char)(intensity * 200);
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0, 0, 0, alpha});
        
        for (int i = 0; i < (int)(intensity * 20); i++) {
            int y = rand() % GetScreenHeight();
            int h = 2 + rand() % 8;
            int alpha2 = (int)(intensity * 150);
            DrawRectangle(0, y, GetScreenWidth(), h, {220, 20, 40, (unsigned char)alpha2});
        }
        
        float scanLineOffset = g_netWorld.scanlineOffset;
        for (int y = 0; y < GetScreenHeight(); y += 4) {
            float scanY = y + fmodf(scanLineOffset, 4.0f);
            DrawRectangle(0, (int)scanY, GetScreenWidth(), 1, {0, 0, 0, (unsigned char)(intensity * 20)});
        }
        
        if (g_netWorld.state == NetWorldState::ENTERING) {
            float progress = g_netWorld.stateTimer / g_netWorld.transitionDuration;
            int w = GetScreenWidth();
            int h = GetScreenHeight();
            
            const char* messages[] = {
                "INITIALIZING CYBERSPACE...",
                "MAPPING NEURAL TOPOLOGY...",
                "SYNCHRONIZING WITH VNET...",
                "JACKING IN..."
            };
            int msgIdx = (int)(progress * 4) % 4;
            
            int fontSize = 24;
            int tw = MeasureText(messages[msgIdx], fontSize);
            int tx = (w - tw) / 2;
            int ty = h / 2 + 60;
            
            Color textCol = {40, 240, 100, (unsigned char)(200 * (0.5f + 0.5f * sinf(g_netWorld.time * 4.0f)))};
            DrawScaledText(messages[msgIdx], tx, ty, fontSize, textCol);
            
            int barX = w / 2 - 150;
            int barY = h / 2 + 100;
            int barW = 300;
            int barH = 8;
            DrawRectangle(barX, barY, barW, barH, {10, 15, 20, 200});
            DrawRectangle(barX, barY, (int)(barW * progress), barH, {40, 240, 100, 200});
            DrawRectangleLines(barX, barY, barW, barH, {40, 240, 100, 150});
        }
        return;
    }

    if (g_netWorld.state == NetWorldState::ACTIVE) {
        ApplyNetWorldShader();
    }
    
    ApplyNetWorldPBR(g_netWorld.camera);

    // ---- 3D SCENE ----
    BeginMode3D(g_netWorld.camera);
    
    DrawNetTerrain();
    DrawGlitchCubes3D();
    DrawNetNodes();
    DrawNetPortals();
    DrawNetPlayer();
    DrawNetEffects();
    DrawDataRings();
    
    EndMode3D();

    EndNetWorldPBR();
    
    // ---- 2D UI OVERLAY ----
    DrawNetUI();
    
    // ---- SCANLINES ----
    float t = g_netWorld.time;
    for (int y = 0; y < GetScreenHeight(); y += 4) {
        float scanY = y + fmodf(t * 30.0f + g_netWorld.scanlineOffset, 4.0f);
        DrawRectangle(0, (int)scanY, GetScreenWidth(), 1, {0, 0, 0, 4});
    }
    
    // ---- VIGNETTE ----
    int w = GetScreenWidth();
    int h = GetScreenHeight();
    DrawRectangle(0, 0, w, 4, {0, 0, 0, 80});
    DrawRectangle(0, h - 4, w, 4, {0, 0, 0, 80});
    DrawRectangle(0, 0, 4, h, {0, 0, 0, 80});
    DrawRectangle(w - 4, 0, 4, h, {0, 0, 0, 80});
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
    float size = g_netWorld.worldSize;
    float halfSize = size / 2.0f;
    float gridSize = size / g_netWorld.gridSize;
    float height = g_netWorld.terrainHeight;
    float t = g_netWorld.time;
    
    float pulse = sinf(t * 1.5f) * 0.3f + 0.7f;
    
    // ---- DATA PILLARS (cyberpunk instead of flat grid) ----
    for (int x = 0; x <= g_netWorld.gridSize; x += 2) {
        for (int z = 0; z <= g_netWorld.gridSize; z += 2) {
            float xPos = -halfSize + x * gridSize;
            float zPos = -halfSize + z * gridSize;
            
            // Skip center area for visibility
            float distFromCenter = sqrtf(xPos*xPos + zPos*zPos);
            if (distFromCenter < 8.0f) continue;
            
            float pillarHeight = 0.3f + sinf(xPos * 0.3f + zPos * 0.2f + t * 0.2f) * 0.2f + 0.3f;
            float alpha = 0.04f + (1.0f - distFromCenter / halfSize) * 0.06f * pulse;
            
            Color col = Fade(COLOR_CYAN, alpha);
            
            // Draw pillar as thin line
            DrawLine3D(
                {xPos, height, zPos},
                {xPos, height + pillarHeight, zPos},
                col
            );
            
            // Glow at top
            if (pillarHeight > 0.4f) {
                DrawSphere({xPos, height + pillarHeight, zPos}, 0.06f, Fade(COLOR_TOXIC, alpha * 0.5f));
            }
        }
    }
    
    // ---- CONCENTRIC RINGS ----
    for (int r = 1; r <= 6; r++) {
        float radius = r * 7.0f;
        float alpha = 0.02f + (1.0f - r / 6.0f) * 0.06f * pulse;
        DrawCircle3D({0, height + 0.1f, 0}, radius, {0, 1, 0}, t * 0.05f * r, Fade(COLOR_CYAN, alpha));
        DrawCircle3D({0, height + 0.1f, 0}, radius, {0, 1, 0}, -t * 0.03f * r, Fade(COLOR_TOXIC, alpha * 0.5f));
    }
    
    // ---- CENTER GLOW (CORE) ----
    DrawCircle3D({0, height + 0.1f, 0}, 4.0f, {0, 1, 0}, 0.0f, Fade(COLOR_TOXIC, 0.4f * pulse));
    DrawCircle3D({0, height + 0.1f, 0}, 2.0f, {0, 1, 0}, 0.0f, Fade(COLOR_TOXIC, 0.6f * pulse));
    DrawCircle3D({0, height + 0.1f, 0}, 1.0f, {0, 1, 0}, 0.0f, Fade(COLOR_AMBER, 0.8f * pulse));
    
    // ---- BOUNDARY GLOW ----
    Color boundaryCol = Fade(COLOR_BLOOD, 0.15f * pulse);
    DrawLine3D({-halfSize, height, -halfSize}, {halfSize, height, -halfSize}, boundaryCol);
    DrawLine3D({halfSize, height, -halfSize}, {halfSize, height, halfSize}, boundaryCol);
    DrawLine3D({halfSize, height, halfSize}, {-halfSize, height, halfSize}, boundaryCol);
    DrawLine3D({-halfSize, height, halfSize}, {-halfSize, height, -halfSize}, boundaryCol);
    
    // Boundary corner glows
    float cornerGlow = 0.15f + 0.1f * pulse;
    DrawCircle3D({-halfSize, height, -halfSize}, 3.0f, {0, 1, 0}, 0.0f, Fade(COLOR_BLOOD, cornerGlow));
    DrawCircle3D({halfSize, height, -halfSize}, 3.0f, {0, 1, 0}, 0.0f, Fade(COLOR_BLOOD, cornerGlow));
    DrawCircle3D({halfSize, height, halfSize}, 3.0f, {0, 1, 0}, 0.0f, Fade(COLOR_BLOOD, cornerGlow));
    DrawCircle3D({-halfSize, height, halfSize}, 3.0f, {0, 1, 0}, 0.0f, Fade(COLOR_BLOOD, cornerGlow));
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