#include "PlaylistView.h"

#include "App.h"
#include "WinUtils.h"

SongView SongView::Infer(UIRect& root)  {
    SongView view;
    view.uRoot = root;
    auto [number, cover, info] = root.Grab<3>();
    auto [infoTitle, infoArtist] = info->Grab<2>();

    view.uNumber     = number;
    view.uCover      = cover;
    view.uInfo       = info;
    view.uInfoTitle  = infoTitle;
    view.uInfoArtist = infoArtist;

    return view;
}

void SongView::AddView(UIRect& container) {
    UIRef root = container.Pack(Dir::TOP, HEIGHT * Length::PX, "pv.sv#", { .padding = PADDING });

    /* number */ root->Pack(Dir::LEFT, 0.5_fh, "sv.Number", { .margin = {{ .top = PADDING * 2, .right = PADDING * 2, .btm = PADDING * 2 }} });
    /* cover */  root->Pack(Dir::LEFT, 1.0_fh,  "sv.Cover", { .margin = {{ .right = PADDING }} });
    UIRef info = root->Pack(Dir::LEFT, 0.45_per, "sv.Info", { .centerY = true });

    /* title */  info->Pack(Dir::TOP, 0.3_per,  "sv.InfoTitle", { .margin = {{ .btm = PADDING / 2 }} });
    /* artist */ info->Pack(Dir::TOP, 0.24_per, "sv.InfoArtist");
}

void SongView::Draw(Canvas& canvas, u32 i, Config::PlaylistLibrary& lib, u32 playlistIdx, u32 trackIdx) {
    [[maybe_unused]] const auto _ = uRoot->BeginDraw(canvas);

    auto& track = lib.playlists[playlistIdx].tracks[trackIdx];

    static constexpr float RADIUS = 5;
    const fColor& bgColor = i % 2 == 1 ? BACKGROUND_SEC : BACKGROUND_ALT;
    uRoot->Background(canvas, bgColor);

    uNumber->Label(canvas, Text::Format("{}", i + 1), LIGHT_GRAY, TextAlign::RIGHT);

    QWith$(uCover->BeginDraw(canvas)) {
        canvas.DrawRoundedTexture(track.cover, { 0, uCover->rect.Size() }, RADIUS);
    }

    QWith$(uInfo->BeginDraw(canvas)) {
        uInfoTitle->Label(canvas, track.title, WHITE, TextAlign::LEFT, true);
        uInfoArtist->Label(canvas, track.artist, LIGHT_GRAY);
    }

    if (uCover->Clicked()) {
        App::Instance->SelectTrack(playlistIdx, trackIdx);
    }
}

