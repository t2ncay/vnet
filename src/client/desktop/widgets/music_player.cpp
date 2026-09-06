#include "music_player.h"
#include "render.h"
#include "game.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>

extern Player g_player;

// ============================================================
// MUSIC PLAYER IMPLEMENTATION
// ============================================================

void MusicPlayer::Init() {
    m_playlist.currentIndex = 0;
    m_playlist.isPlaying = false;
    m_playlist.isShuffled = false;
    m_playlist.isRepeating = false;
    m_playlist.volume = 0.7f;
    m_playlist.progress = 0.0f;
    m_playlist.elapsedTime = 0.0f;
    m_visualizerTime = 0.0f;
    m_glowPulse = 0.0f;
    
    // Initialize visualizer samples
    for (int i = 0; i < 64; i++) {
        m_visualizerSamples[i] = 0.0f;
    }
    
    LoadIcons();
    
    // Load default tracks
    std::vector<MusicTrack> defaultTracks = {
        {"Vanished", "Crystal Castles", "assets/audios/vanished.mp3", {40, 240, 100, 255}, 0.0f},
        {"Crimewave", "Crystal Castles", "assets/audios/crimewave.mp3", {0, 220, 240, 255}, 0.0f},
        {"Runaway", "4tvnex", "assets/audios/runaway.mp3", {40, 240, 100, 255}, 0.0f},
        {"UNKNOWN SIGNAL", "VEKTRA // 18.0 Hz", "assets/audios/tuncay.wav", {220, 20, 40, 255}, 0.0f},
        {"The Day of Night", "AKIRA YAMAOKA", "assets/audios/akira.wav", {220, 20, 40, 255}, 0.0f}
    };
    LoadTracks(defaultTracks);
}

void MusicPlayer::Shutdown() {
    if (m_musicLoaded) {
        UnloadMusicStream(m_music);
        m_musicLoaded = false;
    }
    UnloadIcons();
}

void MusicPlayer::LoadIcons() {
    auto loadIcon = [this](const std::string& key, const std::string& path) {
        Texture2D tex = LoadTexture(path.c_str());
        if (tex.id != 0) {
            m_iconTextures[key] = tex;
            printf("[MUSIC] Loaded icon: %s\n", path.c_str());
        } else {
            printf("[MUSIC] Failed to load icon: %s\n", path.c_str());
        }
    };

    loadIcon("play", "assets/icons/play.png");
    loadIcon("pause", "assets/icons/pause.png");
    loadIcon("next", "assets/icons/next.png");
    loadIcon("prev", "assets/icons/prev.png");
    loadIcon("shuffle", "assets/icons/shuffle.png");
    loadIcon("repeat", "assets/icons/repeat.png");
    loadIcon("volume", "assets/icons/volume.png");
}

void MusicPlayer::UnloadIcons() {
    for (auto& pair : m_iconTextures) {
        UnloadTexture(pair.second);
    }
    m_iconTextures.clear();
}

Texture2D MusicPlayer::GetIcon(const std::string& key) {
    auto it = m_iconTextures.find(key);
    if (it != m_iconTextures.end()) {
        return it->second;
    }
    Texture2D empty = {0};
    return empty;
}

void MusicPlayer::LoadTracks(const std::vector<MusicTrack>& tracks) {
    m_playlist.tracks = tracks;
    m_playlist.currentIndex = 0;
    
    if (!m_playlist.tracks.empty()) {
        LoadTrack(0);
    }
}

void MusicPlayer::LoadTrack(int index) {
    if (index < 0 || index >= (int)m_playlist.tracks.size()) return;
    
    if (m_musicLoaded) {
        UnloadMusicStream(m_music);
        m_musicLoaded = false;
    }
    
    const MusicTrack& track = m_playlist.tracks[index];
    
    m_music = LoadMusicStream(track.filePath.c_str());
    if (m_music.stream.buffer != NULL) {
        m_musicLoaded = true;
        m_playlist.progress = 0.0f;
        m_playlist.elapsedTime = 0.0f;
        SetMusicVolume(m_music, m_playlist.volume);
        m_playlist.tracks[index].duration = GetMusicTimeLength(m_music);
        
        printf("[MUSIC] Loaded: %s (%.2fs)\n", track.title.c_str(), 
               m_playlist.tracks[index].duration);
    } else {
        printf("[MUSIC] Failed to load: %s\n", track.filePath.c_str());
    }
}

