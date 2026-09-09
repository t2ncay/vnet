#include "networld.h"
#include "../render.h"
#include "../../shared/vnet.h"
#include <cmath>
#include <cstdlib>

// CRT shader (existing)
static Shader g_crtShader = {0};
static bool g_shaderLoaded = false;
static bool g_shaderActive = false;
static int g_timeLoc = -1;
static int g_bloomLoc = -1;
static int g_resolutionLoc = -1;
static int g_glitchLoc = -1;
static int g_scanlineLoc = -1;
static int g_vignetteLoc = -1;
static int g_chromaticLoc = -1;
static int g_warpLoc = -1;

// Advanced shader handles
static Shader g_volFogShader = {0};
static Shader g_bloomBrightShader = {0};
static Shader g_bloomBlurShader = {0};
static Shader g_bloomCompositeShader = {0};
static Shader g_distortionShader = {0};

// ============================================================
// LOAD ALL SHADERS
// ============================================================

void LoadNetWorldShader(void) {
    if (g_shaderLoaded) return;
    
    // ==== ASSET LOAD: CRT + ADVANCED SHADERS ====
    g_crtShader = LoadShader(0, "src/client/render/shaders/networld.fs"); // CRT overlay
    g_volFogShader = LoadShader(0, "src/client/render/shaders/volumetric_fog.fs");
    g_bloomBrightShader = LoadShader(0, "src/client/render/shaders/bloom_bright.fs");
    g_bloomBlurShader = LoadShader(0, "src/client/render/shaders/gaussian_blur.fs");
    g_bloomCompositeShader = LoadShader(0, "src/client/render/shaders/bloom_composite.fs");
    g_distortionShader = LoadShader(0, "src/client/render/shaders/distortion_glitch.fs");
    // ==== END ASSET LOAD ====
    
    if (g_crtShader.id != 0) {
        // CRT uniforms
        g_timeLoc = GetShaderLocation(g_crtShader, "time");
        g_resolutionLoc = GetShaderLocation(g_crtShader, "resolution");
        g_glitchLoc = GetShaderLocation(g_crtShader, "glitchIntensity");
        g_scanlineLoc = GetShaderLocation(g_crtShader, "scanlineIntensity");
        g_vignetteLoc = GetShaderLocation(g_crtShader, "vignetteIntensity");
        g_chromaticLoc = GetShaderLocation(g_crtShader, "chromaticAberration");
        g_warpLoc = GetShaderLocation(g_crtShader, "warpAmount");
        g_bloomLoc = GetShaderLocation(g_crtShader, "bloomIntensity");
        g_shaderLoaded = true;
        printf("[SHADER] NetWorld CRT shader loaded successfully.\n");
    } else {
        printf("[SHADER] Failed to load CRT shader.\n");
    }
    
    // Check advanced shaders
    if (g_volFogShader.id == 0)   printf("[SHADER] Volumetric fog shader failed to load.\n");
    if (g_bloomBrightShader.id == 0) printf("[SHADER] Bloom bright shader failed to load.\n");
    if (g_bloomBlurShader.id == 0)    printf("[SHADER] Bloom blur shader failed to load.\n");
    if (g_bloomCompositeShader.id == 0) printf("[SHADER] Bloom composite shader failed to load.\n");
    if (g_distortionShader.id == 0)    printf("[SHADER] Distortion shader failed to load.\n");
}

// ============================================================
// UNLOAD ALL SHADERS
// ============================================================

void UnloadNetWorldShader(void) {
    if (g_shaderLoaded) {
        if (g_shaderActive) {
            EndShaderMode();
            g_shaderActive = false;
        }
        // ==== ASSET UNLOAD: ALL SHADERS ====
        UnloadShader(g_crtShader);
        UnloadShader(g_volFogShader);
        UnloadShader(g_bloomBrightShader);
        UnloadShader(g_bloomBlurShader);
        UnloadShader(g_bloomCompositeShader);
        UnloadShader(g_distortionShader);
        // ==== END ASSET UNLOAD ====
        g_shaderLoaded = false;
        printf("[SHADER] All NetWorld shaders unloaded.\n");
    }
}

// ============================================================
// CRT SHADER CONTROL (unchanged)
// ============================================================

void DisableNetWorldShader(void) {
    if (g_shaderActive) {
        EndShaderMode();
        g_shaderActive = false;
    }
}

