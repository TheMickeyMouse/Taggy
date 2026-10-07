#include "Playbar.h"

#include "App.h"
#include "Track.h"

Playbar::Playbar(UIRect& root, Canvas& canvas)
    : uRoot(root.Pack(Dir::BTM, HEIGHT * Length::PX, { .padding = PADDING })),
      uCover        (uRoot->Pack(Dir::LEFT,  1.0_fh,  { .margin = {{ .right = PADDING * 0.6f }} })),
      uInfo         (uRoot->Pack(Dir::LEFT,  0.2_per, { .margin = {{ .right = PADDING }}, .centerY = true })),
      uControlsExtra(
          (uRoot->Pack(Dir::RIGHT, 1.0_fh,  { .margin = {{ .left = PADDING * 0.6f }} }),
           uRoot->Pack(Dir::RIGHT, 0.2_per, { .margin = {{ .left = PADDING }} }))
      ),
      uControls(uRoot->Pack(Dir::TOP, 1.0_per, { .centerX = true })),

      uInfoTitle (uInfo->Pack(Dir::TOP, 0.32_per)),
      uInfoArtist(uInfo->Pack(Dir::TOP, 0.24_per)),

      uDurationBar(uControls->Pack(Dir::BTM, PROGRESS_BAR_THICKNESS * Length::PX, { .margin = {{ .top = PROGRESS_BAR_THICKNESS }} })),
      uPrevBtn(uControls->Pack(Dir::LEFT, 0.2_per, { .margin = {{ .top = PADDING * 0.6f, .right = PADDING, .btm = PADDING * 0.9f }} })),
      uPlayBtn(uControls->Pack(Dir::LEFT, 0.2_per, { .margin = {{ .top = PADDING * 0.3f, .right = PADDING, .btm = PADDING * 0.7f }} })),
      uNextBtn(uControls->Pack(Dir::LEFT, 0.2_per, { .margin = {{ .top = PADDING * 0.6f, .btm = PADDING * 0.9f }} })),

      // <path d="M5 5a2 2 0 0 1 3.008-1.728l11.997 6.998a2 2 0 0 1 .003 3.458l-12 7A2 2 0 0 1 5 19z"/>,
      vPlay (Icon::FromCode({ 0, 24 }, canvas, [] (Canvas& c) {
          c.StrokeWeight(1);
          c.Fill(1);
          c.DrawTriangle({ 7, 5 }, { 19, 12 }, { 7, 19 });
      })),
      // <rect x="14" y="3" width="5" height="18" rx="1"/><rect x="5" y="3" width="5" height="18" rx="1"/>,
      vPause(Icon::FromCode({ 0, 24 }, canvas, [] (Canvas& c) {
          c.NoStroke();
          c.Fill(1);
          c.DrawRoundedRect({ { 14, 3 }, { 19, 21 } }, 2);
          c.DrawRoundedRect({ { 5, 3 },  { 10, 21 } }, 2);
      })),
      // <path d="M21 4v16"/><path d="M6.029 4.285A2 2 0 0 0 3 6v12a2 2 0 0 0 3.029 1.715l9.997-5.998a2 2 0 0 0 .003-3.432z"/>
      vNext(Icon::FromCode({ 0, 24 }, canvas, [] (Canvas& c) {
          c.StrokeWeight(2);
          c.Fill(1);
          c.DrawLine({ 21, 4 }, { 21, 20 });
          c.DrawTriangle({ 5, 5 }, { 17, 12 }, { 5, 19 });
      })),
      vPrev(Icon::FromCode({ 0, 24 }, canvas, [] (Canvas& c) {
          c.StrokeWeight(2);
          c.Fill(1);
          c.DrawLine({ 3, 4 }, { 3, 20 });
          c.DrawTriangle({ 19, 5 }, { 7, 12 }, { 19, 19 });
      }))
{
}

void Playbar::Update(float dt) {

}

