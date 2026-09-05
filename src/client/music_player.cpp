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
    
    // Initialize visualizer samples
    for (int i = 0; i < 64; i++) {
        m_visualizerSamples[i] = 0.0f;
    }
    
    LoadIcons();
    
    // Load default tracks
    std::vector<MusicTrack> defaultTracks = {
        {"Vanished", "Crystal Castles", "assets/audios/vanished.mp3", {40, 240, 100, 255}, 0.0f},
        {"Crimewave", "Crystal Castles", "assets/audios/crimewave.mp3", {0, 220, 240, 255}, 0.0f},
        {"UNKNOWN SIGNAL", "VEKTRA // 18.0 Hz", "assets/audios/tuncay.wav", {220, 20, 40, 255}, 0.0f}
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
    
    // Load first track
    if (!m_playlist.tracks.empty()) {
        LoadTrack(0);
    }
}

void MusicPlayer::LoadTrack(int index) {
    if (index < 0 || index >= (int)m_playlist.tracks.size()) return;
    
    // Unload previous music
    if (m_musicLoaded) {
        UnloadMusicStream(m_music);
        m_musicLoaded = false;
    }
    
    const MusicTrack& track = m_playlist.tracks[index];
    
    // Load new music
    m_music = LoadMusicStream(track.filePath.c_str());
    if (m_music.stream.buffer != NULL) {
        m_musicLoaded = true;
        m_playlist.progress = 0.0f;
        m_playlist.elapsedTime = 0.0f;
        SetMusicVolume(m_music, m_playlist.volume);
        
        // Get duration
        m_playlist.tracks[index].duration = GetMusicTimeLength(m_music);
        
        printf("[MUSIC] Loaded: %s (%.2fs)\n", track.title.c_str(), 
               m_playlist.tracks[index].duration);
    } else {
        printf("[MUSIC] Failed to load: %s\n", track.filePath.c_str());
    }
}

