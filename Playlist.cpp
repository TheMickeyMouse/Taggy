#include "Playlist.h"

Playlist Playlist::Load(CStr path) {
    Playlist playlist {
        .folderPath = path.Data(),
        .folderPathCStr = path,
        .folderNameCStr = path.RevSplitOnceOn(Cmp::Equals { '\\' })[2_nd]
    };

    [[maybe_unused]] const Debug::Timer timer { "fetching all files in directory", Debug::Logger::GetInternalLog() };

    for (const auto& file : std::filesystem::directory_iterator(playlist.folderPath)) {
        playlist.tracks.Push(Track::UnloadedFromFile(file.path()));
    }

    return playlist;
}

OptRef<Track> Playlist::Request(int i) {
    if (auto t = tracks.TryGet(i)) {
        if (!t->loaded) Track::Load(*t, true);
        return t;
    } else return nullptr;
}
