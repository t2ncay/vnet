#version 330

in vec2 fragTexCoord;
in vec3 fragPosition;
in vec3 fragNormal;
in vec4 fragColor;

out vec4 finalColor;

// ---- Auto-bound by raylib ----
uniform sampler2D texture0;   // albedo
uniform vec4 colDiffuse;      // material tint — MUST be set to WHITE by the app

// ---- Set per-frame in ApplyNetWorldPBR() ----
uniform vec3 viewPos;
uniform float time;

uniform vec3 emissiveColor;
uniform float emissivePower;

uniform vec3 rimColor;
uniform float rimPower;
uniform float rimIntensity;

uniform float scanlineAmount;

uniform vec3 ambientColor;
uniform float ambientIntensity;

// Hard floor so no unlit surface ever falls below this fraction of its
// own albedo. Without it, geometry that isn't near a dynamic light is
// invisible — the shader becomes "emissive + rim only", which is what
// happened before this was added.
const float AMBIENT_FLOOR = 0.35;

#define MAX_LIGHTS 12
uniform int numOfLights;
struct Light {
    int type;
    int enabled;
    vec3 position;
    vec4 color;
    float intensity;
};
uniform Light lights[MAX_LIGHTS];

void main()
{
    vec4 tex = texture(texture0, fragTexCoord);
    vec3 base = tex.rgb * colDiffuse.rgb * fragColor.rgb;
    float alpha = tex.a * colDiffuse.a * fragColor.a;

    // Reconstruct a flat normal for raylib primitives, which never emit
    // vertexNormal. Loaded meshes still use their real normal.
    vec3 Ng = cross(dFdx(fragPosition), dFdy(fragPosition));
    vec3 N = vec3(0.0, 1.0, 0.0);
    if (length(fragNormal) > 0.001) {
        N = normalize(fragNormal);
    } else if (length(Ng) > 0.001) {
        N = normalize(Ng);
    }
    vec3 V = normalize(viewPos - fragPosition);

    float rim = pow(1.0 - clamp(dot(N, V), 0.0, 1.0), rimPower);

    float scan = 1.0 - scanlineAmount * 0.5
               + scanlineAmount * 0.5 * sin(fragPosition.y * 60.0 + time * 4.0);

    // Soft proximity glow. Deliberately not a BRDF — no specular, no
    // energy conservation, just a halo so moving lights read.
    vec3 glow = vec3(0.0);
    for (int i = 0; i < numOfLights && i < MAX_LIGHTS; i++) {
        if (lights[i].enabled == 0) continue;
        vec3 toL = lights[i].position - fragPosition;
        float dist2 = dot(toL, toL);
        float atten = 1.0 / (1.0 + dist2 * 0.015);
        float facing = 0.5 + 0.5 * dot(N, normalize(toL));
        glow += lights[i].color.rgb * atten * facing * lights[i].intensity * 0.02;
    }

    // Ambient = tinted contribution + hard floor. Together this is the
    // "unlit readability light" — surfaces read as their own color, just
    // tinted by the ambient hue and lifted by nearby point lights.
    vec3 ambient = ambientColor * ambientIntensity;
    vec3 totalLight = ambient + glow + vec3(AMBIENT_FLOOR);

    vec3 lit = base * scan * totalLight;
    vec3 emissive = emissiveColor * emissivePower;
    vec3 rimTerm = rimColor * rim * rimIntensity;

    vec3 color = lit + emissive + rimTerm;
    finalColor = vec4(color, alpha);
}