void MusicPlayer::Update(float dt) {
    m_glowPulse += dt * 0.5f;
    
    if (m_musicLoaded && m_playlist.isPlaying) {
        UpdateMusicStream(m_music);
        
        float currentTime = GetMusicTimePlayed(m_music);
        float duration = GetMusicTimeLength(m_music);
        if (duration > 0.0f) {
            m_playlist.progress = currentTime / duration;
            m_playlist.elapsedTime = currentTime;
        }
        
        if (m_playlist.progress >= 0.99f) {
            if (m_playlist.isRepeating) {
                Play();
            } else {
                Next();
            }
        }
        
        // Update visualizer with more dynamic range
        m_visualizerTime += dt;
        for (int i = 0; i < 64; i++) {
            float base = sinf(m_visualizerTime * (1.5f + i * 0.12f)) * 0.5f + 0.5f;
            float noise = (rand() % 100) / 200.0f;
            float sample = (base * 0.6f + noise * 0.4f);
            // Add some "bass" emphasis on lower indices
            if (i < 20) {
                sample += sinf(m_visualizerTime * 0.8f + i * 0.3f) * 0.2f;
            }
            if (sample > 1.0f) sample = 1.0f;
            // Smooth the samples
            m_visualizerSamples[i] = m_visualizerSamples[i] * 0.7f + sample * 0.3f;
        }
    } else {
        // Decay visualizer when paused
        for (int i = 0; i < 64; i++) {
            m_visualizerSamples[i] *= 0.97f;
            if (m_visualizerSamples[i] < 0.01f) m_visualizerSamples[i] = 0.0f;
        }
    }
}

const MusicTrack* MusicPlayer::GetCurrentTrack() const {
    if (m_playlist.currentIndex >= 0 && 
        m_playlist.currentIndex < (int)m_playlist.tracks.size()) {
        return &m_playlist.tracks[m_playlist.currentIndex];
    }
    return nullptr;
}

void MusicPlayer::Play() {
    if (!m_musicLoaded) {
        if (!m_playlist.tracks.empty()) {
            LoadTrack(m_playlist.currentIndex);
        }
        return;
    }
    
    m_playlist.isPlaying = true;
    PlayMusicStream(m_music);
    PushCliLog("[MUSIC]: Playing %s", m_playlist.tracks[m_playlist.currentIndex].title.c_str());
}

void MusicPlayer::Pause() {
    m_playlist.isPlaying = false;
    PauseMusicStream(m_music);
    PushCliLog("[MUSIC]: Paused");
}

void MusicPlayer::TogglePlay() {
    if (m_playlist.isPlaying) {
        Pause();
    } else {
        Play();
    }
}

void MusicPlayer::Next() {
    if (m_playlist.tracks.empty()) return;
    
    int nextIndex;
    if (m_playlist.isShuffled) {
        do {
            nextIndex = rand() % m_playlist.tracks.size();
        } while (nextIndex == m_playlist.currentIndex && m_playlist.tracks.size() > 1);
    } else {
        nextIndex = (m_playlist.currentIndex + 1) % m_playlist.tracks.size();
    }
    
    m_playlist.currentIndex = nextIndex;
    LoadTrack(nextIndex);
    if (m_playlist.isPlaying) {
        Play();
    }
    PushCliLog("[MUSIC]: Next track - %s", m_playlist.tracks[nextIndex].title.c_str());
}