PlaylistView::TopView::TopView(UIRect& root, Canvas& canvas)
    : uRoot       (root.Pack(Dir::TOP,  0.2f * Length::FH, "pv.Title",   { .padding = PADDING, .margin = {{ .btm = PADDING }} })),
      uControlSide(uRoot->Pack(Dir::RIGHT, 40.0_px, "pv.ControlSide",    { .margin = {{ .left = PADDING }} })),
      uIcon       (uRoot->Pack(Dir::LEFT, 1.0_fh,  "pv.PlaylistIcon",    { .margin = {{ .right = PADDING }}, })),
      uSrcInfo    (uRoot->Pack(Dir::TOP,  19.0_px, "pv.PlaylistSrc",     { .margin = {{ .btm = PADDING / 2 }}, })),
      uTitle      (uRoot->Pack(Dir::TOP,  38.0_px, "pv.PlaylistTitle",   { .margin = {{ .btm = PADDING / 2 }}, })),
      uAddBtn     (uControlSide->Pack(Dir::TOP, 1.0_fw, "pv.AddSongBtn", { .margin = {{ .btm = PADDING / 2 }} })),
      uEditBtn    (uControlSide->Pack(Dir::TOP, 1.0_fw, "pv.EditBtn",    { .margin = {{ .btm = PADDING / 2 }} })),
      vIcon(Icon::FromSVG(R"(<svg viewBox="0 0 24 24" fill="none" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M3 14h3a2 2 0 0 1 2 2v3a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-7a9 9 0 0 1 18 0v7a2 2 0 0 1-2 2h-1a2 2 0 0 1-2-2v-3a2 2 0 0 1 2-2h3"/></svg>)", canvas)),
      vAddIcon(Icon::FromCode({ 0, 24 }, canvas, [] (Canvas& c) {
          c.StrokeWeight(1);
          c.DrawLine({ 6, 12 }, { 18, 12 });
          c.DrawLine({ 12, 6 }, { 12, 18 });
      })),
      vEditIcon(Icon::FromSVG(R"(<svg viewBox="0 0 24 24" fill="none" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M21.174 6.812a1 1 0 0 0-3.986-3.987L3.842 16.174a2 2 0 0 0-.5.83l-1.321 4.352a.5.5 0 0 0 .623.622l4.353-1.32a2 2 0 0 0 .83-.497z"/><path d="m15 5 4 4"/></svg>)", canvas))
{}

void PlaylistView::TopView::Update(float dt) {
    if (uAddBtn->Clicked()) {
        if (auto files = WinUtils::OpenFileDialog())
            Outer()->TriggerAddSong(files);
    }
}

void PlaylistView::TopView::Draw(Canvas& canvas) {
    [[maybe_unused]] const auto _ = uRoot->BeginDraw(canvas);
    uRoot->Background(canvas, BACKGROUND_ALT, PADDING / 4, PADDING / 2, PADDING / 4, PADDING / 2);

    static const fColor PLAYLIST_COLOR = 0xa83fd7_rgbf;
    QWith$(uIcon->BeginDraw(canvas)) {
        uIcon->Background(canvas, PLAYLIST_COLOR, PADDING / 2);
        uIcon->Icon(canvas, vIcon, WHITE, 0.5f);
    }

    uSrcInfo->Label(canvas, Text::Format("{} songs", Outer()->GetPlaylist().NumSongs()), LIGHT_GRAY);
    uTitle  ->Label(canvas, Outer()->GetPlaylist().name, WHITE, TextAlign::LEFT, true);

    QWith$(uAddBtn->BeginDraw(canvas)) {
        uAddBtn->Circle(canvas, BACKGROUND);
        uAddBtn->Icon(canvas, vAddIcon, WHITE, 0.8f);
    }
    QWith$(uEditBtn->BeginDraw(canvas)) {
        uEditBtn->Circle(canvas, BACKGROUND);
        uEditBtn->Icon(canvas, vEditIcon, WHITE, 0.6f);
    }
}

PlaylistView::ListView::ListView(UIRect& root)
    : uRoot        (root.Pack(Dir::LEFT, 1.0f * Length::PER,  "pv.Rest")),
      uScrollRegion(uRoot->Pack(Dir::RIGHT, SCROLLBAR_THICKNESS * Length::PX, "pv.ScrollRegion")),
      uScrollbar   (uScrollRegion->Pack(Dir::TOP, 0.0_px, "pv.Scrollbar", { .margin = 2 })),
      uTracks      (uRoot->Pack(Dir::RIGHT, 1.0f * Length::RPER, "pv.Tracks", { .margin = {{ .right = -SCROLLBAR_THICKNESS }} }))
{}

float PlaylistView::ListView::PlaylistHeight() const {
    return SongView::HEIGHT * (float)Outer()->GetPlaylist().NumSongs();
}

float PlaylistView::ListView::VisibleHeight() const {
    return uTracks->rect.Height();
}

float PlaylistView::ListView::MaxScrollAmount() const {
    return PlaylistHeight() - VisibleHeight();
}

void PlaylistView::ListView::Update(float dt) {
    if (uRoot->Hovered()) {
        ScrollBy(-uTracks->ScrollDelta() * (0.9f * SongView::HEIGHT));
    }
    // lets say the full playlist is H units high while the display area is only h units.
    // the scrollbar has to condense H into h. since only h/H is visible the scrollbar height is s = h * (h/H).
    // thus the scrolling region, while looks like h, is actually only h-s.
    const float scrollbarHeight = uScrollbar->rect.Height();
    const float scrollRatio = MaxScrollAmount() / (VisibleHeight() - scrollbarHeight);
    if (uScrollbar->Clicked()) {
        const float expectedScrollAmt = uScrollRegion->ClickPos().y * scrollRatio;
        scrollbarClickYDelta = yPosition.target - expectedScrollAmt;
    } else if (uScrollbar->Pressed()) {
        ScrollTo(uScrollRegion->ClickPos().y * scrollRatio + scrollbarClickYDelta);
    } else if (uScrollRegion->Clicked()) {
        yPosition.target = (uScrollRegion->ClickPos().y - scrollbarHeight / 2) * scrollRatio;
    }

    yPosition.Update(dt);
    if (!yPosition.IsStable()) {
        RefreshScrollingElements();
        RefreshHeight();
    }
}

void PlaylistView::ListView::ScrollBy(float dy) { ScrollTo(yPosition.target + dy); }

void PlaylistView::ListView::ScrollTo(float y) {
    yPosition.target = std::clamp(y, 0.0f, MaxScrollAmount());
}

void PlaylistView::ListView::RefreshScrollingElements() {
    uTracks->padding.top = -std::fmod(yPosition.displayValue, SongView::HEIGHT);
    uTracks->ComputeLayout();
}

void PlaylistView::ListView::RefreshHeight() {
    uScrollRegion->padding.top = yPosition.displayValue * VisibleHeight() / PlaylistHeight();
    uScrollRegion->ComputeLayout();
}

void PlaylistView::ListView::OnLayoutChange() {
    {
        const float fullScrollableHeight = PlaylistHeight(),
                    shownHeight = VisibleHeight(),
                    portion = shownHeight / fullScrollableHeight;

        uScrollbar->length = portion * shownHeight;
    }

    const float height = uTracks->rect.Size().y;
    const int shouldDisplayTracks = 1 + (int)std::ceil(height / SongView::HEIGHT);

    if (shouldDisplayTracks < numShownTracks) { // delete
        uTracks.TruncChildren(shouldDisplayTracks);
    } else for (int i = numShownTracks; i < shouldDisplayTracks; i++) {
        SongView::AddView(uTracks);
    }
    numShownTracks = shouldDisplayTracks;

    uRoot->ComputeLayout();
}

void PlaylistView::ListView::Draw(Canvas& canvas) {
    QWith$(uTracks->BeginDraw(canvas)) {
        [[maybe_unused]] const auto clip = canvas.BeginClip(uTracks->rect);

        int skipAmount = (int)(yPosition.displayValue / SongView::HEIGHT);
        for (const auto [i, child] : Iter::Enumerate(uTracks.Children())) {
            const int physicalIdx = i + skipAmount;
            auto track = Outer()->GetPlaylist().Request(physicalIdx);
            if (!track) continue;

            SongView v = SongView::Infer(child);
            v.Draw(canvas, physicalIdx, *Outer()->playlistLibrary, Outer()->playlistIdx, physicalIdx);
        }
    }

    QWith$(uScrollbar->BeginDraw(canvas)) {
        uScrollbar->Capsule(canvas, LIGHT_GRAY.AddAlpha(uScrollbar->Hovered() || uScrollbar->Pressed() ? 0.3f : 0.2f));
    }
}

Playlist& PlaylistView::GetPlaylist() {
    return playlistLibrary->playlists[playlistIdx];
}

const Playlist& PlaylistView::GetPlaylist() const {
    return playlistLibrary->playlists[playlistIdx];
}

PlaylistView::PlaylistView(UIRect& root, Canvas& canvas)
    : uRoot (root  .Pack(Dir::TOP,  1.0f * Length::RPER, "pv",        { .margin = { ROUNDING_RADIUS, 0 } })),
      uTop  (*uRoot, canvas),
      uList (*uRoot) {

}

void PlaylistView::Update(float dt) {
    uTop.Update(dt);
    uList.Update(dt);
}

void PlaylistView::OnLayoutChange() {
    uList.OnLayoutChange();
}

void PlaylistView::Draw(Canvas& canvas) {
    uTop.Draw(canvas);
    uList.Draw(canvas);
}

void PlaylistView::TriggerAddSong(Span<const Path> paths) {
    GetPlaylist().AddSongs(paths);
}

void PlaylistView::Select(Config::PlaylistLibrary& playlistLib, u32 newPlaylistIdx) {
    playlistLibrary = playlistLib;
    playlistIdx = newPlaylistIdx;
    GetPlaylist().Load();
}
