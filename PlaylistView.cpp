#include "PlaylistView.h"

#include "App.h"

SongView::SongView(UIRect& root)
    : uRoot(root.Pack(Dir::TOP, 80.0_px, { .padding = PADDING, .margin = {{ .btm = PADDING / 2 }} })),
      uCover(uRoot->Pack(Dir::LEFT, 1.0_fh, { .margin = {{ .right = PADDING }} })),
      uInfo (uRoot->Pack(Dir::LEFT, 0.45_per, { .centerY = true })),
      uInfoTitle (uInfo->Pack(Dir::TOP, 0.3_per, { .margin = {{ .btm = PADDING / 2 }} })),
      uInfoArtist(uInfo->Pack(Dir::TOP, 0.24_per))
{}

void SongView::Draw(Canvas& canvas, Track& track) {
    [[maybe_unused]] const auto _ = uRoot->BeginDraw(canvas);

    QWith$(uCover->BeginDraw(canvas)) {
        canvas.DrawRoundedTexture(track.cover, { 0, uCover->rect.Size() }, 5);
    }

    QWith$(uInfo->BeginDraw(canvas)) {
        uInfoTitle->Label(canvas, track.title, WHITE, TextAlign::LEFT, true);
        uInfoArtist->Label(canvas, track.artist, LIGHT_GRAY);
    }

    if (uCover->onClick) {
        App::Instance->SelectTrack(track);
    }
}

PlaylistView::PlaylistView(UIRect& root)
    : uRoot  (root  .Pack(Dir::LEFT, 1.0f * Length::RPER, { .padding = PADDING / 2, .margin = PADDING })),
      uTitle (uRoot->Pack(Dir::TOP,  0.2f * Length::FH,   { .margin = {{ .btm = PADDING / 2 }} })),
      uTracks(uRoot->Pack(Dir::LEFT, 1.0f * Length::PER)) {

    for (auto& uSong : uSongs) {
        uSong = SongView(uTracks);
    }
}

void PlaylistView::Draw(Canvas& canvas, Playlist& playlist) {
    [[maybe_unused]] const auto _ = uRoot->BeginDraw(canvas);
    yPosition += uRoot->scrollDelta * 8.0f;

    uRoot->Background(canvas, BACKGROUND, PADDING);

    QWith$(uTitle->BeginDraw(canvas))
        uTitle->Background(canvas, BACKGROUND_ALT, PADDING / 4, PADDING / 2, PADDING / 4, PADDING / 2);

    QWith$(uTracks->BeginDraw(canvas)) {
        uTracks->Background(canvas, BACKGROUND_ALT, PADDING / 2, PADDING / 4, PADDING / 2, PADDING / 4);
        canvas.transform.pos.y += yPosition;

        for (int i = 0; i < std::min(std::size(uSongs), playlist.tracks.Length()); ++i) {
            uSongs[i].Draw(canvas, playlist.tracks[i]);
        }
    }
}