void MusicPlayer::Previous() {
    if (m_playlist.tracks.empty()) return;
    
    int prevIndex = (m_playlist.currentIndex - 1 + m_playlist.tracks.size()) % 
                    m_playlist.tracks.size();
    m_playlist.currentIndex = prevIndex;
    LoadTrack(prevIndex);
    if (m_playlist.isPlaying) {
        Play();
    }
    PushCliLog("[MUSIC]: Previous track - %s", m_playlist.tracks[prevIndex].title.c_str());
}

void MusicPlayer::ToggleShuffle() {
    m_playlist.isShuffled = !m_playlist.isShuffled;
    PushCliLog("[MUSIC]: Shuffle %s", m_playlist.isShuffled ? "ON" : "OFF");
}

void MusicPlayer::ToggleRepeat() {
    m_playlist.isRepeating = !m_playlist.isRepeating;
    PushCliLog("[MUSIC]: Repeat %s", m_playlist.isRepeating ? "ON" : "OFF");
}

void MusicPlayer::SetVolume(float volume) {
    m_playlist.volume = volume;
    if (m_playlist.volume < 0.0f) m_playlist.volume = 0.0f;
    if (m_playlist.volume > 1.0f) m_playlist.volume = 1.0f;
    if (m_musicLoaded) {
        SetMusicVolume(m_music, m_playlist.volume);
    }
}

void MusicPlayer::Seek(float progress) {
    if (!m_musicLoaded) return;
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;
    float duration = GetMusicTimeLength(m_music);
    SeekMusicStream(m_music, progress * duration);
    m_playlist.progress = progress;
}

// ============================================================
// VEKTRA OS STYLE DRAW FUNCTIONS
// ============================================================

static void DrawCRTScanlines(float x, float y, float w, float h, float offset) {
    for (int i = 0; i < (int)h; i += 4) {
        float scanY = y + i + fmodf(offset, 4.0f);
        unsigned char alpha = (i % 2 == 0) ? 4 : 2;
        DrawScaledRect(x, scanY, w, 1, {0, 0, 0, alpha});
    }
}

static void DrawCornerReticle(float x, float y, float size, Color color, bool topLeft) {
    if (topLeft) {
        DrawScaledRect(x, y, size, 2, color);
        DrawScaledRect(x, y, 2, size, color);
    } else {
        DrawScaledRect(x + size, y, size, 2, color);
        DrawScaledRect(x, y, 2, size, color);
    }
}

