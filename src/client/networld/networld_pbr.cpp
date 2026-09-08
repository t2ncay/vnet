#include "networld.h"
#include "../render.h"
#include "../../shared/vnet.h"
#include <cmath>
#include <cstdlib>
#include <cstring>

// ============================================================
// PBR LIGHT SYSTEM
// ============================================================

#define MAX_LIGHTS 4

typedef enum {
    LIGHT_DIRECTIONAL = 0,
    LIGHT_POINT,
    LIGHT_SPOT
} LightType;

typedef struct {
    int type;
    int enabled;
    Vector3 position;
    Vector3 target;
    Color color;
    float intensity;
    
    // Shader locations
    int typeLoc;
    int enabledLoc;
    int positionLoc;
    int targetLoc;
    int colorLoc;
    int intensityLoc;
} Light;

static Shader g_pbrShader = {0};
static Light g_lights[MAX_LIGHTS] = {0};
static int g_lightCount = 0;
static bool g_pbrLoaded = false;

// Shader uniform locations
static int g_viewPosLoc = -1;
static int g_ambientColorLoc = -1;
static int g_ambientIntensityLoc = -1;
static int g_albedoColorLoc = -1;
static int g_metallicValueLoc = -1;
static int g_roughnessValueLoc = -1;
static int g_emissivePowerLoc = -1;
static int g_emissiveColorLoc = -1;

// ============================================================
// CREATE LIGHT
// ============================================================

static Light CreateLight(int type, Vector3 position, Vector3 target, Color color, float intensity, Shader shader)
{
    Light light = {0};
    if (g_lightCount < MAX_LIGHTS)
    {
        light.enabled = 1;
        light.type = type;
        light.position = position;
        light.target = target;
        light.color = color;
        light.intensity = intensity;
        
        light.enabledLoc = GetShaderLocation(shader, TextFormat("lights[%i].enabled", g_lightCount));
        light.typeLoc = GetShaderLocation(shader, TextFormat("lights[%i].type", g_lightCount));
        light.positionLoc = GetShaderLocation(shader, TextFormat("lights[%i].position", g_lightCount));
        light.targetLoc = GetShaderLocation(shader, TextFormat("lights[%i].target", g_lightCount));
        light.colorLoc = GetShaderLocation(shader, TextFormat("lights[%i].color", g_lightCount));
        light.intensityLoc = GetShaderLocation(shader, TextFormat("lights[%i].intensity", g_lightCount));
        
        // Send initial light data to shader
        int enabledVal = light.enabled;
        int typeVal = light.type;
        float pos[3] = {light.position.x, light.position.y, light.position.z};
        float target[3] = {light.target.x, light.target.y, light.target.z};
        float col[4] = {light.color.r/255.0f, light.color.g/255.0f, light.color.b/255.0f, light.color.a/255.0f};
        
        SetShaderValue(shader, light.enabledLoc, &enabledVal, SHADER_UNIFORM_INT);
        SetShaderValue(shader, light.typeLoc, &typeVal, SHADER_UNIFORM_INT);
        SetShaderValue(shader, light.positionLoc, pos, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, light.targetLoc, target, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, light.colorLoc, col, SHADER_UNIFORM_VEC4);
        SetShaderValue(shader, light.intensityLoc, &light.intensity, SHADER_UNIFORM_FLOAT);
        
        g_lightCount++;
    }
    return light;
}

// ============================================================
// UPDATE LIGHT
// ============================================================

static void UpdateLight(Shader shader, Light light)
{
    int enabledVal = light.enabled;
    int typeVal = light.type;
    float pos[3] = {light.position.x, light.position.y, light.position.z};
    float target[3] = {light.target.x, light.target.y, light.target.z};
    float col[4] = {light.color.r/255.0f, light.color.g/255.0f, light.color.b/255.0f, light.color.a/255.0f};
    
    SetShaderValue(shader, light.enabledLoc, &enabledVal, SHADER_UNIFORM_INT);
    SetShaderValue(shader, light.typeLoc, &typeVal, SHADER_UNIFORM_INT);
    SetShaderValue(shader, light.positionLoc, pos, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, light.targetLoc, target, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, light.colorLoc, col, SHADER_UNIFORM_VEC4);
    SetShaderValue(shader, light.intensityLoc, &light.intensity, SHADER_UNIFORM_FLOAT);
}

// ============================================================
// LOAD PBR SHADER
// ============================================================