void Playbar::Draw(Canvas& canvas, Track& track, const IO::IO& io) {
    [[maybe_unused]] const auto _ = uRoot->BeginDraw(canvas);

    canvas.Fill(BACKGROUND);
    canvas.DrawRect({ 0, uRoot->rect.Size() });

    // canvas.Stroke(ACCENT);
    // canvas.StrokeWeight(PROGRESS_BAR_THICKNESS);
    // canvas.DrawLine(0, { uRoot->rect.Width(), 0 });

    canvas.Fill(BACKGROUND_ALT);

    QWith$(uCover->BeginDraw(canvas)) {
        const fv2 size = uCover->rect.Size();
        canvas.DrawRoundedTexture(SubTexture(track.cover).Flipped(), { 0, size }, size.x / 8.0f);
    }

    QWith$(uInfoTitle->BeginDraw(canvas)) {
        canvas.Stroke(WHITE);
        const float h = uInfoTitle->rect.Height();
        canvas.DrawText(track.title, h, 0, { TextAlign::LEFT, h });
    }
    QWith$(uInfoArtist->BeginDraw(canvas)) {
        canvas.Stroke(LIGHT_GRAY);
        const float h = uInfoArtist->rect.Height();
        canvas.DrawText(track.artist, h, 0, { TextAlign::LEFT, h });
    }

    QWith$(uControls->BeginDraw(canvas)) {
        Icon* icons[] = { &vPrev, track.isPlaying ? &vPause : &vPlay, &vNext };
        for (int i = 1; i < uControls->children.Length(); ++i) QWith$(uControls->children[i]->BeginDraw(canvas)) {
            const auto& child = uControls->children[i];
            const fv2 s = child->GetInnerRect().Size(),
                      h = s.y / icons[i - 1]->viewBox.Size();

            const bool isPlayBtn = i == 2;

            if (isPlayBtn) {
                canvas.Fill(WHITE);
                canvas.DrawCircle(s * 0.5f, s.y * 0.7f);
            }
            canvas.DrawMesh(icons[i - 1]->mesh, { (s.x - s.y) / 2, child->padding.top }, h, isPlayBtn ? BACKGROUND : WHITE);
        }

        if (uPlayBtn->onClick) {
            track.Toggle();
        }
        if (uDurationBar->isMousePressed) {
            const float fraction = std::clamp(uDurationBar->clickPos.x / uDurationBar->rect.Width(), 0.0f, 1.0f);
            requestedMusicSeek = track.durationSec * fraction;
        } else if (requestedMusicSeek) { // not pressed but seek requested
            track.SeekTo(*requestedMusicSeek);
            requestedMusicSeek = nullptr;
        }

        QWith$(uDurationBar->BeginDraw(canvas)) {
            const float trackPlayedSeconds = requestedMusicSeek.UnwrapOr(track.GetPosition());
            const float y = uDurationBar->rect.Height() / 2.0f,
                        w = uDurationBar->rect.Width(),
                        x = w * (trackPlayedSeconds / track.durationSec);
            const bool active = uDurationBar->isHovered;

            canvas.StrokeWeight(active ? 1.3f * y : y);
            canvas.Stroke(DARK_GRAY);
            canvas.DrawLine({ 0, y }, { w, y });

            constexpr float FONT_SIZE = PROGRESS_BAR_THICKNESS * 3;

            canvas.DrawText(
                App::FormatSeconds((int)trackPlayedSeconds),
                FONT_SIZE, { -PADDING / 2, y }, { TextAlign::RIGHT }
            );
            canvas.DrawText(
                App::FormatSeconds((int)track.durationSec),
                FONT_SIZE, { w + PADDING / 2, y }, { TextAlign::LEFT }
            );

            canvas.Stroke(active ? ACCENT : WHITE);
            canvas.DrawLine({ 0, y }, { x, y });

            if (active) {
                canvas.Stroke(WHITE);
                canvas.StrokeWeight(y * 2.2f);
                canvas.DrawPoint({ x, y });

                const fv2 mouseAbove = { uDurationBar->RelativePos(io.GetMousePos()).x, -FONT_SIZE };
                const int mouseSeconds = (int)(mouseAbove.x / w * track.durationSec);

                canvas.NoStroke();
                canvas.Fill(DARK_GRAY);
                canvas.DrawRoundedRect(fRect2D::FromCenter(mouseAbove, { FONT_SIZE * 3, FONT_SIZE * 1.3 }), y * 2);

                canvas.Stroke(WHITE);
                canvas.DrawText(App::FormatSeconds(mouseSeconds), FONT_SIZE, mouseAbove);
            }
        }
    }

    // QWith$(uControlsExtra->BeginDraw(canvas)) {
    //     canvas.DrawRect({ 0, uControlsExtra->rect.Size() });
    // }
}
