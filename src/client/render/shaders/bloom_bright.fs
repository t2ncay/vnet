#version 330

uniform sampler2D texture0;
uniform float threshold;
uniform float knee;

in vec2 fragTexCoord;     // <-- was "texCoord"; raylib's VS outputs fragTexCoord
in vec4 fragColor;
out vec4 finalColor;

void main()
{
    vec3 c = texture(texture0, fragTexCoord).rgb;
    float brightness = max(c.r, max(c.g, c.b));
    // Smoothstep with a knee width so the bright pass doesn't produce hard
    // edges on moving emissive geometry.
    float contribution = smoothstep(threshold, threshold + knee, brightness);
    finalColor = vec4(c * contribution, 1.0);
}