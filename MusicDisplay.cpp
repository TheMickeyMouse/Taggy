#include "MusicDisplay.h"

#include "TagReader.h"
#include "Track.h"
#include "Utils/Iter/Zip.h"

MusicDisplay::MusicDisplay(UIRect& root)
     : uRoot(root.Pack(Dir::RIGHT, WIDTH * Length::PX, { .padding = PADDING })),
       uCover (uRoot->Pack(Dir::TOP, 1.0_fw, { .margin = {{ .btm = PADDING / 2 }} })),
       uTitle (uRoot->Pack(Dir::TOP, TITLE_HEIGHT * Length::PX)),
       uArtist(uRoot->Pack(Dir::TOP, ARTIST_HEIGHT * Length::PX))
{}

void MusicDisplay::Draw(Canvas& canvas, const Track& track) {
    const auto _ = uRoot->BeginDraw(canvas);
    canvas.Fill(BACKGROUND_ALT);
    canvas.DrawRect({ 0, uRoot->rect.Size() });

    QWith$(uCover->BeginDraw(canvas)) {
        const fv2 size = uCover->rect.Size();
        canvas.DrawRoundedTexture(SubTexture(track.cover).Flipped(), { 0, size }, size.x / 32.0f);
    }

    QWith$(uTitle->BeginDraw(canvas)) {
        canvas.Stroke(WHITE);
        canvas.DrawText(track.title, TITLE_HEIGHT, { 0 }, { TextAlign::LEFT | TextAlign::VTOP });
    }
    QWith$(uArtist->BeginDraw(canvas)) {
        canvas.Stroke(LIGHT_GRAY);
        canvas.DrawText(track.artist, ARTIST_HEIGHT, {}, { TextAlign::LEFT | TextAlign::VTOP });
    }
}
