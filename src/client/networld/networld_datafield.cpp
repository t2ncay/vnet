#include "networld.h"
#include "../render.h"
#include "raymath.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>

// ---------------------------------------------------------------------------
// Per-field particle lattice.
//
// 60 points per field. A DrawPoint3D is a single vertex with no color batch
// overhead worth worrying about, so this is nearly free — the reason not to
// push it to 200 is that density above ~80 stops reading as discrete "data"
// and starts reading as a soft cloud, which defeats the whole point.
//
// 8 hex glyphs per field ride on the same lattice. 3D text goes through
// GetWorldToScreen and the font atlas, so it's ~100x the cost of a point.
// Eight per field is enough to sell the "hex dump" idea without wrecking
// the frame budget when three fields are visible.
// ---------------------------------------------------------------------------
static const int FIELD_PARTICLE_COUNT = 60;
static const int FIELD_GLYPH_COUNT    = 8;

struct FieldParticle {
    Vector3 direction;    // unit vector — position on the unit sphere
    float   baseRadius;   // 0..1 — used only by NOISE drift
    float   speed;
    float   phase;
    char    glyph;        // consumed only by the first FIELD_GLYPH_COUNT entries
};

static FieldParticle g_fieldParticles[MAX_DATA_FIELDS][FIELD_PARTICLE_COUNT];
static bool g_dataFieldsInit = false;

// ICE drains shield in fractional amounts across many frames; this
// accumulator carries the remainder so DPS is preserved at low framerates.
static float g_iceDamageAccum = 0.0f;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static Vector3 FieldPrimaryColor(DataFieldType type)
{
    switch (type) {
        case DataFieldType::ENCRYPTION: return {0.20f, 0.85f, 1.00f};  // cyan
        case DataFieldType::ICE:        return {1.00f, 0.55f, 0.15f};  // amber
        case DataFieldType::NOISE:      return {0.85f, 0.20f, 1.00f};  // magenta
    }
    return {1.0f, 1.0f, 1.0f};
}

static void InitFieldParticles(int fieldIndex)
{
    for (int p = 0; p < FIELD_PARTICLE_COUNT; p++) {
        FieldParticle& fp = g_fieldParticles[fieldIndex][p];

        // Uniform distribution on the unit sphere via inverse-CDF. Sampling
        // a normalized cube would cluster points at the poles, which reads
        // as visible grid artifacts on a spherical field.
        float u = (rand() % 1000) / 1000.0f;
        float v = (rand() % 1000) / 1000.0f;
        float theta = 2.0f * PI * u;
        float phi   = acosf(2.0f * v - 1.0f);
        fp.direction = {
            sinf(phi) * cosf(theta),
            cosf(phi),
            sinf(phi) * sinf(theta)
        };

        fp.baseRadius = 0.15f + (rand() % 1000) / 1000.0f * 0.85f;
        fp.speed      = 0.30f + (rand() % 1000) / 1000.0f * 0.60f;
        fp.phase      = (rand() % 1000) / 1000.0f * 2.0f * PI;
        fp.glyph      = "0123456789ABCDEF"[rand() % 16];
    }
}

// ---------------------------------------------------------------------------
// Public: spawn a single field at runtime (CLI, raid events, etc.)
// ---------------------------------------------------------------------------

void SpawnDataField(Vector3 center, float radius, DataFieldType type)
{
    if (g_netWorld.dataFieldCount >= MAX_DATA_FIELDS) return;

    int idx = g_netWorld.dataFieldCount++;
    DataField& f = g_netWorld.dataFields[idx];

    f.type       = type;
    f.center     = center;
    f.radius     = radius;
    f.intensity  = 0.85f;
    f.color      = FieldPrimaryColor(type);
    f.phase      = 0.0f;
    f.damageTimer = 0.0f;
    f.pulseTimer = (rand() % 1000) / 1000.0f * 3.0f;
    f.active     = true;

    InitFieldParticles(idx);
    g_dataFieldsInit = true;
}

