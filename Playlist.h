#pragma once
#include "Track.h"

struct Playlist {
    Path folderPath;
    Str folderPathCStr, folderNameCStr;
    Vec<Track> tracks;

    static Playlist Load(CStr path);

    OptRef<Track> Request(int i);
};
