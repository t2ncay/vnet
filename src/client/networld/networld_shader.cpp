#include "networld.h"
#include "../render.h"
#include "../../shared/vnet.h"
#include <cmath>
#include <cstdio>

// ---------------------------------------------------------------------------
// Post-process shader handles.
//
// Material shaders live in networld_material.cpp — this file owns ONLY the
// full-screen passes that sit after the 3D scene has been rendered to a
// render texture.
// ---------------------------------------------------------------------------
static Shader g_crtShader             = {0};
static Shader g_distortionShader      = {0};
static Shader g_bloomBrightShader     = {0};
static Shader g_bloomBlurShader       = {0};
static Shader g_bloomCompositeShader  = {0};

static bool g_shadersLoaded = false;
static bool g_crtActive     = false;

// ---------------------------------------------------------------------------
// Cached uniform locations.
//
// GetShaderLocation is a string lookup into the linked GL program. Cheap,
// but there's no reason to pay it per-frame when the locations never change.
// ---------------------------------------------------------------------------
struct CrtLocations {
    int resolution        = -1;
    int time              = -1;
    int glitchIntensity   = -1;
    int scanlineIntensity = -1;
    int vignetteIntensity = -1;
    int barrelAmount      = -1;
    int chromaticAmount   = -1;
    int ditherAmount      = -1;
    int exposure          = -1;   // <-- was missing; uninitialized uniform = 0
};
static CrtLocations g_crtLoc;

struct DistortionLocations {
    int time            = -1;
    int glitchIntensity = -1;
    int chromaticOffset = -1;
    int blockSize       = -1;
    int resolution      = -1;
};
static DistortionLocations g_distLoc;

static int g_brightThresholdLoc      = -1;
static int g_brightKneeLoc           = -1;
static int g_blurDirectionLoc        = -1;
static int g_blurResolutionLoc       = -1;
static int g_compositeBloomLoc       = -1;
static int g_compositeIntensityLoc   = -1;

// ===========================================================================
// LOAD
// ===========================================================================

void LoadNetWorldShader(void)
{
    if (g_shadersLoaded) return;

    // ==== ASSET LOAD: NETWORLD POST-PROCESS SHADERS ====
    g_crtShader            = LoadShader(0, "src/client/render/shaders/networld_crt.fs");
    g_distortionShader     = LoadShader(0, "src/client/render/shaders/distortion_glitch.fs");
    g_bloomBrightShader    = LoadShader(0, "src/client/render/shaders/bloom_bright.fs");
    g_bloomBlurShader      = LoadShader(0, "src/client/render/shaders/gaussian_blur.fs");
    g_bloomCompositeShader = LoadShader(0, "src/client/render/shaders/bloom_composite.fs");
    // ==== END ASSET LOAD ====

    if (g_crtShader.id == 0)             printf("[SHADER] networld_crt.fs FAILED to load.\n");
    if (g_distortionShader.id == 0)      printf("[SHADER] distortion_glitch.fs FAILED to load.\n");
    if (g_bloomBrightShader.id == 0)     printf("[SHADER] bloom_bright.fs FAILED to load.\n");
    if (g_bloomBlurShader.id == 0)       printf("[SHADER] gaussian_blur.fs FAILED to load.\n");
    if (g_bloomCompositeShader.id == 0)  printf("[SHADER] bloom_composite.fs FAILED to load.\n");

    if (g_crtShader.id != 0) {
        g_crtLoc.resolution        = GetShaderLocation(g_crtShader, "resolution");
        g_crtLoc.time              = GetShaderLocation(g_crtShader, "time");
        g_crtLoc.glitchIntensity   = GetShaderLocation(g_crtShader, "glitchIntensity");
        g_crtLoc.scanlineIntensity = GetShaderLocation(g_crtShader, "scanlineIntensity");
        g_crtLoc.vignetteIntensity = GetShaderLocation(g_crtShader, "vignetteIntensity");
        g_crtLoc.barrelAmount      = GetShaderLocation(g_crtShader, "barrelAmount");
        g_crtLoc.chromaticAmount   = GetShaderLocation(g_crtShader, "chromaticAmount");
        g_crtLoc.ditherAmount      = GetShaderLocation(g_crtShader, "ditherAmount");
        g_crtLoc.exposure          = GetShaderLocation(g_crtShader, "exposure");

        // Sanity check — if this is -1 the shader source and the location
        // string have drifted apart. Print once at load so you notice.
        if (g_crtLoc.exposure == -1) {
            printf("[SHADER] WARNING: 'exposure' uniform not found in networld_crt.fs.\n");
        }
    }
    if (g_distortionShader.id != 0) {
        g_distLoc.time            = GetShaderLocation(g_distortionShader, "time");
        g_distLoc.glitchIntensity = GetShaderLocation(g_distortionShader, "glitchIntensity");
        g_distLoc.chromaticOffset = GetShaderLocation(g_distortionShader, "chromaticOffset");
        g_distLoc.blockSize       = GetShaderLocation(g_distortionShader, "blockSize");
        g_distLoc.resolution      = GetShaderLocation(g_distortionShader, "resolution");
    }
    if (g_bloomBrightShader.id != 0) {
        g_brightThresholdLoc = GetShaderLocation(g_bloomBrightShader, "threshold");
        g_brightKneeLoc      = GetShaderLocation(g_bloomBrightShader, "knee");
    }
    if (g_bloomBlurShader.id != 0) {
        g_blurDirectionLoc  = GetShaderLocation(g_bloomBlurShader, "direction");
        g_blurResolutionLoc = GetShaderLocation(g_bloomBlurShader, "resolution");
    }
    if (g_bloomCompositeShader.id != 0) {
        g_compositeBloomLoc     = GetShaderLocation(g_bloomCompositeShader, "bloomTexture");
        g_compositeIntensityLoc = GetShaderLocation(g_bloomCompositeShader, "bloomIntensity");
    }

    g_shadersLoaded = true;
    printf("[SHADER] NetWorld post-process shaders loaded.\n");
}

