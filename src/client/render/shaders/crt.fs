// crt.fs – fragment shader
#version 330

uniform sampler2D texture0;
uniform vec2 resolution;
uniform float time;
uniform float intensity;  // Feed jitter intensity here
uniform vec2 scanlineParams; // (frequency, thickness)

in vec2 fragTexCoord;
out vec4 fragColor;

void main() {
    vec2 uv = fragTexCoord;
    
    // Scanlines
    float scanline = sin(uv.y * resolution.y * 0.5) * 0.5 + 0.5;
    scanline = smoothstep(0.1, 0.9, scanline);
    scanline = mix(0.85, 1.0, scanline);
    
    // Vignette
    vec2 center = uv - 0.5;
    float vignette = 1.0 - dot(center, center) * 0.6;
    
    // Chromatic aberration (driven by intensity)
    float chroma = intensity * 0.003;
    vec2 redOffset = vec2(chroma, 0);
    vec2 blueOffset = vec2(-chroma, 0);
    
    float r = texture(texture0, uv + redOffset).r;
    float g = texture(texture0, uv).g;
    float b = texture(texture0, uv + blueOffset).b;
    
    vec3 color = vec3(r, g, b);
    color *= scanline * vignette;
    
    // Curvature
    float dist = length(center);
    float curve = 1.0 + dist * dist * 0.1;
    // (simplified - real barrel distortion is more complex)
    
    fragColor = vec4(color, 1.0);
}