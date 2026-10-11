#include "Track.h"

#include "App.h"
#include "TagReader.h"
#include "Utils/Iter/Zip.h"

Sound Sound::Allocate(u64 audioBytes) {
    return {
        Memory::AllocateRaw(audioBytes + sizeof(Header)),
        audioBytes,
    };
}

Sound::Header* Sound::GetHeader() const {
    return (Header*)data;
}

Sound::Sound(void* data, u64 size) : data(data), audioSize(size) {}

ma_sound* Sound::GetAudio() const  {
    return &GetHeader()->sound;
}

ma_decoder* Sound::GetDecoder() const {
    return &GetHeader()->decoder;
}

Bytes Sound::GetAudioBytes() const {
    return Bytes::Slice((const byte*)data + sizeof(Header), audioSize);
}

BytesMut Sound::GetAudioBytes() {
    return BytesMut::Slice((byte*)data + sizeof(Header), audioSize);
}

Sound::operator bool() const {
    return data;
}

Sound::Sound(Sound&& other) noexcept {
    data = other.data; other.data = nullptr;
    audioSize = other.audioSize;
}

Sound& Sound::operator=(Sound&& other) noexcept {
    this->~Sound();
    new (this) Sound(std::move(other));
    return *this;
}

Sound::~Sound() {
    if (data) ma_sound_uninit(GetAudio());
}

bool Track::Load(Out<Track&> track, Path filename, bool lazyLoad) {
    [[maybe_unused]] Debug::Timer timer { "mp3 reading", Debug::Logger::InternalLog };

    track.filePath = filename;

    TagReader reader { filename };

    auto metadata = reader.ReadV2();
    if (!metadata) {
        Debug::QWarn$("couldn't read {}; skipping...", filename.string().c_str());
        return false;
    }

    for (auto& [tag, field] : Iter::Zip(metadata->tags.Iter(), metadata->tagFields.IterMut())) {
        using namespace ID3v2;
        switch (tag.id) {
            case TagID::APIC: {
                [[maybe_unused]] Debug::Timer timerPng { "png loading", Debug::Logger::InternalLog };
                track.cover = Texture2D::LoadPNGBytes(field.As<PictureField>()->pictureData, { .pixelated = false }, false);
                break;
            }
            case TagID::TIT2: {
                track.title = std::move(field.As<TextField>()->value);
                break;
            }
            case TagID::TPE1: {
                track.artist = std::move(field.As<TextField>()->value);
                break;
            }
            default: continue;
        }
    }

    track.audioBeginPos = metadata->mpegBegin;
    track.file = std::move(reader.file);

    if (!lazyLoad) track.LoadSoundIntoMemory();
    return true;
}

void Track::LoadSoundIntoMemory() {
    if (sound) return;

    {
        file.seekg(0, std::ios::end);
        isize len = file.tellg();
        file.seekg(audioBeginPos, std::ios::beg);
        len -= audioBeginPos;

        sound = Sound::Allocate(len);
        file.read((char*)sound.GetAudioBytes().Data(), len);
    }

    const Bytes trackAudioBytes = sound.GetAudioBytes();
    ma_decoder* decoder = sound.GetDecoder();
    ma_sound* audio = sound.GetAudio();

    ma_decoder_init_memory(trackAudioBytes.Data(), trackAudioBytes.Length(), nullptr, decoder);
    ma_sound_init_from_data_source(App::GetAudioEngine(), decoder, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, nullptr, audio);

    ma_sound_get_length_in_seconds(audio, &durationSec);
}

void Track::Start() {
    LoadSoundIntoMemory();

    isPlaying = true;
    ma_sound* audio = sound.GetAudio();
    ma_sound_reset_stop_time_and_fade(audio);
    ma_sound_set_fade_in_milliseconds(audio, 0.0f, 1.0f, FADE_TIME_MS);
    ma_sound_start(audio);
}

void Track::Stop() {
    LoadSoundIntoMemory();

    isPlaying = false;
    ma_sound* audio = sound.GetAudio();
    ma_sound_stop_with_fade_in_milliseconds(audio, FADE_TIME_MS);
}

void Track::StartImmediate() {
    LoadSoundIntoMemory();

    isPlaying = true;
    ma_sound* audio = sound.GetAudio();
    ma_sound_start(audio);
}

void Track::StopImmediate() {
    LoadSoundIntoMemory();

    isPlaying = false;
    ma_sound* audio = sound.GetAudio();
    ma_sound_stop(audio);
}

void Track::Toggle() {
    isPlaying ? Stop() : Start();
}

void Track::SeekTo(float seconds) {
    ma_sound* audio = sound.GetAudio();
    ma_sound_seek_to_second(audio, seconds);
}

float Track::GetPosition() const {
    float position;
    ma_sound* audio = sound.GetAudio();
    ma_sound_get_cursor_in_seconds(audio, &position);
    return position;
}
