#version 330

uniform sampler2D sceneTexture;   // current frame
uniform sampler2D depthTexture;   // depth from scene
uniform mat4 invProj;             // inverse projection matrix
uniform mat4 invView;             // inverse view matrix
uniform vec3 fogColor;
uniform float fogDensity;
uniform float time;
uniform vec2 resolution;

in vec2 texCoord;
out vec4 fragColor;

vec3 worldPosFromDepth(float depth, vec2 uv) {
    vec4 clipPos = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 worldPos = invProj * invView * clipPos;
    return worldPos.xyz / worldPos.w;
}

void main() {
    float depth = texture(depthTexture, texCoord).r;
    vec3 worldPos = worldPosFromDepth(depth, texCoord);
    vec3 viewDir = normalize(worldPos - vec3(0,0,0)); // camera at origin? adapt.

    // Sample fog density with digital pulse
    float density = fogDensity;
    float pulse = sin(worldPos.y * 0.5 + time * 0.8) * 0.5 + 0.5;
    density += pulse * 0.3;
    // Data‑stream pattern
    float grid = sin(worldPos.x * 2.0 + time) * cos(worldPos.z * 2.0 + time * 1.3);
    density += abs(grid) * 0.1;

    // Integrate along ray (simplified single‑step)
    float fogAmount = 1.0 - exp(-density * depth);
    vec3 sceneColor = texture(sceneTexture, texCoord).rgb;
    vec3 fog = mix(fogColor, vec3(0.0, 0.8, 0.6), pulse * 0.5); // shift cyan
    fragColor = vec4(mix(sceneColor, fog, fogAmount * 0.8), 1.0);
}