#include "Config.h"

#include <fstream>
#include "Cache.h"
#include "Track.h"

namespace Fs = std::filesystem;

Config Config::Load(CStr path) {
    const Path p = path.Data();
    if (!Fs::exists(p) || !Fs::is_directory(p)) {
        Fs::create_directory(p);
        return {};
    }

    Config config;

    const Path playlistPath = p / CONF_PLAYLIST_PATH;
    if (Fs::exists(playlistPath) && Fs::is_directory(playlistPath)) {
        for (const auto& playlistEntry : Fs::directory_iterator(playlistPath)) {
            config.LoadPlaylistsFrom(
                Nh::json::parse(std::ifstream(playlistEntry.path())),
                LegConvert(playlistEntry.path().filename().string())
            );
        }
    } else {
        Fs::create_directory(playlistPath);
    }

    return config;
}

void Config::LoadPlaylistsFrom(const Nh::json& j, Str filename) {
    auto& playlist = playlists.Push({});
    playlist.name = j["name"].get<String>();
    playlist.configFilename = filename;

    for (const auto& s : j["songs"]) {
        playlist.songFilenames.Push(s.get<std::string_view>());
    }
}

void Config::Save(CStr path) const {
    const Path p = path.Data();
    Fs::create_directory(p);

    // {
    //     const Path playlistPath = p / CONF_PLAYLIST_PATH;
    //     Fs::create_directory(playlistPath);
    //
    // }
}
