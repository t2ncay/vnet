#include "raid.h"
#include "../../shared/vnet.h"
#include "../../shared/vnet_protocol.h"
#include "../render.h"
#include "../vnet_client.h"
#include "../game.h"
#include "../desktop/desktop.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <ctime>

// fwd decls
extern Player g_player;

// ============================================================
// RAID TRIGGER PROBABILITIES
// ============================================================
static const float RAID_TRIGGER_HIGH_TRACE = 0.0012f;   // ~4.3% per minute (rare but exciting)
static const float RAID_TRIGGER_MED_TRACE  = 0.0006f;   // ~2.1% per minute
static const float RAID_TRIGGER_LOW_TRACE  = 0.00025f;  // ~0.9% per minute (very rare)
static const float RAID_TRIGGER_FLAGGED_MULT = 1.3f;