#pragma once
#include "Cache.h"
#include "Track.h"

class JsonWriter;

template <class T, auto> using IndexRef = u32;

struct Playlist {
    Path configPath;
    Field<String, "name"> name;
    Field<Vec<Path>, "songs"> trackPaths;
    bool modified = false;

    bool loaded = false;
    Vec<Track> tracks;

    void Write(JsonWriter& w) const;
    void Read(Path inPath);

    void Load();
    void Unload();

    OptRef<Track> Request(int i);
    u32 NumSongs() const;

    void AddSongs(Span<const Path> songs);
};
