#pragma once
#include "miniaudio.h"
#include "UIRect.h"
#include <filesystem>

using Path = std::filesystem::path;

struct Track {
    Path filePath;
    u32 audioBeginPos = 0;

    Texture2D cover;
    String title, artist;

    Box<ma_sound> sound;

    float durationSec = 0.0f;
    bool isPlaying = false;
    bool loaded = false;

    static constexpr u32 FADE_TIME_MS = 200;

    Track() = default;

    static Track UnloadedFromFile(Path fname);
    static bool Load(Out<Track&> track, bool lazyLoad);

    void LoadSoundIntoMemory();

    void Start();
    void Stop();
    void StartImmediate();
    void StopImmediate();
    void Toggle();

    void SeekTo(float seconds);
    float GetPosition() const;
};