#include "MusicDisplay.h"

#include "App.h"
#include "TagReader.h"
#include "Track.h"

MusicDisplay::MusicDisplay(UIRect& root)
     : uRoot  (root.Pack(Dir::RIGHT, Length::Min(WIDTH_PER * Length::FW, 0.5f * Length::FH), { .padding = PADDING, .margin = {{ .left = PADDING }} })),
       uCover (uRoot->Pack(Dir::TOP, 1.0_fw, { .margin = {{ .btm = PADDING / 2 }} })),
       uTitle (uRoot->Pack(Dir::TOP, TITLE_HEIGHT * Length::PX, { .margin = {{ .btm = PADDING / 4 }} })),
       uArtist(uRoot->Pack(Dir::TOP, ARTIST_HEIGHT * Length::PX))
{}

void MusicDisplay::Draw(Canvas& canvas, const Track& track) {
    const auto _ = uRoot->BeginDraw(canvas);
    canvas.NoStroke();
    canvas.Fill(BACKGROUND_ALT);
    canvas.DrawRect({ 0, uRoot->rect.Size() });

    QWith$(uCover->BeginDraw(canvas)) {
        const fv2 size = uCover->rect.Size();
        canvas.DrawRoundedTexture(track.cover, { 0, size }, size.x / 32.0f);
    }

    uTitle->Label(canvas, track.title, WHITE, TextAlign::LEFT, true);
    uArtist->Label(canvas, track.artist, LIGHT_GRAY);
}
