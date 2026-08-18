#include <iostream>

#include "TagReader.h"
#include "Utils/Debug/Logger.h"

using namespace Quasi;

int main() {
    Debug::QInfo$("Hello, World!");

    TagReader tag = { "Aurora - Creo.mp3" };
    Option id3v1 = tag.ReadV1();
    if (id3v1) {
        Debug::QInfo$("Found tag:");
        std::cout << nlohmann::json(*id3v1).dump(2);
    } else {
        Debug::QInfo$("Tag not found!");
    }

    return 0;
}