void MusicPlayer::Update(float dt) {
    if (m_musicLoaded && m_playlist.isPlaying) {
        UpdateMusicStream(m_music);
        
        // Update progress
        float currentTime = GetMusicTimePlayed(m_music);
        float duration = GetMusicTimeLength(m_music);
        if (duration > 0.0f) {
            m_playlist.progress = currentTime / duration;
            m_playlist.elapsedTime = currentTime;
        }
        
        // Auto-advance to next track
        if (m_playlist.progress >= 0.99f) {
            if (m_playlist.isRepeating) {
                Play();
            } else {
                Next();
            }
        }
        
        // Update visualizer
        m_visualizerTime += dt;
        for (int i = 0; i < 64; i++) {
            float base = sinf(m_visualizerTime * (2.0f + i * 0.15f)) * 0.5f + 0.5f;
            float noise = (rand() % 100) / 200.0f;
            m_visualizerSamples[i] = (base * 0.7f + noise * 0.3f) * 
                                     (0.3f + m_playlist.progress * 0.7f);
            // Clamp
            if (m_visualizerSamples[i] > 1.0f) m_visualizerSamples[i] = 1.0f;
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
        // Random track, avoid repeating
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
// DRAW FUNCTIONS WITH PNG ICONS
// ============================================================

void MusicPlayer::Draw(float x, float y, float width, float height) {
    Vector2 refMouse = GetRefMousePos();
    bool hover = RefRectHover(x, y, width, height, refMouse);
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    // ---- BACKGROUND ----
    DrawScaledRect(x, y, width, height, Color{10, 12, 18, 235});
    DrawScaledRectLines(x, y, width, height, 
                        hover ? Color{40, 220, 120, 150} : Color{30, 35, 50, 100});
    
    // ---- HEADER ----
    float headerH = 30.0f;
    DrawScaledRect(x, y, width, headerH, Color{16, 18, 26, 255});
    DrawScaledLine(x, y + headerH, x + width, y + headerH, Color{30, 35, 45, 150});
    DrawScaledText("♫ MUSIC PLAYER", x + 12, y + 8, 10, COLOR_TOXIC);
    
    // Close button
    float closeX = x + width - 22;
    bool closeHover = RefRectHover(closeX, y + 4, 18, 18, refMouse);
    DrawScaledRect(closeX, y + 4, 18, 18, closeHover ? COLOR_BLOOD : Color{30, 35, 50, 150});
    DrawScaledText("✕", closeX + 4, y + 6, 12, closeHover ? COLOR_BLACK : COLOR_GHOST);
    if (clicked && closeHover) {
        PushCliLog("[MUSIC]: Widget minimized");
    }
    
    float contentY = y + headerH + 8.0f;
    float contentH = height - headerH - 8.0f;
    
    // ---- NOW PLAYING (ALBUM ART / INFO) ----
    float artSize = 60.0f;
    float artX = x + 12;
    float artY = contentY + 4;
    
    // Album art placeholder
    DrawScaledRect(artX, artY, artSize, artSize, Color{18, 20, 30, 255});
    DrawScaledRectLines(artX, artY, artSize, artSize, Color{30, 35, 50, 150});
    
    const MusicTrack* current = GetCurrentTrack();
    if (current) {
        // Track info
        float infoX = artX + artSize + 12;
        DrawScaledText(current->title.c_str(), infoX, artY + 4, 12, current->accentColor);
        DrawScaledText(current->artist.c_str(), infoX, artY + 24, 10, COLOR_GHOST);
        
        // Duration
        char durationStr[32];
        int minutes = (int)current->duration / 60;
        int seconds = (int)current->duration % 60;
        snprintf(durationStr, sizeof(durationStr), "%02d:%02d", minutes, seconds);
        float durW = MeasureScaledTextWidth(durationStr, 9);
        DrawScaledText(durationStr, infoX + 150 - durW, artY + 44, 9, COLOR_GHOST);
        
        // Status indicator
        DrawScaledRect(infoX + 150 - 20, artY + 4, 8, 8, 
                       m_playlist.isPlaying ? COLOR_TOXIC : COLOR_AMBER);
    } else {
        DrawScaledText("No track loaded", artX + 6, artY + 22, 11, COLOR_GHOST);
    }
    
    // ---- SPACING: Controls start after album art ----
    float controlsY = artY + artSize + 12.0f;  // Increased from 8 to 12
    
    // ---- PROGRESS BAR (MOVED TO TOP OF CONTROLS AREA) ----
    float barX = x + 12;
    float barY = controlsY;
    float barW = width - 24;
    float barH = 4.0f;
    
    DrawProgressBar(barX, barY, barW);
    
    // ---- CONTROLS (MOVED DOWN TO NOT OVERLAP PROGRESS BAR) ----
    float controlsY2 = barY + 24.0f;  // 24px below progress bar
    DrawControls(x + 12, controlsY2, width - 24);
    
    // ---- VISUALIZER (MOVED FURTHER DOWN) ----
    float vizY = controlsY2 + 56.0f;  // Below controls
    float vizH = 30.0f;
    DrawVisualizer(x + 12, vizY, width - 24, vizH);
    
    // ---- TRACK LIST ----
    float listY = vizY + vizH + 8.0f;
    float listH = height - (listY - y) - 8.0f;
    if (listH > 40) {
        DrawTrackList(x + 4, listY, width - 8, listH);
    }
}

void MusicPlayer::DrawControls(float x, float y, float width) {
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    float btnSize = 30.0f;
    float spacing = 8.0f;  // Reduced from 10 to 8
    float totalWidth = btnSize * 6 + spacing * 5;
    float startX = x + (width - totalWidth) / 2.0f;
    
    // Center buttons vertically in their area
    float btnY = y;
    
    // ---- SHUFFLE BUTTON ----
    float btnX = startX;
    bool hover = RefRectHover(btnX, btnY, btnSize, btnSize, refMouse);
    DrawScaledRect(btnX, btnY, btnSize, btnSize, 
                   hover ? Color{40, 45, 65, 200} : Color{20, 22, 35, 180});
    DrawScaledRectLines(btnX, btnY, btnSize, btnSize, 
                        m_playlist.isShuffled ? COLOR_TOXIC : Color{30, 35, 50, 100});
    
    Texture2D shuffleIcon = GetIcon("shuffle");
    if (shuffleIcon.id != 0) {
        float iconSize = 18.0f;
        float iconX = btnX + (btnSize - iconSize) / 2.0f;
        float iconY = btnY + (btnSize - iconSize) / 2.0f;
        DrawTexturePro(shuffleIcon,
                       {0, 0, (float)shuffleIcon.width, (float)shuffleIcon.height},
                       {SX(iconX), SY(iconY), iconSize * g_uiScale, iconSize * g_uiScale},
                       {0, 0}, 0.0f,
                       m_playlist.isShuffled ? COLOR_TOXIC : COLOR_GHOST);
    } else {
        DrawScaledText("⟳", btnX + 4, btnY + 4, 14, 
                       m_playlist.isShuffled ? COLOR_TOXIC : COLOR_GHOST);
    }
    if (clicked && hover) ToggleShuffle();
    
    btnX += btnSize + spacing;
    
    // ---- PREVIOUS BUTTON ----
    hover = RefRectHover(btnX, btnY, btnSize, btnSize, refMouse);
    DrawScaledRect(btnX, btnY, btnSize, btnSize, 
                   hover ? Color{40, 45, 65, 200} : Color{20, 22, 35, 180});
    DrawScaledRectLines(btnX, btnY, btnSize, btnSize, Color{30, 35, 50, 100});
    
    Texture2D prevIcon = GetIcon("prev");
    if (prevIcon.id != 0) {
        float iconSize = 18.0f;
        float iconX = btnX + (btnSize - iconSize) / 2.0f;
        float iconY = btnY + (btnSize - iconSize) / 2.0f;
        DrawTexturePro(prevIcon,
                       {0, 0, (float)prevIcon.width, (float)prevIcon.height},
                       {SX(iconX), SY(iconY), iconSize * g_uiScale, iconSize * g_uiScale},
                       {0, 0}, 0.0f,
                       hover ? COLOR_TOXIC : COLOR_GHOST);
    } else {
        DrawScaledText("◄◄", btnX + 2, btnY + 4, 12, hover ? COLOR_TOXIC : COLOR_GHOST);
    }
    if (clicked && hover) Previous();
    
    btnX += btnSize + spacing;
    
    // ---- PLAY/PAUSE BUTTON (LARGER, MORE PROMINENT) ----
    float playBtnSize = 38.0f;  // Slightly larger
    float playX = btnX - (playBtnSize - btnSize) / 2.0f;
    float playY = btnY - (playBtnSize - btnSize) / 2.0f;
    bool playHover = RefRectHover(playX, playY, playBtnSize, playBtnSize, refMouse);
    
    DrawScaledRect(playX, playY, playBtnSize, playBtnSize, 
                   playHover ? Color{50, 55, 80, 220} : Color{30, 35, 55, 200});
    DrawScaledRectLines(playX, playY, playBtnSize, playBtnSize, 
                        m_playlist.isPlaying ? COLOR_TOXIC : COLOR_AMBER);
    
    if (m_playlist.isPlaying) {
        Texture2D pauseIcon = GetIcon("pause");
        if (pauseIcon.id != 0) {
            float iconSize = 22.0f;
            float iconX = playX + (playBtnSize - iconSize) / 2.0f;
            float iconY = playY + (playBtnSize - iconSize) / 2.0f;
            DrawTexturePro(pauseIcon,
                           {0, 0, (float)pauseIcon.width, (float)pauseIcon.height},
                           {SX(iconX), SY(iconY), iconSize * g_uiScale, iconSize * g_uiScale},
                           {0, 0}, 0.0f,
                           playHover ? COLOR_TOXIC : COLOR_CYAN);
        } else {
            DrawScaledText("||", playX + 11, playY + 7, 14, COLOR_TOXIC);
        }
    } else {
        Texture2D playIcon = GetIcon("play");
        if (playIcon.id != 0) {
            float iconSize = 22.0f;
            float iconX = playX + (playBtnSize - iconSize) / 2.0f;
            float iconY = playY + (playBtnSize - iconSize) / 2.0f;
            DrawTexturePro(playIcon,
                           {0, 0, (float)playIcon.width, (float)playIcon.height},
                           {SX(iconX), SY(iconY), iconSize * g_uiScale, iconSize * g_uiScale},
                           {0, 0}, 0.0f,
                           playHover ? COLOR_AMBER : COLOR_TOXIC);
        } else {
            DrawScaledText("▶", playX + 11, playY + 7, 14, COLOR_AMBER);
        }
    }
    if (clicked && playHover) TogglePlay();
    
    btnX = playX + playBtnSize + spacing;
    
    // ---- NEXT BUTTON ----
    hover = RefRectHover(btnX, btnY, btnSize, btnSize, refMouse);
    DrawScaledRect(btnX, btnY, btnSize, btnSize, 
                   hover ? Color{40, 45, 65, 200} : Color{20, 22, 35, 180});
    DrawScaledRectLines(btnX, btnY, btnSize, btnSize, Color{30, 35, 50, 100});
    
    Texture2D nextIcon = GetIcon("next");
    if (nextIcon.id != 0) {
        float iconSize = 18.0f;
        float iconX = btnX + (btnSize - iconSize) / 2.0f;
        float iconY = btnY + (btnSize - iconSize) / 2.0f;
        DrawTexturePro(nextIcon,
                       {0, 0, (float)nextIcon.width, (float)nextIcon.height},
                       {SX(iconX), SY(iconY), iconSize * g_uiScale, iconSize * g_uiScale},
                       {0, 0}, 0.0f,
                       hover ? COLOR_TOXIC : COLOR_GHOST);
    } else {
        DrawScaledText("►►", btnX + 2, btnY + 4, 12, hover ? COLOR_TOXIC : COLOR_GHOST);
    }
    if (clicked && hover) Next();
    
    btnX += btnSize + spacing;
    
    // ---- REPEAT BUTTON ----
    hover = RefRectHover(btnX, btnY, btnSize, btnSize, refMouse);
    DrawScaledRect(btnX, btnY, btnSize, btnSize, 
                   hover ? Color{40, 45, 65, 200} : Color{20, 22, 35, 180});
    DrawScaledRectLines(btnX, btnY, btnSize, btnSize, 
                        m_playlist.isRepeating ? COLOR_TOXIC : Color{30, 35, 50, 100});
    
    Texture2D repeatIcon = GetIcon("repeat");
    if (repeatIcon.id != 0) {
        float iconSize = 18.0f;
        float iconX = btnX + (btnSize - iconSize) / 2.0f;
        float iconY = btnY + (btnSize - iconSize) / 2.0f;
        DrawTexturePro(repeatIcon,
                       {0, 0, (float)repeatIcon.width, (float)repeatIcon.height},
                       {SX(iconX), SY(iconY), iconSize * g_uiScale, iconSize * g_uiScale},
                       {0, 0}, 0.0f,
                       m_playlist.isRepeating ? COLOR_TOXIC : COLOR_GHOST);
    } else {
        DrawScaledText("↻", btnX + 4, btnY + 4, 14, 
                       m_playlist.isRepeating ? COLOR_TOXIC : COLOR_GHOST);
    }
    if (clicked && hover) ToggleRepeat();
}

void MusicPlayer::DrawProgressBar(float x, float y, float width) {
    Vector2 refMouse = GetRefMousePos();
    
    float barH = 4.0f;
    float handleR = 6.0f;
    
    // Background
    DrawScaledRect(x, y, width, barH, Color{12, 15, 20, 255});
    DrawScaledRectLines(x, y, width, barH, Color{30, 35, 50, 100});
    
    // Progress fill
    float fillW = width * m_playlist.progress;
    if (fillW < 2.0f) fillW = 2.0f;
    DrawScaledRect(x, y, fillW, barH, COLOR_TOXIC);
    
    // Glow at progress tip
    if (fillW > 5.0f) {
        DrawScaledRect(x + fillW - 4.0f, y - 2, 4, barH + 4, 
                       {40, 240, 100, 60});
    }
    
    // Handle (clickable)
    float handleX = x + fillW;
    bool hover = RefRectHover(handleX - handleR, y - handleR, 
                              handleR * 2, handleR * 2 + barH, refMouse);
    Color handleCol = hover ? COLOR_TOXIC : COLOR_CYAN;
    DrawScaledRect(handleX - handleR/2, y - handleR/2 + 1, 
                   handleR, handleR, hover ? Color{40, 45, 65, 200} : Color{20, 22, 35, 180});
    DrawScaledRectLines(handleX - handleR/2, y - handleR/2 + 1, 
                        handleR, handleR, handleCol);
    
    // Click to seek
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Vector2 refMouse = GetRefMousePos();
        if (RefRectHover(x, y - 10, width, 20, refMouse) || 
            RefRectHover(handleX - handleR, y - handleR, handleR * 2, handleR * 2 + barH, refMouse)) {
            float newProgress = (refMouse.x - x) / width;
            if (newProgress < 0.0f) newProgress = 0.0f;
            if (newProgress > 1.0f) newProgress = 1.0f;
            Seek(newProgress);
        }
    }
    
    // Elapsed time
    char timeStr[32];
    int minutes = (int)m_playlist.elapsedTime / 60;
    int seconds = (int)m_playlist.elapsedTime % 60;
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", minutes, seconds);
    DrawScaledText(timeStr, x, y - 14, 8, COLOR_GHOST);
    
    // Total time (right side)
    const MusicTrack* current = GetCurrentTrack();
    if (current && current->duration > 0.0f) {
        int totalMins = (int)current->duration / 60;
        int totalSecs = (int)current->duration % 60;
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d", totalMins, totalSecs);
        float timeW = MeasureScaledTextWidth(timeStr, 8);
        DrawScaledText(timeStr, x + width - timeW, y - 14, 8, COLOR_GHOST);
    }
}

void MusicPlayer::DrawVisualizer(float x, float y, float width, float height) {
    float barW = width / 64.0f;
    float maxH = height - 4.0f;
    
    for (int i = 0; i < 64; i++) {
        float sample = m_visualizerSamples[i];
        float barH = sample * maxH;
        if (barH < 1.0f) barH = 1.0f;
        
        // Color gradient based on height and position
        Color col;
        if (sample > 0.7f) {
            col = COLOR_BLOOD;
        } else if (sample > 0.4f) {
            col = COLOR_AMBER;
        } else if (sample > 0.2f) {
            col = COLOR_TOXIC;
        } else {
            col = COLOR_CYAN;
        }
        col.a = (unsigned char)(150 + sample * 100);
        
        DrawScaledRect(x + i * barW, y + height - barH - 2, 
                       barW - 1, barH, col);
    }
}

void MusicPlayer::DrawTrackList(float x, float y, float width, float height) {
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    // Section header
    DrawScaledText("PLAYLIST", x + 4, y, 9, COLOR_AMBER);
    DrawScaledLine(x + 4, y + 14, x + width - 4, y + 14, Color{30, 35, 45, 100});
    
    float listY = y + 18;
    float trackH = 22.0f;
    int visible = (int)((height - 20) / trackH);
    if (visible > (int)m_playlist.tracks.size()) {
        visible = (int)m_playlist.tracks.size();
    }
    
    // Scroll offset
    static int scrollOffset = 0;
    
    // Mouse wheel on track list
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
        
        // Background
        Color bg;
        if (isCurrent) {
            bg = Color{40, 50, 80, 100};
        } else if (hover) {
            bg = Color{30, 35, 55, 150};
        } else {
            bg = Color{0, 0, 0, 0};
        }
        DrawScaledRect(x + 2, trackY, width - 4, trackH - 2, bg);
        
        // Playing indicator
        if (isCurrent && m_playlist.isPlaying) {
            DrawScaledText("▶", x + 6, trackY + 4, 10, COLOR_TOXIC);
        } else if (isCurrent) {
            DrawScaledText("▌", x + 6, trackY + 4, 10, COLOR_AMBER);
        } else {
            DrawScaledText(" ", x + 6, trackY + 4, 10, COLOR_GHOST);
        }
        
        // Track number/indicator
        char numStr[8];
        snprintf(numStr, sizeof(numStr), "%02d", i + 1);
        DrawScaledText(numStr, x + 20, trackY + 4, 8, 
                       isCurrent ? COLOR_TOXIC : Color{80, 90, 110, 150});
        
        // Track title
        Color titleCol = isCurrent ? track.accentColor : COLOR_GHOST;
        DrawScaledText(track.title.c_str(), x + 48, trackY + 3, 10, titleCol);
        
        // Artist (on right)
        float artistW = MeasureScaledTextWidth(track.artist.c_str(), 8);
        DrawScaledText(track.artist.c_str(), x + width - artistW - 8, 
                       trackY + 4, 8, Color{80, 90, 110, 150});
        
        // Click to play track
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