// ---------------------------------------------------------------------------
// Public: generate fields across the world
// ---------------------------------------------------------------------------

void InitDataFields(void)
{
    g_netWorld.dataFieldCount = 0;

    const int numRooms = (int)g_netWorld.building.rooms.size();
    if (numRooms == 0) return;

    const float TILE = g_netWorld.building.tileSize;

    // Skip room 0 — that's the spawn room, and spawning the player inside
    // an ICE field they can't see yet is a bug report waiting to happen.
    for (int i = 1; i < numRooms && g_netWorld.dataFieldCount < MAX_DATA_FIELDS; i++) {
        if ((rand() % 100) >= 40) continue;  // ~40% of rooms get a field

        const Rectangle& r = g_netWorld.building.rooms[i];
        DataField& f = g_netWorld.dataFields[g_netWorld.dataFieldCount];

        f.center = {
            (r.x + r.width  * 0.5f) * TILE,
            2.5f,
            (r.y + r.height * 0.5f) * TILE
        };

        // Radius fits inside the room but leaves room to see the boundary.
        float roomSpan = fminf(r.width, r.height) * TILE;
        f.radius = roomSpan * 0.55f + 2.0f;
        if (f.radius < 4.0f)  f.radius = 4.0f;
        if (f.radius > 12.0f) f.radius = 12.0f;

        // Type: rooms that already contain an enemy become ICE (defended),
        // rooms with a portal become ENCRYPTION (worth encrypting),
        // everything else becomes NOISE.
        bool hasEnemy  = false;
        bool hasPortal = false;
        for (const auto& node : g_netWorld.nodes) {
            float dx = node.position.x - f.center.x;
            float dz = node.position.z - f.center.z;
            if (dx * dx + dz * dz < f.radius * f.radius) {
                if (node.type == NodeType::ENEMY)  hasEnemy  = true;
                if (node.type == NodeType::PORTAL) hasPortal = true;
            }
        }
        if (hasEnemy)       f.type = DataFieldType::ICE;
        else if (hasPortal) f.type = DataFieldType::ENCRYPTION;
        else                f.type = DataFieldType::NOISE;

        f.intensity  = 0.60f + (rand() % 40) / 100.0f;
        f.color      = FieldPrimaryColor(f.type);
        f.phase      = (rand() % 1000) / 1000.0f * 2.0f * PI;
        f.damageTimer = 0.0f;
        f.pulseTimer = (rand() % 1000) / 1000.0f * 3.0f;
        f.active     = true;

        InitFieldParticles(g_netWorld.dataFieldCount);
        g_netWorld.dataFieldCount++;
    }

    g_dataFieldsInit = true;
    printf("[DATAFIELD] %d fields generated.\n", g_netWorld.dataFieldCount);
}

// ---------------------------------------------------------------------------
// Public: per-frame visual timer update
// ---------------------------------------------------------------------------

void UpdateDataFields(float dt)
{
    for (int i = 0; i < g_netWorld.dataFieldCount; i++) {
        DataField& f = g_netWorld.dataFields[i];
        if (!f.active) continue;

        f.phase += dt;

        // The scan ring wraps on a fixed 3s period. It's purely cosmetic —
        // the draw code reads pulseTimer / 3.0f to know where along the
        // sweep the ring currently is.
        const float pulsePeriod = 3.0f;
        f.pulseTimer += dt;
        if (f.pulseTimer > pulsePeriod) f.pulseTimer -= pulsePeriod;
    }
}

// ---------------------------------------------------------------------------
// Public: field influence query (0 outside, 1 at center)
// ---------------------------------------------------------------------------

