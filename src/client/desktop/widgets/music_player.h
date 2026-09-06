#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

// ============================================================
// MUSIC PLAYER WIDGET - VEKTRA OS EDITION
// ============================================================

struct MusicTrack {
    std::string title;
    std::string artist;
    std::string filePath;
    Color accentColor;
    float duration;
};

struct Playlist {
    std::vector<MusicTrack> tracks;
    int currentIndex;
    bool isPlaying;
    bool isShuffled;
    bool isRepeating;
    float volume;
    float progress;
    float elapsedTime;
};

class MusicPlayer {
public:
    static MusicPlayer& Get() {
        static MusicPlayer instance;
        return instance;
    }

    void Init();
    void Shutdown();
    void Update(float dt);
    void Draw(float x, float y, float width, float height);

    void Play();
    void Pause();
    void TogglePlay();
    void Next();
    void Previous();
    void ToggleShuffle();
    void ToggleRepeat();
    void SetVolume(float volume);
    void Seek(float progress);

    bool IsPlaying() const { return m_playlist.isPlaying; }
    float GetVolume() const { return m_playlist.volume; }
    int GetCurrentTrackIndex() const { return m_playlist.currentIndex; }
    const MusicTrack* GetCurrentTrack() const;

    void LoadTracks(const std::vector<MusicTrack>& tracks);

    void LoadIcons();
    void UnloadIcons();
    Texture2D GetIcon(const std::string& key);

private:
    MusicPlayer() = default;
    ~MusicPlayer() = default;
    MusicPlayer(const MusicPlayer&) = delete;
    MusicPlayer& operator=(const MusicPlayer&) = delete;

    void DrawTrackList(float x, float y, float width, float height);
    void DrawControls(float x, float y, float width);
    void DrawProgressBar(float x, float y, float width);
    void DrawVisualizer(float x, float y, float width, float height);
    void LoadTrack(int index);

    Playlist m_playlist;
    Music m_music;
    bool m_musicLoaded = false;
    float m_visualizerTime = 0.0f;
    float m_visualizerSamples[64];
    float m_glowPulse = 0.0f;

    std::unordered_map<std::string, Texture2D> m_iconTextures;
};

MusicPlayer& GetMusicPlayer();