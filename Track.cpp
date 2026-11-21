#include "Track.h"

#include "App.h"
#include "TagReader.h"
#include "Utils/Text/UTF.h"

Track Track::UnloadedFromFile(Path fname) {
    Track t;
    t.filePath = std::move(fname);
    return t;
}

namespace Fs = std::filesystem;

bool Track::Load(Out<Track&> track, bool lazyLoad) {
    track.loaded = true;

    auto& cache = App::GetCache();
    if (const auto c = cache.TryFetchTrack(track.filePath); c && !c->CheckOutdated()) {
        track.title       = c->name;
        track.artist      = c->artist;
        track.durationSec = c->durationMs.MapOr([] (int x) { return (float)x / 1000.0f; }, f32s::NAN);

        if (!c->coverFilename) goto requiresRecache;
        track.cover = Texture2D::New(c->FetchCover());
    } else {
        requiresRecache:

        TagReader reader { track.filePath };

        String cachedCoverImgPath;
        reader.SetCallback([&] (ID3v2::TagID tag, u32, ID3v2::Field field) {
            using namespace ID3v2;
            switch (tag) {
                case TagID::APIC: {
                    const Bytes imgBytes = field.As<PictureField>()->pictureData;
                    cachedCoverImgPath = App::GetCache().AddCover(imgBytes);

                    [[maybe_unused]] Debug::Timer timerPng { "png loading", Debug ::Logger::InternalLog };
                    track.cover = Texture2D::LoadPNGBytes(imgBytes, {}, false);
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
                case TagID::TALB: {
                    track.album = std::move(field.As<TextField>()->value);
                    break;
                }
                default:;
            }
        });

        auto metadata = reader.ReadV2();
        if (!metadata) {
            Debug::QWarn$("couldn't read {}; skipping...", Text::WStr(track.filePath.c_str()));
            return false;
        }

        if (track.title.IsEmpty())
            track.title = Text::Utf16To8(track.filePath.filename().c_str()).Assert("couldn't parse string!");

        cache.AddTrack(track, cachedCoverImgPath);

        reader.file.close();
    }

    if (!lazyLoad) {
        track.LoadSoundIntoMemory();
    }

    return true;
}

void Track::LoadSoundIntoMemory() {
    if (sound) return;

    sound = Box<ma_sound>::NewUninit();

    const ma_result result = ma_sound_init_from_file_w(
        App::GetAudioEngine(),
        filePath.c_str(),
        0,
        nullptr,
        nullptr,
        &*sound
    );

    if (f32s::IsNaN(durationSec)) {
        ma_sound_get_length_in_seconds(&*sound, &durationSec);
        if (auto cache = App::GetCache().TryModifyTrack(filePath))
            cache->durationMs = (int)(durationSec * 1000);
    }

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
