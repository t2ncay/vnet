#pragma once
#include "raylib.h"

typedef struct {
    Sound alertSiren;
    Sound glitchScreech;
    Sound flashImpact;
    Sound dataRush;
    Sound powerDown;
    Sound terminalTyping;
    Sound radarBlip;
    Sound successJingle;
    Sound failureAlarm;
    Sound heartbeat;
    Sound lowHum;
} RaidSoundBank;

extern RaidSoundBank g_raidSounds;

void LoadRaidSounds(void);
void UnloadRaidSounds(void);
void PlayRaidSound(const char* name);
void StopRaidSounds(void);
void SetRaidVolume(float volume);