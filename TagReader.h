#pragma once
#include <fstream>

#include "Utils/Numeric.h"
#include "Utils/Option.h"
#include "nlohmann-json.h"

using namespace Quasi;

struct alignas(void*) ID3v1 {
    // according to https://wiki.hydrogenaudio.org/index.php?title=ID3v1
    // | Song title  | 30 characters |
    // | Artist      | 30 characters |
    // | Album       | 30 characters |
    // | Year        | 4 characters  |
    // | Comment     | 30 characters |
    // | Genre       | 1 byte        |
    char _header[3]; // initial padding; 3 bytes; always TAG so not used.
    char title[30];
    char artist[30];
    char album[30];
    char year[4];
    char comment[30];
    byte genre;
    static char* GENRE_LIST[80];
};
static_assert(sizeof(ID3v1) == 128 && "ID3v1 must be 128 bytes (according to spec)!");
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ID3v1, title, artist, album, year, comment, genre)

class TagReader {
    std::ifstream file;
public:
    TagReader(const char* filename);
    Option<ID3v1> ReadV1();
};