void MusicPlayer::Draw(float x, float y, float width, float height) {
    Vector2 refMouse = GetRefMousePos();
    bool hover = RefRectHover(x, y, width, height, refMouse);
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    float t = m_glowPulse;
    float pulse = sinf(t) * 0.3f + 0.7f;
    
    // ---- BACKGROUND WITH VEKTRA OS STYLE ----
    DrawScaledRect(x, y, width, height, Color{6, 8, 14, 240});
    DrawScaledRectLines(x, y, width, height, 
                        hover ? Color{0, 220, 240, 100} : Color{30, 35, 50, 80});
    
    // ---- CORNER RETICLES ----
    Color retCol = {0, 220, 240, (unsigned char)(pulse * 100 + 50)};
    float retSize = 12.0f;
    DrawCornerReticle(x + 4, y + 4, retSize, retCol, true);
    DrawCornerReticle(x + width - 4 - retSize, y + 4, retSize, retCol, false);
    DrawCornerReticle(x + 4, y + height - 4 - retSize, retSize, retCol, true);
    DrawCornerReticle(x + width - 4 - retSize, y + height - 4 - retSize, retSize, retCol, false);
    
    // ---- HEADER ----
    float headerH = 34.0f;
    DrawScaledRect(x + 4, y + 4, width - 8, headerH, Color{10, 12, 20, 200});
    DrawScaledLine(x + 4, y + 4 + headerH, x + width - 4, y + 4 + headerH, Color{30, 35, 50, 100});
    
    // VEKTRA OS styled header
    DrawScaledText("VEKTRA // AUDIO", x + 14, y + 12, 11, Color{0, 220, 240, 200});
    
    // Status indicator with pulse
    Color statusCol = m_playlist.isPlaying ? 
        Color{40, 240, 100, (unsigned char)(pulse * 200 + 55)} : 
        Color{220, 20, 40, (unsigned char)(pulse * 100 + 50)};
    DrawScaledRect(x + width - 24, y + 12, 8, 8, statusCol);
    DrawScaledText(m_playlist.isPlaying ? "LIVE" : "IDLE", x + width - 56, y + 11, 9, 
                   m_playlist.isPlaying ? COLOR_TOXIC : COLOR_GHOST);
    
    // [ DEPRECATED ] Close button
    // float closeX = x + width - 22;
    // bool closeHover = RefRectHover(closeX, y + 6, 16, 16, refMouse);
    // DrawScaledRect(closeX, y + 6, 16, 16, closeHover ? Color{220, 20, 40, 180} : Color{30, 35, 50, 120});
    // DrawScaledText("✕", closeX + 4, y + 8, 10, closeHover ? COLOR_BLACK : COLOR_GHOST);
    // if (clicked && closeHover) {
    //    PushCliLog("[MUSIC]: Widget minimized");
    // }
    
    float contentY = y + headerH + 8.0f;
    float contentH = height - headerH - 12.0f;
    
    // ---- ALBUM ART / INFO ----
    float artSize = 70.0f;
    float artX = x + 12;
    float artY = contentY + 4;
    
    // Album art with glow
    DrawScaledRect(artX - 2, artY - 2, artSize + 4, artSize + 4, 
                   {0, 220, 240, (unsigned char)(pulse * 20)});
    DrawScaledRect(artX, artY, artSize, artSize, Color{12, 14, 22, 255});
    DrawScaledRectLines(artX, artY, artSize, artSize, Color{30, 35, 50, 150});
    
    const MusicTrack* current = GetCurrentTrack();
    if (current) {
        float infoX = artX + artSize + 12;
        
        // Title with accent color
        DrawScaledText(current->title.c_str(), infoX, artY + 2, 14, current->accentColor);
        
        // Artist
        DrawScaledText(current->artist.c_str(), infoX, artY + 22, 10, COLOR_GHOST);
        
        // VEKTRA OS tag
        DrawScaledText("◆ COMSEC AUDIO STREAM", infoX, artY + 38, 8, {80, 90, 110, 150});
        
        // Duration
        char durationStr[32];
        int minutes = (int)current->duration / 60;
        int seconds = (int)current->duration % 60;
        snprintf(durationStr, sizeof(durationStr), "%02d:%02d", minutes, seconds);
        float durW = MeasureScaledTextWidth(durationStr, 9);
        DrawScaledText(durationStr, infoX + 170 - durW, artY + 44, 9, COLOR_GHOST);
        
        // Status indicator
        DrawScaledRect(infoX + 170 - 16, artY + 2, 8, 8, statusCol);
    } else {
        DrawScaledText("NO SIGNAL", artX + 10, artY + 28, 12, COLOR_GHOST);
    }
    
    // ---- SPACING ----
    float controlsY = artY + artSize + 12.0f;
    
    // ---- PROGRESS BAR ----
    float barX = x + 12;
    float barY = controlsY;
    float barW = width - 24;
    float barH = 4.0f;
    
    DrawScaledRect(barX, barY, barW, barH, Color{12, 15, 20, 255});
    DrawScaledRectLines(barX, barY, barW, barH, Color{30, 35, 50, 80});
    
    float fillW = barW * m_playlist.progress;
    if (fillW < 2.0f) fillW = 2.0f;
    DrawScaledRect(barX, barY, fillW, barH, Color{0, 220, 240, 200});
    
    // Glow at progress tip
    if (fillW > 5.0f) {
        DrawScaledRect(barX + fillW - 4.0f, barY - 2, 4, barH + 4, 
                       {0, 220, 240, 40});
    }
    
    // Handle
    float handleX = barX + fillW;
    float handleR = 5.0f;
    bool hoverHandle = RefRectHover(handleX - handleR, barY - handleR, 
                                    handleR * 2, handleR * 2 + barH, refMouse);
    DrawScaledRect(handleX - handleR/2, barY - handleR/2 + 1, 
                   handleR, handleR, hoverHandle ? Color{0, 220, 240, 200} : Color{20, 22, 35, 180});
    DrawScaledRectLines(handleX - handleR/2, barY - handleR/2 + 1, 
                        handleR, handleR, Color{0, 220, 240, 100});
    
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if (RefRectHover(barX, barY - 10, barW, 20, refMouse) || 
            RefRectHover(handleX - handleR, barY - handleR, handleR * 2, handleR * 2 + barH, refMouse)) {
            float newProgress = (refMouse.x - barX) / barW;
            if (newProgress < 0.0f) newProgress = 0.0f;
            if (newProgress > 1.0f) newProgress = 1.0f;
            Seek(newProgress);
        }
    }
    
    // Time labels
    char timeStr[32];
    int mins = (int)m_playlist.elapsedTime / 60;
    int secs = (int)m_playlist.elapsedTime % 60;
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", mins, secs);
    DrawScaledText(timeStr, barX, barY - 14, 8, Color{80, 90, 110, 150});
    
    if (current && current->duration > 0.0f) {
        int totalMins = (int)current->duration / 60;
        int totalSecs = (int)current->duration % 60;
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d", totalMins, totalSecs);
        float timeW = MeasureScaledTextWidth(timeStr, 8);
        DrawScaledText(timeStr, barX + barW - timeW, barY - 14, 8, Color{80, 90, 110, 150});
    }
    
    // ---- CONTROLS ----
    float controlsY2 = barY + 22.0f;
    DrawControls(x + 12, controlsY2, width - 24);
    
    // ---- VISUALIZER ----
    float vizY = controlsY2 + 52.0f;
    float vizH = 28.0f;
    DrawVisualizer(x + 12, vizY, width - 24, vizH);
    
    // ---- TRACK LIST ----
    float listY = vizY + vizH + 8.0f;
    float listH = height - (listY - y) - 8.0f;
    if (listH > 40) {
        DrawTrackList(x + 4, listY, width - 8, listH);
    }
    
    // ---- CRT SCANLINES ----
    DrawCRTScanlines(x, y, width, height, t * 30.0f);
    
    // ---- VIGNETTE ----
    DrawScaledRect(x, y, width, 2, {0, 0, 0, 60});
    DrawScaledRect(x, y + height - 2, width, 2, {0, 0, 0, 60});
    DrawScaledRect(x, y, 2, height, {0, 0, 0, 60});
    DrawScaledRect(x + width - 2, y, 2, height, {0, 0, 0, 60});
}

