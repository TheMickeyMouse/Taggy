#pragma once
#include "Playlist.h"
#include "UIRect.h"

class SongView {
public:
    UIRef uRoot, uCover, uInfo;
    UIRef uInfoTitle, uInfoArtist;

    static constexpr float PADDING = 10;

    SongView() = default;
    SongView(UIRect& root);

    void Draw(Canvas& canvas, Track& track);
};

class PlaylistView {
public:
    UIRef uRoot, uTitle, uTracks;

    float yPosition = 0;

    static constexpr float PADDING = 20;
    static constexpr int PAGE = 5;

    SongView uSongs[PAGE];

    PlaylistView(UIRect& root);

    void Draw(Canvas& canvas, Playlist& playlist);
};
