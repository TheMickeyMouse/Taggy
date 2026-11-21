#include "Cache.h"

#include <fstream>

#include "Json.h"
#include "Track.h"
#include "Utils/Debug/Timer.h"
#include "Utils/Text/UTF.h"

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

void std::filesystem::WriteJsonValue(JsonWriter& w, FileTime time) {
    w.Number(time.time_since_epoch().count());
}

void std::filesystem::WriteJsonValue(JsonWriter& w, const Path& path) {
    w.StrLit(Text::Utf16To8(path.c_str()).Assert("bad utf16 path!"));
}

void std::filesystem::ReadFileTime(FileTime* time, Str& s) {
    i64 sinceEpoch = 0;
    JsonReader::NumDecoder(&sinceEpoch, s);
    *time = FileTime(FileTime::duration(sinceEpoch));
}

void std::filesystem::ReadPath(Path* path, Str& s) {
    String x;
    JsonReader::StrDecoder(&x, s);
    *path = Path(std::string_view(x.Data(), x.Length()));
}

namespace Fs = std::filesystem;

void TrackCache::Write(JsonWriter& w) const {
    w.BeginObject();
        w.Field("file",         filename),
        w.Field("name",         name),
        w.Field("artist",       artist),
        w.Field("album",        album),
        w.Field("duration",     durationMs),
        w.Field("cover",        coverFilename),
        w.Field("lastModified", lastModifiedTime),
    w.EndObject();
}

void TrackCache::Read(Str& input) {
    JsonReader r = JsonReader::ForObject();
    r.Accept   ("file",         &filename, &std::filesystem::ReadPath);
    r.AcceptStr("name",         &name);
    r.AcceptStr("artist",       &artist);
    r.AcceptStr("album",        &album);
    r.Accept   ("duration",     &durationMs, JsonReader::Opt<int, &JsonReader::NumDecoder>);
    r.AcceptStr("cover",        &coverFilename);
    r.Accept   ("lastModified", &lastModifiedTime, std::filesystem::ReadFileTime);

    r.Process(input);
}

Image TrackCache::FetchCover() const {
    return Image::LoadPNG(Text::Format("{}{}", Cache::C_COVERS_PATH, coverFilename).IntoCStr(), false);
}

bool TrackCache::CheckOutdated() const {
    return Fs::last_write_time(filename) > lastModifiedTime;
}

Hashing::Hash TrackCache::GetHashCode() const {
    return Hashing::HashObject(filename);
}

void Cache::Songs::WriteDefault() {
    if (Fs::exists(FILE)) return;
    Text::WriteFile(FILE, "[]");
}

void Cache::Songs::Write(JsonWriter& w) const {
    w.BeginArray();
    for (auto& song : tracks) {
        song.Write(w);
    }
    w.EndArray();
}

void Cache::Songs::Read(Str input) {
    JsonReader r = JsonReader::ForArray(
        &tracks, +[] (HashSet<TrackCache>* c, Str& s) {
            TrackCache t;
            t.Read(s);
            c->Insert(t);
        }
    );
    r.Process(input);
}

void Cache::General::WriteDefault() {
    if (Fs::exists(FILE)) return;
    Text::WriteFile(FILE, "{ \"coverIDcounter\": 1 }");
}

void Cache::General::Write(JsonWriter& w) const {
    w.BeginObject();
    w.Field("coverIDcounter", coverIDcounter);
    w.EndObject();
}

void Cache::General::Read(Str input) {
    JsonReader r = JsonReader::ForObject();
    r.AcceptNum("coverIDcounter", &coverIDcounter);
    r.Process(input);
}

void Cache::RepairDefaultProfile() {
    // cache/
    // ├─ covers/
    // │  ├─ [uuid-string].png
    // │  └─ ...
    // ├─ general.json
    // └─ songs.json
    Fs::create_directory(C_ROOT_PATH);
    Fs::create_directory(C_COVERS_PATH);

    Songs::WriteDefault();
    General::WriteDefault();
}

Cache Cache::Read() {
    [[maybe_unused]] Debug::Timer timer { "loading cache metadata", Debug::Logger::GetInternalLog() };

    const Path p = C_ROOT_PATH;
    if (!Fs::exists(p) || !Fs::is_directory(p)) {
        RepairDefaultProfile();
    }

    Cache cache;
    if (const auto s = Text::ReadFileBinary(Songs::FILE))   cache.songs.Read(*s);
    if (const auto s = Text::ReadFileBinary(General::FILE)) cache.general.Read(*s);
    return cache;
}

void Cache::Write() const {
    const Path p = C_ROOT_PATH;

    Fs::create_directory(p);

    if (songs.modified) { // songs.json
        JsonWriter w;
        songs.Write(w);
        Text::WriteFile(Songs::FILE, w.Result());
    }

    { // general.json
        JsonWriter w;
        general.Write(w);
        Text::WriteFile(General::FILE, w.Result());
    }
}

String Cache::AddCover(Bytes imgBytes) {
    String cachedName = Text::Format("{}.png", ToHexString(general.coverIDcounter++).AsStr());
    Text::WriteFileBinary(
        Text::Format("{}{}", C_COVERS_PATH, cachedName).IntoCStr(),
        imgBytes
    );
    return cachedName;
}

OptRef<const TrackCache> Cache::TryFetchTrack(const Path& path) const {
    return songs.tracks.Get(path);
}

OptRef<TrackCache> Cache::TryModifyTrack(const Path& path) {
    auto t = songs.tracks.Get(path);
    if (!t) return nullptr;
    songs.modified = true;
    return t;
}

void Cache::AddTrack(const Track& track, String coverPath) {
    songs.tracks.Insert({
        .filename         = track.filePath,
        .coverFilename    = std::move(coverPath),
        .name             = track.title,
        .artist           = track.artist,
        .album            = track.album,
        .durationMs       = (int)(track.durationSec * 1000.0f),
        .lastModifiedTime = FileTime::clock::now(),
    });
    songs.modified = true;
}