void MusicPlayer::DrawControls(float x, float y, float width) {
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    float t = m_glowPulse;
    float pulse = sinf(t) * 0.3f + 0.7f;
    
    float btnSize = 28.0f;
    float spacing = 6.0f;
    float totalWidth = btnSize * 6 + spacing * 5;
    float startX = x + (width - totalWidth) / 2.0f;
    float btnY = y;
    
    // ---- SHUFFLE ----
    float btnX = startX;
    bool hover = RefRectHover(btnX, btnY, btnSize, btnSize, refMouse);
    DrawScaledRect(btnX, btnY, btnSize, btnSize, 
                   hover ? Color{20, 25, 45, 200} : Color{10, 12, 20, 160});
    DrawScaledRectLines(btnX, btnY, btnSize, btnSize, 
                        m_playlist.isShuffled ? Color{0, 220, 240, 200} : Color{30, 35, 50, 100});
    DrawScaledText("⇆", btnX + 6, btnY + 5, 12, 
                   m_playlist.isShuffled ? COLOR_CYAN : COLOR_GHOST);
    if (clicked && hover) ToggleShuffle();
    
    btnX += btnSize + spacing;
    
    // ---- PREVIOUS ----
    hover = RefRectHover(btnX, btnY, btnSize, btnSize, refMouse);
    DrawScaledRect(btnX, btnY, btnSize, btnSize, 
                   hover ? Color{20, 25, 45, 200} : Color{10, 12, 20, 160});
    DrawScaledRectLines(btnX, btnY, btnSize, btnSize, Color{30, 35, 50, 100});
    DrawScaledText("◄◄", btnX + 4, btnY + 5, 12, hover ? COLOR_CYAN : COLOR_GHOST);
    if (clicked && hover) Previous();
    
    btnX += btnSize + spacing;
    
    // ---- PLAY/PAUSE ----
    float playBtnSize = 36.0f;
    float playX = btnX - (playBtnSize - btnSize) / 2.0f;
    float playY = btnY - (playBtnSize - btnSize) / 2.0f;
    bool playHover = RefRectHover(playX, playY, playBtnSize, playBtnSize, refMouse);
    
    DrawScaledRect(playX, playY, playBtnSize, playBtnSize, 
                   playHover ? Color{0, 220, 240, 80} : Color{10, 14, 24, 200});
    DrawScaledRectLines(playX, playY, playBtnSize, playBtnSize, 
                        m_playlist.isPlaying ? Color{0, 220, 240, 200} : Color{30, 35, 50, 150});
    
    if (m_playlist.isPlaying) {
        DrawScaledText("▐▐", playX + 12, playY + 8, 14, COLOR_CYAN);
    } else {
        DrawScaledText("▶", playX + 13, playY + 8, 14, COLOR_TOXIC);
    }
    if (clicked && playHover) TogglePlay();
    
    btnX = playX + playBtnSize + spacing;
    
    // ---- NEXT ----
    hover = RefRectHover(btnX, btnY, btnSize, btnSize, refMouse);
    DrawScaledRect(btnX, btnY, btnSize, btnSize, 
                   hover ? Color{20, 25, 45, 200} : Color{10, 12, 20, 160});
    DrawScaledRectLines(btnX, btnY, btnSize, btnSize, Color{30, 35, 50, 100});
    DrawScaledText("►►", btnX + 4, btnY + 5, 12, hover ? COLOR_CYAN : COLOR_GHOST);
    if (clicked && hover) Next();
    
    btnX += btnSize + spacing;
    
    // ---- REPEAT ----
    hover = RefRectHover(btnX, btnY, btnSize, btnSize, refMouse);
    DrawScaledRect(btnX, btnY, btnSize, btnSize, 
                   hover ? Color{20, 25, 45, 200} : Color{10, 12, 20, 160});
    DrawScaledRectLines(btnX, btnY, btnSize, btnSize, 
                        m_playlist.isRepeating ? Color{0, 220, 240, 200} : Color{30, 35, 50, 100});
    DrawScaledText("↻", btnX + 6, btnY + 5, 14, 
                   m_playlist.isRepeating ? COLOR_CYAN : COLOR_GHOST);
    if (clicked && hover) ToggleRepeat();
}

