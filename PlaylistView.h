#pragma once
#include "Config.h"
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

    void Draw(Canvas& canvas, u32 i, Config::PlaylistLibrary& lib, u32 playlistIdx, u32 trackIdx);
};

#define INNER_ELEMENT(OUTER, E) \
    OUTER* Outer() { return QGetterMut$(Outer); } \
    const OUTER* Outer() const { return (const OUTER*)(void*)((u8*)this - offsetof(OUTER, E)); }

class PlaylistView {
    UIRef uRoot;

    class TopView {
    public:
        UIRef uRoot;
        UIRef uControlSide;
        UIRef uIcon, uSrcInfo, uTitle;
        UIRef uAddBtn, uEditBtn;
        Icon vIcon, vAddIcon, vEditIcon;

        static constexpr float PADDING = 10;

        TopView(UIRect& root, Canvas& canvas);

        void Update(float dt);
        void Draw(Canvas& canvas);

        INNER_ELEMENT(PlaylistView, uTop);
    } uTop;

    class ListView {
    public:
        UIRef uRoot;
        UIRef uScrollRegion, uScrollbar;
        UIRef uTracks;

        int numShownTracks = 0;
        Smooth<float> yPosition = { 0, Speed::REALLY_FAST };
        float scrollbarClickYDelta = 0;

        static constexpr float SCROLLBAR_THICKNESS = 10;

        ListView(UIRect& root);

        float PlaylistHeight() const;
        float VisibleHeight() const;
        float MaxScrollAmount() const;
        void Update(float dt);

        void ScrollBy(float dy);
        void ScrollTo(float y);
        void RefreshScrollingElements();
        void RefreshHeight();

        void OnLayoutChange();
        void Draw(Canvas& canvas);

        INNER_ELEMENT(PlaylistView, uList);
    } uList;

    OptRef<Config::PlaylistLibrary> playlistLibrary;
    IndexRef<Playlist, &PlaylistView::playlistLibrary> playlistIdx = 0;
    Playlist& GetPlaylist();
    const Playlist& GetPlaylist() const;

public:
    static constexpr float PADDING = 20;

    PlaylistView(UIRect& root, Canvas& canvas);

    void Update(float dt);
    void OnLayoutChange();
    void Draw(Canvas& canvas);

    void TriggerAddSong(Span<const Path> paths);

    void Select(Config::PlaylistLibrary& playlistLib, u32 newPlaylistIdx);
};
