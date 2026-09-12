#version 330

uniform sampler2D texture0;
uniform vec2 resolution;
uniform float time;
uniform float glitchIntensity;

uniform float scanlineIntensity;
uniform float vignetteIntensity;
uniform float barrelAmount;
uniform float chromaticAmount;
uniform float ditherAmount;

// Final multiplier. 1.0 is neutral; 1.15 is a mild lift; 0.9 dims the frame.
// Exposed as a uniform so grading is tunable live without touching the
// material shader.
uniform float exposure;

in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

vec2 barrel(vec2 uv, float amount)
{
    vec2 cc = uv - 0.5;
    return uv + cc * dot(cc, cc) * amount;
}

float bayer4(vec2 p)
{
    int x = int(mod(p.x, 4.0));
    int y = int(mod(p.y, 4.0));
    int idx = x + y * 4;
    float m[16] = float[16](
         0.0,  8.0,  2.0, 10.0,
        12.0,  4.0, 14.0,  6.0,
         3.0, 11.0,  1.0,  9.0,
        15.0,  7.0, 13.0,  5.0
    );
    return m[idx] / 16.0 - 0.5;
}

float hash12(vec2 p)
{
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

// Additive split-tone. Crucially, this ADDS to the source rather than mixing
// toward a tint — mixing toward a dark tint darkens mid-tones, which is the
// opposite of what a grade should do.
vec3 grade(vec3 c)
{
    float lum = dot(c, vec3(0.299, 0.587, 0.114));

    vec3 shadowLift    = vec3(0.030, 0.008, 0.055);  // cool blue
    vec3 highlightLift = vec3(0.060, 0.005, 0.025);  // warm magenta

    c += mix(shadowLift, highlightLift, smoothstep(0.0, 0.85, lum));
    return c;
}

void main()
{
    vec2 uv = fragTexCoord;

    // ---- 1. Geometric warp ----
    uv = barrel(uv, barrelAmount * (1.0 + glitchIntensity * 2.0));

    float row = floor(uv.y * resolution.y / 6.0);
    float tear = step(0.9, hash12(vec2(row, floor(time * 30.0))));
    uv.x += tear * glitchIntensity * (hash12(vec2(row, time)) - 0.5) * 0.08;

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        finalColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    // ---- 2. Chromatic aberration ----
    vec2 center = uv - 0.5;
    float r2 = dot(center, center);
    float split = chromaticAmount * r2 * (1.0 + glitchIntensity * 5.0);
    vec2 dir = normalize(center + vec2(1e-5));

    vec3 col;
    col.r = texture(texture0, uv + dir * split).r;
    col.g = texture(texture0, uv).g;
    col.b = texture(texture0, uv - dir * split).b;

    // ---- 3. Scanlines ----
    float scanPhase = uv.y * resolution.y;
    float scanline = 1.0 - scanlineIntensity * 0.5 * (0.5 + 0.5 * sin(scanPhase * 3.14159));
    col *= scanline;

    // ---- 4. Grade ----
    col = grade(col);

    // ---- 5. Ordered dithering ----
    col += bayer4(uv * resolution) * ditherAmount;

    // ---- 6. Vignette ----
    float vig = 1.0 - vignetteIntensity * r2 * 1.9;
    col *= clamp(vig, 0.0, 1.0);

    // ---- 7. Film grain ----
    col += (hash12(uv * resolution + vec2(time * 91.0, time * 47.0)) - 0.5) * 0.02;

    // ---- 8. Exposure ----
    col *= exposure;

    finalColor = vec4(col, 1.0) * fragColor;
}