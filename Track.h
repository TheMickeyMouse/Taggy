#pragma once
#include "miniaudio.h"
#include "UIRect.h"
#include <filesystem>
#include <fstream>

using Path = std::filesystem::path;

struct Sound {
    struct Header {
        ma_sound sound;
        ma_decoder decoder;
        /* byte rawAudioBytes[...] */
    };
    void* data = nullptr;
    u64 audioSize = 0;

    static Sound Allocate(u64 audioBytes);
private:
    Header* GetHeader() const;

    Sound(void* data, u64 size);
public:
    ma_sound* GetAudio() const;
    ma_decoder* GetDecoder() const;
    Bytes GetAudioBytes() const;
    BytesMut GetAudioBytes();

    explicit operator bool() const;

    Sound() = default;
    Sound(const Sound&) = delete;
    Sound& operator=(const Sound&) = delete;
    Sound(Sound&&) noexcept;
    Sound& operator=(Sound&&) noexcept;
    ~Sound();
};

struct Track {
    Path filePath;
    std::ifstream file;
    u32 audioBeginPos;

    Texture2D cover;
    String title, artist;

    Sound sound = {};

    float durationSec;
    bool isPlaying = false;

    static constexpr u32 FADE_TIME_MS = 200;

    Track() = default;

    static bool Load(Out<Track&> track, Path filename, bool lazyLoad);

    void LoadSoundIntoMemory();

    void Start();
    void Stop();
    void StartImmediate();
    void StopImmediate();
    void Toggle();

    void SeekTo(float seconds);
    float GetPosition() const;
};