void MusicPlayer::DrawVisualizer(float x, float y, float width, float height) {
    float barW = width / 64.0f;
    float maxH = height - 4.0f;
    float t = m_glowPulse;
    
    for (int i = 0; i < 64; i++) {
        float sample = m_visualizerSamples[i];
        float barH = sample * maxH;
        if (barH < 0.5f) barH = 0.5f;
        
        // VEKTRA OS color gradient
        Color col;
        if (sample > 0.7f) {
            col = {220, 20, 40, (unsigned char)(180 + sample * 75)};
        } else if (sample > 0.4f) {
            col = {0, 220, 240, (unsigned char)(160 + sample * 95)};
        } else if (sample > 0.2f) {
            col = {40, 240, 100, (unsigned char)(140 + sample * 115)};
        } else {
            col = {80, 90, 110, (unsigned char)(100 + sample * 155)};
        }
        
        // Add glow pulse
        float glowPulse = sinf(t * 2.0f + i * 0.2f) * 0.2f + 0.8f;
        col.a = (unsigned char)(col.a * glowPulse);
        
        float barX = x + i * barW;
        float barY = y + height - barH - 2;
        
        DrawScaledRect(barX, barY, barW - 1, barH, col);
        
        // Glow on top of high bars
        if (sample > 0.5f) {
            float glowH = barH * 0.2f;
            Color glowCol = col;
            glowCol.a = (unsigned char)(col.a * 0.3f);
            DrawScaledRect(barX, barY - glowH, barW - 1, glowH, glowCol);
        }
    }
}

