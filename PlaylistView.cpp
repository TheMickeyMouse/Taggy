#include "PlaylistView.h"

#include "App.h"

SongView SongView::Infer(UIRect& root)  {
    SongView view;
    view.uRoot = root;
    auto [cover, info] = root.Grab<2>();
    auto [infoTitle, infoArtist] = info->Grab<2>();

    view.uCover      = cover;
    view.uInfo       = info;
    view.uInfoTitle  = infoTitle;
    view.uInfoArtist = infoArtist;

    return view;
}

void SongView::AddView(UIRect& container) {
    UIRef root = container.Pack(Dir::TOP, HEIGHT * Length::PX, "pv.sv#", { .padding = PADDING, .margin = {{ .btm = PADDING / 2 }} });

    /* cover */  root->Pack(Dir::LEFT, 1.0_fh,   "pv.sv.Cover", { .margin = {{ .right = PADDING }} });
    UIRef info = root->Pack(Dir::LEFT, 0.45_per, "pv.sv.Info", { .centerY = true });

    /* title */  info->Pack(Dir::TOP, 0.3_per,  "pv.sv.InfoTitle", { .margin = {{ .btm = PADDING / 2 }} });
    /* artist */ info->Pack(Dir::TOP, 0.24_per, "pv.sv.InfoArtist");
}

void SongView::Draw(Canvas& canvas, Track& track) {
    [[maybe_unused]] const auto _ = uRoot->BeginDraw(canvas);

    QWith$(uCover->BeginDraw(canvas)) {
        canvas.DrawRoundedTexture(track.cover, { 0, uCover->rect.Size() }, 5);
    }

    QWith$(uInfo->BeginDraw(canvas)) {
        uInfoTitle->Label(canvas, track.title, WHITE, TextAlign::LEFT, true);
        uInfoArtist->Label(canvas, track.artist, LIGHT_GRAY);
    }

    if (uCover->GetState(UIRect::CLICKED)) {
        App::Instance->SelectTrack(track);
    }
}

PlaylistView::PlaylistView(UIRect& root)
    : uRoot  (root  .Pack(Dir::LEFT, 1.0f * Length::RPER, "pv",        { .padding = PADDING / 2, .margin = PADDING })),
      uTitle (uRoot->Pack(Dir::TOP,  0.2f * Length::FH,   "pv.Title",  { .margin = {{ .btm = PADDING / 2 }} })),
      uTracks(uRoot->Pack(Dir::LEFT, 1.0f * Length::PER,  "pv.Tracks")) {}

void PlaylistView::Update() {
    const float scroll = uTracks->ScrollDelta();
    yPosition -= scroll * 8.0f;
    if (scroll != 0.0f) {
        uTracks->padding.top = -std::fmod(yPosition, SongView::HEIGHT + SongView::PADDING);
        uTracks->ComputeLayout();
    }
}

void PlaylistView::OnLayoutChange() {
    const float height = uTracks->rect.Size().y;
    int shouldDisplayTracks = 1 + (int)std::ceil(height / SongView::FULL_HEIGHT);
    if (shouldDisplayTracks < numShownTracks) { // delete
        uTracks.TruncChildren(shouldDisplayTracks);
    } else for (int i = numShownTracks; i < shouldDisplayTracks; i++) {
        SongView::AddView(uTracks);
    }
    numShownTracks = shouldDisplayTracks;

    uTracks->ComputeLayout();
}

void PlaylistView::Draw(Canvas& canvas, Playlist& playlist) {
    [[maybe_unused]] const auto _ = uRoot->BeginDraw(canvas);

    uRoot->Background(canvas, BACKGROUND, PADDING);

    QWith$(uTitle->BeginDraw(canvas))
        uTitle->Background(canvas, BACKGROUND_ALT, PADDING / 4, PADDING / 2, PADDING / 4, PADDING / 2);

    QWith$(uTracks->BeginDraw(canvas)) {
        canvas.ForceDrawCurrentBatch();
        Render::EnableScissor();
        iRect2D rect = (iRect2D)uTracks->rect;
        const int ymin = rect.min.y;
        rect.min.y = (int)App::Instance->Height() - rect.max.y;
        rect.max.y = (int)App::Instance->Height() - ymin;
        Render::SetScissorRect(rect);

        uTracks->Background(canvas, BACKGROUND_ALT, PADDING / 2, PADDING / 4, PADDING / 2, PADDING / 4);

        int skipAmount = (int)(yPosition / SongView::FULL_HEIGHT);
        for (const auto [i, child] : Iter::Enumerate(uTracks.Children())) {
            auto track = playlist.Request(i + skipAmount);
            if (!track) continue;

            SongView v = SongView::Infer(child);
            v.Draw(canvas, *track);
        }
        canvas.ForceDrawCurrentBatch();
        Render::DisableScissor();
    }
}
