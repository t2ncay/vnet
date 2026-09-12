#include "networld.h"
#include "../render.h"
#include <cstdio>

// ---------------------------------------------------------------------------
// NOTE: filename is historical. This file now owns the UNLIT material shader
// (networld.vs / networld.fs). Public API names are kept unchanged so the
// rest of the codebase (and networld.h) doesn't have to move.
//
// If you want to rename this file to networld_material.cpp, the only changes
// required elsewhere are the CMakeLists entry and the header comment.
// ---------------------------------------------------------------------------

static Shader g_materialShader = {0};
static bool   g_materialLoaded = false;

// Uniform locations
static int g_viewPosLoc          = -1;
static int g_timeLoc             = -1;
static int g_emissiveColorLoc    = -1;
static int g_emissivePowerLoc    = -1;
static int g_rimColorLoc         = -1;
static int g_rimPowerLoc         = -1;
static int g_rimIntensityLoc     = -1;
static int g_scanlineAmountLoc   = -1;
static int g_ambientColorLoc     = -1;
static int g_ambientIntensityLoc = -1;
static int g_numLightsLoc        = -1;

// Per-light uniform locations, resolved once. Lookup up-front because these
// are indexed uniform names ("lights[3].color") and doing the string walk
// per-frame is pure waste.
struct LightLocations {
    int enabled  [MAX_DYNAMIC_LIGHTS] = {};
    int type     [MAX_DYNAMIC_LIGHTS] = {};
    int position [MAX_DYNAMIC_LIGHTS] = {};
    int color    [MAX_DYNAMIC_LIGHTS] = {};
    int intensity[MAX_DYNAMIC_LIGHTS] = {};
};
static LightLocations g_lightLoc;

// ===========================================================================
// LOAD
// ===========================================================================

