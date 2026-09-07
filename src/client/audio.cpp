#include "audio.h"
#include "render.h"
#include <cstring>   // <-- ADD THIS for strcmp
#include <cstdlib>   // for rand (if you use it)

// Define the global sound bank
RaidSoundBank g_raidSounds;

void LoadRaidSounds(void) {
    // Zero out the struct first to avoid uninitialized members
    memset(&g_raidSounds, 0, sizeof(RaidSoundBank));
    
    // Load each sound (replace paths with your actual files)
    g_raidSounds.alertSiren     = LoadSound("assets/audio/raid_alert.wav");
    g_raidSounds.glitchScreech  = LoadSound("assets/audio/glitch_screech.wav");
    g_raidSounds.flashImpact    = LoadSound("assets/audio/flash_impact.wav");
    g_raidSounds.dataRush       = LoadSound("assets/audio/data_rush.wav");
    g_raidSounds.powerDown      = LoadSound("assets/audio/power_down.wav");
    g_raidSounds.terminalTyping = LoadSound("assets/audio/terminal_type.wav");
    g_raidSounds.radarBlip      = LoadSound("assets/audio/radar_blip.wav");
    g_raidSounds.successJingle  = LoadSound("assets/audio/success_jingle.wav");
    g_raidSounds.failureAlarm   = LoadSound("assets/audio/failure_alarm.wav");
    g_raidSounds.heartbeat      = LoadSound("assets/audio/heartbeat.wav");
    g_raidSounds.lowHum         = LoadSound("assets/audio/low_hum.wav");

    // Set default volume
    SetRaidVolume(0.7f);
}

void UnloadRaidSounds(void) {
    UnloadSound(g_raidSounds.alertSiren);
    UnloadSound(g_raidSounds.glitchScreech);
    UnloadSound(g_raidSounds.flashImpact);
    UnloadSound(g_raidSounds.dataRush);
    UnloadSound(g_raidSounds.powerDown);
    UnloadSound(g_raidSounds.terminalTyping);
    UnloadSound(g_raidSounds.radarBlip);
    UnloadSound(g_raidSounds.successJingle);
    UnloadSound(g_raidSounds.failureAlarm);
    UnloadSound(g_raidSounds.heartbeat);
    UnloadSound(g_raidSounds.lowHum);
}

void PlayRaidSound(const char* name) {
    if (strcmp(name, "alert") == 0) PlaySound(g_raidSounds.alertSiren);
    else if (strcmp(name, "glitch") == 0) PlaySound(g_raidSounds.glitchScreech);
    else if (strcmp(name, "flash") == 0) PlaySound(g_raidSounds.flashImpact);
    else if (strcmp(name, "data") == 0) PlaySound(g_raidSounds.dataRush);
    else if (strcmp(name, "powerdown") == 0) PlaySound(g_raidSounds.powerDown);
    else if (strcmp(name, "type") == 0) PlaySound(g_raidSounds.terminalTyping);
    else if (strcmp(name, "radar") == 0) PlaySound(g_raidSounds.radarBlip);
    else if (strcmp(name, "success") == 0) PlaySound(g_raidSounds.successJingle);
    else if (strcmp(name, "failure") == 0) PlaySound(g_raidSounds.failureAlarm);
    else if (strcmp(name, "heartbeat") == 0) PlaySound(g_raidSounds.heartbeat);
    else if (strcmp(name, "hum") == 0) PlaySound(g_raidSounds.lowHum);
}

void StopRaidSounds(void) {
    StopSound(g_raidSounds.alertSiren);
    StopSound(g_raidSounds.glitchScreech);
    StopSound(g_raidSounds.flashImpact);
    StopSound(g_raidSounds.dataRush);
    StopSound(g_raidSounds.powerDown);
    StopSound(g_raidSounds.terminalTyping);
    StopSound(g_raidSounds.radarBlip);
    StopSound(g_raidSounds.successJingle);
    StopSound(g_raidSounds.failureAlarm);
    StopSound(g_raidSounds.heartbeat);
    StopSound(g_raidSounds.lowHum);
}

void SetRaidVolume(float volume) {
    SetSoundVolume(g_raidSounds.alertSiren, volume);
    SetSoundVolume(g_raidSounds.glitchScreech, volume * 0.6f);
    SetSoundVolume(g_raidSounds.flashImpact, volume * 1.2f);
    SetSoundVolume(g_raidSounds.dataRush, volume * 0.8f);
    SetSoundVolume(g_raidSounds.powerDown, volume * 1.0f);
    SetSoundVolume(g_raidSounds.terminalTyping, volume * 0.5f);
    SetSoundVolume(g_raidSounds.radarBlip, volume * 0.4f);
    SetSoundVolume(g_raidSounds.successJingle, volume * 1.0f);
    SetSoundVolume(g_raidSounds.failureAlarm, volume * 1.0f);
    SetSoundVolume(g_raidSounds.heartbeat, volume * 0.7f);
    SetSoundVolume(g_raidSounds.lowHum, volume * 0.3f);
}