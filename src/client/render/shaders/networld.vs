#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec3 vertexTangent;
in vec4 vertexColor;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec3 fragPosition;
out vec3 fragNormal;
out vec3 fragTangent;
out vec4 fragColor;

// Uniforms
uniform mat4 mvp;
uniform mat4 model;
uniform mat4 view;

void main()
{
    // Send texture coordinates
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    
    // Calculate fragment position and normal in world space
    fragPosition = vec3(model * vec4(vertexPosition, 1.0));
    fragNormal = normalize(mat3(model) * vertexNormal);
    fragTangent = normalize(mat3(model) * vertexTangent);
    
    // Calculate final vertex position
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}