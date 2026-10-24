#include "Track.h"

#include "App.h"
#include "TagReader.h"
#include "Utils/Iter/Zip.h"

Track Track::UnloadedFromFile(Path fname) {
    Track t;
    t.filePath = std::move(fname);
    return t;
}

bool Track::Load(Out<Track&> track, bool lazyLoad) {
    [[maybe_unused]] Debug::Timer timer { "mp3 reading", Debug::Logger::InternalLog };
    track.loaded = true;

    TagReader reader { track.filePath };

    auto metadata = reader.ReadV2();
    if (!metadata) {
        Debug::QWarn$("couldn't read {}; skipping...", track.filePath.string().c_str());
        return false;
    }

    for (auto& [tag, field] : Iter::Zip(metadata->tags.Iter(), metadata->tagFields.IterMut())) {
        using namespace ID3v2;
        switch (tag.id) {
            case TagID::APIC: {
                timer.Stop();
                [[maybe_unused]] Debug::Timer timerPng { "png loading", Debug::Logger::InternalLog };
                track.cover = Texture2D::LoadPNGBytes(field.As<PictureField>()->pictureData, { .pixelated = false }, false);
                timer.Start();
                break;
            }
            case TagID::TIT2: {
                track.title = std::move(field.As<TextField>()->value);
                break;
            }
            case TagID::TPE1: {
                track.artist = field.As<TextField>()->value.Replace('/', ", ");
                break;
            }
            default: continue;
        }
    }

    track.audioBeginPos = metadata->mpegBegin;

    if (!lazyLoad) {
        reader.file.close();
        track.LoadSoundIntoMemory();
    }

    return true;
}

void Track::LoadSoundIntoMemory() {
    if (sound) return;

    sound = Box<ma_sound>::NewUninit();

    const auto pathString = filePath.string();
    const ma_result result = ma_sound_init_from_file(
        App::GetAudioEngine(),
        pathString.c_str(),
        0,
        nullptr,
        nullptr,
        &*sound
    );

    ma_sound_get_length_in_seconds(&*sound, &durationSec);

    if (result != MA_SUCCESS) {
        Debug::QError$("mp3 load err code {}", (int)result);
    }
}

void Track::Start() {
    LoadSoundIntoMemory();

    isPlaying = true;
    ma_sound_reset_stop_time_and_fade(&*sound);
    ma_sound_set_fade_in_milliseconds(&*sound, 0.0f, 1.0f, FADE_TIME_MS);
    ma_sound_start(&*sound);
}

void Track::Stop() {
    LoadSoundIntoMemory();

    isPlaying = false;
    ma_sound_stop_with_fade_in_milliseconds(&*sound, FADE_TIME_MS);
}

void Track::StartImmediate() {
    LoadSoundIntoMemory();

    isPlaying = true;
    ma_sound_start(&*sound);
}

void Track::StopImmediate() {
    LoadSoundIntoMemory();

    isPlaying = false;
    ma_sound_stop(&*sound);
}

void Track::Toggle() {
    isPlaying ? Stop() : Start();
}

void Track::SeekTo(float seconds) {
    ma_sound_seek_to_second(&*sound, seconds);
}

float Track::GetPosition() const {
    float position;
    ma_sound_get_cursor_in_seconds(&*sound, &position);
    return position;
}