// ===========================================================================
// UNLOAD
// ===========================================================================

void UnloadNetWorldShader(void)
{
    if (!g_shadersLoaded) return;
    if (g_crtActive) { EndShaderMode(); g_crtActive = false; }

    // ==== ASSET UNLOAD: NETWORLD POST-PROCESS SHADERS ====
    UnloadShader(g_crtShader);
    UnloadShader(g_distortionShader);
    UnloadShader(g_bloomBrightShader);
    UnloadShader(g_bloomBlurShader);
    UnloadShader(g_bloomCompositeShader);
    // ==== END ASSET UNLOAD ====

    g_shadersLoaded = false;
    printf("[SHADER] NetWorld post-process shaders unloaded.\n");
}

// ===========================================================================
// CRT PASS CONTROL
// ===========================================================================

void DisableNetWorldShader(void)
{
    if (g_crtActive) { EndShaderMode(); g_crtActive = false; }
}

void ApplyNetWorldShader(void)
{
    if (!g_shadersLoaded || g_crtShader.id == 0) return;
    if (!g_netWorld.active) return;
    if (g_netWorld.state == NetWorldState::ENTERING ||
        g_netWorld.state == NetWorldState::EXITING) return;
    if (g_crtActive) return;

    BeginShaderMode(g_crtShader);
    g_crtActive = true;

    float res[2] = {(float)GetScreenWidth(), (float)GetScreenHeight()};
    SetShaderValue(g_crtShader, g_crtLoc.resolution, res, SHADER_UNIFORM_VEC2);

    float t = g_netWorld.time;
    SetShaderValue(g_crtShader, g_crtLoc.time, &t, SHADER_UNIFORM_FLOAT);

    // Gameplay-driven glitch plus a rare single-frame spike so the shader
    // reads as "alive" even when nothing is happening.
    float glitch = g_netWorld.glitchIntensity;
    if ((rand() % 1000) < 4) glitch = fminf(glitch + 0.5f, 1.0f);
    SetShaderValue(g_crtShader, g_crtLoc.glitchIntensity, &glitch, SHADER_UNIFORM_FLOAT);

    float scan = 0.32f;
    SetShaderValue(g_crtShader, g_crtLoc.scanlineIntensity, &scan, SHADER_UNIFORM_FLOAT);

    float vig = 0.30f;
    SetShaderValue(g_crtShader, g_crtLoc.vignetteIntensity, &vig, SHADER_UNIFORM_FLOAT);

    float barrel = 0.045f;
    SetShaderValue(g_crtShader, g_crtLoc.barrelAmount, &barrel, SHADER_UNIFORM_FLOAT);

    float chroma = 0.0015f;
    SetShaderValue(g_crtShader, g_crtLoc.chromaticAmount, &chroma, SHADER_UNIFORM_FLOAT);

    float dither = 1.0f / 200.0f;
    SetShaderValue(g_crtShader, g_crtLoc.ditherAmount, &dither, SHADER_UNIFORM_FLOAT);

    // Neutral exposure is 1.0. This was the missing line — without it the
    // uniform stays at its GLSL default of 0.0 and the entire CRT pass
    // multiplies the frame by zero.
    float exposure = 1.10f;
    SetShaderValue(g_crtShader, g_crtLoc.exposure, &exposure, SHADER_UNIFORM_FLOAT);
}

