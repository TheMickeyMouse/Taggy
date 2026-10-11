#include "Playlist.h"

Playlist Playlist::Load(CStr path, int pageNum) {
    Playlist playlist { .folderPath = path.Data() };

    for (const auto& file : std::filesystem::directory_iterator(playlist.folderPath)) {
        playlist.tracks.ResizeExtraDefault(1);
        Track::Load(playlist.tracks.Last(), file.path(), true);
        if (playlist.tracks.Length() >= pageNum) break;
    }

    return playlist;
}
