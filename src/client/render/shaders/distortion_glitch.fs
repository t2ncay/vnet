#version 330

// Raylib automatically binds the active texture to texture0
uniform sampler2D texture0; 

uniform float time;
uniform float glitchIntensity;
uniform float chromaticOffset;
uniform float blockSize;
uniform vec2 resolution;

in vec2 fragTexCoord;
in vec4 fragColor; 

out vec4 finalColor;

void main() {
    vec2 uv = fragTexCoord;
    
    // Grid distortion scaled by glitchIntensity so it stops wobbling when idle
    float wave = sin(uv.x * 30.0 + time * 2.0) * 0.02;
    wave += cos(uv.y * 25.0 + time * 1.7) * 0.02;
    uv += vec2(wave, 0.0) * glitchIntensity;

    // Glitch displacement (spike)
    float spike = step(0.95, sin(time * 100.0 + uv.y * 200.0));
    float offset = (spike + 1.0) * chromaticOffset * glitchIntensity;
    vec2 uvR = uv + vec2(offset, 0.0);
    vec2 uvB = uv - vec2(offset, 0.0);

    float r = texture(texture0, uvR).r;
    float g = texture(texture0, uv).g;
    float b = texture(texture0, uvB).b;

    // Block corruption
    float block = step(0.98, sin(uv.y * blockSize + time * 5.0));
    vec2 blockUV = floor(uv * blockSize) / blockSize;
    vec3 blockColor = texture(texture0, blockUV).rgb;
    
    vec3 color = vec3(r, g, b);
    color = mix(color, blockColor, block * glitchIntensity * 0.5);

    finalColor = vec4(color, 1.0) * fragColor; 
}