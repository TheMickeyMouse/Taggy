#include "Cache.h"

#include <fstream>

#include "Track.h"
#include "Utils/Debug/Timer.h"

Str Quasi::LegConvert(std::string_view s) {
    return Str::Slice(s.data(), s.size());
}

std::string_view Quasi::LegConvert(Str s) {
    return { s.Data(), s.Length() };
}

void Quasi::to_json(Nh::json& j, Str s) {
    j = LegConvert(s);
}

void Quasi::from_json(const Nh::json& j, String& s) {
    s = LegConvert(j.get<std::string_view>());
}

Bytes Quasi::GetBytesOfPath(const Path& p) {
    const auto& nativeString = p.native();
    return Spans::Slice(nativeString.c_str(), nativeString.size()).AsBytes();
}

Array<char, 8> Quasi::ToHexString(u32 number) {
    Array<char, 8> hex;

    u64 x = number;
    // x = 00 00 00 00 ab cd ef gh
    x |= x << 16;
    // x = 00 00 ab cd ?? ?? ef gh
    x = ((x << 8) & 0x00FF0000'00FF0000) | // 00 ab 00 00 00 ef 00 00
         (x       & 0x000000FF'000000FF);  // 00 00 00 cd 00 00 00 gh
    // 00 ab 00 cd 00 ef 00 gh
    x |= x << 4;
    // 0a ?b 0c ?d 0e ?f 0g ?h
    x &= 0x0F0F0F0F'0F0F0F0F;

    // if each hex digit of x is represented by a letter (aka >= 10)
    x += 0x06060606'06060606;
    x += ((x >> 4) & 0x01010101'01010101) * 7;
    x += 0x2A2A2A2A'2A2A2A2A;

    Memory::WriteU64Big(x, hex.Data());
    return hex;
}

void std::filesystem::to_json(Nh::json& j, FileTime time) {
    j = time.time_since_epoch().count();
}

void std::filesystem::from_json(const Nh::json& j, FileTime& time) {
    time = FileTime(Debug::Nanosecond(j.get<i64>()));
}

namespace Fs = std::filesystem;

void SongCache::ToJson(Nh::json& j) const {
    j = {
        { "file",         filename },
        { "name",         name },
        { "artist",       artist },
        { "album",        album },
        { "duration",     durationSec },
        { "cover",        coverFilename },
        { "lastModified", lastModifiedTime },
    };
}

void SongCache::FromJson(const Nh::json& j) {
    j.at("file")        .get_to(filename);
    j.at("name")        .get_to(name);
    j.at("artist")      .get_to(artist);
    j.at("album")       .get_to(album);
    if (const auto d = j.at("duration"); d.is_number())
        durationSec = d.get<float>();
    else durationSec = f32s::NAN;
    j.at("cover")       .get_to(coverFilename);
    j.at("lastModified").get_to(lastModifiedTime);

    if (!Fs::exists(Path(Cache::C_COVERS_PATH) / coverFilename.Data())) {
        coverFilename = "";
    } else coverFilename.AddNullTerm();
}

void SongCache::Record(const Track& track) {
    filename         = track.filePath;
    name             = track.title;
    artist           = track.artist;
    album            = track.album;
    durationSec      = track.durationSec;
    lastModifiedTime = FileTime::clock::now();
}

Image SongCache::FetchCover() const {
    return Image::LoadPNG(Text::Format("{}{}", Cache::C_COVERS_PATH, coverFilename).IntoCStr());
}

Hashing::Hash SongCache::GetHashCode() const {
    return Hashing::HashObject(filename);
}

void Cache::RepairDefaultProfile() {
    // cache/
    // ├─ covers/
    // │  ├─ [uuid-string].png
    // │  └─ ...
    // ├─ general.json
    // └─ songs.json
    const Path root = C_ROOT_PATH;
    Fs::create_directory(root);
    Fs::create_directory(C_COVERS_PATH);

    if (const Path p = root / C_GENERAL_FILE; !Fs::exists(p)) {
        std::ofstream emptyFile = p;
        emptyFile.write("{ \"coverIDcounter\": 1 }", Q_STRLIT_LEN("{ \"coverIDcounter\": 1 }"));
    }

    if (const Path p = root / C_SONGS_FILE; !Fs::exists(p)) {
        std::ofstream emptyFile = p;
        emptyFile.write("[]", 2);
    }
}

Cache Cache::Load(CStr path) {
    [[maybe_unused]] Debug::Timer timer { "loading cache metadata", Debug::Logger::GetInternalLog() };

    const Path p = path.Data();
    if (!Fs::exists(p) || !Fs::is_directory(p)) {
        RepairDefaultProfile();
    }

    Cache cache;
    cache.LoadSongs(p);
    cache.LoadGeneral(p);
    return cache;
}

void Cache::LoadSongs(const Path& root) {
    const Path p = root / C_SONGS_FILE;
    Nh::json j = Nh::json::parse(std::ifstream(p));

    for (auto& entry : j) {
        songs.Insert(entry.get<SongCache>());
    }
}

void Cache::LoadGeneral(const Path& root) {
    const Path p = root / C_GENERAL_FILE;
    Nh::json j = Nh::json::parse(std::ifstream(p));

    coverIDcounter = j["coverIDcounter"];
}

void Cache::AddCoverToCache(Bytes imgBytes, SongCache& songEntry) {
    songEntry.coverFilename = Text::Format("{}.png", ToHexString(coverIDcounter++).AsStr());
    Text::WriteFileBinary(
        Text::Format("{}{}", C_COVERS_PATH, songEntry.coverFilename).IntoCStr(),
        imgBytes
    );
}

void Cache::WriteSongsTo(Nh::json& j) const {
    for (auto& song : songs) {
        j.push_back(song);
    }
}

void Cache::WriteGeneralTo(Nh::json& j) const {
    j["coverIDcounter"] = coverIDcounter;
}

void Cache::Save(CStr path) const {
    if (!modified) return;

    const Path p = path.Data();

    Fs::create_directory(p);

    {
        std::ofstream output { p / C_SONGS_FILE };
        Nh::json jSongs;
        WriteSongsTo(jSongs);
        output << jSongs;
    }

    {
        std::ofstream output { p / C_GENERAL_FILE };
        Nh::json jGeneral;
        WriteGeneralTo(jGeneral);
        output << jGeneral;
    }
}
