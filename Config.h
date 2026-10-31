#pragma once
#include "Utils/CStr.h"
#include "Utils/String.h"
#include "Utils/Vec.h"
#include "nlohmann-json.h"
#include "Track.h"

using namespace Quasi;
using namespace Math;
namespace Nh = nlohmann;

struct PlaylistConfig {
    String configFilename; // config/playlists/...
    String name;
    Vec<Path> songFilenames;
};

class Config {
public:
    // config/
    // ├─ playlists/
    // │  ├─ likedsongs.json
    // │  └─ ...
    // └─ settings.json
    static constexpr const char
        *CONF_ROOT_PATH = "config/",
        *CONF_PLAYLIST_PATH = "playlists/";

    Vec<PlaylistConfig> playlists; // stored in ./playlists/

    static Config Load(CStr path = CONF_ROOT_PATH);
    void LoadPlaylistsFrom(const Nh::json& j, Str filename);

    void Save(CStr path = CONF_ROOT_PATH) const;
};