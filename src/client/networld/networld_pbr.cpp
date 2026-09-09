#include "networld.h"
#include "../render.h"
#include "../../shared/vnet.h"
#include <cmath>
#include <cstdlib>
#include <cstring>

static Shader g_pbrShader = {0};
static bool g_pbrLoaded = false;
static Texture2D g_defaultWhite = {0};
static Texture2D g_defaultNormal = {0};
static Texture2D g_defaultBlack = {0};

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
// LOAD PBR SHADER
// ============================================================

void LoadNetWorldPBR(void)
{
    if (g_pbrLoaded) return;
    
    // ==== ASSET LOAD: PBR SHADER + DEFAULT TEXTURES ====
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
        
        g_viewPosLoc = g_pbrShader.locs[SHADER_LOC_VECTOR_VIEW];
        g_ambientColorLoc = GetShaderLocation(g_pbrShader, "ambientColor");
        g_ambientIntensityLoc = GetShaderLocation(g_pbrShader, "ambientIntensity");
        g_albedoColorLoc = g_pbrShader.locs[SHADER_LOC_COLOR_DIFFUSE];
        g_metallicValueLoc = GetShaderLocation(g_pbrShader, "metallicValue");
        g_roughnessValueLoc = GetShaderLocation(g_pbrShader, "roughnessValue");
        g_emissivePowerLoc = GetShaderLocation(g_pbrShader, "emissivePower");
        g_emissiveColorLoc = GetShaderLocation(g_pbrShader, "emissiveColor");
        
        // ---- Create default textures ----
        Image whiteImg = GenImageColor(1, 1, WHITE);
        g_defaultWhite = LoadTextureFromImage(whiteImg);
        UnloadImage(whiteImg);
        
        Image normalImg = GenImageColor(1, 1, Color{128, 128, 255, 255}); // (0.5,0.5,1)
        g_defaultNormal = LoadTextureFromImage(normalImg);
        UnloadImage(normalImg);
        
        Image blackImg = GenImageColor(1, 1, BLACK);
        g_defaultBlack = LoadTextureFromImage(blackImg);
        UnloadImage(blackImg);
        
        // ---- Bind default textures to shader ----
        SetShaderValueTexture(g_pbrShader, g_pbrShader.locs[SHADER_LOC_MAP_ALBEDO], g_defaultWhite);
        SetShaderValueTexture(g_pbrShader, g_pbrShader.locs[SHADER_LOC_MAP_NORMAL], g_defaultNormal);
        SetShaderValueTexture(g_pbrShader, g_pbrShader.locs[SHADER_LOC_MAP_METALNESS], g_defaultWhite);
        SetShaderValueTexture(g_pbrShader, g_pbrShader.locs[SHADER_LOC_MAP_EMISSION], g_defaultBlack);
        
        // ---- Default material parameters ----
        Vector4 defaultAlbedo = {1.0f, 1.0f, 1.0f, 1.0f};
        SetShaderValue(g_pbrShader, g_albedoColorLoc, &defaultAlbedo, SHADER_UNIFORM_VEC4);
        float metallic = 0.0f;
        SetShaderValue(g_pbrShader, g_metallicValueLoc, &metallic, SHADER_UNIFORM_FLOAT);
        float roughness = 0.5f;
        SetShaderValue(g_pbrShader, g_roughnessValueLoc, &roughness, SHADER_UNIFORM_FLOAT);
        float emissivePower = 0.0f;
        SetShaderValue(g_pbrShader, g_emissivePowerLoc, &emissivePower, SHADER_UNIFORM_FLOAT);
        Vector4 emissiveCol = {0.0f, 0.0f, 0.0f, 0.0f};
        SetShaderValue(g_pbrShader, g_emissiveColorLoc, &emissiveCol, SHADER_UNIFORM_VEC4);
        
        // ---- Ambient ----
        float ambientIntensity = 0.15f;
        Color ambientColor = {30, 40, 80, 255};
        Vector3 ambientNormalized = {ambientColor.r/255.0f, ambientColor.g/255.0f, ambientColor.b/255.0f};
        SetShaderValue(g_pbrShader, g_ambientColorLoc, &ambientNormalized, SHADER_UNIFORM_VEC3);
        SetShaderValue(g_pbrShader, g_ambientIntensityLoc, &ambientIntensity, SHADER_UNIFORM_FLOAT);
        
        g_pbrLoaded = true;
        printf("[PBR] NetWorld PBR shader loaded successfully.\n");
    }
    else
    {
        printf("[PBR] Failed to load NetWorld PBR shader.\n");
    }
    // ==== END ASSET LOAD ====
}

// ============================================================
// APPLY PBR SHADER (uploads dynamic lights)
// ============================================================

void ApplyNetWorldPBR(Camera3D camera)
{
    if (!g_pbrLoaded) return;
    if (!g_netWorld.active || g_netWorld.state != NetWorldState::ACTIVE) return;
    
    // Update view position
    float cameraPos[3] = {camera.position.x, camera.position.y, camera.position.z};
    SetShaderValue(g_pbrShader, g_viewPosLoc, cameraPos, SHADER_UNIFORM_VEC3);
    
    // Upload number of lights
    int lightCount = g_netWorld.dynamicLightCount;
    int lightCountLoc = GetShaderLocation(g_pbrShader, "numOfLights");
    SetShaderValue(g_pbrShader, lightCountLoc, &lightCount, SHADER_UNIFORM_INT);
    
    // Upload each light
    for (int i = 0; i < lightCount && i < MAX_DYNAMIC_LIGHTS; i++) {
        DynamicLight& l = g_netWorld.dynamicLights[i];
        char name[64];
        
        snprintf(name, sizeof(name), "lights[%i].enabled", i);
        int enabled = l.active ? 1 : 0;
        SetShaderValue(g_pbrShader, GetShaderLocation(g_pbrShader, name), &enabled, SHADER_UNIFORM_INT);
        
        snprintf(name, sizeof(name), "lights[%i].type", i);
        int type = 1; // point light
        SetShaderValue(g_pbrShader, GetShaderLocation(g_pbrShader, name), &type, SHADER_UNIFORM_INT);
        
        snprintf(name, sizeof(name), "lights[%i].position", i);
        SetShaderValue(g_pbrShader, GetShaderLocation(g_pbrShader, name), &l.position, SHADER_UNIFORM_VEC3);
        
        snprintf(name, sizeof(name), "lights[%i].color", i);
        Vector4 col = {l.color.x, l.color.y, l.color.z, 1.0f};
        SetShaderValue(g_pbrShader, GetShaderLocation(g_pbrShader, name), &col, SHADER_UNIFORM_VEC4);
        
        snprintf(name, sizeof(name), "lights[%i].intensity", i);
        SetShaderValue(g_pbrShader, GetShaderLocation(g_pbrShader, name), &l.intensity, SHADER_UNIFORM_FLOAT);
    }
    
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
// UNLOAD PBR SHADER AND DEFAULT TEXTURES
// ============================================================

void UnloadNetWorldPBR(void)
{
    if (g_pbrLoaded)
    {
        // ==== ASSET UNLOAD: PBR SHADER & TEXTURES ====
        UnloadShader(g_pbrShader);
        UnloadTexture(g_defaultWhite);
        UnloadTexture(g_defaultNormal);
        UnloadTexture(g_defaultBlack);
        g_pbrLoaded = false;
        printf("[PBR] NetWorld PBR shader and default textures unloaded.\n");
        // ==== END ASSET UNLOAD ====
    }
}