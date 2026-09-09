// In the main() of the PBR shader, after lighting:
#ifdef PORTAL
    vec2 uv = fragTexCoord; // or compute screen‑space UV
    vec2 center = vec2(0.5, 0.5);
    vec2 delta = uv - center;
    float radius = length(delta);
    float angle = atan(delta.y, delta.x) + time * 0.5;
    float spiral = sin(angle * 3.0 + radius * 10.0) * 0.5 + 0.5;
    float distortion = portalDistortion * spiral * (1.0 - radius);
    vec2 distortedUV = uv + delta * distortion * 0.1;
    // sample from the scene? For per‑object we can just tint:
    color = mix(color, portalColor, spiral * 0.3);
#endif