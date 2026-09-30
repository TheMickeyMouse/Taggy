#include "Track.h"

#include "TagReader.h"
#include "Utils/Iter/Zip.h"

Track Track::Load(CStr filename) {
    Track track;
    TagReader reader { filename.Data() };
    auto metadata = reader.ReadV2();
    if (!metadata) {
        Debug::QError$("failed to read music file!");
        return track;
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
    return track;
}
