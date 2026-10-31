#include "Playlist.h"

#include "Config.h"

Playlist Playlist::Load(const PlaylistConfig& conf) {
    Playlist playlist { .name = conf.name };

    [[maybe_unused]] const Debug::Timer timer { "fetching all files in directory", Debug::Logger::GetInternalLog() };

    for (const auto& filename : conf.songFilenames) {
        playlist.tracks.Push(Track::UnloadedFromFile(filename));
    }

    return playlist;
}

OptRef<Track> Playlist::Request(int i) {
    if (auto t = tracks.TryGet(i)) {
        if (!t->loaded) Track::Load(*t, true);
        return t;
    } else return nullptr;
}