void EndNetWorldShader(void)
{
    if (g_shadersLoaded && g_crtActive) {
        EndShaderMode();
        g_crtActive = false;
    }
}

// ===========================================================================
// BLOOM
// ===========================================================================

void ApplyBloom(RenderTexture2D scene, RenderTexture2D output)
{
    if (!g_shadersLoaded) return;
    if (g_bloomBrightShader.id == 0 || g_bloomBlurShader.id == 0) return;

    Rectangle srcScene = {0, 0, (float)scene.texture.width,  -(float)scene.texture.height};
    Rectangle dstScene = {0, 0, (float)output.texture.width,  (float)output.texture.height};

    Rectangle srcBloom = {0, 0, (float)g_netWorld.bloomRT.texture.width,  -(float)g_netWorld.bloomRT.texture.height};
    Rectangle dstBloom = {0, 0, (float)g_netWorld.bloomRT.texture.width,   (float)g_netWorld.bloomRT.texture.height};

    Rectangle srcTemp  = {0, 0, (float)g_netWorld.blurTemp.texture.width, -(float)g_netWorld.blurTemp.texture.height};
    Rectangle dstTemp  = {0, 0, (float)g_netWorld.blurTemp.texture.width,  (float)g_netWorld.blurTemp.texture.height};

    // ---- 1. Bright pass: scene -> bloomRT ----
    BeginTextureMode(g_netWorld.bloomRT);
        ClearBackground(BLACK);
        BeginShaderMode(g_bloomBrightShader);
            float threshold = 0.45f;
            float knee      = 0.30f;
            SetShaderValue(g_bloomBrightShader, g_brightThresholdLoc, &threshold, SHADER_UNIFORM_FLOAT);
            SetShaderValue(g_bloomBrightShader, g_brightKneeLoc,      &knee,      SHADER_UNIFORM_FLOAT);
            DrawTexturePro(scene.texture, srcScene, dstBloom, {0, 0}, 0.0f, WHITE);
        EndShaderMode();
    EndTextureMode();

    // ---- 2. Horizontal blur: bloomRT -> blurTemp ----
    BeginTextureMode(g_netWorld.blurTemp);
        ClearBackground(BLACK);
        BeginShaderMode(g_bloomBlurShader);
            Vector2 dirH = {1.0f, 0.0f};
            Vector2 resH = {(float)g_netWorld.bloomRT.texture.width,
                            (float)g_netWorld.bloomRT.texture.height};
            SetShaderValue(g_bloomBlurShader, g_blurDirectionLoc,  &dirH, SHADER_UNIFORM_VEC2);
            SetShaderValue(g_bloomBlurShader, g_blurResolutionLoc, &resH, SHADER_UNIFORM_VEC2);
            DrawTexturePro(g_netWorld.bloomRT.texture, srcBloom, dstTemp, {0, 0}, 0.0f, WHITE);
        EndShaderMode();
    EndTextureMode();

    // ---- 3. Vertical blur: blurTemp -> bloomRT ----
    BeginTextureMode(g_netWorld.bloomRT);
        ClearBackground(BLACK);
        BeginShaderMode(g_bloomBlurShader);
            Vector2 dirV = {0.0f, 1.0f};
            Vector2 resV = {(float)g_netWorld.blurTemp.texture.width,
                            (float)g_netWorld.blurTemp.texture.height};
            SetShaderValue(g_bloomBlurShader, g_blurDirectionLoc,  &dirV, SHADER_UNIFORM_VEC2);
            SetShaderValue(g_bloomBlurShader, g_blurResolutionLoc, &resV, SHADER_UNIFORM_VEC2);
            DrawTexturePro(g_netWorld.blurTemp.texture, srcTemp, dstBloom, {0, 0}, 0.0f, WHITE);
        EndShaderMode();
    EndTextureMode();

    // ---- 4. Composite: scene + bloom -> output ----
    BeginTextureMode(output);
        ClearBackground(BLACK);
        if (g_bloomCompositeShader.id != 0) {
            BeginShaderMode(g_bloomCompositeShader);
                SetShaderValueTexture(g_bloomCompositeShader, g_compositeBloomLoc, g_netWorld.bloomRT.texture);
                float intensity = g_netWorld.bloomIntensity * 2.5f;
                SetShaderValue(g_bloomCompositeShader, g_compositeIntensityLoc, &intensity, SHADER_UNIFORM_FLOAT);
                DrawTexturePro(scene.texture, srcScene, dstScene, {0, 0}, 0.0f, WHITE);
            EndShaderMode();
        } else {
            DrawTexturePro(scene.texture, srcScene, dstScene, {0, 0}, 0.0f, WHITE);
            BeginBlendMode(BLEND_ADDITIVE);
                DrawTexturePro(g_netWorld.bloomRT.texture, srcBloom, dstScene, {0, 0}, 0.0f,
                               Fade(WHITE, g_netWorld.bloomIntensity));
            EndBlendMode();
        }
    EndTextureMode();
}