void ApplyNetWorldShader(void) {
    if (!g_shaderLoaded) return;
    if (!g_netWorld.active) return;
    if (g_netWorld.state == NetWorldState::ENTERING || 
        g_netWorld.state == NetWorldState::EXITING) return;
    if (g_shaderActive) return;
    
    BeginShaderMode(g_crtShader);
    g_shaderActive = true;
    
    if (g_timeLoc != -1) SetShaderValue(g_crtShader, g_timeLoc, &g_netWorld.time, SHADER_UNIFORM_FLOAT);
    if (g_resolutionLoc != -1) {
        float res[2] = {(float)GetScreenWidth(), (float)GetScreenHeight()};
        SetShaderValue(g_crtShader, g_resolutionLoc, res, SHADER_UNIFORM_VEC2);
    }
    if (g_glitchLoc != -1) {
        float intensity = g_netWorld.glitchIntensity;
        if ((rand() % 1000) < 5) intensity += 0.6f;
        intensity = fminf(intensity, 1.0f);
        SetShaderValue(g_crtShader, g_glitchLoc, &intensity, SHADER_UNIFORM_FLOAT);
    }
    if (g_scanlineLoc != -1) {
        float scanline = 0.8f + sinf(g_netWorld.time * 2.0f) * 0.1f;
        SetShaderValue(g_crtShader, g_scanlineLoc, &scanline, SHADER_UNIFORM_FLOAT);
    }
    if (g_vignetteLoc != -1) {
        float vignette = 0.8f;
        SetShaderValue(g_crtShader, g_vignetteLoc, &vignette, SHADER_UNIFORM_FLOAT);
    }
    if (g_chromaticLoc != -1) {
        float chroma = 1.0f + g_netWorld.glitchIntensity * 6.0f;
        SetShaderValue(g_crtShader, g_chromaticLoc, &chroma, SHADER_UNIFORM_FLOAT);
    }
    if (g_warpLoc != -1) {
        float warp = 0.12f;
        SetShaderValue(g_crtShader, g_warpLoc, &warp, SHADER_UNIFORM_FLOAT);
    }
}

void EndNetWorldShader(void) {
    if (g_shaderLoaded && g_shaderActive) {
        EndShaderMode();
        g_shaderActive = false;
    }
}

// ============================================================
// ADVANCED PASS FUNCTIONS
// ============================================================

void ApplyVolumetricFog(RenderTexture2D scene, RenderTexture2D output) {
    if (g_volFogShader.id == 0) return;
    BeginTextureMode(output);
        ClearBackground(BLACK);
        BeginShaderMode(g_volFogShader);
            float time = g_netWorld.time;
            SetShaderValue(g_volFogShader, GetShaderLocation(g_volFogShader, "time"), &time, SHADER_UNIFORM_FLOAT);
            // Additional uniforms (fogColor, density, etc.) can be set here.
            DrawTexture(scene.texture, 0, 0, WHITE);
        EndShaderMode();
    EndTextureMode();
}

