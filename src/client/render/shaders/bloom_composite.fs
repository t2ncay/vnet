#version 330

uniform sampler2D texture0;       // base scene (auto-bound)
uniform sampler2D bloomTexture;   // bound via SetShaderValueTexture()
uniform float bloomIntensity;

in vec2 fragTexCoord;             // <-- was "texCoord"
in vec4 fragColor;
out vec4 finalColor;

void main()
{
    vec3 scene = texture(texture0, fragTexCoord).rgb;
    vec3 bloom = texture(bloomTexture, fragTexCoord).rgb * bloomIntensity;

    // Screen blend. Unlike additive this never clips to white — emissive
    // pixels bloom outward into color instead of saturating to a flat blob.
    vec3 result = 1.0 - (1.0 - scene) * (1.0 - bloom);

    finalColor = vec4(result, 1.0) * fragColor;
}