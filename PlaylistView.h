#pragma once
#include "Playlist.h"
#include "UIRect.h"

class SongView {
public:
    UIRef uRoot, uCover, uInfo;
    UIRef uInfoTitle, uInfoArtist;

    static constexpr float HEIGHT = 80, PADDING = 10, FULL_HEIGHT = HEIGHT + PADDING;

    static SongView Infer(UIRect& root);
    static void AddView(UIRect& container);

    void Draw(Canvas& canvas, Track& track);
};

class PlaylistView {
public:
    UIRef uRoot, uTitle, uTracks;
    int numShownTracks = 0;
    float yPosition = 0;

    static constexpr float PADDING = 20;

    PlaylistView(UIRect& root);

    void Update();
    void OnLayoutChange();
    void Draw(Canvas& canvas, Playlist& playlist);
};
