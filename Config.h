#pragma once
#include "Playlist.h"
#include "Utils/Vec.h"

class Config {
public:
    // config/
    // ├─ playlists/
    // │  ├─ likedsongs.json
    // │  └─ ...
    // └─ settings.json
    static constexpr const char* CONF_ROOT_PATH = "config/";

    struct PlaylistLibrary {
        static constexpr const char* PATH = "config/playlists/";
        Vec<Playlist> playlists; // stored in ./playlists/

        void Write() const;
        void Read();
    } playlistLibrary;

    static Config Read();
    void Write() const;
};