void LoadNetWorldPBR(void)
{
    if (g_pbrLoaded) return;
    
    // ==== ASSET LOAD: PBR VERTEX+FRAGMENT SHADER ====
    // GPU resource acquired here. Every light created below via
    // CreateLight() only holds shader-uniform locations (ints), not GPU
    // handles of its own, so it needs no separate unload step — it lives
    // and dies with g_pbrShader. Must be paired with UnloadNetWorldPBR().
    g_pbrShader = LoadShader(
        "src/client/render/shaders/networld.vs",
        "src/client/render/shaders/networld.fs"
    );
    
    if (g_pbrShader.id != 0)
    {
        // Setup texture locations
        g_pbrShader.locs[SHADER_LOC_MAP_ALBEDO] = GetShaderLocation(g_pbrShader, "albedoMap");
        g_pbrShader.locs[SHADER_LOC_MAP_METALNESS] = GetShaderLocation(g_pbrShader, "mraMap");
        g_pbrShader.locs[SHADER_LOC_MAP_NORMAL] = GetShaderLocation(g_pbrShader, "normalMap");
        g_pbrShader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(g_pbrShader, "emissiveMap");
        g_pbrShader.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocation(g_pbrShader, "albedoColor");
        g_pbrShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(g_pbrShader, "viewPos");
        
        // Get other uniform locations
        g_viewPosLoc = g_pbrShader.locs[SHADER_LOC_VECTOR_VIEW];
        g_ambientColorLoc = GetShaderLocation(g_pbrShader, "ambientColor");
        g_ambientIntensityLoc = GetShaderLocation(g_pbrShader, "ambientIntensity");
        g_albedoColorLoc = g_pbrShader.locs[SHADER_LOC_COLOR_DIFFUSE];
        g_metallicValueLoc = GetShaderLocation(g_pbrShader, "metallicValue");
        g_roughnessValueLoc = GetShaderLocation(g_pbrShader, "roughnessValue");
        g_emissivePowerLoc = GetShaderLocation(g_pbrShader, "emissivePower");
        g_emissiveColorLoc = GetShaderLocation(g_pbrShader, "emissiveColor");
        
        // Setup light count
        int lightCountLoc = GetShaderLocation(g_pbrShader, "numOfLights");
        int maxLightCount = MAX_LIGHTS;
        SetShaderValue(g_pbrShader, lightCountLoc, &maxLightCount, SHADER_UNIFORM_INT);
        
        // ---- BRIGHTER AMBIENT ----
        float ambientIntensity = 0.15f; // Was 0.08f
        Color ambientColor = {30, 40, 80, 255}; // Brighter blue
        Vector3 ambientNormalized = {ambientColor.r/255.0f, ambientColor.g/255.0f, ambientColor.b/255.0f};
        SetShaderValue(g_pbrShader, g_ambientColorLoc, &ambientNormalized, SHADER_UNIFORM_VEC3);
        SetShaderValue(g_pbrShader, g_ambientIntensityLoc, &ambientIntensity, SHADER_UNIFORM_FLOAT);

        // ---- BRIGHTER CYBERPUNK LIGHTS ----
        // Neon cyan (much brighter)
        g_lights[0] = CreateLight(LIGHT_POINT, Vector3{-8.0f, 5.0f, -6.0f}, Vector3{0,0,0}, 
                                Color{0, 220, 255, 255}, 25.0f, g_pbrShader);

        // Neon magenta/pink
        g_lights[1] = CreateLight(LIGHT_POINT, Vector3{8.0f, 5.0f, -6.0f}, Vector3{0,0,0}, 
                                Color{255, 0, 120, 255}, 25.0f, g_pbrShader);

        // Toxic green (very bright)
        g_lights[2] = CreateLight(LIGHT_POINT, Vector3{0.0f, 8.0f, 8.0f}, Vector3{0,0,0}, 
                                Color{40, 255, 120, 255}, 30.0f, g_pbrShader);

        // Amber glow from core
        g_lights[3] = CreateLight(LIGHT_POINT, Vector3{0.0f, 3.0f, 0.0f}, Vector3{0,0,0}, 
                                Color{255, 180, 40, 255}, 15.0f, g_pbrShader);
        
        g_pbrLoaded = true;
        printf("[PBR] NetWorld PBR shader loaded successfully.\n");
    }
    else
    {
        printf("[PBR] Failed to load NetWorld PBR shader.\n");
        printf("[PBR] Make sure shader files exist at:\n");
        printf("[PBR]   src/client/render/shaders/networld.vs\n");
        printf("[PBR]   src/client/render/shaders/networld.fs\n");
    }
    // ==== END ASSET LOAD ====
}

// ============================================================
// APPLY PBR SHADER
// ============================================================

void ApplyNetWorldPBR(Camera3D camera)
{
    if (!g_pbrLoaded) return;
    if (!g_netWorld.active) return;
    if (g_netWorld.state != NetWorldState::ACTIVE) return;
    
    // Update view position
    float cameraPos[3] = {camera.position.x, camera.position.y, camera.position.z};
    SetShaderValue(g_pbrShader, g_viewPosLoc, cameraPos, SHADER_UNIFORM_VEC3);
    
    // Update all lights
    for (int i = 0; i < g_lightCount; i++)
    {
        UpdateLight(g_pbrShader, g_lights[i]);
    }
    
    // Begin shader mode
    BeginShaderMode(g_pbrShader);
}

// ============================================================
// END PBR SHADER
// ============================================================

void EndNetWorldPBR(void)
{
    if (g_pbrLoaded)
    {
        EndShaderMode();
    }
}

// ============================================================
// UNLOAD PBR SHADER
// ============================================================

void UnloadNetWorldPBR(void)
{
    if (g_pbrLoaded)
    {
        // ==== ASSET UNLOAD: PBR VERTEX+FRAGMENT SHADER ====
        // Must run before the graphics context shuts down, and must not be
        // called more than once for the same successful LoadShader() call.
        UnloadShader(g_pbrShader);
        g_pbrLoaded = false;
        printf("[PBR] NetWorld PBR shader unloaded.\n");
        // ==== END ASSET UNLOAD ====
    }
}