#include "Track.h"

#include "TagReader.h"
#include "Utils/Iter/Zip.h"

Track::~Track() {
    ma_sound_uninit(&audio);
}

bool Track::Load(Out<Track&> track, CStr filename, ma_engine* engine) {
    TagReader reader { filename.Data() };
    auto metadata = reader.ReadV2();
    if (!metadata) {
        Debug::QError$("failed to read music file!");
        return false;
    }

    for (auto& [tag, field] : Iter::Zip(metadata->tags.Iter(), metadata->tagFields.IterMut())) {
        using namespace ID3v2;
        switch (tag.id) {
            case TagID::APIC: {
                track.cover = Texture2D::LoadPNGBytes(field.As<PictureField>()->pictureData, { .pixelated = false });
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

    // grab the mpeg data into memory
    {
        auto& f = reader.file;
        f.seekg(0, std::ios::end);
        const isize len = f.tellg();
        f.seekg(metadata->mpegBegin, std::ios::beg);

        track.rawAudioBytes = ArrayBox<char>::AllocateUninit(len);
        f.read(track.rawAudioBytes.Data(), len);
    }

    ma_decoder_init_memory(track.rawAudioBytes.Data(), track.rawAudioBytes.Length(), nullptr, &track.audioDecoder);
    ma_sound_init_from_data_source(engine, &track.audioDecoder, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, nullptr, &track.audio);

    ma_sound_get_length_in_seconds(&track.audio, &track.durationSec);

    return true;
}

void Track::Start() {
    isPlaying = true;
    ma_sound_reset_stop_time_and_fade(&audio);
    ma_sound_set_fade_in_milliseconds(&audio, 0.0f, 1.0f, FADE_TIME_MS);
    ma_sound_start(&audio);
}

void Track::Stop() {
    isPlaying = false;
    ma_sound_stop_with_fade_in_milliseconds(&audio, FADE_TIME_MS);
}

void Track::StartImmediate() {
    isPlaying = true;
    ma_sound_start(&audio);
}

void Track::StopImmediate() {
    isPlaying = false;
    ma_sound_stop(&audio);
}

void Track::Toggle() {
    isPlaying ? Stop() : Start();
}

void Track::SeekTo(float seconds) {
    ma_sound_seek_to_second(&audio, seconds);
}

float Track::GetPosition() const {
    float position;
    ma_sound_get_cursor_in_seconds(&audio, &position);
    return position;
}
