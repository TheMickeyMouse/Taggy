#pragma once
#include "miniaudio.h"
#include "UIRect.h"

struct Track {
    Texture2D cover;
    String title, artist;

    ArrayBox<char> rawAudioBytes;
    ma_decoder audioDecoder;
    ma_sound audio;

    float durationSec;
    bool isPlaying = false;

    static constexpr u32 FADE_TIME_MS = 200;

    ~Track();

    static bool Load(Out<Track&> track, CStr filename, ma_engine* engine);

    void Start();
    void Stop();
    void StartImmediate();
    void StopImmediate();
    void Toggle();

    void SeekTo(float seconds);
    float GetPosition() const;
};