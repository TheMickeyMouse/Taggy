#include "TagReader.h"

TagReader::TagReader(const char* filename)
    : file(filename, std::ios::in | std::ios::binary) {}

Option<ID3v1> TagReader::ReadV1() {
    file.seekg(-128, std::ios_base::end);
    ID3v1 tag = {};
    file.read((char*)&tag._header, sizeof(ID3v1));

    // header MUST BE "TAG"
    if ((Memory::ReadU32Big(tag._header) >> 8) != "TAG"_u32) {
        return nullptr;
    } else return tag;
}
