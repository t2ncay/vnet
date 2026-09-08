#version 330

// Input vertex attributes
in vec2 fragTexCoord;
in vec3 fragPosition;
in vec3 fragNormal;
in vec3 fragTangent;
in vec4 fragColor;

// Output fragment color
out vec4 finalColor;

// Uniforms
uniform vec3 viewPos;
uniform vec3 ambientColor;
uniform float ambientIntensity;

// Texture maps
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D mraMap;
uniform sampler2D emissiveMap;

// Material properties
uniform vec4 albedoColor;
uniform float metallicValue;
uniform float roughnessValue;
uniform float emissivePower;
uniform vec4 emissiveColor;

// Lights
#define MAX_LIGHTS 4
uniform int numOfLights;

struct Light {
    int type;
    int enabled;
    vec3 position;
    vec3 target;
    vec4 color;
    float intensity;
};

uniform Light lights[MAX_LIGHTS];

// Constants
const float PI = 3.14159265359;

// ---- PBR FUNCTIONS ----
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ---- MAIN ----
void main() {
    // ---- 1. SAMPLE TEXTURES ----
    vec4 albedoSampled = texture(albedoMap, fragTexCoord);
    vec3 albedo = albedoSampled.rgb * albedoColor.rgb;
    float alpha = albedoSampled.a * albedoColor.a;
    
    vec3 mraSampled = texture(mraMap, fragTexCoord).rgb;
    float metalness = mraSampled.r * metallicValue;
    float roughness = mraSampled.g * roughnessValue;
    float ao = mraSampled.b;
    
    // Normal map
    vec3 normal = normalize(fragNormal);
    vec3 tangent = normalize(fragTangent);
    vec3 bitangent = normalize(cross(normal, tangent));
    mat3 TBN = mat3(tangent, bitangent, normal);
    vec3 normalMap = texture(normalMap, fragTexCoord).xyz * 2.0 - 1.0;
    normal = normalize(TBN * normalMap);
    
    vec3 emissive = texture(emissiveMap, fragTexCoord).rgb * emissiveColor.rgb * emissivePower;
    
    // ---- 2. LIGHTING CALCULATIONS ----
    vec3 V = normalize(viewPos - fragPosition);
    vec3 F0 = mix(vec3(0.04), albedo, metalness);
    
    vec3 Lo = vec3(0.0);
    
    for (int i = 0; i < numOfLights && i < MAX_LIGHTS; i++) {
        if (lights[i].enabled == 0) continue;
        
        vec3 L = vec3(0.0);
        float attenuation = 1.0;
        float distance = 0.0;
        
        if (lights[i].type == 1) {
            L = lights[i].position - fragPosition;
            distance = length(L);
            L = normalize(L);
            attenuation = 1.0 / (distance * distance + 1.0);
        } else if (lights[i].type == 0) {
            L = normalize(-lights[i].position);
            attenuation = 1.0;
        }
        
        vec3 H = normalize(V + L);
        float NdotL = max(dot(normal, L), 0.0);
        
        float NDF = DistributionGGX(normal, H, roughness);
        float G = GeometrySmith(normal, V, L, roughness);
        vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metalness;
        
        vec3 numerator = NDF * G * F;
        float denom = 4.0 * max(dot(normal, V), 0.0) * NdotL + 0.0001;
        vec3 specular = numerator / denom;
        
        vec3 radiance = lights[i].color.rgb * lights[i].intensity * attenuation;
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }
    
    // ---- 3. AMBIENT LIGHT (MUCH BRIGHTER) ----
    float ambientIntensityFinal = 0.15f; // Even brighter!
    vec3 ambient = ambientColor * ambientIntensityFinal * albedo * ao;
    vec3 color = ambient + Lo;
    
    // ---- 4. EMISSIVE (MUCH BRIGHTER) ----
    color += emissive * 3.0; // Triple emissive intensity
    
    // ---- 5. POST-PROCESS ----
    // ACES tone mapping (cinematic)
    color = (color * (2.51 * color + 0.03)) / (color * (2.43 * color + 0.59) + 0.14);
    
    // Gamma correction
    color = pow(color, vec3(1.0/2.2));
    
    // Add bloom glow
    float luminance = dot(color, vec3(0.299, 0.587, 0.114));
    color += vec3(0.0, 0.4, 0.8) * luminance * 0.2;
    
    // Cyan tint boost
    color.g *= 1.1;
    color.b *= 1.15;
    
    // ---- 6. OUTPUT ----
    finalColor = vec4(color, alpha);
}