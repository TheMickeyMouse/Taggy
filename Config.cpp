#include "Config.h"

#include <fstream>
#include "Cache.h"
#include "Json.h"
#include "Track.h"

namespace Fs = std::filesystem;

void Config::PlaylistLibrary::Write() const {
    Fs::create_directory(PATH);
    for (const auto& playlist : playlists) {
        if (!playlist.modified) continue;

        JsonWriter w;
        playlist.Write(w);

        auto out = std::ofstream(playlist.configPath);
        Text::WriteFile(out, w.Result());
    }
}

void Config::PlaylistLibrary::Read() {
    Fs::create_directory(PATH);
    for (const auto& playlistEntry : Fs::directory_iterator(PATH)) {
        if (playlistEntry.path().extension() != ".json") continue;

        Playlist playlist;
        playlist.Read(playlistEntry.path());

        playlists.Push(std::move(playlist));
    }
}

Config Config::Read() {
    const Path p = CONF_ROOT_PATH;
    if (!Fs::exists(p) || !Fs::is_directory(p)) {
        Fs::create_directory(p);
        return {};
    }

    Config config;

    config.playlistLibrary.Read();

    return config;
}

void Config::Write() const {
    Fs::create_directory(CONF_ROOT_PATH);

    playlistLibrary.Write();

    // {
    //     const Path playlistPath = p / CONF_PLAYLIST_PATH;
    //     Fs::create_directory(playlistPath);
    //
    // }
}
