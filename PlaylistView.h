#pragma once
#include "Icon.h"
#include "Playlist.h"
#include "Smooth.h"
#include "UIRect.h"

class SongView {
public:
    UIRef uRoot, uNumber, uCover, uInfo;
    UIRef uInfoTitle, uInfoArtist;

    static constexpr float HEIGHT = 80, PADDING = 10;

    static SongView Infer(UIRect& root);
    static void AddView(UIRect& container);

    void Draw(Canvas& canvas, u32 i, Track& track);
};

class PlaylistView {
public:
    UIRef uRoot, uTitle, uRest;
    UIRef uPlaylistIcon, uPlaylistSrc, uPlaylistTitle;
    UIRef uScrollRegion, uScrollbar;
    UIRef uTracks;

    int numShownTracks = 0;
    Smooth<float> yPosition = { 0, Speed::REALLY_FAST };
    float scrollbarClickYDelta = 0;
    OptRef<Playlist> playlist;

    Icon vPlaylistIcon;

    static constexpr float PADDING = 20;
    static constexpr float SCROLLBAR_THICKNESS = 10;

    PlaylistView(UIRect& root, Canvas& canvas, OptRef<Playlist> playlist = nullptr);

    float PlaylistHeight() const;
    float VisibleHeight() const;
    float MaxScrollAmount() const;

    void ScrollBy(float dy);
    void ScrollTo(float y);
    void Update(float dt);
    void UpdateScrollingElements();
    void OnLayoutChange();
    void Draw(Canvas& canvas);
};