void LoadNetWorldPBR(void)
{
    if (g_materialLoaded) return;

    // ==== ASSET LOAD: NETWORLD MATERIAL SHADER ====
    g_materialShader = LoadShader(
        "src/client/render/shaders/networld.vs",
        "src/client/render/shaders/networld.fs"
    );
    // ==== END ASSET LOAD ====

    if (g_materialShader.id == 0) {
        printf("[MATERIAL] NetWorld material shader FAILED to load.\n");
        return;
    }

    // raylib auto-populates SHADER_LOC_MAP_DIFFUSE ("texture0") and
    // SHADER_LOC_COLOR_DIFFUSE ("colDiffuse") by name. Everything else is
    // ours to look up.
    g_viewPosLoc          = GetShaderLocation(g_materialShader, "viewPos");
    g_timeLoc             = GetShaderLocation(g_materialShader, "time");
    g_emissiveColorLoc    = GetShaderLocation(g_materialShader, "emissiveColor");
    g_emissivePowerLoc    = GetShaderLocation(g_materialShader, "emissivePower");
    g_rimColorLoc         = GetShaderLocation(g_materialShader, "rimColor");
    g_rimPowerLoc         = GetShaderLocation(g_materialShader, "rimPower");
    g_rimIntensityLoc     = GetShaderLocation(g_materialShader, "rimIntensity");
    g_scanlineAmountLoc   = GetShaderLocation(g_materialShader, "scanlineAmount");
    g_ambientColorLoc     = GetShaderLocation(g_materialShader, "ambientColor");
    g_ambientIntensityLoc = GetShaderLocation(g_materialShader, "ambientIntensity");
    g_numLightsLoc        = GetShaderLocation(g_materialShader, "numOfLights");

    char name[64];
    for (int i = 0; i < MAX_DYNAMIC_LIGHTS; i++) {
        snprintf(name, sizeof(name), "lights[%i].enabled",   i);
        g_lightLoc.enabled[i]   = GetShaderLocation(g_materialShader, name);
        snprintf(name, sizeof(name), "lights[%i].type",      i);
        g_lightLoc.type[i]      = GetShaderLocation(g_materialShader, name);
        snprintf(name, sizeof(name), "lights[%i].position",  i);
        g_lightLoc.position[i]  = GetShaderLocation(g_materialShader, name);
        snprintf(name, sizeof(name), "lights[%i].color",     i);
        g_lightLoc.color[i]     = GetShaderLocation(g_materialShader, name);
        snprintf(name, sizeof(name), "lights[%i].intensity", i);
        g_lightLoc.intensity[i] = GetShaderLocation(g_materialShader, name);
    }

    // raylib does NOT set colDiffuse for immediate-mode primitives
    // (DrawCube / DrawSphere / DrawLine3D) the way DrawModel does, so we
    // set it once at load time. Without this the albedo multiplication
    // in the fragment shader evaluates to zero and every surface renders
    // as pure emissive + rim — the "too dark" bug.
    int colDiffuseLoc = GetShaderLocation(g_materialShader, "colDiffuse");
    Vector4 whiteDiffuse = {1.0f, 1.0f, 1.0f, 1.0f};
    SetShaderValue(g_materialShader, colDiffuseLoc, &whiteDiffuse, SHADER_UNIFORM_VEC4);

    // ---- Material defaults ----
    // Tuned for the unlit-plus-ambient-floor formula in networld.fs. These
    // are one-shot: gameplay can override per-draw later via public setters.
    static const Vector3 kAmbientColor      = {0.45f, 0.50f, 0.65f};
    static const float   kAmbientIntensity  = 0.55f;
    static const Vector3 kEmissiveTint      = {0.06f, 0.09f, 0.14f};
    static const float   kEmissivePower     = 1.0f;
    static const Vector3 kRimColor          = {0.40f, 0.90f, 1.00f};
    static const float   kRimPower          = 2.5f;
    static const float   kRimIntensity      = 0.45f;
    static const float   kScanlineAmount    = 0.08f;

    SetShaderValue(g_materialShader, g_ambientColorLoc,     &kAmbientColor,     SHADER_UNIFORM_VEC3);
    SetShaderValue(g_materialShader, g_ambientIntensityLoc, &kAmbientIntensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(g_materialShader, g_emissiveColorLoc,    &kEmissiveTint,     SHADER_UNIFORM_VEC3);
    SetShaderValue(g_materialShader, g_emissivePowerLoc,    &kEmissivePower,    SHADER_UNIFORM_FLOAT);
    SetShaderValue(g_materialShader, g_rimColorLoc,         &kRimColor,         SHADER_UNIFORM_VEC3);
    SetShaderValue(g_materialShader, g_rimPowerLoc,         &kRimPower,         SHADER_UNIFORM_FLOAT);
    SetShaderValue(g_materialShader, g_rimIntensityLoc,     &kRimIntensity,     SHADER_UNIFORM_FLOAT);
    SetShaderValue(g_materialShader, g_scanlineAmountLoc,   &kScanlineAmount,   SHADER_UNIFORM_FLOAT);

    g_materialLoaded = true;
    printf("[MATERIAL] NetWorld unlit material shader loaded.\n");
}

// ===========================================================================
// APPLY
// ===========================================================================

void ApplyNetWorldPBR(Camera3D camera)
{
    if (!g_materialLoaded) return;
    if (!g_netWorld.active || g_netWorld.state != NetWorldState::ACTIVE) return;

    float camPos[3] = {camera.position.x, camera.position.y, camera.position.z};
    SetShaderValue(g_materialShader, g_viewPosLoc, camPos, SHADER_UNIFORM_VEC3);

    float t = g_netWorld.time;
    SetShaderValue(g_materialShader, g_timeLoc, &t, SHADER_UNIFORM_FLOAT);

    int lightCount = g_netWorld.dynamicLightCount;
    SetShaderValue(g_materialShader, g_numLightsLoc, &lightCount, SHADER_UNIFORM_INT);

    for (int i = 0; i < lightCount && i < MAX_DYNAMIC_LIGHTS; i++) {
        const DynamicLight& l = g_netWorld.dynamicLights[i];

        int enabled = l.active ? 1 : 0;
        SetShaderValue(g_materialShader, g_lightLoc.enabled[i], &enabled, SHADER_UNIFORM_INT);

        int type = 1; // point light — this material has no directional path
        SetShaderValue(g_materialShader, g_lightLoc.type[i], &type, SHADER_UNIFORM_INT);

        SetShaderValue(g_materialShader, g_lightLoc.position[i], &l.position, SHADER_UNIFORM_VEC3);

        Vector4 col = {l.color.x, l.color.y, l.color.z, 1.0f};
        SetShaderValue(g_materialShader, g_lightLoc.color[i], &col, SHADER_UNIFORM_VEC4);

        SetShaderValue(g_materialShader, g_lightLoc.intensity[i], &l.intensity, SHADER_UNIFORM_FLOAT);
    }

    BeginShaderMode(g_materialShader);
}

void EndNetWorldPBR(void)
{
    if (g_materialLoaded) EndShaderMode();
}

// ===========================================================================
// UNLOAD
// ===========================================================================

void UnloadNetWorldPBR(void)
{
    if (!g_materialLoaded) return;
    // ==== ASSET UNLOAD: NETWORLD MATERIAL SHADER ====
    UnloadShader(g_materialShader);
    // ==== END ASSET UNLOAD ====
    g_materialLoaded = false;
    printf("[MATERIAL] NetWorld material shader unloaded.\n");
}