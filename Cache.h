#pragma once
#include "Track.h"
#include "Utils/String.h"
#include "Utils/HashMap.h"

class JsonWriter;

namespace Quasi {
    Bytes GetBytesOfPath(const Path& p);

    // measured in nanoseconds
    using FileTime = std::filesystem::file_time_type;

    template <> struct Hashing::Hasher<Path> {
        Hash operator()(const Path& p) const { return HashBytes(GetBytesOfPath(p)); }
    };

    Array<char, 8> ToHexString(u32 number);
}

namespace std::filesystem {
    void WriteJsonValue(JsonWriter& w, FileTime time);
    void WriteJsonValue(JsonWriter& w, const Path& path);
    void ReadFileTime(FileTime* time, Str& s);
    void ReadPath(Path* path, Str& s);
}

template <class T, Text::FixedString A> using Field = T;

// json structure:
// "[song filename]": {
//     "name":     "[song name]",
//     "artist":   "[artist name]",
//     "album":    "[album name]",
//     "duration":  [duration],
//     "cover":    "[cover file location]",
//     "lastModified": [modified time],
// }
struct TrackCache {
    Path filename;
    Field<String,      "cover">        coverFilename;
    Field<String,      "name">         name;
    Field<String,      "artist">       artist;
    Field<String,      "album">        album;
    Field<Option<int>, "duration">     durationMs;
    Field<FileTime,    "lastModified"> lastModifiedTime;

    void Write(JsonWriter& w) const;
    void Read(Str& input);

    Image FetchCover() const;

    bool CheckOutdated() const;

    Hashing::Hash GetHashCode() const;
    bool operator==(const TrackCache& other) const { return filename == other.filename; }
    bool operator==(const Path& other) const { return filename == other; }
};

class Cache {
public:
    // cache/
    // ├─ covers/
    // │  ├─ [uuid-string].png
    // │  └─ ...
    // ├─ general.json
    // └─ songs.json

    static constexpr const char
        *C_ROOT_PATH    = "cache/",
        *C_COVERS_PATH  = "cache/covers/";

    struct Songs {
        static constexpr const char* FILE = "cache/songs.json";

        HashSet<TrackCache> tracks;
        bool modified = false;

        static void WriteDefault();
        void Write(JsonWriter& w) const;
        void Read(Str input);
    } songs;

    struct General {
        static constexpr const char* FILE = "cache/general.json";

        Field<int, "coverIDcounter"> coverIDcounter = 1;

        static void WriteDefault();
        void Write(JsonWriter& w) const;
        void Read(Str input);
    } general;

    static void RepairDefaultProfile();
    static Cache Read();
    void Write() const;

    String AddCover(Bytes imgBytes);
    OptRef<const TrackCache> TryFetchTrack(const Path& path) const;
    OptRef<TrackCache> TryModifyTrack(const Path& path);
    void AddTrack(const Track& track, String coverPath);
};