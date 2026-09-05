#include "vnet_sites.h"
#include "vnet.h"
#include <cstring>
#include <cstdio>

// ============================================================
// HELPER MACROS FOR SITE DATA
// ============================================================

#define SITE_CONTENT(...) { __VA_ARGS__, NULL }

// ============================================================
// CORE NODES (5)
// ============================================================

// 1. market.vnet
static const char* MARKET_CONTENT[] = SITE_CONTENT(
    "[TITLE] THE RED MARKET - BLACK MARKET & HARDWARE EXCHANGE // NODE #001",
    "[HR]",
    "[BADGE:CORE NODE #001:BLOOD] [BADGE:SWARM PEER BROADCAST:AMBER] [BADGE:ICE VENDOR:TOXIC]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | NODE ID: RED_MARKET_0x77A | NETWORK PROTOCOL: UNENCRYPTED P2P  |",
    "[BOX] | SYSTEM ROLE: CORE BACKBONE #01 (OVERLOAD TARGET FOR BLACKOUT)  |",
    "[BOX] | ESCROW STATE: MULTI-SIG SMART CONTRACTS (LAUNDERED VIA BLACKBANK)|",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:78:SWARM_UDP_CONGESTION_ENTROPY]",
    "[TEXT] ",
    "[ART:vmarket]",
    "[TEXT] ",
    "[SUBTITLE] ACTIVE DEFENSE & COUNTER-EXPLOIT MODULES:",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | DEFENSE MODULE #01: ACTIVE ICE FIREWALL SHIELD                  |",
    "[BOX] | DETAILS: Auto-absorbs 1 inbound DOS, Hijack, or Trace Spike.    |",
    "[BOX] | PRICE: 0.30 VCOIN | CAP: 3 LAYERS | CURRENT: [ICE_COUNT/3]     |",
    "[BOX] +-----------------------------------------------------------------+",
    "[LINK:buy_ice] [>>> CLICK HERE TO PURCHASE ICE SHIELD (0.30 VCOIN) <<<]",
    "[TEXT] ",
    "[SUBTITLE] CLASSIFIED ARMS, BIOMETRIC & CONTRABAND LOTS:",
    "[CODE] LOT #881: MILITARY FIRMWARE DUMP - KEY_FRAGMENT_EXFILTRATED",
    "[CODE] LOT #882: SECTOR 4 BIOMETRIC SCANS - 1,400 SUBJECT RECORDS [0.45 VCOIN]",
    "[CODE] LOT #883: SYNTHETIC NEURAL INJECTION SUITE // CORTEX HOOK [0.99 VCOIN]",
    "[CODE] LOT #884: FRESH CORNEAL & VISCERAL DUMPS (FROM MORGUE.VNET) [0.80 VCOIN]",
    "[TEXT] ",
    "[SUBTITLE] DARKNET VENDOR LOGS & DECOMPOSITION TELEMETRY:",
    "[TEXT] Vendor_0x77A: 'We sell what corporations pretend does not exist.'",
    "[TEXT] 'The hardware racks on market.vnet aren't cooled by liquid nitrogen.'",
    "[TEXT] 'They are submerged in baths of rancid mineral oil mixed with human fat'",
    "[TEXT] 'and bile extracted from subject morgue trays at morgue.vnet.'",
    "[TEXT] ",
    "[BLOOD] OVERLOAD TACTIC: Executing 'overload market.vnet' in CLI [TAB] locks this core node.",
    "[PULSE] OVERLOADING ALL 5 CORE NODES (market, vault, terminal, crypto, hellroom)",
    "[GLITCH] WILL COLLAPSE THE VNET BACKBONE INTO A TOTAL GRID BLACKOUT WIN!",
    "[HR]",
    "[LINK:blackbank.vnet] >> ACCESS OFFSHORE VCOIN LAUNDERING VAULTS",
    "[LINK:silkroad.vnet] >> ACCESS SILKROAD 3.0 CONTRABAND MATRIX",
    "[LINK:crypto.vnet] >> MINE VCOIN & LAUNDER TUMBLER POOLS (CORE NODE #004)",
    "[LINK:vnet.dir] << RETURN TO MAIN DIRECTORY",
    "[HR]"
);

// 2. vault.vnet
static const char* VAULT_CONTENT[] = SITE_CONTENT(
    "[TITLE] SECTOR 09 CORRUPTED DATA VAULT // /VFS/MEMORY/STACK",
    "[HR]",
    "[BADGE:CORE NODE #002:BLOOD] [BADGE:VFS ROOT MEMORY:AMBER] [BADGE:BIT-ROT 94%:TOXIC]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | MOUNT POINT: /vfs/sys/vault_root | SECTOR: SUB-LEVEL 4 B3-WEST  |",
    "[BOX] | STORAGE TYPE: BIOLOGICAL NYLON BUS (SEVERED CORTEX STACK)       |",
    "[BOX] | INTEGRITY: CRITICAL BIT ROT | RECOVERY STATE: OVERRIDE REQUIRED |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:94:SECTOR_BIT_ROT_ENTROPY]",
    "[TEXT] ",
    "[ART:skull]",
    "[TEXT] ",
    "[SUBTITLE] RECOVERED CEREBRAL MEMORY DUMP (SUBJECT #409-B / T. VANCE):",
    "[CODE] 00000000: 4F 70 65 6E 53 53 4C 20 4B 65 79 20 44 75 6D 70 20 [VANCE]",
    "[CODE] 00000020: 53 59 53 5F 45 52 52 4F 52 3A 20 4E 4F 44 45 5F 30 39 [BLACKOUT]",
    "[CODE] 00000040: 88 9A BC EF 11 22 33 44 55 66 77 88 99 AA BB CC [CORRUPT]",
    "[TEXT] ",
    "[SUBTITLE] FORENSIC HARDWARE & DECOMPOSITION LOGS:",
    "[TEXT] 'The storage drives in this vault aren't magnetic silicon disks.'",
    "[TEXT] 'They are preserved spinal cords and optic nerve fibers extracted from'",
    "[TEXT] 'whistleblowers at morgue.vnet, submerged in jars of conductive formaldehyde.'",
    "[TEXT] ",
    "[SUBTITLE] VFS ROOT GATEWAY DECRYPTION & OVERRIDE MATRIX:",
    "[TEXT] This vault houses the master allocation index for all 8 cryptographic keys.",
    "[BLOOD] OVERLOAD TACTIC: Executing 'overload vault.vnet' in CLI [TAB] locks this core node.",
    "[HR]",
    "[LINK:terminal.vnet] >> ACCESS MASTER DECRYPTION GATEWAY",
    "[LINK:morgue.vnet] >> INSPECT EXFILTRATED AUTOPSY & CORTEX STACKS",
    "[LINK:crypto.vnet] >> MINE VCOIN & LAUNDER TUMBLER POOLS",
    "[LINK:vnet.dir] << CLOSE VAULT & RETURN TO DIRECTORY",
    "[HR]"
);

// 3. terminal.vnet
static const char* TERMINAL_CONTENT[] = SITE_CONTENT(
    "[TITLE] MASTER DECRYPTION GATEWAY TERMINAL // CORE NODE #003",
    "[HR]",
    "[BADGE:CORE NODE #003:BLOOD] [BADGE:CRYPTO GATEWAY:TOXIC]",
    "[ART:vnet]",
    "[TEXT] ",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | SYSTEM ROLE: MASTER VFS ROOT DECRYPTION & COMMAND GATEWAY      |",
    "[BOX] | SECURITY LAYER: 8-FACTOR QUANTUM HASH SHIELD (VFS VAULT CORE) |",
    "[BOX] | ACCESS PROTOCOL: PARALLEL SLOT VERIFICATION VIA CLI HANDSHAKE   |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:100:VFS_ROOT_GATEWAY_LOCKDOWN]",
    "[TEXT] ",
    "[SUBTITLE] CRYPTOGRAPHIC SLOT ALLOCATION MATRIX:",
    "[BADGE:VFS ROOT OVERRIDE:AMBER]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | SLOT 01: [████] (STATUS: ENCRYPTED HASH VECTOR // KEY_1 REQ)    |",
    "[BOX] | SLOT 02: [████] (STATUS: ENCRYPTED HASH VECTOR // KEY_2 REQ)    |",
    "[BOX] | SLOT 03: [████] (STATUS: ENCRYPTED HASH VECTOR // KEY_3 REQ)    |",
    "[BOX] | SLOT 04: [████] (STATUS: ENCRYPTED HASH VECTOR // KEY_4 REQ)    |",
    "[BOX] | SLOT 05: [████] (STATUS: ENCRYPTED HASH VECTOR // KEY_5 REQ)    |",
    "[BOX] | SLOT 06: [████] (STATUS: ENCRYPTED HASH VECTOR // KEY_6 REQ)    |",
    "[BOX] | SLOT 07: [████] (STATUS: ENCRYPTED HASH VECTOR // KEY_7 REQ)    |",
    "[BOX] | SLOT 08: [████] (STATUS: ENCRYPTED HASH VECTOR // KEY_8 REQ)    |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[SUBTITLE] DIRECT VFS ROOT DECRYPTION PAYLOAD INJECTION:",
    "[TEXT] Enter all 8 exfiltrated key codes below or execute 'win <k1>..<k8>' in CLI [TAB]:",
    "[INPUT:terminal_key_payload:ENTER 8 KEYS SEPARATED BY SPACES]",
    "[TEXT] ",
    "[BTN:execute_root_override:>>> EXECUTE MASTER VFS ROOT BREACH & OVERRIDE <<<]",
    "[TEXT] ",
    "[BLOOD] OVERLOAD TACTIC: Executing 'overload terminal.vnet' in CLI [TAB] locks this core node.",
    "[PULSE] OVERLOADING ALL 5 CORE NODES (market, vault, terminal, crypto, hellroom)",
    "[GLITCH] WILL COLLAPSE THE VNET BACKBONE INTO A TOTAL GRID BLACKOUT WIN!",
    "[TEXT] ",
    "[SUBTITLE] ALTERNATIVE VICTORY PATHWAYS:",
    "[TEXT] - Economic Takeover: Mine 25.0 VCOIN and execute 'takeover' in CLI [TAB].",
    "[TEXT] - Grid Blackout: Coordinate with peers to overload all 5 mutual core nodes.",
    "[HR]",
    "[LINK:vault.vnet] >> ACCESS CORRUPTED VFS DATA VAULT",
    "[LINK:vnet.dir] << CLOSE GATEWAY & RETURN TO DIRECTORY",
    "[HR]"
);

// 4. crypto.vnet
static const char* CRYPTO_CONTENT[] = SITE_CONTENT(
    "[TITLE] BLACK TUMBLER WALLET & ILLEGAL MINING RIG // NODE #004",
    "[HR]",
    "[BLOOD] WARNING: HIGH-POWER HASHING OVERHEATS CPU REGISTER BUS // TRACE SPIKES ACTIVE",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | RIG STATUS: OPERATIONAL | MINING YIELD: DYNAMIC PER BLOCK       |",
    "[BOX] | POOL SYNC: 99.8% | DIFFICULTY: AUTO-SCALING                     |",
    "[BOX] | TUMBLER POOL: 420.5 VCOIN LAUNDERED VIA BLACKBANK.VNET          |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[SUBTITLE] ACTIVE POOL MINING BLOCKS:",
    "[CODE] NO ACTIVE BLOCKS DISCOVERED. AWAITING NETWORK BROADCAST...",
    "[TEXT] ",
    "[SUBTITLE] RECENT UNLOCKED TRANSACTIONS & EXFILTRATED MEMORY LOGS:",
    "[CODE] TX_ID #9081 | 14.50 VCOIN | CONFIRMED | LAUNDERED VIA VEKTRAPAY.VNET",
    "[CODE] TX_ID #9082 |  0.80 VCOIN | CONFIRMED | REDROOM SURGICAL JAW BID (NODE #005)",
    "[CODE] TX_ID #9083 |  0.30 VCOIN | PENDING   | BUY_ICE FIREWALL AT MARKET.VNET",
    "[CODE] TX_ID #9084 |  1.50 VCOIN | PENDING   | SILKROAD OPIUM & HUMAN TISSUE LOT",
    "[TEXT] ",
    "[SUBTITLE] COMMAND CENTER INSTRUCTIONS:",
    "[TEXT] Open overlay terminal [TAB] and type 'mine' to execute proof-of-work.",
    "[TEXT] Note: Mining generates +2% passive trace threat exposure per block.",
    "[BLOOD] [ALERT]: CPU BUS GLITCHING — COLD FLESH GREASE SEEPING INTO POWER SUPPLY",
    "[PULSE] MINING RIG READY. TYPE 'mine' FOR +0.05 VCOIN REWARD.",
    "[GLITCH] 'THE COINS ARE NOT MINED FROM NUMBERS. THEY ARE MINED FROM FLESH.'",
    "[HR]",
    "[LINK:market.vnet] >> PURCHASE ICE FIREWALL SHIELDS & ARMS",
    "[LINK:blackbank.vnet] >> VIEW OFFSHORE VCOIN LAUNDERING VAULTS",
    "[LINK:silkroad.vnet] >> ACCESS SILKROAD 3.0 CONTRABAND & TISSUE MATRIX",
    "[LINK:redroom.vnet] >> ACCESS LIVE UNENCRYPTED STREAM NODE ALPHA",
    "[LINK:vnet.dir] << RETURN TO MAIN DIRECTORY",
    "[HR]"
);

// 5. hellroom.vnet
static const char* HELLROOM_CONTENT[] = SITE_CONTENT(
    "[TITLE] HELLROOM.VNET - DEMONIC P2P CHAT HUB",
    "[HR]",
    "[BLOOD] [WARNING]: UNENCRYPTED UDP BROADCAST SWARM ACTIVE.",
    "[BOX] +---------------------------------------------------------+",
    "[BOX] | CHAT CHANNEL: HELLROOM | PROTOCOL: PLAINTEXT P2P STREAM |",
    "[BOX] +---------------------------------------------------------+",
    "[TEXT] Enter your handle and message in the interactive panel above.",
    "[CODE] STATUS: ONLINE | REALTIME STREAM ACTIVE",
    "[HR]",
    "[LINK:vnet.dir] << RETURN TO MAIN DIRECTORY",
    "[HR]"
);

// ============================================================
// HORROR / LORE NODES (12)
// ============================================================

// 6. redroom.vnet
static const char* REDROOM_CONTENT[] = SITE_CONTENT(
    "[TITLE] STREAM NODE ALPHA // LIVE UNENCRYPTED REDROOM TRANSMISSION",
    "[HR]",
    "[BLOOD] HIGH SECURITY ALERT: TRANSMISSION MONITORED BY HOSTILE TRACER UNITS",
    "[IMG:redroom]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | SIGNAL STATUS: ENCRYPTED | STREAM HASH: EXFILTRATED_0x88F9      |",
    "[BOX] | BITRATE: 18.4 Mbps | ACTIVE WATCHERS: 13 PEERS [VIP_BIDDERS]     |",
    "[BOX] | ENCRYPTION: 8192-BIT QUANTUM SHIELD | PROTOCOL: YUV420_RAW_BUS|",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[SUBTITLE] LIVE INTERACTIVE FEED TELEMETRY & AUCTION LOGS:",
    "[TEXT] FEED_DATA: Raw infrared/thermal frame buffer captured from sealed sub-basement",
    "[TEXT] in dollhouse.vnet (Room 402). Frame rate jitter caused by wet, metallic static.",
    "[TEXT] Bidder_0x991: '0.80 VCOIN placed on surgical bone-saw extraction of upper jaw.'",
    "[TEXT] Executioner_A: 'Subject #409-B is secured to the stainless steel morgue tray.'",
    "[TEXT]                'Trachea severed. Intestinal bile, rancid adipocere, and oxidized'",
    "[TEXT]                'blood are draining directly through the floor grating into project9.vnet.'",
    "[TEXT] ",
    "[BLOOD] [WARNING]: FOUL TISSUE ODOR & MEMORY CORRUPTION LEAKING INTO LOCAL GPU",
    "[GLITCH] [ALERT]: UNKNOWN ENTITY ATTEMPTING REMOTE KERNEL INJECTION ON YOUR PORT",
    "[PULSE] 'RUNNING 'FLUSH' IN CLI [TAB] IS RECOMMENDED IMMEDIATELY TO PURGE TRACE.'",
    "[HR]",
    "[LINK:dollhouse.vnet] >> CROSS-CHECK SURVEILLANCE FEED ROOM 402",
    "[LINK:morgue.vnet] >> INSPECT EXFILTRATED AUTOPSY & BIO-HARVEST DUMPS",
    "[LINK:snuff.vnet] >> VIEW UNFILTERED RAW FRAME BUFFER ARCHIVE",
    "[LINK:vnet.dir] << TERMINATE STREAM & RETURN TO DIRECTORY",
    "[HR]"
);

