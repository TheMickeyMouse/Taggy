#include <iostream>

#include "TagReader.h"
#include "Utils/Debug/Logger.h"
#include "Utils/Iter/Zip.h"

using namespace Quasi;

int main() {
    Debug::QInfo$("Hello, World!");

    TagReader tagReader = { "Cheap Thrills - Sia.mp3" };

    if (const auto m = tagReader.ReadV2(); m) {
        const auto& metadata = *m;
        Text::Print("Found tag: ID3v2.{}.{}; {} bytes in total", metadata.version >> 8, metadata.version & 0xFF, metadata.size);
        for (const auto& [tag, data] : Iter::Zip(metadata.tags.Iter(), metadata.tagData.Iter())) {
            Text::Print("\n\t{:<16} ({} bytes): ", ID3v2::GetTagRules(tag.id).properName, tag.size);
            PrintPayload(data);
        }
    } else {
        Text::PrintLn("Tag not found!");
    }

    return 0;
}