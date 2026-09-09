#version 330

uniform sampler2D sceneTexture;
uniform vec2 direction;
uniform vec2 resolution;

in vec2 texCoord;
out vec4 fragColor;

void main() {
    vec2 texelSize = 1.0 / resolution;
    vec3 result = vec3(0.0);
    // 9‑tap weights for a separable Gaussian blur
    float weights[9] = float[](
        0.016216, 0.054054, 0.1216216, 0.1945946, 0.227027,
        0.1945946, 0.1216216, 0.054054, 0.016216
    );
    for (int i = 0; i < 9; i++) {
        int offset = i - 4;                       // -4 … +4
        vec2 offsetVec = vec2(float(offset)) * direction * texelSize * 1.5;
        result += texture(sceneTexture, texCoord + offsetVec).rgb * weights[i];
    }
    fragColor = vec4(result, 1.0);
}