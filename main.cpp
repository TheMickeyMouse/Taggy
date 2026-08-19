#include <iostream>

#include "TagReader.h"
#include "Utils/Debug/Logger.h"

using namespace Quasi;

int main() {
    Debug::QInfo$("Hello, World!");

    TagReader tag = { "Aurora - Creo.mp3" };

    if (const auto m = tag.ReadV2(); m) {
        const auto& metadata = *m;
        Debug::QInfo$("Found tag: ID3v2.{}.{}; {} bytes in total", metadata.version >> 8, metadata.version & 0xFF, metadata.size);
        for (const auto& t : metadata.tags) {
            Debug::QInfo$("\t{:<15} : size {}", ID3v2::GetTagIDJsonName(t.id), t.size);
        }
    } else {
        Debug::QInfo$("Tag not found!");
    }

    return 0;
}