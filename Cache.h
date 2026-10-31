#pragma once
#include "nlohmann-json.h"
#include "Track.h"
#include "Utils/String.h"
#include "Utils/CStr.h"
#include "Utils/HashMap.h"
#include "Utils/Vec.h"

using namespace Quasi;
using namespace Math;
namespace Nh = nlohmann;

namespace Quasi {
    // legacy conversions
    Str LegConvert(std::string_view s);
    std::string_view LegConvert(Str s);
    template <class T> Vec<T> LegConvert(std::vector<T> xs) {
        Vec<T> v = Vec<T>::Compose(xs.data(), xs.size(), xs.capacity());
        Memory::ForgetValue(xs);
        return v;
    }

    void to_json(Nh::json& j, Str s);
    void from_json(const Nh::json& j, String& s);
    template <class T> void to_json(Nh::json& j, Span<T> xs) {
        j = std::span<T> { xs.Data(), xs.Length() };
    }
    template <class T> void from_json(const Nh::json& j, Vec<T>& xs) {
        xs = LegConvert(j.get<std::vector<T>>());
    }

    Bytes GetBytesOfPath(const Path& p);

    // measured in nanoseconds
    using FileTime = std::filesystem::file_time_type;

    template <> struct Hashing::Hasher<Path> {
        Hash operator()(const Path& p) const { return HashBytes(GetBytesOfPath(p)); }
    };

    Array<char, 8> ToHexString(u32 number);
}


namespace std::filesystem {
    void to_json  (Nh::json& j, FileTime time);
    void from_json(const Nh::json& j, FileTime& time);
}

// json structure:
// "[song filename]": {
//     "name":       "[song name]",
//     "artist":     "[artist name]",
//     "album":      "[album name]",
//     "durationMs":  [duration],
//     "cover":      "[cover file location]",
//     "lastModified": [modified time],
// }
struct SongCache {
    Path filename;
    String coverFilename;
    String name, artist, album;
    float durationSec;
    FileTime lastModifiedTime;

    void ToJson(Nh::json& j) const;
    void FromJson(const Nh::json& j);

    void Record(const Track& track);
    Image FetchCover() const;

    Hashing::Hash GetHashCode() const;
    bool operator==(const SongCache& other) const { return filename == other.filename; }
    bool operator==(const Path& other) const { return filename == other; }
};

inline void to_json  (Nh::json& j, const SongCache& songCache) { songCache.ToJson(j); }
inline void from_json(const Nh::json& j, SongCache& songCache) { songCache.FromJson(j); }

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
        *C_COVERS_PATH  = "cache/covers/",
        *C_SONGS_FILE   = "songs.json",
        *C_GENERAL_FILE = "general.json";

    HashSet<SongCache> songs;
    int coverIDcounter = 1;

    bool modified = false;

    static void RepairDefaultProfile();

    static Cache Load(CStr path = C_ROOT_PATH);
    void LoadSongs(const Path& root);
    void LoadGeneral(const Path& root);

    void AddCoverToCache(Bytes imgBytes, SongCache& songEntry);

    void WriteSongsTo(Nh::json& j) const;
    void WriteGeneralTo(Nh::json& j) const;

    void Save(CStr path = C_ROOT_PATH) const;
};