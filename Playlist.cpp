#include "Playlist.h"

#include <fstream>

#include "Config.h"
#include "Json.h"
#include "Utils/Text/UTF.h"

namespace Fs = std::filesystem;

void Playlist::Write(JsonWriter& w) const {
    w.BeginObject();
    w.Field("name", name);
    w.Key("songs"); w.BeginArray();
    for (const auto& p : trackPaths)
        w.Value(p);
    w.EndArray();
    w.EndObject();
}

void Playlist::Read(Path inPath) {
    std::ifstream f { inPath };
    const String in = Text::ReadFile(f).Assert("couldn't read file");
    Str inStr = in;

    configPath = std::move(inPath);

    JsonReader r = JsonReader::ForObject();
    r.AcceptStr("name", &name);
    r.AcceptVec<Fs::ReadPath>("songs", &trackPaths);
    r.Process(inStr);
}

void Playlist::Load() {
    if (!loaded) {
        loaded = true;
        for (const auto& p : trackPaths) {
            tracks.Push(Track::UnloadedFromFile(p));
        }
    }
}

void Playlist::Unload() {
    tracks.Clear();
    loaded = false;
}

OptRef<Track> Playlist::Request(int i) {
    if (auto t = tracks.TryGet(i)) {
        if (!t->loaded) Track::Load(*t, true);
        return t;
    } else return nullptr;
}

u32 Playlist::NumSongs() const {
    return (u32)tracks.Length();
}

void Playlist::AddSongs(Span<const Path> songs) {
    if (!songs) return;

    modified = true;
    for (const auto& filename : songs) if (filename.extension() == ".mp3") {
        trackPaths.Push(filename);
        tracks.Push(Track::UnloadedFromFile(filename));
    } else {
        Debug::QWarn$("skipping song {}", Text::WStr(filename.c_str()));
    }
}
