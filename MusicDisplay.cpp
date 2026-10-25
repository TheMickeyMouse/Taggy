#include "MusicDisplay.h"

#include "App.h"
#include "TagReader.h"
#include "Track.h"

MusicDisplay::MusicDisplay(UIRect& root)
     : uRoot  (root  .Pack(Dir::RIGHT, Length::Min(WIDTH_PER, 0.5_fh), "md", { .padding = PADDING })),
       uCover (uRoot->Pack(Dir::TOP, 1.0_fw,                     "md.Cover", { .margin = {{ .btm = PADDING / 2 }} })),
       uTitle (uRoot->Pack(Dir::TOP, TITLE_HEIGHT * Length::PX,  "md.Title", { .margin = {{ .btm = PADDING / 4 }} })),
       uArtist(uRoot->Pack(Dir::TOP, ARTIST_HEIGHT * Length::PX, "md.Artist"))
{}

void MusicDisplay::Draw(Canvas& canvas, const Track& track) {
    const auto _ = uRoot->BeginDraw(canvas);
    uRoot->Background(canvas, BACKGROUND_ALT, 0, 0, ROUNDING_RADIUS, ROUNDING_RADIUS);

    QWith$(uCover->BeginDraw(canvas)) {
        const fv2 size = uCover->rect.Size();
        canvas.DrawRoundedTexture(track.cover, { 0, size }, size.x / 32.0f);
    }

    uTitle->Label(canvas, track.title, WHITE, TextAlign::LEFT, true);
    uArtist->Label(canvas, track.artist, LIGHT_GRAY);
}