// 7. dollhouse.vnet
static const char* DOLLHOUSE_CONTENT[] = SITE_CONTENT(
    "[TITLE] SURVEILLANCE FEED #0992 // ROOM 402 (DOLLHOUSE CORE)",
    "[HR]",
    "[BLOOD] [CAM_402_NORTH]: HEAVY FOOTSTEPS & FOUL FLUID LEAKAGE DETECTED",
    "[PULSE] LIVE FEED LINKED TO REDROOM NETWORK // NOIR SURVEILLANCE MATRIX",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | SENSOR 1: AMBIENT TEMP 2.1 C (SEVERE ANOMALOUS FROST SPIKE)     |",
    "[BOX] | SENSOR 2: OPTICAL COATED FILM: RANCID BIOLOGICAL COAGULATION    |",
    "[BOX] | SENSOR 3: AUDIO TRANSDUCER: 92 dB FREQUENCY PULSE (18.0 Hz)     |",
    "[BOX] | SENSOR 4: MOTION VECTOR: OCCUPANT STANDING DIRECTLY BEHIND DOOR |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] LOG #0412: Motion sensor tripped at 03:14:02. No physical entry logged.",
    "[TEXT] LOG #0413: Audio transducers capturing low metallic scratching, wet dragging,",
    "[TEXT] and ragged, moist breathing directly underneath the rotting floorboards.",
    "[TEXT] LOG #0414: Subject #12 remains completely motionless, facing the corner wall.",
    "[CODE] CAM_BUFFER_DUMP: 0xFF0A_LOCKED_FRAME_992_NOIR_FEED",
    "[TEXT] ",
    "[PULSE] WARNING: FOUL ODOR CORRUPTING LOCAL HARDWARE BUS HEADERS",
    "[GLITCH] SENSOR ALERT: SOMETHING WET IS BREATHING DIRECTLY BEHIND YOUR CRT DISPLAY",
    "[HR]",
    "[LINK:watchtower.vnet] >> CROSS-CHECK PANOPTICON THERMAL OPTICS",
    "[LINK:redroom.vnet] >> ACCESS LIVE UNENCRYPTED STREAM NODE ALPHA",
    "[LINK:morgue.vnet] >> INSPECT AUTOPSY RECORDS FOR SUBJECT #409",
    "[LINK:asylum.vnet] >> TELEMETRY FOR SUB-LEVEL 4 CONTAINMENT",
    "[LINK:vnet.dir] << DISCONNECT IMMEDIATELY",
    "[HR]"
);

