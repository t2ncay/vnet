#include "networld.h"
#include "../render.h"
#include "../../shared/vnet.h"
#include <cmath>
#include <cstdlib>

static Shader g_crtShader = {0};
static bool g_shaderLoaded = false;
static bool g_shaderActive = false;
static int g_timeLoc = -1;
static int g_resolutionLoc = -1;
static int g_glitchLoc = -1;
static int g_scanlineLoc = -1;
static int g_vignetteLoc = -1;
static int g_chromaticLoc = -1;
static int g_warpLoc = -1;

void LoadNetWorldShader(void) {
    if (g_shaderLoaded) return;
    
    g_crtShader = LoadShader(0, "src/client/render/shaders/networld.fs");
    
    if (g_crtShader.id != 0) {
        g_timeLoc = GetShaderLocation(g_crtShader, "time");
        g_resolutionLoc = GetShaderLocation(g_crtShader, "resolution");
        g_glitchLoc = GetShaderLocation(g_crtShader, "glitchIntensity");
        g_scanlineLoc = GetShaderLocation(g_crtShader, "scanlineIntensity");
        g_vignetteLoc = GetShaderLocation(g_crtShader, "vignetteIntensity");
        g_chromaticLoc = GetShaderLocation(g_crtShader, "chromaticAberration");
        g_warpLoc = GetShaderLocation(g_crtShader, "warpAmount");
        
        g_shaderLoaded = true;
        printf("[SHADER] NetWorld CRT shader loaded successfully.\n");
    } else {
        printf("[SHADER] Failed to load NetWorld CRT shader.\n");
    }
}

void UnloadNetWorldShader(void) {
    if (g_shaderLoaded) {
        if (g_shaderActive) {
            EndShaderMode();
            g_shaderActive = false;
        }
        UnloadShader(g_crtShader);
        g_shaderLoaded = false;
        printf("[SHADER] NetWorld CRT shader unloaded.\n");
    }
}

void DisableNetWorldShader(void) {
    if (g_shaderActive) {
        EndShaderMode();
        g_shaderActive = false;
        printf("[SHADER] Shader forcefully disabled.\n");
    }
}

void ApplyNetWorldShader(void) {
    if (!g_shaderLoaded) return;
    if (!g_netWorld.active) return;
    
    if (g_netWorld.state == NetWorldState::ENTERING || 
        g_netWorld.state == NetWorldState::EXITING) {
        return;
    }
    
    if (g_shaderActive) return;
    
    BeginShaderMode(g_crtShader);
    g_shaderActive = true;
    
    // ---- UPDATE UNIFORMS ----
    if (g_timeLoc != -1) {
        SetShaderValue(g_crtShader, g_timeLoc, &g_netWorld.time, SHADER_UNIFORM_FLOAT);
    }
    
    if (g_resolutionLoc != -1) {
        // NOTE: If rendering to a smaller RenderTexture, pass THAT resolution here, not screen width.
        float res[2] = {(float)GetScreenWidth(), (float)GetScreenHeight()};
        SetShaderValue(g_crtShader, g_resolutionLoc, res, SHADER_UNIFORM_VEC2);
    }
    
    if (g_glitchLoc != -1) {
        float intensity = g_netWorld.glitchIntensity;
        // Random spikes (less frequent but sharper)
        if ((rand() % 1000) < 5) intensity += 0.6f;
        intensity = fminf(intensity, 1.0f);
        SetShaderValue(g_crtShader, g_glitchLoc, &intensity, SHADER_UNIFORM_FLOAT);
    }
    
    if (g_scanlineLoc != -1) {
        // Scale 0.0 to 1.0 for phosphor visibility
        float scanline = 0.8f + sinf(g_netWorld.time * 2.0f) * 0.1f;
        SetShaderValue(g_crtShader, g_scanlineLoc, &scanline, SHADER_UNIFORM_FLOAT);
    }
    
    if (g_vignetteLoc != -1) {
        float vignette = 0.8f; // Nice dark edge framing
        SetShaderValue(g_crtShader, g_vignetteLoc, &vignette, SHADER_UNIFORM_FLOAT);
    }
    
    if (g_chromaticLoc != -1) {
        // Scales with glitch intensity
        float chroma = 1.0f + g_netWorld.glitchIntensity * 6.0f;
        SetShaderValue(g_crtShader, g_chromaticLoc, &chroma, SHADER_UNIFORM_FLOAT);
    }
    
    if (g_warpLoc != -1) {
        // Keeps the CRT monitor bulge subtle but visible
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