// ===========================================================================
// DISTORTION
// ===========================================================================

void ApplyDistortion(RenderTexture2D source, RenderTexture2D dest)
{
    if (!g_shadersLoaded || g_distortionShader.id == 0) {
        BeginTextureMode(dest);
            ClearBackground(BLACK);
            Rectangle src = {0, 0, (float)source.texture.width, -(float)source.texture.height};
            Rectangle dst = {0, 0, (float)dest.texture.width,   (float)dest.texture.height};
            DrawTexturePro(source.texture, src, dst, {0, 0}, 0.0f, WHITE);
        EndTextureMode();
        return;
    }

    float t = g_netWorld.time;
    float glitch = g_netWorld.glitchIntensity;
    float chroma = 0.005f;
    float block  = 24.0f;
    Vector2 res = {(float)source.texture.width, (float)source.texture.height};

    SetShaderValue(g_distortionShader, g_distLoc.time,            &t,      SHADER_UNIFORM_FLOAT);
    SetShaderValue(g_distortionShader, g_distLoc.glitchIntensity, &glitch, SHADER_UNIFORM_FLOAT);
    SetShaderValue(g_distortionShader, g_distLoc.chromaticOffset, &chroma, SHADER_UNIFORM_FLOAT);
    SetShaderValue(g_distortionShader, g_distLoc.blockSize,       &block,  SHADER_UNIFORM_FLOAT);
    SetShaderValue(g_distortionShader, g_distLoc.resolution,      &res,    SHADER_UNIFORM_VEC2);

    BeginTextureMode(dest);
        BeginShaderMode(g_distortionShader);
            Rectangle src = {0, 0, (float)source.texture.width, -(float)source.texture.height};
            Rectangle dst = {0, 0, (float)dest.texture.width,   (float)dest.texture.height};
            DrawTexturePro(source.texture, src, dst, {0, 0}, 0.0f, WHITE);
        EndShaderMode();
    EndTextureMode();
}