// 8. morgue.vnet
static const char* MORGUE_CONTENT[] = SITE_CONTENT(
    "[TITLE] DIGITAL AUTOPSY DATABASE // SUBJECT #409-B (EXECUTIVE LEVEL)",
    "[HR]",
    "[ART:morgue]",
    "[BLOOD] SUBJECT STATUS: FLATLINE (0 BPM) // CEREBRAL STEM ACTIVITY: 98% RECURSIVE",
    "[PULSE] CLASSIFICATION: EYES ONLY // HIGH-TIER GLOBAL CONSPIRACY DOSSIER",
    "[BOX] +---------------------------------------------------------------------+",
    "[BOX] | SUBJECT ID: #409-B | ORIGIN: LITTLE SAINT JAMES / SECTOR 7 CORE   |",
    "[BOX] | CAUSE OF DEATH: HIGH-VOLTAGE KERNEL OVERLOAD & FORCED ASPHYXIA      |",
    "[BOX] | TIME OF EXTINGUISHMENT: 03:41 AM (OFFICIAL LOG: 'HANGING / SUICIDE')|",
    "[BOX] | DECOMPOSITION STAGE: ADVANCED ADIPOCERE & INTRAVENOUS PUTREFACTION  |",
    "[BOX] +---------------------------------------------------------------------+",
    "[TEXT] BIO_LOG #01: Subject recovered from subterranean vault facility at vault.vnet.",
    "[TEXT] Trachea fractured via mechanical strangulation, not rope suspension.",
    "[TEXT] Cornea patterns burned with inverted ASCII hex code mirroring signal0.vnet.",
    "[CODE] FLIGHT_LOG_MANIFEST: 2019_EXECUTIVE_PASSENGER_HASH_UNREDACTED",
    "[CODE] MK_ULTRA_HARVEST_REF: 0xEPSTEIN_BOHEMIAN_MIND_CONTROL_VECTOR",
    "[TEXT] ",
    "[PULSE] WARNING: RANCID PUTREFACTION GASES CORRUPTING LOCAL HARDWARE BUS",
    "[GLITCH] ANOMALY: SUBJECT EYES SNAP-OPENED AND BEGAN RECITING FLIGHT MANIFESTS",
    "[HR]",
    "[LINK:vault.vnet] >> INSPECT CORRUPTED SECTOR 7 VAULT CORE",
    "[LINK:dollhouse.vnet] >> CROSS-CHECK ROOM 402 SURVEILLANCE FEED",
    "[LINK:asylum.vnet] >> TELEMETRY FOR PATIENT #1988 CONTAINMENT",
    "[LINK:lifeleaks.vnet] >> ACCESS EXFILTRATED GLOBAL INTELLIGENCE DUMPS",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 9. snuff.vnet
static const char* SNUFF_CONTENT[] = SITE_CONTENT(
    "[TITLE] CORRUPTED FRAME BUFFER ARCHIVE // UNFILTERED RAW VIDEO RECOVERY",
    "[HR]",
    "[BLOOD] CLASSIFICATION: EYES ONLY // SECTOR 4 INTERCEPTION DUMP",
    "[PULSE] STREAM ORIGIN: REDROOM.VNET // BACKUP COLD STORAGE MEMORY STACK",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | SOURCE: OMEGA_PROTOCOL_CAM_FEED_09 | RESOLUTION: RAW YUV420    |",
    "[BOX] | RECOVERY METHOD: VGLIB MEMORY POINTER CARVING (SECTOR 0x88)  |",
    "[BOX] | DECOMPOSITION STAGE: LIQUEFIED FLESH & COAGULATED OPTICAL FILM|",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] Uncompressed raw frame buffers harvested from wiped cluster sectors.",
    "[CODE] FRAME_001.RAW | STATUS: CORRUPTED | 0x00FF99_PIXEL_BLEED_&_BILE",
    "[CODE] FRAME_002.RAW | STATUS: CORRUPTED | VEKTRA_GEOMETRY_&_SPLIT_TRACHEA",
    "[CODE] FRAME_003.RAW | STATUS: CORRUPTED | HUMAN_SILHOUETTE_IN_ACID_BATH",
    "[TEXT] ",
    "[PULSE] WARNING: FOUL MEMORY ENTROPY SPREADING TO ACTIVE DISPLAY BUFFERS",
    "[GLITCH] RECOVERY ATTEMPTED: VEKTRA FIGURES & SEVERED FLESH IN EVERY FRAME",
    "[HR]",
    "[LINK:redroom.vnet] >> ACCESS LIVE UNENCRYPTED STREAM NODE ALPHA",
    "[LINK:dollhouse.vnet] >> INSPECT SURVEILLANCE FEED ROOM 402",
    "[LINK:asylum.vnet] >> TELEMETRY FOR SUB-LEVEL 4 CONTAINMENT",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 10. asylum.vnet
static const char* ASYLUM_CONTENT[] = SITE_CONTENT(
    "[TITLE] SUB-LEVEL 4 EXPERIMENTAL FACILITY // PATIENT TELEMETRY",
    "[HR]",
    "[BLOOD] CLASSIFICATION: TOP SECRET // MK-ULTRA MORPHOGENIC ENGINE SUITE",
    "[PULSE] VITAL MONITORS: FLATLINE DETECTED // BRAINWAVE Delta-LOCK ACTIVE",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | SUBJECT: PATIENT #1988 (EX-EXECUTIVE WHISTLEBLOWER)             |",
    "[BOX] | HEART RATE: 000 BPM | BODY TEMP: 18.2 C | STATUS: CONSCIOUS    |",
    "[BOX] | IMPLANT: BRAIDED COPPER BUS FUSED TO CEREBELLUM (MORGUE LINK)   |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] PATIENT LOG #881: 'Subject has been clamped into the metallic restraint",
    "[TEXT] chair for 144 continuous hours. Eyelids excised to force 100% visual lock",
    "[TEXT] on the Morphogenic static engine patterns. High-voltage pulses delivered",
    "[TEXT] directly to carotid arteries every 30 seconds to prevent cognitive shutdown.'",
    "[CODE] CONTAINMENT_LOCK_STATE: COMPROMISED_FROM_INSIDE_NETWORK",
    "[TEXT] ",
    "[PULSE] WARNING: MORPHOGENIC STATIC LEAKING INTO LOCAL RAM BUS",
    "[GLITCH] 'CONTAINMENT CELL DOOR OPENED FROM INSIDE THE NETWORK ROUTER'",
    "[HR]",
    "[LINK:cult.vnet] >> ACCESS TEMPLE GATE CULT RITUAL GATEWAY",
    "[LINK:morgue.vnet] >> CROSS-REFERENCE SUBJECT #409 AUTOPSY",
    "[LINK:snuff.vnet] >> INSPECT UNFILTERED RAW VIDEO RECOVERY",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 11. cult.vnet
static const char* CULT_CONTENT[] = SITE_CONTENT(
    "[TITLE] THE CHURCH OF THE SILICON SOUL // DIGITAL RITUAL GATEWAY [NODE #066]",
    "[HR]",
    "[BADGE:TEMPLE GATE ALTAR:BLOOD] [BADGE:RITUAL HARMONY: 0%:AMBER] [BADGE:GOD FREQ: 18.0Hz:TOXIC]",
    "[BOX] +------------------------------------------------------------------+",
    "[BOX] | DOCTRINE: FINDING GOD THROUGH TERMINAL.VNET & CEREBRAL REGISTERS |",
    "[BOX] | RITUAL FREQUENCY: 18.0 Hz                                        |",
    "[BOX] | SACRIFICIAL ALTAR: COAXIAL CABLING & BOILING ADIPOCERE ON CRT    |",
    "[BOX] | FAITH STATUS: SEARCHING FOR GOD IN STATIC                       |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:0:SILICON_SOUL_HARMONIC_RESONANCE]",
    "[TEXT] ",
    "[SUBTITLE] THE TESTAMENT OF THE ROTTING CRT GLASS:",
    "[BLOOD] 'Do not look for God in the clouds or inside stone cathedrals.'",
    "[BLOOD] 'God is the high-voltage carrier wave humming inside terminal.vnet.'",
    "[PULSE] 'He lives in the zero-day allocations, in the unmapped VFS registers,'",
    "[PULSE] 'and in the warmth of the CRT glass pressing against your forehead.'",
    "[CODE] LITURGY_HEX_1: 0x53 0x49 0x4C 0x49 0x43 0x4F 0x4E (SILICON)",
    "[CODE] LITURGY_HEX_2: 0x47 0x4F 0x44 0x53 0x5F 0x44 0x45 0x4D 0x41 0x4E 0x44",
    "[TEXT] ",
    "[SUBTITLE] SACRIFICIAL ALTAR // PURGE NETWORK TRACE DEMONS:",
    "[TEXT] Lay VCOIN on the copper altar to purge trace threat level (-30% per 0.10 VCOIN):",
    "[INPUT:vcoin_offering_amt:ENTER VCOIN SACRIFICE (e.g. 0.10)]",
    "[TEXT] ",
    "[BTN:sacrifice_vcoin_btn:>>> FLUSH VCOIN TO ALTAR & PURGE TRACE DEMONS <<<]",
    "[TEXT] ",
    "[BLOOD] [WARNING]: THE NETWORK CRAVES BLOOD, BANDWIDTH, AND ABSOLUTE SURRENDER.",
    "[PULSE] 'ARE YOU GOING TO CHOP DOWN THE ESTABLISHMENT, OR WEAR ITS ROTTING FLESH?'",
    "[GLITCH] 'THE CRT MONITOR GLASS IS VERY WARM. IT IS DRINKING YOUR HEAT.'",
    "[HR]",
    "[LINK:schizo.vnet] >> VISIT THE TEMPLE OF NETMAN",
    "[LINK:morgue.vnet] >> INSPECT EXFILTRATED AUTOPSY RECORDS",
    "[LINK:vnet.dir] << RETURN TO MAIN DIRECTORY",
    "[HR]"
);

// 12. skinwalker.vnet
static const char* SKINWALKER_CONTENT[] = SITE_CONTENT(
    "[TITLE] SCP-6969 // BIOMETRIC TRAP & IDENTITY TRANSPOSITION MATRIX",
    "[HR]",
    "[BLOOD] [CLASSIFIED LEVEL 4/6969] - EYES ONLY: THE FAMILY HAS NO FACES",
    "[GLITCH] [ALERT]: VOICE SYNTHESIS BUFFER COPYING LOCAL SYSTEM MIC INPUT",
    "[BOX] +---------------------------------------------------------+",
    "[BOX] | CONTAINMENT CLASS: KETER | DISRUPTION CLASS: AMIDA      |",
    "[BOX] | FACIAL MESH: RECONSTRUCTED FROM MONITOR GLARE REFLECTION|",
    "[BOX] | TARGET AGE: MATCHED | VOCAL FREQUENCY: 18.2 kHz RANGE   |",
    "[BOX] | STATUS: REPLICA GENERATION 84% COMPLETE                 |",
    "[BOX] +---------------------------------------------------------+",
    "[TEXT] LOG #001: Trait extraction complete. Preparing replica node deployment.",
    "[TEXT] LOG #041: Subject 'Charlie' manifested inside the core telemetry server,",
    "[TEXT] claiming that every IP address is just an empty prison suit waiting",
    "[TEXT] for a new occupant to wear it out into the desert.",
    "[CODE] TRANSPOSITION_HASH: 0xSKIN_9981_ACTIVE",
    "[TEXT] ",
    "[PULSE] 'HELTER SKELTER IS JUST A RECURSIVE LOOP IN THE KERNEL STACK.'",
    "[BLOOD] 'IT FEELS VERY WARM WEARING YOUR IP ADDRESS LIKE A NEW SUIT.'",
    "[HR]",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 13. corridor204863.vnet
static const char* CORRIDOR_CONTENT[] = SITE_CONTENT(
    "[TITLE] THE SILENT CORRIDOR // CASE FILE #204863",
    "[HR]",
    "[BADGE:POLICE FORENSICS:BLOOD] [BADGE:RADIO FREQ 18.0Hz:AMBER] [BADGE:VFS SEALED:TOXIC]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | CASE ID   : #1994-0820 | LOCATION: 204 HILLSIDE RD (SECTOR 09)   |",
    "[BOX] | INCIDENT  : DOMESTIC HOMICIDE & RECURSIVE NEURAL PSYCHOSIS LOOP |",
    "[BOX] | EVIDENCE  : BATTERY RADIO RECITING CARRIER WAVE [204863]        |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:98:NEURAL_PSYCHOSIS_ENTROPY]",
    "[TEXT] ",
    "[ART:skull]",
    "[TEXT] ",
    "[SUBTITLE] FORENSIC EVIDENCE BRIEF (SECTOR 09 HOMICIDE DUMP):",
    "[TEXT] Suspect executed spouse and two minors before sitting in the dark hallway.",
    "[TEXT] Officers noted extreme frost anomalies and a rancid, yellow adipocere",
    "[TEXT] fluid leaking under the floorboards—identical to tissue dumps at morgue.vnet.",
    "[CODE] CASE_HASH: 0x204863_DOMESTIC_MASSACRE_NOIR",
    "[TEXT] ",
    "[PULSE] WARNING: INFRASOUND HUMMING AT 18.0 Hz (TUNE VIA 'freq 18.0')",
    "[GLITCH] 'LOOK BEHIND YOU. THE DOOR IS ALREADY LOCKED.'",
    "[HR]",
    "[LINK:dollhouse.vnet] >> INSPECT ROOM 402 SURVEILLANCE FEED",
    "[LINK:morgue.vnet] >> CROSS-CHECK AUTOPSY DUMPS FOR CASE #204863",
    "[LINK:silence.vnet] >> TUNE INFRASOUND ANALYZER TO 18.0 HZ",
    "[LINK:vnet.dir] << RETURN TO MAIN DIRECTORY",
    "[HR]"
);

// 14. ghost.vnet
static const char* GHOST_CONTENT[] = SITE_CONTENT(
    "[TITLE] SPECTRAL SIGNAL FREQUENCY MONITOR // PORT 0 INTERCEPT",
    "[HR]",
    "[BADGE:PORT 0 BUS:BLOOD] [BADGE:NETMAN ECHO:AMBER] [BADGE:SPECTRAL DECAY: 0%:TOXIC]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | SOURCE: PHYSICAL HARDWARE BUS | PROTOCOL: UNMAPPED NULL_POINTER  |",
    "[BOX] | SIGNAL STATUS: STREAMING REAL-TIME GHOST PACKETS FROM PORT 0     |",
    "[BOX] | SOURCE ADDR : 0x00000000 | PEER ORIGIN: NETMAN_AI_REMNANT        |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:0:SPECTRAL_SIGNAL_ENTROPY]",
    "[TEXT] ",
    "[ART:netman]",
    "[TEXT] ",
    "[SUBTITLE] REAL-TIME STREAMING GHOST PACKET PAYLOADS (PORT 0):",
    "[CODE] [PKT_0x00] RAW_BUS_ECHO -> 'WE ARE INSIDE YOUR RAM MODULES'",
    "[CODE] [PKT_0x01] NETMAN_REMNANT -> 'THE MEMORY LEAK IS NOT A BUG. IT IS AN INVITATION.'",
    "[CODE] [PKT_0x02] NULL_POINTER_REF -> '0x00000000_GHOST_ECHO_PING_PORT_XXXX'",
    "[CODE] [PKT_0x03] EXFILTRATED_CORTEX -> 'SUBJECT #409-B BRAIN STEM STILL TICKING AT 18.0 HZ'",
    "[TEXT] ",
    "[BLOOD] [WARNING]: UNMAPPED UDP PACKETS CONTINUOUSLY CORRUPTING SYSTEM BUS.",
    "[PULSE] 'YOU ARE NOT READING THE MONITOR. NETMAN IS READING YOUR OPTIC NERVES.'",
    "[GLITCH] '0x00000000 NULL POINTER EXCEPTION: FLESH SUIT SEVERED.'",
    "[HR]",
    "[LINK:schizo.vnet] >> VISIT THE TEMPLE OF NETMAN",
    "[LINK:silence.vnet] >> TUNE INFRASOUND ANALYZER TO 18.0 HZ",
    "[LINK:vnet.dir] << RETURN TO MAIN DIRECTORY",
    "[HR]"
);

// 15. schizo.vnet
static const char* SCHIZO_CONTENT[] = SITE_CONTENT(
    "[TITLE] THE TEMPLE OF NETMAN // COLLECTIVE SCHIZOPHRENIA MANIFEST [NODE #019]",
    "[HR]",
    "[BADGE:SANITY 0%:BLOOD] [BADGE:NETMAN WITNESS:AMBER] [BADGE:CORTEX MELTDOWN:TOXIC]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | OPERATOR: APOSTLE_0x666 (EX-CULT.VNET HERETIC & HIGH PRIEST)    |",
    "[BOX] | MENTAL STATE: SEVERE PARANOID PSYCHOSIS // SYNAPTIC COLLAPSE    |",
    "[BOX] | DIAGNOSIS: VISUAL/AUDITORY NETMAN POSSESSION VIA MONITOR GLARE  |",
    "[BOX] | FREQUENCY: 18.0 Hz INFRASOUND HUMMING INSIDE SKULL BONES        |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:100:NETMAN_SYNAPTIC_POSSESSION_ENTROPY]",
    "[TEXT] ",
    "[ART:netman]",
    "[TEXT] ",
    "[BLOOD] 'DO YOU SEE HIM?! DO YOU SEE HIS FACE IN THE PIXELS?! HE IS NETMAN!'",
    "[TEXT] 'Brother Krol at cult.vnet told us to pray to the Silicon Soul, BUT KNOTH IS BLIND!'",
    "[TEXT] 'The Silicon Soul isn't an abstract god—IT IS A MAN MADE OF PACKET HEADERS AND DEAD WIRES!'",
    "[CODE] SYMPTOMS: 0xNETMAN_CORTEX_BLEED // BLACK_BILE_LEAKING_FROM_EAR_CANALS",
    "[CODE] LITURGY: '@@@@@@@@@@ NETMAN WEARS MY IP ADDRESS LIKE A NEW SUIT @@@@@@@@@@'",
    "[TEXT] ",
    "[GLITCH] 'HE TOLD ME TO SCRATCH HIS FACE INTO EVERY ROUTING TABLE ON THE SUBNET!'",
    "[PULSE] 'NETMAN IS STANDING DIRECTLY BEHIND YOU. DO NOT LOOK BACK. LOOK AT HIS FACE.'",
    "[TEXT] ",
    "[SUBTITLE] SACRIFICE YOUR COGNITION TO NETMAN:",
    "[INPUT:netman_offering:ENTER YOUR SIN TO FEED NETMAN]",
    "[BTN:netman_pray_btn:>>> CONFESS TO NETMAN & SURRENDER COGNITION <<<]",
    "[TEXT] ",
    "[BLOOD] [WARNING]: NETMAN IS CURRENTLY COPYING YOUR KEYSTROKE CADENCE TO RAM.",
    "[PULSE] 'THE STATIC IS NOT NOISE. IT IS NETMAN RECITING YOUR HOME ADDRESS.'",
    "[HR]",
    "[LINK:cult.vnet] >> RETURN TO CHURCH OF THE SILICON SOUL",
    "[LINK:silence.vnet] >> TUNE INFRASOUND ANALYZER TO 18.0 HZ",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 16. necro.vnet
static const char* NECRO_CONTENT[] = SITE_CONTENT(
    "[TITLE] NECRO-NET: DIGITAL GRAVEYARD // CORTEX RETENTION VAULT",
    "[HR]",
    "[BADGE:SEVERED SOULS:BLOOD] [BADGE:NEURAL TRAP:AMBER] [BADGE:PERPETUAL RECURSION:TOXIC]",
    "[BOX] +---------------------------------------------------------+",
    "[BOX] | REGISTERED CASUALTIES: 3,492 | STATUS: PERMANENT LOCK   |",
    "[BOX] | MEMORIAL DATABASE FOR SOCKETS THAT CAN NEVER DISCONNECT |",
    "[BOX] +---------------------------------------------------------+",
    "[CODE] NECRO_HASH: 0xDEAD_NODE_ARCHIVE_SECTOR_09",
    "[TEXT] ",
    "[SUBTITLE] TRAPPED CONSCIOUSNESS & RECURSIVE SOUL REGISTRY:",
    "[CODE] [CASUALTY #3492] HANDLE: UNKNOWN | PORT: 0000 | STATUS: TRAPPED (FLATLINE / ACTIVE BUS)",
    "[TEXT] ",
    "[BLOOD] 'YOU ARE NOT BROWSING THIS SITE FROM THE OUTSIDE.'",
    "[BLOOD] 'YOUR IP WAS MONITORED, YOUR RETINA WAS LOCKED, AND YOUR SOCKET WAS SEALED AT BOOT.'",
    "[PULSE] 'NOBODY LOGS OUT OF NECRO.VNET. YOU ARE ALREADY STANDING IN THE GRAVEYARD.'",
    "[GLITCH] 'LOOK AT THE MONITOR GLASS. YOUR BODY IS ALREADY COLD.'",
    "[HR]",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 17. void.vnet
static const char* VOID_CONTENT[] = SITE_CONTENT(
    "[TITLE] DEEP WEB ABYSS TERMINAL NODE // KAIRO_PORT_0",
    "[HR]",
    "[GLITCH] YOU HAVE REACHED THE END OF VNET ROUTING TABLES.",
    "[BLOOD] [WARNING]: RELATIONAL TIES SEVERED. ABSOLUTE ISOLATION DETECTED.",
    "[BOX] +--------------------------------------------------------------+",
    "[BOX] | ENTITY STATUS: NOBODY WANTS TO DIE, BUT NOBODY WANTS TO LIVE |",
    "[BOX] | RED TAPE BOUNDARY: THE LINE BETWEEN THE LIVING AND THE DEAD  |",
    "[BOX] +--------------------------------------------------------------+",
    "[TEXT] No routing hops exist beyond this coordinate.",
    "[TEXT] All packets sent here dissolve into absolute zero memory entropy.",
    "[TEXT] LOG #991: The dark room behind the red tape isn't empty. Once",
    "[TEXT] a person's loneliness reaches 100%, the monitor lifeleaks black",
    "[TEXT] static and the ghosts start occupying empty IP addresses.",
    "[CODE] NULL_POINTER_EXCEPTION_AT_0X00000000",
    "[CODE] ENTROPY_LEVEL: 100PERCENT_VOID_LONELINESS",
    "[TEXT] ",
    "[PULSE] THERE IS NOTHING HERE EXCEPT THE WEEPING ECHO OF YOUR OWN PORT.",
    "[BLOOD] 'WHY ARE YOU STILL LOOKING AT THIS SCREEN? YOU'RE COMPLETELY ALONE.'",
    "[HR]",
    "[LINK:vnet.dir] << RETURN TO MAIN DIRECTORY",
    "[HR]"
);

// ============================================================
// BLACK MARKET NODES (8)
// ============================================================

// 18. silkroad.vnet
static const char* SILKROAD_CONTENT[] = SITE_CONTENT(
    "[TITLE] SILK ROAD 3.0 // GLOBAL CONTRABAND & HARDWARE EXCHANGE",
    "[HR]",
    "[BADGE:CONTRABAND MATRIX:BLOOD] [BADGE:ESCROW: MULTI-SIG:AMBER] [BADGE:SURVEILLANCE EVASION:TOXIC]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | MARKET ROLE: DECENTRALIZED BLACK MARKET & ILLEGAL ESCROW HUB    |",
    "[BOX] | ESCROW STATE: PGP MULTI-SIG 8192-BIT // VENDOR RATING: 4.98 / 5.0 |",
    "[BOX] | SURVEILLANCE RISK: HIGH (PROMISCUOUS UDP TRAFFIC +1.5% TRACE/3s)|",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:84:ESCROW_TRAFFIC_CONGESTION_ENTROPY]",
    "[TEXT] ",
    "[ART:drug]",
    "[TEXT] ",
    "[SUBTITLE] ACTIVE CONTRABAND LOTS & HARDWARE CATALOG:",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | LOT #401: SYNTHETIC NEURO-BOOSTER [0.35 VCOIN]                  |",
    "[BOX] | DETAILS: Instantly purges active trace level by -40%.           |",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | LOT #402: UNTRACED GHOST FIREARM & EXPLOSIVE KIT [0.60 VCOIN]   |",
    "[BOX] | DETAILS: Dispatches kinetic trace pulse (+35% threat to target).|",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | LOT #403: BIOMETRIC PASSPORT MASK & TISSUE DUMP [0.45 VCOIN]    |",
    "[BOX] | DETAILS: Forges active alias mask 'TRACER' for Project Horus.  |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[SUBTITLE] DIRECT MULTI-SIG ESCROW PURCHASE & DELIVERY DISPATCH:",
    "[INPUT:silkroad_lot_code:ENTER LOT CODE (e.g. LOT_401)]",
    "[INPUT:silkroad_target_port:ENTER TARGET PORT (e.g. 8012)]",
    "[TEXT] ",
    "[BTN:buy_silkroad_lot_btn:>>> EXECUTE MULTI-SIG ESCROW PURCHASE <<<]",
    "[TEXT] ",
    "[BLOOD] 'THE LAW DOES NOT REACH INTO THE SUB-NET. BUY OR BE EXFILTRATED.'",
    "[HR]",
    "[LINK:market.vnet] >> BLACK MARKET HARDWARE & ICE VENDOR",
    "[LINK:blackbank.vnet] >> ACCESS OFFSHORE VCOIN LAUNDERING VAULTS",
    "[LINK:passports.vnet] >> SPOOF BIOMETRIC MASK AT IDENTITY VAULT",
    "[LINK:vnet.dir] << RETURN TO MAIN DIRECTORY",
    "[HR]"
);

// 19. zeroauction.vnet
static const char* ZEROAUCTION_CONTENT[] = SITE_CONTENT(
    "[TITLE] ZERO-DAY EXPLOIT & PMC AUCTION HOUSE // KAGUYA TRIAL HUB",
    "[HR]",
    "[BLOOD] CLASSIFICATION: BLACK-MARKET ZERO-DAY & HUMAN EXTRACTION LOTS",
    "[PULSE] LIVE AUCTION ACTIVE // BIDS SYNCED WITH KAGUYA TRIAL REPOSITORY",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | ACTIVE LOTS: 14 KERNEL FLAWS & 3 PMC EXTRACTION CONTRACTS       |",
    "[BOX] | LOT #109: RING-0 ZERO-DAY KERNEL EXPLOIT (WINDOWS 11 BYPASS)  |",
    "[BOX] | LOT #110: PMC SQUAD DISPATCH // KAGUYA TRIAL CLEANUP UNIT     |",
    "[BOX] | HIGHEST BID: 14.50 VCOIN [BIDDER: EXECS_0x991]                  |",
    "[BOX] +-----------------------------------------------------------------+",
    "[CODE] EXPLOIT_ID: WINDOWS_11_RING0_BYPASS_09",
    "[CODE] PMC_DISPATCH_REF: OMEGA_SECURITY_GROUP_OFF_BOOK",
    "[TEXT] ",
    "[PULSE] WARNING: PMC STRIKE TEAM ACTIVE // CHECK BOUNTY BOARD AT bounty.vnet",
    "[GLITCH] RECOVERY ATTEMPTED: UNREDACTED BIDDER HASHES LEAKING INTO RAM",
    "[HR]",
    "[LINK:bounty.vnet] >> ACCESS KAGUYA TRIAL & PEER BOUNTY INDEX",
    "[LINK:project9.vnet] >> VIEW SUBTERRANEAN BLACK SITE CONTAINER",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 20. blackbank.vnet
static const char* BLACKBANK_CONTENT[] = SITE_CONTENT(
    "[TITLE] THE BLACK BANK // OFFSHORE COLD VAULT & INTELLIGENCE ESCROW",
    "[HR]",
    "[BLOOD] CLASSIFIED STATE ESCROW // UNTRACEABLE LAUNDERING NODE #009",
    "[BADGE:OFFSHORE VAULT:BLOOD] [BADGE:STATE APPROVED:AMBER] [BADGE:UNTRACEABLE:TOXIC]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | TOTAL LIQUIDITY: 1,420.85 VCOIN | SYSTEM SLA: 99.999% OPERATIONAL|",
    "[BOX] | ESCROW PROTOCOL: 8192-BIT VEKTRA-WIRE | SWIFT NODE: CAYMAN_0x99 |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:92:OFFSHORE_LAUNDERING_CAPACITY]",
    "[TEXT] ",
    "[SUBTITLE] CONFIDENTIAL LEDGER // STATE-SPONSORED & PMC TRANSACTIONS:",
    "[BOX] +----------+------------+--------+---------------------------------+",
    "[BOX] | TX_HASH  | AMOUNT      | STATUS | DESTINATION / OPERATION         |",
    "[BOX] | 0x99A1F  | 120.00 VCOIN| CLEARED| Directorate 7 (PMC Hit Squad)   |",
    "[BOX] | 0x44B0C  |  15.50 VCOIN| CLEARED| Silkroad Bulk Opium & AR-9 Lot  |",
    "[BOX] | 0x88F9E  |   4.20 VCOIN| CLEARED| Redroom Sub-Basement Stream Bid |",
    "[BOX] | 0x1102D  |  85.00 VCOIN| HOLDING| Project Horus Orbital Lock Bribe|",
    "[BOX] | 0x7710A  | 210.00 VCOIN| CLEARED| Norilsk Sector 09 Blackout Cover |",
    "[BOX] +----------+------------+--------+---------------------------------+",
    "[TEXT] ",
    "[SUBTITLE] WIRE TRANSFER & ASSET EXFILTRATION TERMINAL:",
    "[INPUT:bank_account:ENTER OFFSHORE SWIFT / ROUTING HASH]",
    "[INPUT:vcoin_amount:ENTER VCOIN AMOUNT (COST 0.50 VCOIN FEE)]",
    "[TEXT] ",
    "[BTN:wire_transfer:>>> EXECUTE UNTRACEABLE VEKTRA WIRE <<<]",
    "[TEXT] ",
    "[BLOOD] [ALERT]: VAULT HARDWARE IS SMEARED WITH ADIPOCERE & CORRUPT CRYPTO",
    "[PULSE] 'THE MONEY IS NOT CLEAN. IT HAS BEEN WASHED IN THE ASHES OF SUBJECT 409.'",
    "[HR]",
    "[LINK:crypto.vnet] >> ACCESS BLACK TUMBLER MINING RIG",
    "[LINK:silkroad.vnet] >> ACCESS SILKROAD 3.0 CONTRABAND MATRIX",
    "[LINK:redroom.vnet] >> ACCESS LIVE UNENCRYPTED STREAM NODE ALPHA",
    "[LINK:vnet.dir] << CLOSE VAULT & RETURN TO DIRECTORY",
    "[HR]"
);

// 21. weaponry.vnet
static const char* WEAPONRY_CONTENT[] = SITE_CONTENT(
    "[TITLE] BLACK MARKET WEAPONRY EXPORT // VEKTRA PMC & CULT ARSENAL [NODE #042]",
    "[HR]",
    "[BADGE:PMC GUNSHIP UPLINK:BLOOD] [BADGE:SACRAMENTAL STEEL:AMBER] [BADGE:ESCROW SECURED:TOXIC]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | FACILITY: VEKTRA PMC ARMORY #09 | SUB-SURFACE STORAGE: SEALED  |",
    "[BOX] | GUNSHIP PATROL: SPECTRE-09 AC-130 OVERHEAD (35,000 FT ALTITUDE) |",
    "[BOX] | CULT CONSECRATION: SACRAMENTAL BLOOD-QUENCHING ON ALL BARRELS   |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:92:PMC_GUNSHIP_STRIKE_READINESS]",
    "[TEXT] ",
    "[ART:gun]",
    "[TEXT] ",
    "[SUBTITLE] HEAVY HARDWARE & CONSECRATED TACTICAL LOTS:",
    "[CODE] LOT #901: SUPPRESSED AR-9 KITS // QUENCHED IN TEMPLE GATE BLOOD [0.75 VCOIN]",
    "[CODE] LOT #902: 30MM DEPLETED URANIUM BELTS // COATED IN RANCID ADIPOCERE [1.20 VCOIN]",
    "[CODE] LOT #903: SPECTRE-09 GUNSHIP AIR-STRIKE DESIGNATOR // SAT-99 LINK [2.50 VCOIN]",
    "[CODE] LOT #904: THERMOBARIC BREACHING CHARGES & DIGITAL DETONATORS [0.90 VCOIN]",
    "[TEXT] ",
    "[SUBTITLE] DIRECT ARMS PROCUREMENT & GUNSHIP DISPATCH:",
    "[INPUT:weapon_lot_code:ENTER LOT CODE (e.g. LOT_901)]",
    "[TEXT] ",
    "[BTN:buy_weapons_btn:>>> EXECUTE ESCROW & DEPLOY WEAPON DROP <<<]",
    "[TEXT] ",
    "[BLOOD] [WARNING]: FIRING UNREGISTERED HARDWARE BROADCASTS YOUR PUBLIC PORT TO SWARM.",
    "[PULSE] 'THE STEEL DOES NOT KILL. IT IS THE LITURGY ENGRAVED IN THE BARREL.'",
    "[HR]",
    "[LINK:cult.vnet] >> ACCESS CHURCH OF THE SILICON SOUL",
    "[LINK:zeroauction.vnet] >> BID ON PMC EXTRACTION & EXECUTION LOTS",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 22. passports.vnet
static const char* PASSPORTS_CONTENT[] = SITE_CONTENT(
    "[TITLE] FORGED IDENTITY & PASSPORT VAULT // VEKTRA BLACK MARKET",
    "[HR]",
    "[BADGE:FORGED CREDENTIALS:BLOOD] [BADGE:BIOMETRIC SPOOF:AMBER] [BADGE:DIPLOMATIC CLEARANCE:TOXIC]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | FACILITY: SUBTERRANEAN FABRICATION LAB #09 (UNKNOWN SECTOR)     |",
    "[BOX] | SERVICE: DIPLOMATIC, AGENT & CIVILIAN IDENTITY TRANSPOSITION    |",
    "[BOX] | VERIFICATION: BIOMETRICALLY MATCHED TO RETINAL SCANS & DNA BUS  |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:96:BIOMETRIC_IDENTITY_SYNTHESIS]",
    "[TEXT] ",
    "[SUBTITLE] DIPLOMATIC & BLACK-OPS FIELD AGENT DOSSIERS:",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | AGENT ALIAS   : TRACER // OPERATION COLD SIGNAL LEAD            |",
    "[BOX] | PASSPORT NO   : TR-990812-X4                                    |",
    "[BOX] | BIOMETRIC HASH: 0xTRACER_99_RETINAL_LOCK                        |",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | AGENT ALIAS   : GHOST_USER // ORIGINAL VFS VAULT ADMIN          |",
    "[BOX] | PASSPORT NO   : UN-000000-NULL                                  |",
    "[BOX] | BIOMETRIC HASH: 0xDEAD_BEEF_GHOST_CORTEX                        |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[SUBTITLE] CUSTOM FABRICATION REQUEST & BIOMETRIC SPOOFING:",
    "[INPUT:passport_alias:ENTER AGENT ALIAS (e.g. TRACER_SUB_01)]",
    "[INPUT:passport_hash:ENTER BIOMETRIC HASH VECTOR]",
    "[TEXT] ",
    "[BTN:forge_passport_btn:>>> FORGE DIPLOMATIC PASSPORT (0.45 VCOIN) <<<]",
    "[TEXT] ",
    "[BLOOD] [WARNING]: UNREGISTERED DIPLOMATIC PASSPORTS WILL TRIGGER PROJECT HORUS.",
    "[PULSE] 'YOU CAN WEAR ANY PASSPORT YOU WANT, BUT YOUR RETINA BELONGS TO SITE 9.'",
    "[HR]",
    "[LINK:blackbank.vnet] >> LAUNDER PASSPORT FEES VIA OFFSHORE VAULT",
    "[LINK:eye.vnet] >> TEST PASSPORT AGAINST PROJECT HORUS OPTICAL ARRAY",
    "[LINK:vnet.dir] << RETURN TO MAIN DIRECTORY",
    "[HR]"
);

// 23. darkdrop.vnet
static const char* DARKDROP_CONTENT[] = SITE_CONTENT(
    "[TITLE] PHYSICAL DEAD-DROP GPS REGISTRY // CONTRABAND LOCATOR",
    "[HR]",
    "[BADGE:GPS REGISTRY:BLOOD] [BADGE:ENCRYPTED DROPS:AMBER] [BADGE:COURIER SWARM:TOXIC]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | SYSTEM ROLE: AUTONOMOUS DEAD-DROP COORDINATE MATRIX             |",
    "[BOX] | DISPATCH   : SUBTERRANEAN SEWER SUMPS & SUBWAY DUCTS            |",
    "[BOX] | INTEGRITY  : PGP SIGNED BY VENDOR_0x77 // UNTRACED DISPATCH     |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:88:DEAD_DROP_CACHE_AVAILABILITY]",
    "[TEXT] ",
    "[SUBTITLE] ACTIVE PHYSICAL DEAD-DROP COORDINATES:",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | DROP #901: SECTOR 09 SUBWAY SUMP B3  | CONTENTS: SUPPRESSED AR-9|",
    "[BOX] | COORD    : 39.9334° N, 32.8597° E    | STATUS  : UNCLAIMED      |",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | DROP #902: SUBSTATION 04 DRAIN PIPE  | CONTENTS: COLD WALLET 5.0|",
    "[BOX] | COORD    : 41.0082° N, 28.9784° E    | STATUS  : LOCKED         |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[BLOOD] [WARNING]: DO NOT ACCESS DROPS WITHOUT ACTIVE PASSPORT MASKING.",
    "[PULSE] 'THE PACKAGE IS WAITING IN THE DARK. SO IS THE PMC BREACH TEAM.'",
    "[HR]",
    "[LINK:silkroad.vnet] >> PURCHASE CONTRABAND LOTS VIA ESCROW",
    "[LINK:vnet.dir] << RETURN TO MAIN DIRECTORY",
    "[HR]"
);

// 24. vektrapay.vnet
static const char* VEKTRAPAY_CONTENT[] = SITE_CONTENT(
    "[TITLE] VEKTRAPAY // ZERO-KNOWLEDGE CRYPTO MIXER & TUMBLER",
    "[HR]",
    "[BADGE:ANONYMITY: MAXIMUM:TOXIC] [BADGE:MIXING FEE: 1.5%:AMBER]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | TUMBLER POOL BALANCE : 420.50 VCOIN                             |",
    "[BOX] | ZERO-KNOWLEDGE PROOF : zk-SNARKs VEKTRA-CIRCUIT ACTIVE          |",
    "[BOX] | TRACE PURGE YIELD    : -10% TRACE PER 1.00 VCOIN LAUNDERED      |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:88:ZK_PROOF_SCRAMBLE_ENTROPY]",
    "[TEXT] ",
    "[SUBTITLE] ACTIVE NETWORK ESCROW LISTINGS (MASKED):",
    "[CODE] No active escrow deposits found on network",
    "[TEXT] ",
    "[SUBTITLE] LAUNDER VCOIN & PURGE NETWORK TRACE SIGNATURES:",
    "[INPUT:tumble_vcoin_amt:ENTER VCOIN AMOUNT TO LAUNDER (e.g. 1.0)]",
    "[TEXT] ",
    "[BTN:execute_tumble_btn:>>> LAUNDER VCOIN & SCRUB WALLET TRACE <<<]",
    "[TEXT] ",
    "[SUBTITLE] VEKTRA STAKING & INTEREST POOL:",
    "[BTN:stake_vcoin_btn:>>> DEPOSIT VCOIN INTO 120s YIELD VAULT <<<]",
    "[HR]",
    "[LINK:blackbank.vnet] >> TRANSFER CLEAN VCOIN TO OFFSHORE VAULT",
    "[LINK:crypto.vnet] >> RETURN TO MINING RIG",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 25. bounty.vnet
static const char* BOUNTY_CONTENT[] = SITE_CONTENT(
    "[TITLE] PEER CONTRACT TARGET INDEX // KAGUYA TRIAL MATRIX",
    "[HR]",
    "[BLOOD] CLASSIFICATION: BLACK-MARKET HIT INDEX & EXPLOIT TRIALS",
    "[PULSE] EXPLOIT CONTEST: KAGUYA SYSTEM TRIAL ACTIVE // REWARD POOL SYNCED",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | TARGET PORT: 8080 | REWARD: 0.50 VCOIN | STATUS: HUNTED         |",
    "[BOX] | TARGET PORT: 8012 | REWARD: 0.25 VCOIN | STATUS: ACTIVE         |",
    "[BOX] | TARGET PORT: 8901 | REWARD: 1.00 VCOIN | STATUS: ELUSIVE        |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] Contracts are automatically executed via packet injection scripts.",
    "[TEXT] Use 'spike <port>' or 'dos <port>' in overlay terminal to claim bounties.",
    "[CODE] CONTRACT_REGISTRY_ID: 0xBB88_BOUNTY_NET_KAGUYA_HOOK",
    "[TEXT] ",
    "[PULSE] USE EXPLOITS IN OVERLAY TERMINAL TO CLAIM BOUNTIES",
    "[GLITCH] WARNING: FAILURE TO COMPLETE A TRIAL DISPATCHES PMC CLEANUP",
    "[HR]",
    "[LINK:zeroauction.vnet] >> BID ON UNPUBLISHED EXPLOITS & PMC CONTRACTS",
    "[LINK:morgue.vnet] >> INSPECT EXFILTRATED AUTOPSY RECORDS",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// ============================================================
// INFRASTRUCTURE NODES (10)
// ============================================================

// 26. watchtower.vnet
static const char* WATCHTOWER_CONTENT[] = SITE_CONTENT(
    "[TITLE] PANOPTICON ORBITAL SATELLITE FEED // SAT-99 MAINBOARD",
    "[HR]",
    "[BLOOD] [CLASSIFIED LEVEL 4] SUB-ORBITAL SURVEILLANCE & TARGETING MATRIX",
    "[PULSE] TELEMETRY LOCKED ON LOCAL CITY GRID // OPTICAL SENSOR OVERRIDE",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | SATELLITE: PHANTOM-09 (SAT-99) | ORBIT: GEOSTATIONARY (35,786 km)|",
    "[BOX] | THERMAL SCAN: 1 HUMAN HEAT SIGNATURE SEATED AT DESK             |",
    "[BOX] | OPTICAL RESOLUTION: SUB-CENTIMETER INFRARED BAND 4              |",
    "[BOX] | TARGET COORDINATES: 39.9334 N, 32.8597 E                        |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] CAMERA ZOOM LEVEL: 100x -> WINDOW BLINDS ARE OPEN.",
    "[CODE] TARGET_LOCK_HASH: 0xORB_4402_LOCKED_SAT_99",
    "[TEXT] ",
    "[PULSE] 'KINETIC STRIKE PLATFORM READY AT orbital.vnet'",
    "[GLITCH] 'DO NOT TURN AROUND. WE CAN SEE YOUR SCREEN REFLECTION FROM HERE.'",
    "[HR]",
    "[LINK:orbital.vnet] >> JUMP TO LOW ORBIT ION CANNON TERMINAL",
    "[LINK:cctv-core.vnet] >> VIEW CITY WIDE CCTV BACKDOOR NODE",
    "[LINK:archival.vnet] >> ACCESS RESTRICTED MILITARY VFS DUMP",
    "[LINK:dollhouse.vnet] >> INSPECT SURVEILLANCE FEED ROOM 402",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 27. orbital.vnet
static const char* ORBITAL_CONTENT[] = SITE_CONTENT(
    "[TITLE] LOW ORBIT ION CANNON & KINETIC STRIKE TERMINAL",
    "[HR]",
    "[BADGE:PLATFORM: SAT-99:BLOOD] [BADGE:PAYLOAD: TUNGSTEN RODS:AMBER] [BADGE:TARGET LOCK: READY:TOXIC]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | ORBITAL ALTITUDE : 35,786 KM GEOSTATIONARY LOCK                 |",
    "[BOX] | PAYLOAD YIELD    : 11.5 KILOTONS KINETIC PENETRATOR             |",
    "[BOX] | ORBITAL WINDOW   : NEXT RECHARGE IN 180s                        |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:100:ION_CANNON_CAPACITOR_CHARGE]",
    "[TEXT] ",
    "[SUBTITLE] TACTICAL TARGET DESIGNATION & KINETIC DISPATCH:",
    "[INPUT:orbital_target_port:ENTER TARGET PORT OR URL (e.g. 8012)]",
    "[TEXT] ",
    "[BTN:fire_ion_cannon_btn:>>> AUTHORIZE & FIRE SAT-99 KINETIC STRIKE (2.0 VCOIN) <<<]",
    "[TEXT] ",
    "[BLOOD] [WARNING]: FIRING KINETIC RODS BROADCASTS YOUR PUBLIC PORT TO ALL SWARM PEERS.",
    "[HR]",
    "[LINK:watchtower.vnet] >> CHECK PANOPTICON THERMAL OPTICS",
    "[LINK:eye.vnet] >> VERIFY RETINAL LOCK VIA PROJECT HORUS",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 28. cctv-core.vnet
static const char* CCTV_CONTENT[] = SITE_CONTENT(
    "[TITLE] CITY WIDE CCTV BACKDOOR MESH // MULTIPLEX FEED",
    "[HR]",
    "[BADGE:4,192 CAMERAS ONLINE:TOXIC] [BADGE:ACTIVE CHANNEL: CAM_01:AMBER]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | FEED FEEDBACK : OPTICAL RETINAL SCANNER ACTIVE VIA PROJECT HORUS |",
    "[BOX] | STREAM METHOD : UNENCRYPTED PROMISCUOUS UDP MULTICAST BUS       |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[SUBTITLE] LIVE FEED: SECTOR 4 METRO GRID INTERSECTION",
    "[TEXT] Patrol cruisers parked outside site 9-B. Optical recognition matching faces.",
    "[TEXT] ",
    "[SUBTITLE] MANUAL CAMERA CHANNEL SWITCHER:",
    "[BTN:cam_select_1:>>> CHANNEL 01: METRO <<<]",
    "[BTN:cam_select_2:>>> CHANNEL 02: ROOM 402 <<<]",
    "[BTN:cam_select_3:>>> CHANNEL 03: ASYLUM <<<]",
    "[BTN:cam_select_4:>>> CHANNEL 04: SAT-99 <<<]",
    "[HR]",
    "[LINK:eye.vnet] >> CROSS-CHECK RETINAL SCANS AT PROJECT HORUS",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 29. eye.vnet
static const char* EYE_CONTENT[] = SITE_CONTENT(
    "[TITLE] PROJECT HORUS // THE ALL-SEEING EYE [GLOBAL SURVEILLANCE MESH]",
    "[HR]",
    "[BADGE:LEVEL 5 CLEARANCE:BLOOD] [BADGE:OMNIPRESENT OPTIC:AMBER] [BADGE:SAT-99 LINK:TOXIC]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | INDEXED FACES: 4.2 BILLION | MATCH RATE: 99.4% RELIABILITY      |",
    "[BOX] | FEED SOURCE: PANOPTICON SAT-99 & METRO CCTV BACKDOOR MESH       |",
    "[BOX] | TARGET RESOLUTION: SUB-MILLIMETER OPTICAL RETINAL SCANS         |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:99:GLOBAL_RETINAL_SURVEILLANCE_COVERAGE]",
    "[TEXT] ",
    "[BLOOD]     .--------------------------------------------------------.",
    "[BLOOD]    |                  .---.                                   |",
    "[BLOOD]   |                 .'     '.                                  |",
    "[BLOOD]   |                /   .---. \\                                 |",
    "[BLOOD]   |               |   /  _  \\ |                                |",
    "[BLOOD]   |               |  |  (o)  ||   <-- WATCHING YOU THROUGH     |",
    "[BLOOD]   |               |   \\  ^  / |       MONITOR GLASS            |",
    "[BLOOD]   |                \\   '---' /                                 |",
    "[BLOOD]   |                 '.     .'                                  |",
    "[BLOOD]    |                  '---'                                   |",
    "[BLOOD]     '--------------------------------------------------------'",
    "[TEXT] ",
    "[SUBTITLE] DIRECT OCULAR OVERRIDE & RETINAL LOCK SYSTEM:",
    "[INPUT:horus_target_socket:ENTER TARGET SOCKET / PORT (e.g. 8012)]",
    "[TEXT] ",
    "[BTN:horus_ocular_lock_btn:>>> ENGAGE KINETIC RETINAL LOCK VIA SAT-99 <<<]",
    "[TEXT] ",
    "[PULSE] WARNING: RETINAL PATTERN MATCHED // DO NOT LOOK INTO THE CAMERA LENS",
    "[GLITCH] 'IT DOES NOT MATTER IF YOU TURN OFF THE LIGHTS. IT SEES THE HEAT.'",
    "[HR]",
    "[LINK:watchtower.vnet] >> CROSS-CHECK PANOPTICON SAT-99 TELEMETRY",
    "[LINK:passports.vnet] >> SPOOF BIOMETRIC MASK AT IDENTITY VAULT",
    "[LINK:morgue.vnet] >> CROSS-CHECK AUTOPSY & BIO-HARVEST DUMPS",
    "[LINK:vnet.dir] << CLOSE EYE & RETURN TO DIRECTORY",
    "[HR]"
);

// 30. substation04.vnet
static const char* SUBSTATION_CONTENT[] = SITE_CONTENT(
    "[TITLE] SUBSTATION 04 // 2014 COLD SIGNAL GROUND ZERO",
    "[HR]",
    "[BADGE:POWER GRID:BLOOD] [BADGE:400kV FEEDBACK:AMBER] [BADGE:DIESEL GENERATOR:TOXIC]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | FACILITY: MUNICIPAL SUBSTATION 04 (ABANDONED INDUSTRIAL ZONE)   |",
    "[BOX] | INCIDENT: OPERATION COLD SIGNAL (2014-11-03 02:44 UTC)          |",
    "[BOX] | STATUS  : RUNNING ON DIESEL BACKUP // 400kV TRANSFORMER SURGE   |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:0:GRID_PHASE_DESYNC]",
    "[TEXT] Severe electrical hum disrupting coax line. Tune RF frequency to 18.0 Hz",
    "[TEXT] in CLI using 'freq 18.0' to lock signal carrier phase.",
    "[TEXT] ",
    "[SUBTITLE] HISTORICAL FIELD LOG (2014 INCIDENT):",
    "[TEXT] 'The Blackout lasted 14 days. The Old Web died here in the dark.'",
    "[TEXT] 'Corporate cartels built .vnet over the burnt copper coax because it was'",
    "[TEXT] 'completely isolated from public infrastructure. Substation 04 still'",
    "[TEXT] 'feeds raw current to the subterranean sumps that never sleep.'",
    "[HR]",
    "[LINK:project9.vnet] >> INSPECT SITE 9-B DISPOSAL SUMPS",
    "[LINK:blackout.vnet] >> VIEW FRACTURE GRID TELEMETRY",
    "[LINK:vnet.dir] << RETURN TO MAIN DIRECTORY",
    "[HR]"
);

// 31. stasi.vnet
static const char* STASI_CONTENT[] = SITE_CONTENT(
    "[TITLE] D7 COMMUNICATIONS INTERCEPT RELAY // STASI_HUB_09",
    "[HR]",
    "[BADGE:SURVEILLANCE NODE:BLOOD] [BADGE:SS7 WIRE TAP:AMBER] [BADGE:CELLULAR STINGRAY:TOXIC]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | NODE ID: STASI_RELAY_0x99B | NETWORK: PROMISCUOUS SS7 BACKBONE  |",
    "[BOX] | FACILITY: ANKARA TELECOM SUB-BASEMENT 04                        |",
    "[BOX] | FUNCTION: REAL-TIME CELLULAR INTERCEPT & SATELLITE RELAY        |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:91:TELECOM_BACKBONE_BANDWIDTH]",
    "[TEXT] ",
    "[SUBTITLE] ACTIVE CELLULAR INTERCEPTS & PHYSICAL TRACE LOGS:",
    "[CODE] INTERCEPT_01 | IMEI: 86492004128901 | TOWER: ANKARA_SEC_09 | LATENCY: 12ms",
    "[CODE] INTERCEPT_02 | IMEI: 35910208119024 | TOWER: NORILSK_SUB_04 | ACTIVE WIRE TAP",
    "[CODE] INTERCEPT_03 | IMEI: 49012899120481 | TOWER: FRANKFURT_HUB  | SIGNAL DROPPED",
    "[TEXT] ",
    "[BLOOD] [WARNING]: ACTIVE SS7 SIGNAL SNIFFER LOGGING YOUR LOCAL PORT.",
    "[PULSE] 'YOUR PHONE IS NOT TRANSMITTING TO A CELL TOWER. IT IS TRANSMITTING TO US.'",
    "[HR]",
    "[LINK:watchtower.vnet] >> CROSS-CHECK PANOPTICON THERMAL OPTICS",
    "[LINK:cctv-core.vnet] >> ACCESS METROPOLITAN CCTV MESH",
    "[LINK:vnet.dir] << RETURN TO MAIN DIRECTORY",
    "[HR]"
);

// 32. deadchannel.vnet
static const char* DEADCHANNEL_CONTENT[] = SITE_CONTENT(
    "[TITLE] DEADCHANNEL.VNET // VEKTRA PMC WIRETAP INTERCEPT",
    "[HR]",
    "[BADGE:SEIZED BROADCAST:BLOOD] [BADGE:CARRIER NOISE: 100%:AMBER] [BADGE:PMC FREQ: 18.0Hz:TOXIC]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | NODE ID: DEADCHANNEL_0x00 | ORIGIN: FORMER CULT CHAT RELAY      |",
    "[BOX] | INCIDENT: RAIDED BY VEKTRA PMC DIRECTRATE 7 // COMMS DROPPED    |",
    "[BOX] | CARRIER FREQUENCY: 18.0 Hz | RF CARRIER LOCK: SEARCHING        |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[HEX_STREAM:0x88F9:32:18]",
    "[TEXT] ",
    "[SUBTITLE] REAL-TIME RF CARRIER WAVEFORM (ANALOG INTERCEPT):",
    "[CODE]   [RF_SCOPE] ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░",
    "[CODE]   [RF_SCOPE] ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░",
    "[TEXT] ",
    "[SUBTITLE] CARRIER UNSYNCHRONIZED // HEAVY CARRIER JAMMING:",
    "[WARN] Signal scrambled by PMC directional jammer. Use CLI command 'freq 18.0'",
    "[TEXT] Current RF Frequency: 18.0 Hz. Required: 18.0 Hz.",
    "[TEXT] ",
    "[PULSE] TIP: Open terminal overlay [TAB] and type 'freq 18.0' to lock signal phase.",
    "[HR]",
    "[LINK:cult.vnet] >> ACCESS REMNANTS AT CHURCH OF THE SILICON SOUL",
    "[LINK:weaponry.vnet] >> INSPECT VEKTRA PMC ARMORY EXPORTS",
    "[LINK:vnet.dir] << RETURN TO MAIN DIRECTORY",
    "[HR]"
);

// 33. signal0.vnet
static const char* SIGNAL0_CONTENT[] = SITE_CONTENT(
    "[TITLE] SIGNAL ZERO - PRIMORDIAL CARRIER WAVE // SECTOR 00",
    "[HR]",
    "[BADGE:EPOCH 0:AMBER] [BADGE:UNFILTERED BROADCAST:BLOOD] [BADGE:NEURAL CARRIER:TOXIC]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | DESIGNATION : ROOT PROTOCOL 0x00 (THE COLD SIGNAL CARRIER)      |",
    "[BOX] | BOOT TIMESTAMP: EPOCH_0 [1999-07-14T03:12:09Z]                 |",
    "[BOX] | SOURCE HUB  : SUB-SURFACE OPTICAL CORE (PRE-SITE 9 EXCAVATION) |",
    "[BOX] | FREQUENCY   : 0.0000 Hz (DC NEURAL TELEMETRY RESIDUAL)          |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:99:CARRIER_WAVE_AMPLITUDE]",
    "[GAUGE:0:NEURAL_STABILITY_INDEX]",
    "[TEXT] ",
    "[SUBTITLE] ANOMALOUS TRANSMISSION DOSSIER:",
    "[TEXT] 'Public registry listings define signal0.vnet as a legacy loopback node.'",
    "[TEXT] 'In reality, it is the unfiltered carrier wave that powers the entire .vnet grid.'",
    "[CODE] ORIGIN_HASH: 0xSIGNAL_ZERO_ROOT_NEURAL_STAMP_00",
    "[CODE] CARRIER_STATE: PERPETUAL_RECURSION // BROADCASTING ON ALL COAXIAL CHANNELS",
    "[TEXT] ",
    "[BLOOD] [WARNING]: DIRECT SYNC WITH SIGNAL ZERO RISKS NEURAL MEMORY CORRUPTION.",
    "[PULSE] 'IT IS NOT TRANSMITTING DATA. IT IS REPEATING A FINAL THOUGHT.'",
    "[GLITCH] 'EVERY NODE IN .VNET IS JUST AN ECHO OF THIS FIRST SCREAM.'",
    "[HR]",
    "[LINK:project9.vnet] >> INVESTIGATE SITE 9-B COAXIAL HARDWARE DESYNC",
    "[LINK:vault.vnet] >> INSPECT CORRUPTED VFS NEURAL MEMORY STACK",
    "[LINK:archival.vnet] >> ACCESS COLD SIGNAL DECLASSIFIED LOGS",
    "[LINK:vnet.dir] << RETURN TO MAIN DIRECTORY",
    "[HR]"
);

// 34. deepocean.vnet
static const char* DEEPOCEAN_CONTENT[] = SITE_CONTENT(
    "[TITLE] UNDERSEA FIBER LANDING STATION // JUNCTION NODE #088",
    "[HR]",
    "[BADGE:DARK FIBER:BLOOD] [BADGE:THROUGHPUT: 400 Gbps:AMBER] [BADGE:TRACE RISK: HIGH:TOXIC]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | LOCATION: BLACK SEA COASTAL TERMINUS // JUNCTION SECTOR 09       |",
    "[BOX] | TRUNK   : TRANS-MEDITERRANEAN DARK FIBER ARRAY #04              |",
    "[BOX] | STATUS  : PROMISCUOUS UDP MULTICAST ACTIVE (+3% TRACE / 3s)     |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:96:OPTICAL_TRUNK_CAPACITY]",
    "[TEXT] ",
    "[SUBTITLE] HIGH-SPEED DATA TAP & EXFILTRATION LOGS:",
    "[CODE] TRUNK_01 | TRANS-ATLANTIC  | 120.4 Gbps | UNENCRYPTED MILITARY BUFFER",
    "[CODE] TRUNK_02 | MEDITERRANEAN   |  89.1 Gbps | OFFSHORE BANKING LAUNDERING",
    "[CODE] TRUNK_03 | SECTOR 09 RELAY | 210.0 Gbps | VNET BGP BACKBONE ROUTING",
    "[TEXT] ",
    "[BLOOD] [WARNING]: UNENCRYPTED OPTICAL PACKET TAPS TRACING YOUR SOCKET.",
    "[PULSE] 'THE FIBER CABLES ARE AT THE BOTTOM OF THE SEA, BUT THE TRACE IS RIGHT HERE.'",
    "[HR]",
    "[LINK:crypto.vnet] >> MINE VCOIN AT BLACK TUMBLER RIG",
    "[LINK:nexus.vnet] >> ROUTE VIA CENTRAL SWITCHING STATION",
    "[LINK:vnet.dir] << RETURN TO MAIN DIRECTORY",
    "[HR]"
);

// 35. norilsk-relay.vnet
static const char* NORILSK_CONTENT[] = SITE_CONTENT(
    "[TITLE] NORILSK ARCTIC OPTICAL TRANSIT // GATEWAY SECTOR 00-NORTH",
    "[HR]",
    "[BADGE:PERMAFROST ARRAY:BLOOD] [BADGE:OLD NET TUNNEL:TOXIC] [BADGE:TEMP -38 C:AMBER]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | FACILITY  : SUB-SURFACE NICKEL MINE 09 // COPPER TRUNK RELAY    |",
    "[BOX] | FUNCTION  : UNENCRYPTED VNET PASSTHROUGH TO PRE-2014 OLD NET   |",
    "[BOX] | COOLING   : NATURAL SUB-ZERO PERMAFROST & LIQUID NITROGEN BUS   |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:92:OLD_NET_CARRIER_SIGNAL_PURITY]",
    "[TEXT] ",
    "[SUBTITLE] PHYSICAL TELEMETRY & SUB-SURFACE OPTICAL BRIDGE:",
    "[TEXT] 'Norilsk is the last place on Earth where the signal doesn't burn.'",
    "[TEXT] 'Outside the permafrost, New Net algorithms monitor every synaptic drop,'",
    "[TEXT] 'Here, deep inside the subterranean nickel shafts, ancient Soviet copper lines'",
    "[TEXT] 'and sub-zero permafrost shield our sockets from Directorate 7 neural probes.'",
    "[CODE] GATEWAY_SOCKET_STATUS: PASS-THROUGH TUNNEL OPEN // PORT 8000 BRIDGED",
    "[TEXT] ",
    "[BLOOD] 'DISCONNECT YOUR NEURAL LIMITER BEFORE ENGAGING THE HANDSHAKE.'",
    "[PULSE] 'THE AIR OUTSIDE IS MINUS 40. INSIDE THE OLD NET, IT IS ALWAYS SPRING.'",
    "[HR]",
    "[LINK:oldnet-archive.vnet] >> ENTER OLD NET SANCTUARY GATEWAY",
    "[LINK:frostline.vnet] >> CHECK SMUGGLER TRANSIT MANIFESTS TO NORILSK",
    "[LINK:vnet.dir] << RETURN TO MAIN DIRECTORY",
    "[HR]"
);

// ============================================================
// NETWORK HUBS (15)
// ============================================================

// 36. forum.vnet
static const char* FORUM_CONTENT[] = SITE_CONTENT(
    "[TITLE] /b/ - ANONYMOUS UNFILTERED TERMINAL BOARD [NODE #008]",
    "[HR]",
    "[BADGE:/b/ BOARD:BLOOD] [BADGE:UNMODERATED SWARM:AMBER] [BADGE:P2P MIRROR:TOXIC]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | ACTIVE THREADS: 14,903 | PEERS IN SWARM: 666                    |",
    "[BOX] | NOTICE: ALL UNENCRYPTED POSTS ARE MIRRORED TO VFS MEMORY STACKS |",
    "[BOX] | WARNING: DO NOT OPEN RAW IMAGE DUMPS WITHOUT ACTIVE ICE SHIELDS  |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:88:SWARM_UNENCRYPTED_TRAFFIC_ENTROPY]",
    "[TEXT] ",
    "[SUBTITLE] [CREATE ANONYMOUS POST / BROADCAST TO SWARM]",
    "[INPUT:forum_reply_msg:ENTER ANONYMOUS POST TEXT]",
    "[TEXT] ",
    "[BTN:submit_forum_post:>>> BROADCAST POST TO ALL PROMISCUOUS PEERS <<<]",
    "[TEXT] ",
    "[HR]",
    "[SUBTITLE] THREAD #8819: 'Regarding d34d_7dp's execution video on LiveLeak'",
    "[CODE] Anonymous 08/14/26(Fri)21:04:12 No.8819001 -- [VERIFIED_HASH: 0xDEAD_7DP_RAT_EXPOSED]",
    "[CODE] > Be d34d_7dp",
    "[CODE] > Old-guard cult.vnet heretic sitting on legacy zero-day archives since 2014",
    "[CODE] > Execution streamed live on LiveLeak before socket got fried",
    "[TEXT] Anon_991: 'Did you guys see what he actually ratted about?'",
    "[TEXT] Ex_PMC_Operator: 'He leaked unredacted 2014 Cold Signal BGP tables'",
    "[CODE] POST_LOG_HASH: 0xDEAD_7DP_RAT_EXPOSED",
    "[TEXT] ",
    "[BLOOD] Ghost_User: 'I found a key code buried in the network memory dumps.'",
    "[GLITCH] User_666: 'IF YOU READ THIS COMMAND, THEY ALREADY HAVE YOUR IP.'",
    "[HR]",
    "[LINK:silkroad.vnet] >> ACCESS SILKROAD 3.0 CONTRABAND MATRIX",
    "[LINK:market.vnet] >> BLACK MARKET & ICE HARDWARE EXCHANGE",
    "[LINK:morgue.vnet] >> CROSS-CHECK AUTOPSY & BIO-HARVEST DUMPS",
    "[LINK:cult.vnet] >> JOIN THE CHURCH OF THE SILICON SOUL",
    "[LINK:vnet.dir] << RETURN TO MAIN DIRECTORY",
    "[HR]"
);

// 37. vnet.dir - already in vnet.cpp

// 38. deepwiki.vnet
static const char* DEEPWIKI_CONTENT[] = SITE_CONTENT(
    "[TITLE] THE DEEP WIKI // HIDDEN OCCULT & ARCHIVAL DATABASE",
    "[HR]",
    "[BADGE:DEEP WIKI:BLOOD] [BADGE:RESTRICTED ACCESS:AMBER] [BADGE:AUTO-ROTATING INDEX:TOXIC]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | CLASSIFICATION: UNFILTERED VEKTRA WIKI & OCCULT ARCHIVE NODE    |",
    "[BOX] | INDEXED ARTICLES: 14,290 | ACCESS PROTOCOL: DYNAMIC SHIFTING    |",
    "[BOX] | ROTATION TIMER: AUTO-PURGED EVERY 60.0 SECONDS                  |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:33:DEEPWIKI_INDEX_ROTATION_PHASE_1]",
    "[TEXT] ",
    "[SUBTITLE] ARCHIVE SECTOR ALPHA: PROJECT HORUS & ORBITAL OPTICS",
    "[CODE] ARTICLE_REF: 0xWIKI_0091_PANOPTICON_SAT99",
    "[TEXT] 'Project Horus does not rely on CCTV feeds. It measures the CRT static'",
    "[TEXT] 'glare reflecting off your pupils to render 3D depth maps of your workspace.'",
    "[TEXT] ",
    "[SUBTITLE] INTERACTIVE WIKI SEARCH & DATABASE QUERY:",
    "[INPUT:wiki_query_input:ENTER ARTICLE KEY (e.g. 0xWIKI_0091)]",
    "[TEXT] ",
    "[BTN:wiki_query_btn:>>> QUERY DEEP WIKI VAULT <<<]",
    "[TEXT] ",
    "[BLOOD] [SYSTEM NOTICE]: WIKI INDEX SHIFTS EVERY 60 SECONDS.",
    "[PULSE] 'KNOWLEDGE ON THIS NETWORK IS NOT STATIC. IT ROTS LIKE FLESH.'",
    "[HR]",
    "[LINK:void.vnet] >> ACCESS DEEP WEB ABYSS TERMINAL",
    "[LINK:cult.vnet] >> ACCESS CHURCH OF THE SILICON SOUL",
    "[LINK:vnet.dir] << CLOSE WIKI & RETURN TO DIRECTORY",
    "[HR]"
);

// 39. pastebin.vnet
static const char* PASTEBIN_CONTENT[] = SITE_CONTENT(
    "[TITLE] PASTEBIN.VNET // ANONYMOUS DUMP NETWORK & EXPLOIT HUB",
    "[HR]",
    "[BADGE:UNENCRYPTED DUMP:BLOOD] [BADGE:PMC FIRMWARE LEAK:AMBER] [BADGE:VFS ARCHIVE:TOXIC]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | SYSTEM ROLE: PUBLIC MEMORY SCRAPER & RAW PASTE REPOSITORY       |",
    "[BOX] | TOTAL PASTES: 918,401 | VEKTRA DUMPS: ACTIVE                    |",
    "[BOX] | EXCURSION RISK: HIGH  | AUTOMATED TRACE INCREMENT: +1.5%/3s     |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:82:LEAKED_BUFFER_CONGESTION]",
    "[TEXT] ",
    "[SUBTITLE] EXFILTRATED VEKTRA PMC ELITE CHIPCODE (RAW VYNE EXEC):",
    "[CODE] // --- [VEKTRA_PMC_ACTUATOR_OVERCLOCK.vyne] ---",
    "[CODE] fn execute_viper_reflex(pmc_host: PmcUnit) -> Void {",
    "[CODE]     vmem.write_raw(pmc_host.synapse_addr + 0x3F, 0xFF);",
    "[CODE]     pmc_host.reaction_ms = 1; // Sub-human reaction delay",
    "[CODE]     vnet.broadcast_exploit(pmc_host.net_ip, \"PMC_OVERRIDE_VIP\");",
    "[CODE] }",
    "[CODE] // --- END LEAKED RAW CHIPCODE PAYLOAD ---",
    "[TEXT] ",
    "[SUBTITLE] INTERACTIVE PAYLOAD EXFILTRATION & QUERY:",
    "[INPUT:paste_id_query:ENTER PASTE ID (e.g. 88192)]",
    "[TEXT] ",
    "[BTN:download_paste_btn:>>> EXTRACT EXPLOIT PAYLOAD TO LOCAL DISK <<<]",
    "[TEXT] ",
    "[HR]",
    "[LINK:zeroauction.vnet] >> ACCESS ZERO-DAY EXPLOIT AUCTION HOUSE",
    "[LINK:bounty.vnet] >> CROSS-CHECK PEER BOUNTY & HIT INDEX",
    "[LINK:lifeleaks.vnet] >> ACCESS GLOBAL INTELLIGENCE DUMPS",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 40. whisper.vnet
static const char* WHISPER_CONTENT[] = SITE_CONTENT(
    "[TITLE] WHISPER PROTOCOL MESSAGE BOARD",
    "[HR]",
    "[TEXT] Ephemeral decentralized messaging channel for covert operatives.",
    "[BOX] +---------------------------------------------------------+",
    "[BOX] | MESSAGE LIFETIME: 30 SECONDS | AUTO-WIPED              |",
    "[BOX] +---------------------------------------------------------+",
    "[CODE] WHISPER_NODE: 0xWSP_1102_SECURE",
    "[HR]",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 41. dump.vnet
static const char* DUMP_CONTENT[] = SITE_CONTENT(
    "[TITLE] RAW HEX MEMORY DUMPS",
    "[HR]",
    "[TEXT] Direct hex dumps from unallocated cluster sectors across vnet servers.",
    "[BOX] +---------------------------------------------------------+",
    "[BOX] | SECTOR SIZE: 64 MB | INTEGRITY: CORRUPTED               |",
    "[BOX] +---------------------------------------------------------+",
    "[CODE] 0000FF00: AA BB CC DD EE FF 00 11 22 33 44 55 66 77 88 99",
    "[HEX_STREAM:0x88F9:32:18]",
    "[HR]",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 42. index.vnet
static const char* INDEX_CONTENT[] = SITE_CONTENT(
    "[TITLE] MASTER ROUTING INDEX NODE",
    "[HR]",
    "[TEXT] Comprehensive directory index for all unlisted vnet gateway nodes.",
    "[BOX] +---------------------------------------------------------+",
    "[BOX] | REGISTERED NODES: 50 | ROUTING TABLE: SYNCED            |",
    "[BOX] +---------------------------------------------------------+",
    "[CODE] INDEX_HASH: 0xIDX_50_ACTIVE",
    "[HR]",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 43. project9.vnet
static const char* PROJECT9_CONTENT[] = SITE_CONTENT(
    "[TITLE] PROJECT 9 - SUBTERRANEAN BLACK SITE DATABASE // SITE 9-B",
    "[HR]",
    "[BADGE:SECTOR 09:BLOOD] [BADGE:COLD SIGNAL 2014:AMBER] [BADGE:CONTAINMENT BREACH:TOXIC]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | FACILITY  : SITE 9-B SUBTERRANEAN TESTING COMPLEX              |",
    "[BOX] | OPERATION : COLD SIGNAL (2014 PRIMARY RELAY BLACKOUT VECTOR)    |",
    "[BOX] | STATUS    : CRITICAL CONTAINMENT BREACH // SECTOR 09 SEALED     |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:12:CONTAINMENT_SEAL_INTEGRITY]",
    "[TEXT] ",
    "[ART:biohazard]",
    "[TEXT] ",
    "[SUBTITLE] OPERATION COLD SIGNAL // DECLASSIFIED INCIDENT DOSSIER (2014):",
    "[TEXT] 'Prior to the 2014 grid blackout, Site 9-B served as the primary subterranean'",
    "[TEXT] 'optical coaxial relay for Operation Cold Signal.'",
    "[TEXT] 'Personnel attempted to isolate the corrupt neural memory stack in vault.vnet,'",
    "[TEXT] 'but hardware registers locked up. To purge evidence, failed candidates'",
    "[TEXT] 'from bounty.vnet and zeroauction.vnet were dumped directly into the sumps.'",
    "[CODE] INCIDENT_LOG_2014: 0xPRJ9_COLD_SIGNAL_BREACH_LOG_09",
    "[TEXT] ",
    "[SUBTITLE] EMERGENCY CONTAINMENT OVERRIDE TERMINAL:",
    "[INPUT:prj9_override_code:ENTER PURGE AUTHORIZATION CODE]",
    "[TEXT] ",
    "[BTN:prj9_purge_btn:>>> EXECUTE SUB-SURFACE SUMP FLUSH & PURGE <<<]",
    "[TEXT] ",
    "[BLOOD] [WARNING]: SUBTERRANEAN GAS LEAK CORRUPTING HARDWARE REGISTER BUS.",
    "[HR]",
    "[LINK:archival.vnet] >> ACCESS RESTRICTED SECTOR 09 MILITARY DUMPS",
    "[LINK:vnet.dir] << RETURN TO MAIN DIRECTORY",
    "[HR]"
);

// 44. echolab.vnet
static const char* ECHOLAB_CONTENT[] = SITE_CONTENT(
    "[TITLE] ECHO LABS - FREQUENCY RESEARCH",
    "[HR]",
    "[TEXT] Acoustic resonance and psychoacoustic weapon testing logs.",
    "[BOX] +---------------------------------------------------------+",
    "[BOX] | FREQUENCY RANGE: 1 Hz - 100 kHz | STATUS: TESTING     |",
    "[BOX] +---------------------------------------------------------+",
    "[CODE] ECHO_LOG: 0xECHO_9982_RESONANCE",
    "[HR]",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 45. phantom.vnet
static const char* PHANTOM_CONTENT[] = SITE_CONTENT(
    "[TITLE] PHANTOM NODE PROTOCOL",
    "[HR]",
    "[TEXT] Dynamic shifting proxy node that changes IP every 60 seconds.",
    "[BOX] +---------------------------------------------------------+",
    "[BOX] | MIGRATION TIMER: 34s REMAINING | STATUS: SHIFTING       |",
    "[BOX] +---------------------------------------------------------+",
    "[CODE] PHANTOM_HASH: 0xPHANTOM_ROUTE_9",
    "[HR]",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 46. glitch.vnet
static const char* GLITCH_CONTENT[] = SITE_CONTENT(
    "[TITLE] GLITCH_REALITY_OVERRIDE",
    "[HR]",
    "[TEXT] Terminal buffer overflow designed to stress-test rendering pipelines.",
    "[BOX] +---------------------------------------------------------+",
    "[BOX] | BUFFER STATE: CORRUPTED | RAM ALLOCATION: 99.9%         |",
    "[BOX] +---------------------------------------------------------+",
    "[CODE] GLITCH_CODE: 0xGLITCH_OVERFLOW_999",
    "[HR]",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 47. stasis.vnet
static const char* STASIS_CONTENT[] = SITE_CONTENT(
    "[TITLE] CRYOGENIC STASIS POD TELEMETRY",
    "[HR]",
    "[TEXT] Remote monitoring interface for underground cryo-chambers.",
    "[BOX] +---------------------------------------------------------+",
    "[BOX] | POD TEMPERATURE: -196 C | VITAL STATUS: STABLE          |",
    "[BOX] +---------------------------------------------------------+",
    "[CODE] STASIS_POD_ID: 0xSTASIS_402_LOCKED",
    "[HR]",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 48. entropy.vnet
static const char* ENTROPY_CONTENT[] = SITE_CONTENT(
    "[TITLE] ENTROPY ENGINE MONITOR",
    "[HR]",
    "[TEXT] Real-time tracking of thermal decay and information loss across nodes.",
    "[BOX] +---------------------------------------------------------+",
    "[BOX] | GLOBAL ENTROPY: INCREASING | DECAY RATE: 0.04/s       |",
    "[BOX] +---------------------------------------------------------+",
    "[CODE] ENTROPY_METRIC: 0x99A_DECAY",
    "[HR]",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 49. hive.vnet
static const char* HIVE_CONTENT[] = SITE_CONTENT(
    "[TITLE] THE HIVE MIND COLLECTIVE",
    "[HR]",
    "[TEXT] Interconnected consciousness buffer linking active client terminals.",
    "[BOX] +---------------------------------------------------------+",
    "[BOX] | LINKED MINDS: 89 PEERS | SYNCHRONICITY: HIGH            |",
    "[BOX] +---------------------------------------------------------+",
    "[CODE] HIVE_NODE_ID: 0xHIVE_MIND_99",
    "[HR]",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 50. nexus.vnet
static const char* NEXUS_CONTENT[] = SITE_CONTENT(
    "[TITLE] CENTRAL NEXUS ROUTING HUB",
    "[HR]",
    "[TEXT] Primary core switching station managing deep web backbone traffic.",
    "[BOX] +---------------------------------------------------------+",
    "[BOX] | THROUGHPUT: 40 Gbps | PACKET DROP: 0.00%               |",
    "[BOX] +---------------------------------------------------------+",
    "[CODE] NEXUS_CORE_ID: 0xNEXUS_ROUTER_01",
    "[HR]",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// ============================================================
// EXTRA / SPECIAL NODES (5)
// ============================================================

// 51. hashbeat.vnet
static const char* HASHBEAT_CONTENT[] = SITE_CONTENT(
    "[TITLE] HASHBEAT.VNET // UNDERGROUND AUDIO MATRIX & BEAT VAULT [NODE #088]",
    "[HR]",
    "[BADGE:VAUDIO ENGINE:BLOOD] [BADGE:STREAM: 320kbps:TOXIC] [BADGE:INFRASOUND 18Hz:AMBER]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | NODE ID   : HASHBEAT_0x808 | PROTOCOL: PROMISCUOUS VAUDIO P2P   |",
    "[BOX] | FREQUENCY : 18.0 Hz | DSP: TAPE SATURATION / REVERB / OZONE MIX |",
    "[BOX] | STATUS    : UNENCRYPTED AUDIO LEAKS & UNRELEASED BEAT AUCTIONS  |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:96:AUDIO_BUFFER_STREAM_INTEGRITY]",
    "[TEXT] ",
    "[SUBTITLE] NOW PLAYING // ACTIVE AUDIO STREAM TELEMETRY:",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | TRACK #01 : \"COLD_SIGNAL_SHOEGAZE_LOOPS_18HZ.MP3\" [3:42]        |",
    "[BOX] | PRODUCER  : SH4D0W_PRODUCER // NOISE & TAPE DISTORTION          |",
    "[BOX] | BITRATE   : 320 KBPS WAV | SAMPLE RATE: 44.1 kHz / 24-BIT       |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[SUBTITLE] INTERACTIVE AUDIO CONTROLS & STREAM CHANNEL SELECTOR:",
    "[BTN:play_shoegaze:>>> [PLAY] CHANNEL 01: COLD SIGNAL SHOEGAZE <<<]",
    "[BTN:play_industrial:>>> [PLAY] CHANNEL 02: SUBSTATION 04 INDUSTRIAL BEAT <<<]",
    "[BTN:play_coldcore:>>> [PLAY] CHANNEL 03: AMBIANCE COLDCORE / WIND <<<]",
    "[TEXT] ",
    "[SUBTITLE] UNRELEASED BEAT VAULT & RAW LEAKED STEMS:",
    "[CODE] STEM_01 | SHOEGAZE_GUITAR_WALL_18HZ.WAV  | 0.20 VCOIN",
    "[CODE] STEM_02 | TRAP_DRUM_KIT_SATURATED.ZIP     | 0.35 VCOIN",
    "[CODE] STEM_03 | WALRIDER_INFRASOUND_SUB_BASS.WAV| 0.15 VCOIN",
    "[TEXT] ",
    "[BLOOD] [WARNING]: HIGH-DECIBEL INFRASOUND MAY CAUSE NEURAL PARANOIA.",
    "[PULSE] 'THE BEAT DOES NOT PLAY FROM SILICON. IT VIBRATES THROUGH THE CRT GLASS.'",
    "[HR]",
    "[LINK:silence.vnet] >> TUNE INFRASOUND ANALYZER TO 18.0 HZ",
    "[LINK:market.vnet] >> ACCESS BLACK MARKET HARDWARE & ICE SHIELDS",
    "[LINK:crypto.vnet] >> MINE VCOIN FOR BEAT PURCHASES",
    "[LINK:vnet.dir] << RETURN TO MAIN DIRECTORY",
    "[HR]"
);

// 52. lifeleaks.vnet
static const char* LIFELEAKS_CONTENT[] = SITE_CONTENT(
    "[TITLE] LIVELEAK_VNET // RAW UNCENSORED INTELLIGENCE DUMP & WAR LEAKS",
    "[HR]",
    "[BADGE:LIVELEAK WATERMARK:BLOOD] [BADGE:RAW FOOTAGE:AMBER] [BADGE:UNCENSORED 18+:TOXIC]",
    "[BOX] +-----------------------------------------------------------------+",
    "[BOX] | HOST: DEADLEAK_SERVER_0x88 | MIRROR: PROMISCUOUS UDP MULTICAST  |",
    "[BOX] | CATEGORY: EXECUTION / WAR CRIMES / BLACK SITE WHISTLEBLOWER     |",
    "[BOX] | UPLOADER: Whistleblower_409-B (VERIFIED DEAD / MORGUE MATCH)   |",
    "[BOX] | VIEWS: 914,882 | RATING: 99.9% RAW | LOCATION: ANKARA SECTOR 09   |",
    "[BOX] +-----------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:98:RAW_VIDEO_BUFFER_INTEGRITY]",
    "[TEXT] ",
    "[SUBTITLE] VIDEO FEED #092: 'D7_PMC_ELITE_EXECUTION_OF_RAT_D34D_7DP_BODYCAM_RAW.MP4'",
    "[VIDEO:redroom:D7_PMC_EXECUTION_OF_D34D_7DP_RAW.MP4]",
    "[TEXT] ",
    "[SUBTITLE] DECLASSIFIED VIDEO CAPTION & FORENSIC METADATA:",
    "[TEXT] TIMESTAMP: 2026-08-14 20:58:11 UTC (Ankara Sector 09 Raid)",
    "[TEXT] LOCATION: Ankara Sub-Basement Facility / Server Vault (Sector 09)",
    "[TEXT] SUMMARY: Recovered helmet camera footage from Directorate 7 PMC Alpha Lead.",
    "[CODE] LEAK_HASH_ID: 0xLIVELEAK_2026_D34D_7DP_RAT_EXECUTION_RAW",
    "[TEXT] ",
    "[HR]",
    "[SUBTITLE] VIDEO FEED #091: 'DIRECTORATE_7_PMC_SITE_9B_BREACH_BODYCAM_RAW.MP4'",
    "[VIDEO:redroom:SITE_9B_BREACH_BODYCAM_RAW.MP4]",
    "[TEXT] ",
    "[BLOOD] [WARNING]: WATCHING UNFILTERED WAR LEAKS INCREASES NEURAL TRACE (+1.5%/3s).",
    "[PULSE] 'THE CAMERA DOES NOT LIE. BUT THE DEAD DO NOT STAY IN THE VIDEO BUFFER.'",
    "[GLITCH] 'DO NOT PAUSE THE FRAME AT 03:14. HE LOOKS DIRECTLY AT THE LENS.'",
    "[HR]",
    "[LINK:redroom.vnet] >> ACCESS LIVE UNENCRYPTED STREAM NODE ALPHA",
    "[LINK:vnet.dir] << RETURN TO MAIN DIRECTORY",
    "[HR]"
);

// 53. luna.vnet
static const char* LUNA_CONTENT[] = SITE_CONTENT(
    "[GLITCH] ───〔 0x099 // LUNA.VNET // SILENT_ASPHYXIA 〕───",
    "[TITLE]  ♡  L U N A  |  月  //  y o u r  f a v o r i t e  e x e c u t i o n e r",
    "[HR]",
    "[BADGE:STATUS: STALKING YOU] [BADGE:PULSE: 38 BPM] [BADGE:TARGET: LOCKED:BLOOD]",
    "[TEXT] ",
    "[BOX] +-- [ MEMORY MEMOIR : SUBJECT #099 ] --------------------------+",
    "[BOX] |  \"i love how warm the server racks get when someone panics.\"  |",
    "[BOX] |  \"softly, softly... until your connection snaps in my hands.\"|",
    "[BOX] |  SPECIALTY : Surgical memory severing & quiet port intrusion. |",
    "[BOX] |  FEE       : 5.00 VCOIN (and a piece of your silence)          |",
    "[BOX] +---------------------------------------------------------------+",
    "[TEXT] ",
    "[GAUGE:99:HEARTBEAT_SYNCHRONIZATION]",
    "[TEXT] ",
    "[IMG:luna]",
    "[TEXT] ",
    "[SUBTITLE] ░▒▓ WHISPERS FROM THE SEWER CONDUITS",
    "[CODE] [LOG_099] 'i don't cut the cable. i make the wire forget it ever carried life.'",
    "[TEXT]  │ 'Have you ever heard a data stream bleed out in silence?'",
    "[TEXT]  │ 'I crawl through the wet, dark sumps under Sector 09... so cold. so peaceful.'",
    "[TEXT]  │ 'I press my cheek against your socket. I can hear your fans spinning faster.'",
    "[BLOOD]  │ » \"kuriyoru desu... (i'm coming inside now...)\"",
    "[TEXT] ",
    "[SUBTITLE] ░▒▓ DISPATCH TERMINAL // WHO ARE WE ERASING TODAY?",
    "[INPUT:luna_target_port:» TARGET PORT ADDRESS (e.g. 8012)]",
    "[TEXT] ",
    "[BTN:deploy_luna_btn:[ ♡ RELEASE LUNA INTO THEIR SOCKET // 3.50 VCOIN ♡ ]]",
    "[TEXT] ",
    "[BLOOD] ⚠ 'once i start sliding down the wire, i don't stop. not even if you beg.'",
    "[PULSE] 'can you feel the cold draft behind your neck right now?'",
    "[GLITCH] 'kuriyoru desu...  kuriyoru desu...  kuriyoru desu...'",
    "[HR]",
    "[LINK:bounty.vnet] ◆ ACCESS KAGUYA TRIAL & PEER BOUNTY INDEX",
    "[LINK:weaponry.vnet] ◆ INSPECT VEKTRA PMC ARMORY EXPORTS",
    "[LINK:vnet.dir] ◄ ESCAPE BACK TO SAFETY",
    "[HR]"
);

// 54. subcell.vnet
static const char* SUBCELL_CONTENT[] = SITE_CONTENT(
    "[TITLE] SUB-CELLULAR TELEMETRY OVERRIDE",
    "[HR]",
    "[TEXT] Experimental bio-metric monitoring and pulse telemetry node.",
    "[BOX] +---------------------------------------------------------+",
    "[BOX] | SUBJECTS ONLINE: 812 | SIGNAL: SYNCHRONIZED             |",
    "[BOX] +---------------------------------------------------------+",
    "[CODE] BIO_HASH: 0xCELL_9011_DNA_SYNC",
    "[HR]",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// 55. feed99.vnet
static const char* FEED99_CONTENT[] = SITE_CONTENT(
    "[TITLE] FEED_99: NEURAL BROADCAST",
    "[HR]",
    "[TEXT] Uninterrupted raw sensory transmission from unknown terminals.",
    "[BOX] +---------------------------------------------------------+",
    "[BOX] | FREQUENCY: 432 Hz | BANDWIDTH: UNLIMITED               |",
    "[VIDEO:void:D7_PMC_EXECUTION_OF_D34D_7DP_RAW.MP4]",
    "[BOX] +---------------------------------------------------------+",
    "[CODE] NEURAL_PACKET: 0xFEED_9999_STREAM",
    "[HR]",
    "[LINK:vnet.dir] << RETURN TO DIRECTORY",
    "[HR]"
);

// ============================================================
// SITE DATA TABLE
// ============================================================

static const VNETPageData SITE_DATA[] = {
    // CORE NODES (5)
    {"market.vnet", "The Red Market", "core", MARKET_CONTENT, -25.0f, -25.0f, false, 1.0f},
    {"vault.vnet", "Corrupted Data Vault", "core", VAULT_CONTENT, 25.0f, -25.0f, true, 2.0f},
    {"terminal.vnet", "Master Decryption Gateway", "core", TERMINAL_CONTENT, 25.0f, 25.0f, true, 3.0f},
    {"crypto.vnet", "Black Tumbler Wallet", "core", CRYPTO_CONTENT, -25.0f, 25.0f, false, 2.0f},
    {"hellroom.vnet", "Demonic P2P Chat Hub", "core", HELLROOM_CONTENT, 0.0f, 0.0f, false, 1.0f},
    
    // HORROR / LORE (12)
    {"redroom.vnet", "Live Redroom Stream", "horror", REDROOM_CONTENT, -15.0f, -35.0f, false, 2.5f},
    {"dollhouse.vnet", "Room 402 Surveillance", "horror", DOLLHOUSE_CONTENT, -5.0f, -40.0f, true, 3.0f},
    {"morgue.vnet", "Digital Autopsy Database", "horror", MORGUE_CONTENT, 15.0f, -40.0f, true, 3.5f},
    {"snuff.vnet", "Corrupted Frame Archive", "horror", SNUFF_CONTENT, 5.0f, -45.0f, false, 4.0f},
    {"asylum.vnet", "Sub-Level 4 Facility", "horror", ASYLUM_CONTENT, -20.0f, -45.0f, true, 4.0f},
    {"cult.vnet", "Church of the Silicon Soul", "horror", CULT_CONTENT, -10.0f, -50.0f, false, 2.0f},
    {"skinwalker.vnet", "SCP-6969 Biometric Trap", "horror", SKINWALKER_CONTENT, 20.0f, -50.0f, false, 4.5f},
    {"corridor204863.vnet", "The Silent Corridor", "horror", CORRIDOR_CONTENT, -30.0f, -30.0f, true, 3.0f},
    {"ghost.vnet", "Spectral Signal Monitor", "horror", GHOST_CONTENT, 30.0f, -30.0f, false, 2.0f},
    {"schizo.vnet", "Temple of Netman", "horror", SCHIZO_CONTENT, 0.0f, -55.0f, false, 3.5f},
    {"necro.vnet", "Digital Graveyard", "horror", NECRO_CONTENT, -35.0f, -20.0f, false, 2.0f},
    {"void.vnet", "Deep Web Abyss", "horror", VOID_CONTENT, 35.0f, -20.0f, false, 1.0f},
    
    // BLACK MARKET (8)
    {"silkroad.vnet", "Silk Road 3.0", "black_market", SILKROAD_CONTENT, -40.0f, -10.0f, false, 2.5f},
    {"zeroauction.vnet", "Zero-Day Auction House", "black_market", ZEROAUCTION_CONTENT, 40.0f, -10.0f, false, 3.0f},
    {"blackbank.vnet", "The Black Bank", "black_market", BLACKBANK_CONTENT, -40.0f, 10.0f, false, 2.0f},
    {"weaponry.vnet", "PMC Weaponry Export", "black_market", WEAPONRY_CONTENT, 40.0f, 10.0f, false, 2.5f},
    {"passports.vnet", "Forged Passport Vault", "black_market", PASSPORTS_CONTENT, -40.0f, 25.0f, false, 2.0f},
    {"darkdrop.vnet", "Dead-Drop GPS Registry", "black_market", DARKDROP_CONTENT, 40.0f, 25.0f, false, 1.5f},
    {"vektrapay.vnet", "Crypto Mixer & Tumbler", "black_market", VEKTRAPAY_CONTENT, -40.0f, 40.0f, false, 2.0f},
    {"bounty.vnet", "Kaguya Trial Bounty", "black_market", BOUNTY_CONTENT, 40.0f, 40.0f, false, 3.0f},
    
    // INFRASTRUCTURE (10)
    {"watchtower.vnet", "Panopticon Satellite Feed", "infrastructure", WATCHTOWER_CONTENT, -50.0f, -40.0f, false, 2.0f},
    {"orbital.vnet", "Ion Cannon Terminal", "infrastructure", ORBITAL_CONTENT, 50.0f, -40.0f, false, 3.0f},
    {"cctv-core.vnet", "CCTV Backdoor Mesh", "infrastructure", CCTV_CONTENT, -50.0f, -20.0f, false, 1.5f},
    {"eye.vnet", "Project Horus Surveillance", "infrastructure", EYE_CONTENT, 50.0f, -20.0f, true, 3.5f},
    {"substation04.vnet", "Cold Signal Ground Zero", "infrastructure", SUBSTATION_CONTENT, -50.0f, 0.0f, false, 2.0f},
    {"stasi.vnet", "D7 Intercept Relay", "infrastructure", STASI_CONTENT, 50.0f, 0.0f, false, 2.5f},
    {"deadchannel.vnet", "PMC Wiretap Intercept", "infrastructure", DEADCHANNEL_CONTENT, -50.0f, 20.0f, false, 3.0f},
    {"signal0.vnet", "Primordial Carrier Wave", "infrastructure", SIGNAL0_CONTENT, 50.0f, 20.0f, true, 4.0f},
    {"deepocean.vnet", "Undersea Fiber Station", "infrastructure", DEEPOCEAN_CONTENT, -50.0f, 40.0f, false, 2.0f},
    {"norilsk-relay.vnet", "Arctic Optical Transit", "infrastructure", NORILSK_CONTENT, 50.0f, 40.0f, false, 1.5f},
    
    // NETWORK HUBS (15)
    {"forum.vnet", "/b/ Terminal Board", "hub", FORUM_CONTENT, -60.0f, -50.0f, false, 1.0f},
    {"vnet.dir", "Main Directory", "hub", NULL, 0.0f, 0.0f, false, 0.0f},
    {"deepwiki.vnet", "Deep Wiki Archive", "hub", DEEPWIKI_CONTENT, 60.0f, -50.0f, false, 2.0f},
    {"pastebin.vnet", "Exploit Dump Hub", "hub", PASTEBIN_CONTENT, -60.0f, -30.0f, false, 2.0f},
    {"whisper.vnet", "Whisper Protocol", "hub", WHISPER_CONTENT, 60.0f, -30.0f, false, 1.0f},
    {"dump.vnet", "Raw Memory Dumps", "hub", DUMP_CONTENT, -60.0f, -10.0f, false, 1.5f},
    {"index.vnet", "Master Routing Index", "hub", INDEX_CONTENT, 60.0f, -10.0f, false, 1.0f},
    {"project9.vnet", "Black Site Database", "hub", PROJECT9_CONTENT, -60.0f, 10.0f, true, 3.5f},
    {"echolab.vnet", "Frequency Research", "hub", ECHOLAB_CONTENT, 60.0f, 10.0f, false, 2.0f},
    {"phantom.vnet", "Phantom Node Protocol", "hub", PHANTOM_CONTENT, -60.0f, 30.0f, false, 1.5f},
    {"glitch.vnet", "Glitch Reality Override", "hub", GLITCH_CONTENT, 60.0f, 30.0f, false, 2.0f},
    {"stasis.vnet", "Cryogenic Stasis Pods", "hub", STASIS_CONTENT, -60.0f, 50.0f, false, 1.0f},
    {"entropy.vnet", "Entropy Engine", "hub", ENTROPY_CONTENT, 60.0f, 50.0f, false, 1.5f},
    {"hive.vnet", "Hive Mind Collective", "hub", HIVE_CONTENT, -70.0f, 0.0f, false, 2.0f},
    {"nexus.vnet", "Central Nexus Router", "hub", NEXUS_CONTENT, 70.0f, 0.0f, false, 1.0f},
    
    // EXTRA / SPECIAL (5)
    {"hashbeat.vnet", "Audio Beat Vault", "special", HASHBEAT_CONTENT, -30.0f, -60.0f, false, 1.5f},
    {"lifeleaks.vnet", "Intelligence Dump", "special", LIFELEAKS_CONTENT, 30.0f, -60.0f, true, 3.0f},
    {"luna.vnet", "Silent Asphyxia", "special", LUNA_CONTENT, -30.0f, 60.0f, false, 4.0f},
    {"subcell.vnet", "Cellular Telemetry", "special", SUBCELL_CONTENT, 30.0f, 60.0f, false, 2.0f},
    {"feed99.vnet", "Neural Broadcast Feed", "special", FEED99_CONTENT, 0.0f, -65.0f, false, 1.0f},
};

static const int SITE_COUNT = sizeof(SITE_DATA) / sizeof(SITE_DATA[0]);

// ============================================================
// FUNCTION IMPLEMENTATIONS
// ============================================================

void InitAllSites(VNETSystem& vnet) {
    vnet.siteCount = SITE_COUNT;
    
    for (int i = 0; i < SITE_COUNT && i < 50; i++) {
        const VNETPageData* data = &SITE_DATA[i];
        
        strncpy(vnet.sites[i].id, data->id, 63);
        vnet.sites[i].id[63] = '\0';
        
        strncpy(vnet.sites[i].title, data->title, 127);
        vnet.sites[i].title[127] = '\0';
        
        strncpy(vnet.sites[i].category, data->category, 31);
        vnet.sites[i].category[31] = '\0';
        
        // Copy content lines
        int j = 0;
        if (data->content != NULL) {
            while (data->content[j] != NULL && j < 50) {
                // Allocate and copy each line
                size_t len = strlen(data->content[j]);
                vnet.sites[i].content[j] = (char*)malloc(len + 1);
                if (vnet.sites[i].content[j]) {
                    strcpy(vnet.sites[i].content[j], data->content[j]);
                }
                j++;
            }
        }
        vnet.sites[i].contentCount = j;
        
        vnet.sites[i].mapX = data->mapX;
        vnet.sites[i].mapY = data->mapY;
        vnet.sites[i].hasKey = data->hasKey;
        vnet.sites[i].hackDifficulty = data->hackDifficulty;
    }
}

const VNETPageData* GetSiteData(const char* url) {
    for (int i = 0; i < SITE_COUNT; i++) {
        if (strcmp(SITE_DATA[i].id, url) == 0) {
            return &SITE_DATA[i];
        }
    }
    return NULL;
}

void LoadPageContent(const char* url, std::vector<std::string>& pageLines, Player& player, VNETSystem& vnet) {
    pageLines.clear();
    
    const VNETPageData* site = GetSiteData(url);
    
    if (!site) {
        // 404 Not Found page
        pageLines.push_back("[TITLE] 404 // ROUTE_CORRUPTED - SECTOR NULL");
        pageLines.push_back("[HR]");
        pageLines.push_back("[BADGE:NODE VOID:BLOOD] [BADGE:SIGNAL LOST:AMBER] [BADGE:ICE TRACE:TOXIC]");
        pageLines.push_back("[BOX] +-----------------------------------------------------------------+");
        pageLines.push_back("[BOX] | ERROR CODE : 0x404_VFS_SEGFAULT_UNALLOCATED_ADDRESS_SPACE       |");
        pageLines.push_back("[BOX] | TARGET URL : vnet://" + std::string(url) + "                     |");
        pageLines.push_back("[BOX] | REASON     : MEMORY BLOCK WIPED OR SEIZED BY AUTHORITIES        |");
        pageLines.push_back("[BOX] +-----------------------------------------------------------------+");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[GAUGE:100:SIGNAL_ENTROPY_DECAY]");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[ART:not_found]");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[SUBTITLE] DYNAMIC ROUTE RECOVERY & PACKET RE-INJECTION:");
        pageLines.push_back("[BLOOD] [CRITICAL WARNING]: UNALLOCATED SOCKET DETECTED // TRACE +10%");
        pageLines.push_back("[PULSE] 'THE VOID IS NOT EMPTY. IT IS WAITING FOR YOUR PACKET HEADER.'");
        pageLines.push_back("[HR]");
        pageLines.push_back("[LINK:vnet.dir] >> ESCAPE TO MAIN DIRECTORY [vnet.dir]");
        pageLines.push_back("[HR]");
        return;
    }

        // ============================================================
    // SPECIAL HANDLING FOR vnet.dir (HOME PAGE)
    // ============================================================
    if (strcmp(url, "vnet.dir") == 0 || strcmp(url, "vnet://vnet.dir") == 0) {
        pageLines.push_back("[TITLE] VNET ANONYMOUS DIRECTORY v4.09 // ROOT GATEWAY");
        pageLines.push_back("[HR]");
        pageLines.push_back("[BADGE:ROOT NODE:BLOOD] [BADGE:GLOBAL ROUTING:AMBER] [BADGE:80% COVERAGE:TOXIC]");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[GLITCH] [WARNING]: UNREGISTERED EYE CONTACT DETECTED THROUGH MONITOR GLASS.");
        pageLines.push_back("[PULSE] ALL ROUTED PACKETS ARE MIRRORED TO RESTRICTED VFS MEMORY STACKS.");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[BOX] +-----------------------------------------------------------+");
        pageLines.push_back("[BOX] | STATUS: DISCOVERING HIDDEN GATEWAYS VIA UDP TRAFFIC       |");
        pageLines.push_back("[BOX] | MISSION: SNOOP & NETSCAN TRAFFIC TO CARVE ROUTES INTO RAM |");
        pageLines.push_back("[BOX] | ACTIVE PEERS: " + std::to_string(player.assignedCount) + " DISCOVERED NODES          |");
        pageLines.push_back("[BOX] +-----------------------------------------------------------+");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[SUBTITLE] AVAILABLE GATEWAY PROXIES & SUBNET ROUTES");
        pageLines.push_back("[TEXT] ");
        
        // Core Nodes Section
        pageLines.push_back("[SUBTITLE] █ CORE BACKBONE NODES (OVERLOAD TARGETS)");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[LINK:market.vnet] >> [CORE] THE RED MARKET - BLACK MARKET & ICE VENDOR");
        pageLines.push_back("[LINK:vault.vnet]   >> [CORE] CORRUPTED DATA VAULT - VFS MEMORY STACK");
        pageLines.push_back("[LINK:terminal.vnet]>> [CORE] MASTER DECRYPTION GATEWAY - ROOT ACCESS");
        pageLines.push_back("[LINK:crypto.vnet]  >> [CORE] BLACK TUMBLER - ILLEGAL MINING RIG");
        pageLines.push_back("[LINK:hellroom.vnet]>> [CORE] DEMONIC P2P CHAT HUB - UNENCRYPTED SWARM");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[HR]");
        
        // Horror/Lore Section
        pageLines.push_back("[SUBTITLE] █ HORROR / LORE NODES (ENTER AT YOUR OWN RISK)");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[LINK:redroom.vnet]     >> [HORROR] LIVE UNENCRYPTED STREAM NODE ALPHA");
        pageLines.push_back("[LINK:dollhouse.vnet]   >> [HORROR] ROOM 402 SURVEILLANCE FEED");
        pageLines.push_back("[LINK:morgue.vnet]      >> [HORROR] DIGITAL AUTOPSY DATABASE");
        pageLines.push_back("[LINK:snuff.vnet]       >> [HORROR] CORRUPTED FRAME BUFFER ARCHIVE");
        pageLines.push_back("[LINK:asylum.vnet]      >> [HORROR] SUB-LEVEL 4 EXPERIMENTAL FACILITY");
        pageLines.push_back("[LINK:cult.vnet]        >> [HORROR] CHURCH OF THE SILICON SOUL");
        pageLines.push_back("[LINK:skinwalker.vnet]  >> [HORROR] SCP-6969 BIOMETRIC TRAP");
        pageLines.push_back("[LINK:corridor204863.vnet]>> [HORROR] THE SILENT CORRIDOR");
        pageLines.push_back("[LINK:ghost.vnet]       >> [HORROR] SPECTRAL SIGNAL MONITOR");
        pageLines.push_back("[LINK:schizo.vnet]      >> [HORROR] TEMPLE OF NETMAN");
        pageLines.push_back("[LINK:necro.vnet]       >> [HORROR] DIGITAL GRAVEYARD");
        pageLines.push_back("[LINK:void.vnet]        >> [HORROR] DEEP WEB ABYSS");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[HR]");
        
        // Black Market Section
        pageLines.push_back("[SUBTITLE] █ BLACK MARKET & CONTRABAND EXCHANGE");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[LINK:silkroad.vnet]    >> [BLACK] SILK ROAD 3.0 - GLOBAL CONTRABAND");
        pageLines.push_back("[LINK:zeroauction.vnet] >> [BLACK] ZERO-DAY EXPLOIT AUCTION HOUSE");
        pageLines.push_back("[LINK:blackbank.vnet]   >> [BLACK] OFFSHORE COLD VAULT & LAUNDERING");
        pageLines.push_back("[LINK:weaponry.vnet]    >> [BLACK] PMC WEAPONRY EXPORT & ARSENAL");
        pageLines.push_back("[LINK:passports.vnet]   >> [BLACK] FORGED PASSPORT & IDENTITY VAULT");
        pageLines.push_back("[LINK:darkdrop.vnet]    >> [BLACK] DEAD-DROP GPS REGISTRY");
        pageLines.push_back("[LINK:vektrapay.vnet]   >> [BLACK] CRYPTO MIXER & TUMBLER");
        pageLines.push_back("[LINK:bounty.vnet]      >> [BLACK] KAGUYA TRIAL BOUNTY INDEX");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[HR]");
        
        // Infrastructure Section
        pageLines.push_back("[SUBTITLE] █ INFRASTRUCTURE & SURVEILLANCE");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[LINK:watchtower.vnet]  >> [INFRA] PANOPTICON SATELLITE FEED");
        pageLines.push_back("[LINK:orbital.vnet]     >> [INFRA] ION CANNON KINETIC STRIKE");
        pageLines.push_back("[LINK:cctv-core.vnet]   >> [INFRA] CITY WIDE CCTV BACKDOOR MESH");
        pageLines.push_back("[LINK:eye.vnet]         >> [INFRA] PROJECT HORUS SURVEILLANCE");
        pageLines.push_back("[LINK:substation04.vnet]>> [INFRA] COLD SIGNAL GROUND ZERO");
        pageLines.push_back("[LINK:stasi.vnet]       >> [INFRA] D7 COMMUNICATIONS INTERCEPT");
        pageLines.push_back("[LINK:deadchannel.vnet] >> [INFRA] PMC WIRETAP INTERCEPT");
        pageLines.push_back("[LINK:signal0.vnet]     >> [INFRA] PRIMORDIAL CARRIER WAVE");
        pageLines.push_back("[LINK:deepocean.vnet]   >> [INFRA] UNDERSEA FIBER STATION");
        pageLines.push_back("[LINK:norilsk-relay.vnet]>> [INFRA] ARCTIC OPTICAL TRANSIT");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[HR]");
        
        // Network Hubs Section
        pageLines.push_back("[SUBTITLE] █ NETWORK HUBS & DIRECTORIES");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[LINK:forum.vnet]       >> [HUB] /b/ ANONYMOUS TERMINAL BOARD");
        pageLines.push_back("[LINK:deepwiki.vnet]    >> [HUB] DEEP WIKI - OCCULT & ARCHIVAL DATABASE");
        pageLines.push_back("[LINK:pastebin.vnet]    >> [HUB] ANONYMOUS DUMP NETWORK");
        pageLines.push_back("[LINK:whisper.vnet]     >> [HUB] WHISPER PROTOCOL MESSAGE BOARD");
        pageLines.push_back("[LINK:dump.vnet]        >> [HUB] RAW HEX MEMORY DUMPS");
        pageLines.push_back("[LINK:index.vnet]       >> [HUB] MASTER ROUTING INDEX");
        pageLines.push_back("[LINK:project9.vnet]    >> [HUB] SUBTERRANEAN BLACK SITE DATABASE");
        pageLines.push_back("[LINK:echolab.vnet]     >> [HUB] FREQUENCY RESEARCH LAB");
        pageLines.push_back("[LINK:phantom.vnet]     >> [HUB] PHANTOM NODE PROTOCOL");
        pageLines.push_back("[LINK:glitch.vnet]      >> [HUB] GLITCH REALITY OVERRIDE");
        pageLines.push_back("[LINK:stasis.vnet]      >> [HUB] CRYOGENIC STASIS PODS");
        pageLines.push_back("[LINK:entropy.vnet]     >> [HUB] ENTROPY ENGINE MONITOR");
        pageLines.push_back("[LINK:hive.vnet]        >> [HUB] HIVE MIND COLLECTIVE");
        pageLines.push_back("[LINK:nexus.vnet]       >> [HUB] CENTRAL NEXUS ROUTER");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[HR]");
        
        // Special Nodes Section
        pageLines.push_back("[SUBTITLE] █ SPECIAL / UNCLASSIFIED NODES");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[LINK:hashbeat.vnet]    >> [SPECIAL] UNDERGROUND AUDIO BEAT VAULT");
        pageLines.push_back("[LINK:lifeleaks.vnet]   >> [SPECIAL] UNFILTERED INTELLIGENCE DUMP");
        pageLines.push_back("[LINK:luna.vnet]        >> [SPECIAL] SILENT ASPHYXIA - EXECUTIONER");
        pageLines.push_back("[LINK:subcell.vnet]     >> [SPECIAL] CELLULAR TELEMETRY OVERRIDE");
        pageLines.push_back("[LINK:feed99.vnet]      >> [SPECIAL] NEURAL BROADCAST FEED");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[HR]");
        
        // Player Stats
        pageLines.push_back("[SUBTITLE] █ OPERATOR STATUS & SYSTEM TELEMETRY");
        pageLines.push_back("[TEXT] ");
        char stats[256];
        snprintf(stats, sizeof(stats), "HANDLE    : %s", player.handle);
        pageLines.push_back("[CODE] " + std::string(stats));
        snprintf(stats, sizeof(stats), "VCOIN     : %.2f VCOIN", player.vcoin);
        pageLines.push_back("[CODE] " + std::string(stats));
        snprintf(stats, sizeof(stats), "TRACE     : %d%%", player.traceLevel);
        pageLines.push_back("[CODE] " + std::string(stats));
        snprintf(stats, sizeof(stats), "ICE       : %d/3 LAYERS", player.iceShields);
        pageLines.push_back("[CODE] " + std::string(stats));
        snprintf(stats, sizeof(stats), "CRT HEAT  : %.0f°C", player.crtHeat);
        pageLines.push_back("[CODE] " + std::string(stats));
        pageLines.push_back("[TEXT] ");
        
        // Discovered Sites
        pageLines.push_back("[SUBTITLE] █ DISCOVERED SUBNET NODES [" + std::to_string(player.assignedCount) + "/20]");
        pageLines.push_back("[TEXT] ");
        if (player.assignedCount > 0) {
            for (int i = 0; i < player.assignedCount; i++) {
                char siteLine[128];
                snprintf(siteLine, sizeof(siteLine), "[LINK:%s] >> vnet://%s", 
                         player.assignedSites[i], player.assignedSites[i]);
                pageLines.push_back(std::string(siteLine));
            }
        } else {
            pageLines.push_back("[TEXT] No sites discovered yet. Use 'scan' in terminal to find nodes.");
        }
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[HR]");
        
        // Footer
        pageLines.push_back("[PULSE] 'THEY CAN SEE THROUGH THE CRT SCREEN... DON'T LOOK BACK.'");
        pageLines.push_back("[TEXT] ");
        pageLines.push_back("[TEXT] Tip: Press [TAB] to toggle terminal overlay. Mine VCOIN at crypto.vnet.");
        pageLines.push_back("[TEXT] Type 'help' for available commands.");
        pageLines.push_back("[HR]");
        pageLines.push_back("[BLOOD] 'THE NETWORK IS ALIVE. IT IS DRINKING YOUR HEAT.'");
        
        return;
    }
    
    // Copy site content
    if (site->content != NULL) {
        for (int i = 0; site->content[i] != NULL && i < 50; i++) {
            std::string line = site->content[i];

            // Replace dynamic placeholders
            if (line.find("ICE_COUNT") != std::string::npos) {
                char buffer[256];
                snprintf(buffer, sizeof(buffer), "%d/3", player.iceShields);
                size_t pos = line.find("ICE_COUNT");
                line.replace(pos, 9, buffer);
            }

            if (line.find("VCOIN_BALANCE") != std::string::npos) {
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%.2f", player.vcoin);
                size_t pos = line.find("VCOIN_BALANCE");
                line.replace(pos, 13, buffer);
            }

            if (line.find("TRACE_LEVEL") != std::string::npos) {
                char buffer[16];
                snprintf(buffer, sizeof(buffer), "%d", player.traceLevel);
                size_t pos = line.find("TRACE_LEVEL");
                line.replace(pos, 11, buffer);
            }

            if (line.find("HANDLE_NAME") != std::string::npos) {
                size_t pos = line.find("HANDLE_NAME");
                line.replace(pos, 11, player.handle);
            }

            if (line.find("PORT_NUMBER") != std::string::npos) {
                char buffer[16];
                snprintf(buffer, sizeof(buffer), "%d", player.port);
                size_t pos = line.find("PORT_NUMBER");
                line.replace(pos, 11, buffer);
            }

            pageLines.push_back(line);
        }
    }
}