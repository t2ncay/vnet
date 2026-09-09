#version 330

uniform sampler2D scene;
uniform float threshold;

in vec2 texCoord;
out vec4 fragColor;

void main() {
    vec3 color = texture(scene, texCoord).rgb;
    float brightness = max(color.r, max(color.g, color.b));
    float contribution = smoothstep(threshold, threshold + 0.2, brightness);
    fragColor = vec4(color * contribution, 1.0);
}