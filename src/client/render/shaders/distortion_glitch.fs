#version 330

uniform sampler2D texture0;
uniform float time;
uniform float glitchIntensity;
uniform float chromaticOffset;
uniform float blockSize;
uniform vec2 resolution;

in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

void main()
{
    vec2 uv = fragTexCoord;

    // ---------- 1. Idle waveform ----------
    // Almost invisible at rest; this is the "screen is live" wobble, not
    // a gameplay effect. Scaled by glitchIntensity so it vanishes entirely
    // when the game is calm.
    float wave = sin(uv.x * 30.0 + time * 2.0) * 0.015
               + cos(uv.y * 25.0 + time * 1.7) * 0.015;
    uv.x += wave * glitchIntensity;

    // ---------- 2. Row displacement ----------
    float row = floor(uv.y * resolution.y / 8.0);
    float rowJitter = (hash(vec2(row, floor(time * 24.0))) - 0.5) * 0.06;
    uv.x += rowJitter * glitchIntensity;

    // ---------- 3. Chromatic split ----------
    float spike = step(0.9, sin(time * 90.0 + uv.y * 200.0));
    float offset = (0.3 + spike) * chromaticOffset * glitchIntensity;
    vec3 color;
    color.r = texture(texture0, uv + vec2(offset, 0.0)).r;
    color.g = texture(texture0, uv).g;
    color.b = texture(texture0, uv - vec2(offset, 0.0)).b;

    // ---------- 4. Block corruption ----------
    vec2 blockUV = floor(uv * blockSize) / blockSize;
    float blockTrigger = step(0.97, hash(blockUV + floor(time * 12.0)));
    color = mix(color, texture(texture0, blockUV).rgb, blockTrigger * glitchIntensity);

    // ---------- 5. Inversion bands ----------
    float inv = step(0.995, hash(vec2(floor(uv.y * 40.0), floor(time * 30.0))));
    color = mix(color, 1.0 - color, inv * glitchIntensity * 0.7);

    finalColor = vec4(color, 1.0) * fragColor;
}