#include <iostream>

#include "TagReader.h"
#include "Utils/Array.h"
#include "Utils/Debug/Logger.h"
#include "Utils/Iter/Zip.h"

using namespace Quasi;

int main() {
    Debug::QInfo$("Hello, World!");

    constexpr const char* FILE_PATH = "Cheap Thrills - Sia_lyrics.mp3";
    TagReader tagReader = { FILE_PATH };

    if (const auto m = tagReader.ReadV2(); m) {
        const auto& metadata = *m;
        Text::Print("Found tag: ID3v2.{}.{}; {} bytes in total", metadata.tagVersion >> 8, metadata.tagVersion & 0xFF, metadata.size);
        for (const auto& [tag, data] : Iter::Zip(metadata.tags.Iter(), metadata.tagData.Iter())) {
            Text::Print("\n\t{:<16} ({} bytes): ", ID3v2::GetTagRules(tag.id).properName, tag.size);
            PrintPayload(data);

            if (tag.id == ID3v2::TagID::APIC) {
                Text::WriteFileBinary("cover.png", data.As<ID3v2::PictureField>()->pictureData);
            }
        }

        tagReader.file.seekg(0, std::ios::end);
        Text::PrintLn("\nTotal file size: {} bytes", std::filesystem::file_size(FILE_PATH));
        Text::PrintLn("Statistics:\n\tMPEG Version {} Layer {}\n\tBitrate: {}kbps, Sampling rate: {}Hz, Channels: {}",
            (Array {{ "_", "1", "2", "2.5" }})[metadata.mpgVersion],
            (Array {{ "_", "I", "II", "III" }})[metadata.layerVersion],
            metadata.bitrateKbps,
            metadata.samplingRate,
            (int)metadata.channels
        );
    } else {
        Text::PrintLn("Tag not found!");
    }

    return 0;
}