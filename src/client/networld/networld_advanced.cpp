#include "networld.h"
#include "../render.h"
#include "raymath.h"

static Shader g_volFogShader = {0};
static Shader g_bloomBrightShader = {0};
static Shader g_bloomBlurShader = {0};
static Shader g_bloomCompositeShader = {0};
static Shader g_distortionShader = {0};
static bool g_advancedLoaded = false;

void LoadAdvancedShaders() {
    if (g_advancedLoaded) return;
    // ==== ASSET LOAD: ADVANCED SHADERS ====
    g_volFogShader = LoadShader(0, "src/client/render/shaders/volumetric_fog.fs");
    g_bloomBrightShader = LoadShader(0, "src/client/render/shaders/bloom_bright.fs");
    g_bloomBlurShader = LoadShader(0, "src/client/render/shaders/gaussian_blur.fs");
    g_bloomCompositeShader = LoadShader(0, "src/client/render/shaders/bloom_composite.fs");
    g_distortionShader = LoadShader(0, "src/client/render/shaders/distortion_glitch.fs");
    // ==== END ASSET LOAD ====
    g_advancedLoaded = true;
}

void UnloadAdvancedShaders() {
    if (!g_advancedLoaded) return;
    // ==== ASSET UNLOAD: ADVANCED SHADERS ====
    UnloadShader(g_volFogShader);
    UnloadShader(g_bloomBrightShader);
    UnloadShader(g_bloomBlurShader);
    UnloadShader(g_bloomCompositeShader);
    UnloadShader(g_distortionShader);
    // ==== END ASSET UNLOAD ====
    g_advancedLoaded = false;
}