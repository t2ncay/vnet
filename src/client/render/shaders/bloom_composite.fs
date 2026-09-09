#version 330

uniform sampler2D sceneTexture;
uniform sampler2D bloomTexture;
uniform float bloomIntensity;

in vec2 texCoord;
out vec4 fragColor;

void main() {
    vec3 sceneColor = texture(sceneTexture, texCoord).rgb;
    vec3 bloomColor = texture(bloomTexture, texCoord).rgb;
    vec3 finalColor = sceneColor + bloomColor * bloomIntensity;
    fragColor = vec4(finalColor, 1.0);
}