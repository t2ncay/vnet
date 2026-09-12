#version 330

uniform sampler2D texture0;
uniform vec2 direction;    // (1,0) for horizontal, (0,1) for vertical
uniform vec2 resolution;

in vec2 fragTexCoord;      // <-- was "texCoord"
in vec4 fragColor;
out vec4 finalColor;

void main()
{
    vec2 step = direction / resolution;

    // 13-tap Gaussian, sigma ~3.0, DC gain = 1.0. Same spread as two passes
    // of a 9-tap but only one pass of texture fetches per axis.
    float w[7] = float[](
        0.196482, 0.174666, 0.121622, 0.065861, 0.027630, 0.008977, 0.002105
    );

    vec3 sum = texture(texture0, fragTexCoord).rgb * w[0];
    for (int i = 1; i < 7; i++) {
        vec2 o = step * float(i) * 1.4;
        sum += texture(texture0, fragTexCoord + o).rgb * w[i];
        sum += texture(texture0, fragTexCoord - o).rgb * w[i];
    }

    finalColor = vec4(sum, 1.0);
}