void ApplyBloom(RenderTexture2D scene, RenderTexture2D output) {
    if (g_bloomBrightShader.id == 0 || g_bloomBlurShader.id == 0) return;
    
    // Reusable rectangles to handle proper scaling and Y-axis flipping
    Rectangle srcScene = { 0, 0, (float)scene.texture.width, -(float)scene.texture.height };
    Rectangle destScene = { 0, 0, (float)output.texture.width, (float)output.texture.height };
    
    Rectangle srcBloom = { 0, 0, (float)g_netWorld.bloomRT.texture.width, -(float)g_netWorld.bloomRT.texture.height };
    Rectangle destBloom = { 0, 0, (float)g_netWorld.bloomRT.texture.width, (float)g_netWorld.bloomRT.texture.height };
    
    Rectangle srcBlurTemp = { 0, 0, (float)g_netWorld.blurTemp.texture.width, -(float)g_netWorld.blurTemp.texture.height };
    Rectangle destBlurTemp = { 0, 0, (float)g_netWorld.blurTemp.texture.width, (float)g_netWorld.blurTemp.texture.height };

    // Bright pass
    BeginTextureMode(g_netWorld.bloomRT);
        ClearBackground(BLACK);
        BeginShaderMode(g_bloomBrightShader);
            float threshold = 0.8f;
            SetShaderValue(g_bloomBrightShader, GetShaderLocation(g_bloomBrightShader, "threshold"), &threshold, SHADER_UNIFORM_FLOAT);
            DrawTexturePro(scene.texture, srcScene, destBloom, {0, 0}, 0.0f, WHITE);
        EndShaderMode();
    EndTextureMode();

    // Horizontal blur
    BeginTextureMode(g_netWorld.blurTemp);
        ClearBackground(BLACK);
        BeginShaderMode(g_bloomBlurShader);
            Vector2 dirH = {1.0f, 0.0f};
            Vector2 resH = {(float)g_netWorld.bloomRT.texture.width, (float)g_netWorld.bloomRT.texture.height};
            SetShaderValue(g_bloomBlurShader, GetShaderLocation(g_bloomBlurShader, "direction"), &dirH, SHADER_UNIFORM_VEC2);
            SetShaderValue(g_bloomBlurShader, GetShaderLocation(g_bloomBlurShader, "resolution"), &resH, SHADER_UNIFORM_VEC2);
            DrawTexturePro(g_netWorld.bloomRT.texture, srcBloom, destBlurTemp, {0, 0}, 0.0f, WHITE);
        EndShaderMode();
    EndTextureMode();

    // Vertical blur
    BeginTextureMode(g_netWorld.bloomRT);
        ClearBackground(BLACK);
        BeginShaderMode(g_bloomBlurShader);
            Vector2 dirV = {0.0f, 1.0f};
            Vector2 resV = {(float)g_netWorld.blurTemp.texture.width, (float)g_netWorld.blurTemp.texture.height};
            SetShaderValue(g_bloomBlurShader, GetShaderLocation(g_bloomBlurShader, "direction"), &dirV, SHADER_UNIFORM_VEC2);
            SetShaderValue(g_bloomBlurShader, GetShaderLocation(g_bloomBlurShader, "resolution"), &resV, SHADER_UNIFORM_VEC2);
            DrawTexturePro(g_netWorld.blurTemp.texture, srcBlurTemp, destBloom, {0, 0}, 0.0f, WHITE);
        EndShaderMode();
    EndTextureMode();

    // Composite
    BeginTextureMode(output);
        ClearBackground(BLACK);
        // Draw base scene
        DrawTexturePro(scene.texture, srcScene, destScene, {0, 0}, 0.0f, WHITE); 
        
        // Additive blend the scaled-up bloom target
        BeginBlendMode(BLEND_ADDITIVE);
            float intensity = g_netWorld.bloomIntensity;
            DrawTexturePro(g_netWorld.bloomRT.texture, srcBloom, destScene, {0, 0}, 0.0f, Fade(WHITE, intensity));
        EndBlendMode();
    EndTextureMode();
}

void ApplyDistortion(RenderTexture2D scene) {
    if (g_distortionShader.id == 0) {
        printf("[WARN] Distortion shader not loaded, skipping.\n");
        return;
    }

    // Set uniforms
    float time = g_netWorld.time;
    float glitch = g_netWorld.glitchIntensity;
    float chromatic = 0.02f;
    float block = 20.0f;
    Vector2 res = {(float)GetScreenWidth(), (float)GetScreenHeight()};

    SetShaderValue(g_distortionShader, GetShaderLocation(g_distortionShader, "time"), &time, SHADER_UNIFORM_FLOAT);
    SetShaderValue(g_distortionShader, GetShaderLocation(g_distortionShader, "glitchIntensity"), &glitch, SHADER_UNIFORM_FLOAT);
    SetShaderValue(g_distortionShader, GetShaderLocation(g_distortionShader, "chromaticOffset"), &chromatic, SHADER_UNIFORM_FLOAT);
    SetShaderValue(g_distortionShader, GetShaderLocation(g_distortionShader, "blockSize"), &block, SHADER_UNIFORM_FLOAT);
    SetShaderValue(g_distortionShader, GetShaderLocation(g_distortionShader, "resolution"), &res, SHADER_UNIFORM_VEC2);

    // Begin shader mode and draw a flipped rectangle
    BeginShaderMode(g_distortionShader);
        // Flip the texture vertically to correct OpenGL's upside-down rendering
        Rectangle srcRect = {0, 0, (float)scene.texture.width, -(float)scene.texture.height};
        Rectangle dstRect = {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()};
        DrawTexturePro(scene.texture, srcRect, dstRect, Vector2{0, 0}, 0.0f, WHITE);
    EndShaderMode();
}