float GetDataFieldInfluence(Vector3 position, DataFieldType type)
{
    float best = 0.0f;

    for (int i = 0; i < g_netWorld.dataFieldCount; i++) {
        const DataField& f = g_netWorld.dataFields[i];
        if (!f.active || f.type != type) continue;

        float dx = position.x - f.center.x;
        float dy = position.y - f.center.y;
        float dz = position.z - f.center.z;
        float distSq = dx * dx + dy * dy + dz * dz;
        if (distSq >= f.radius * f.radius) continue;

        float dist = sqrtf(distSq);
        float influence = (1.0f - dist / f.radius) * f.intensity;
        if (influence > best) best = influence;
    }

    return best;
}

// ---------------------------------------------------------------------------
// Public: gameplay consequences (called from UpdateNetWorld after physics)
// ---------------------------------------------------------------------------

void UpdateDataFieldEffects(float dt)
{
    if (!g_netWorld.active || g_netWorld.state != NetWorldState::ACTIVE) return;

    Vector3 p = g_netWorld.player.position;

    // ---- ICE: shield / health drain ----
    float iceInf = GetDataFieldInfluence(p, DataFieldType::ICE);
    if (iceInf > 0.05f) {
        // Rate scales from 3/s at the boundary to 12/s at center. Blocks
        // shield regen so retreating to the edge doesn't out-heal the drain.
        float dps = 3.0f + 9.0f * iceInf;
        g_netWorld.player.shieldRegenTimer = 0.0f;

        g_iceDamageAccum += dps * dt;
        if (g_iceDamageAccum >= 1.0f) {
            int dmg = (int)g_iceDamageAccum;
            g_iceDamageAccum -= (float)dmg;

            if (g_netWorld.player.shield > 0) {
                g_netWorld.player.shield -= dmg;
                if (g_netWorld.player.shield < 0) {
                    g_netWorld.player.health += g_netWorld.player.shield;
                    g_netWorld.player.shield = 0;
                }
            } else {
                g_netWorld.player.health -= dmg;
            }
            if (g_netWorld.player.health < 0) g_netWorld.player.health = 0;
        }

        TriggerGlitch(0.15f * iceInf);
    } else {
        // Decay the accumulator when out of any ICE field so it doesn't
        // carry stale fractional damage into the next encounter.
        g_iceDamageAccum *= 0.9f;
    }

    // ---- NOISE: baseline glitch ----
    float noiseInf = GetDataFieldInfluence(p, DataFieldType::NOISE);
    if (noiseInf > 0.05f) {
        TriggerGlitch(0.20f * noiseInf);
    }
}

// ---------------------------------------------------------------------------
// Public: 3D draw
// ---------------------------------------------------------------------------