void MusicPlayer::DrawTrackList(float x, float y, float width, float height) {
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    float t = m_glowPulse;
    
    // Section header
    DrawScaledText("PLAYLIST", x + 4, y, 9, Color{0, 220, 240, 150});
    DrawScaledLine(x + 4, y + 14, x + width - 4, y + 14, Color{30, 35, 50, 80});
    
    float listY = y + 18;
    float trackH = 22.0f;
    int visible = (int)((height - 20) / trackH);
    if (visible > (int)m_playlist.tracks.size()) {
        visible = (int)m_playlist.tracks.size();
    }
    
    static int scrollOffset = 0;
    
    float wheel = GetMouseWheelMove();
    if (RefRectHover(x, listY, width, height - 20, refMouse)) {
        scrollOffset -= (int)(wheel * 2);
        if (scrollOffset < 0) scrollOffset = 0;
        int maxScroll = (int)m_playlist.tracks.size() - visible;
        if (scrollOffset > maxScroll) scrollOffset = maxScroll;
        if (scrollOffset < 0) scrollOffset = 0;
    }
    
    for (int i = scrollOffset; i < (int)m_playlist.tracks.size() && 
                                i < scrollOffset + visible; i++) {
        float trackY = listY + (i - scrollOffset) * trackH;
        if (trackY > y + height - trackH) break;
        
        const MusicTrack& track = m_playlist.tracks[i];
        bool isCurrent = (i == m_playlist.currentIndex);
        bool hover = RefRectHover(x + 2, trackY, width - 4, trackH - 2, refMouse);
        
        Color bg;
        if (isCurrent) {
            bg = {0, 220, 240, 30};
        } else if (hover) {
            bg = {20, 25, 45, 100};
        } else {
            bg = {0, 0, 0, 0};
        }
        DrawScaledRect(x + 2, trackY, width - 4, trackH - 2, bg);
        
        // Current track indicator
        if (isCurrent) {
            float pulse = sinf(t * 3.0f) * 0.3f + 0.7f;
            DrawScaledRect(x + 2, trackY, 3, trackH - 2, 
                           {0, 220, 240, (unsigned char)(pulse * 150 + 50)});
        }
        
        // Track number
        char numStr[8];
        snprintf(numStr, sizeof(numStr), "%02d", i + 1);
        DrawScaledText(numStr, x + 18, trackY + 4, 8, 
                       isCurrent ? COLOR_CYAN : Color{80, 90, 110, 100});
        
        // Track title
        Color titleCol = isCurrent ? track.accentColor : COLOR_GHOST;
        DrawScaledText(track.title.c_str(), x + 42, trackY + 3, 10, titleCol);
        
        // Duration
        char durStr[16];
        int mins = (int)track.duration / 60;
        int secs = (int)track.duration % 60;
        snprintf(durStr, sizeof(durStr), "%02d:%02d", mins, secs);
        float durW = MeasureScaledTextWidth(durStr, 8);
        DrawScaledText(durStr, x + width - durW - 8, trackY + 4, 8, Color{80, 90, 110, 100});
        
        if (clicked && hover) {
            m_playlist.currentIndex = i;
            LoadTrack(i);
            Play();
            PushCliLog("[MUSIC]: Playing %s", track.title.c_str());
        }
    }
}

// ============================================================
// GLOBAL ACCESS
// ============================================================

MusicPlayer& GetMusicPlayer() {
    return MusicPlayer::Get();
}