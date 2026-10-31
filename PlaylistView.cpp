#include "PlaylistView.h"

#include "App.h"

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

void SongView::Draw(Canvas& canvas, u32 i, Track& track) {
    [[maybe_unused]] const auto _ = uRoot->BeginDraw(canvas);

    constexpr float RADIUS = 5;
    const fColor bgColor = i % 2 == 1 ? BACKGROUND_ALT + 0.03f : BACKGROUND_ALT;
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
        App::Instance->SelectTrack(track);
    }
}

PlaylistView::PlaylistView(UIRect& root, Canvas& canvas, OptRef<Playlist> playlist)
    : uRoot        (root  .Pack(Dir::TOP,  1.0f * Length::RPER, "pv",        { .margin = { ROUNDING_RADIUS, 0 } })),
      uTitle       (uRoot->Pack(Dir::TOP,  0.2f * Length::FH,   "pv.Title",  { .padding = 2 * PADDING / 3, .margin = {{ .btm = PADDING / 2 }} })),
      uRest        (uRoot->Pack(Dir::LEFT, 1.0f * Length::PER,  "pv.Rest")),

      uPlaylistIcon (uTitle->Pack(Dir::LEFT, 1.0_fh,  "pv.PlaylistIcon",  { .margin = {{ .right = PADDING / 2 }}, })),
      uPlaylistSrc  (uTitle->Pack(Dir::TOP,  17.0_px, "pv.PlaylistSrc",   { .margin = {{ .btm = PADDING / 4 }}, })),
      uPlaylistTitle(uTitle->Pack(Dir::TOP,  0.5_fh,  "pv.PlaylistTitle", { .margin = {{ .btm = PADDING / 4 }}, })),

      uScrollRegion(uRest->Pack(Dir::RIGHT, SCROLLBAR_THICKNESS * Length::PX, "pv.ScrollRegion")),
      uScrollbar   (uScrollRegion->Pack(Dir::TOP, 0.0_px, "pv.Scrollbar", { .margin = 2 })),

      uTracks      (uRest->Pack(Dir::RIGHT, 1.0f * Length::RPER, "pv.Tracks", { .margin = {{ .right = -SCROLLBAR_THICKNESS }} })),

      playlist(playlist),
      vPlaylistIcon(Icon::FromSVG(R"(<svg viewBox="0 0 24 24" fill="none" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M3 14h3a2 2 0 0 1 2 2v3a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-7a9 9 0 0 1 18 0v7a2 2 0 0 1-2 2h-1a2 2 0 0 1-2-2v-3a2 2 0 0 1 2-2h3"/></svg>)", canvas))
{}

float PlaylistView::PlaylistHeight() const {
    return SongView::HEIGHT * (float)playlist->tracks.Length();
}

float PlaylistView::VisibleHeight() const {
    return uTracks->rect.Height();
}

float PlaylistView::MaxScrollAmount() const {
    return PlaylistHeight() - VisibleHeight();
}

void PlaylistView::ScrollBy(float dy) { ScrollTo(yPosition.target + dy); }

void PlaylistView::ScrollTo(float y) {
    yPosition.target = std::clamp(y, 0.0f, MaxScrollAmount());
}

void PlaylistView::Update(float dt) {
    if (uRest->Hovered()) {
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
    if (!yPosition.IsStable()) UpdateScrollingElements();
}

void PlaylistView::UpdateScrollingElements() {
    uTracks->padding.top = -std::fmod(yPosition.displayValue, SongView::HEIGHT);
    uTracks->ComputeLayout();

    uScrollRegion->padding.top = yPosition.displayValue * VisibleHeight() / PlaylistHeight();
    uScrollRegion->ComputeLayout();
}

void PlaylistView::OnLayoutChange() {
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

    uRest->ComputeLayout();
}

void PlaylistView::Draw(Canvas& canvas) {
    [[maybe_unused]] const auto _ = uRoot->BeginDraw(canvas);

    QWith$(uTitle->BeginDraw(canvas)) {
        uTitle->Background(canvas, BACKGROUND_ALT, PADDING / 4, PADDING / 2, PADDING / 4, PADDING / 2);

        static const fColor PLAYLIST_COLOR = 0xa83fd7_rgbf;
        QWith$(uPlaylistIcon->BeginDraw(canvas)) {
            uPlaylistIcon->Background(canvas, PLAYLIST_COLOR, PADDING / 2);
            const float s = uPlaylistIcon->rect.Width(), iconWidth = vPlaylistIcon.viewBox.Width();
            canvas.DrawMesh(vPlaylistIcon.mesh, s / 4, s / (2 * iconWidth), WHITE);
        }

        uPlaylistSrc  ->Label(canvas, "[placeholder]", LIGHT_GRAY);
        uPlaylistTitle->Label(canvas, playlist->name, WHITE, TextAlign::LEFT, true);
    }

    QWith$(uTracks->BeginDraw(canvas)) {
        [[maybe_unused]] const auto clip = canvas.BeginClip(uTracks->rect);

        int skipAmount = (int)(yPosition.displayValue / SongView::HEIGHT);
        for (const auto [i, child] : Iter::Enumerate(uTracks.Children())) {
            const int physicalIdx = i + skipAmount;
            auto track = playlist->Request(physicalIdx);
            if (!track) continue;

            SongView v = SongView::Infer(child);
            v.Draw(canvas, physicalIdx, *track);
        }
    }

    QWith$(uScrollbar->BeginDraw(canvas)) {
        uScrollbar->Capsule(canvas, LIGHT_GRAY.AddAlpha(uScrollbar->Hovered() || uScrollbar->Pressed() ? 0.3f : 0.2f));
    }
}
