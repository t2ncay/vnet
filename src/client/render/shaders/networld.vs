#version 330

// Vertex attributes as supplied by raylib for the default vertex layout.
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

out vec2 fragTexCoord;
out vec3 fragPosition;
out vec3 fragNormal;
out vec4 fragColor;

// Raylib auto-populates both of these for any custom shader:
//   mvp       -> SHADER_LOC_MATRIX_MVP
//   matModel  -> SHADER_LOC_MATRIX_MODEL
uniform mat4 mvp;
uniform mat4 matModel;

void main()
{
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    // World-space position and normal. The fragment shader needs these for
    // rim lighting and light falloff; both are computed here rather than in
    // the fragment shader so we pay the mat4 mult per-vertex, not per-pixel.
    vec4 worldPos = matModel * vec4(vertexPosition, 1.0);
    fragPosition = worldPos.xyz;
    fragNormal = mat3(matModel) * vertexNormal;

    gl_Position = mvp * vec4(vertexPosition, 1.0);
}