void DrawDataFields(void)
{
    if (!g_dataFieldsInit) return;

    float t = g_netWorld.time;
    Vector3 playerPos = g_netWorld.player.position;

    for (int i = 0; i < g_netWorld.dataFieldCount; i++) {
        const DataField& f = g_netWorld.dataFields[i];
        if (!f.active) continue;

        // Skip fields far enough away that nothing they draw could be more
        // than a sub-pixel dot. 60 units is generous.
        float distToPlayer = Vector3Distance(playerPos, f.center);
        if (distToPlayer > f.radius + 60.0f) continue;

        Color fcol = {
            (unsigned char)(f.color.x * 255),
            (unsigned char)(f.color.y * 255),
            (unsigned char)(f.color.z * 255),
            255
        };

        // ---- 1. Boundary shell ----
        // Low-alpha wireframe with a coarse ring count. The point of the
        // coarse tessellation is that it reads as a technical overlay — a
        // "scan boundary" — rather than a soft volumetric cloud.
        float shellPulse = 0.5f + 0.5f * sinf(f.phase * 1.2f);
        float shellAlpha = (0.06f + 0.05f * shellPulse) * f.intensity;
        DrawWireSphere(f.center, f.radius, Fade(fcol, shellAlpha), 6, 12);

        // ---- 2. Rising scan ring ----
        // A horizontal ring sweeping bottom-to-top on the 3s period, radius
        // clamped to the sphere's cross-section at the current height. This
        // is the element that reads most clearly as "something is scanning
        // this area" — it's what sells the field as diegetic rather than
        // decorative.
        float pulseT = f.pulseTimer / 3.0f;
        float ringY  = f.center.y - f.radius + pulseT * 2.0f * f.radius;
        float dyFromCenter = ringY - f.center.y;
        float ringRadiusAtY = sqrtf(fmaxf(0.0f,
                                f.radius * f.radius - dyFromCenter * dyFromCenter));
        if (ringRadiusAtY > 0.05f) {
            // Brightest at the middle of the sweep, dim at both ends so the
            // wrap-around isn't a visible pop.
            float ringFade = 1.0f - fabsf(pulseT - 0.5f) * 2.0f;
            DrawCircle3D({f.center.x, ringY, f.center.z}, ringRadiusAtY,
                         {0, 1, 0}, 0.0f,
                         Fade(fcol, 0.35f * ringFade * f.intensity));
        }

        // ---- 3. Interior particle lattice ----
        // Each particle drifts radially along its own unit direction. The
        // direction of drift is the field's personality: ICE consumes
        // inward, ENCRYPTION broadcasts outward, NOISE jitters in place.
        for (int p = 0; p < FIELD_PARTICLE_COUNT; p++) {
            const FieldParticle& fp = g_fieldParticles[i][p];

            float drift;
            switch (f.type) {
                case DataFieldType::ICE:
                    drift = 1.0f - fmodf(t * fp.speed * 0.25f + fp.phase, 1.0f);
                    break;
                case DataFieldType::ENCRYPTION:
                    drift = fmodf(t * fp.speed * 0.25f + fp.phase, 1.0f);
                    break;
                case DataFieldType::NOISE:
                default:
                    drift = fp.baseRadius + 0.15f * sinf(t * fp.speed + fp.phase);
                    break;
            }

            float r = f.radius * drift;
            Vector3 pos = {
                f.center.x + fp.direction.x * r,
                f.center.y + fp.direction.y * r,
                f.center.z + fp.direction.z * r
            };

            // Fade toward both extremes of drift so particles don't pop at
            // the center or the boundary.
            float edgeFade = 1.0f - fabsf(drift - 0.5f) * 1.4f;
            if (edgeFade < 0.0f) edgeFade = 0.0f;
            float alpha = edgeFade * f.intensity * 0.75f;

            DrawPoint3D(pos, Fade(fcol, alpha));
        }

        // ---- 4. Hex glyphs ----
        // A sparse set of 3D characters on the same lattice. Slightly slower
        // drift than the points so they don't read as being glued together.
        for (int p = 0; p < FIELD_GLYPH_COUNT; p++) {
            const FieldParticle& fp = g_fieldParticles[i][p];

            float drift;
            if (f.type == DataFieldType::ICE) {
                drift = 1.0f - fmodf(t * 0.10f + fp.phase, 1.0f);
            } else {
                drift = fmodf(t * 0.10f + fp.phase, 1.0f);
            }

            float r = f.radius * (0.30f + 0.60f * drift);
            Vector3 pos = {
                f.center.x + fp.direction.x * r,
                f.center.y + fp.direction.y * r + 1.0f,
                f.center.z + fp.direction.z * r
            };

            // Cull glyphs that would project off-screen before calling
            // GetWorldToScreen a second time inside DrawScaledText3D.
            Vector2 sp = GetWorldToScreen(pos, g_netWorld.camera);
            if (sp.x < -50 || sp.y < -50 ||
                sp.x > GetScreenWidth() + 50 ||
                sp.y > GetScreenHeight() + 50) continue;

            char str[2] = { fp.glyph, '\0' };
            float glyphAlpha = (0.15f + 0.25f * sinf(t * 1.5f + (float)p)) * f.intensity;
            if (glyphAlpha < 0.05f) glyphAlpha = 0.05f;

            DrawScaledText(str, (int)sp.x, (int)sp.y, 10, Fade(fcol, glyphAlpha));
        }
    }
}