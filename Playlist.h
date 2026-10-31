#pragma once
#include "Track.h"

struct PlaylistConfig;

struct Playlist {
    Str name;
    Vec<Track> tracks;

    static Playlist Load(const PlaylistConfig& conf);

    OptRef<Track> Request(int i);
};
