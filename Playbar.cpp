#include "Playbar.h"

#include "Track.h"

Playbar::Playbar(UIRect& root, Canvas& canvas)
    : uRoot(root.Pack(Dir::BTM, HEIGHT * Length::PX, PADDING)),
      uCover        (uRoot->Pack(Dir::LEFT,  1.0_fh,  0, {{ .right = PADDING * 0.6f }})),
      uInfo         (uRoot->Pack(Dir::LEFT,  0.2_per, 0, {{ .right = PADDING }}, true)),
      uControlsExtra(
          (uRoot->Pack(Dir::RIGHT, 1.0_fh,  0, {{ .left = PADDING * 0.6f }}),
           uRoot->Pack(Dir::RIGHT, 0.2_per, 0, {{ .left = PADDING }}))
      ),
      uControls(uRoot->Pack(Dir::TOP, 1.0_per, 0, 0, true)),

      uInfoTitle (uInfo->Pack(Dir::TOP, 0.32_per)),
      uInfoArtist(uInfo->Pack(Dir::TOP, 0.24_per)),

      vPlay (Icon::FromSVG(R"(<svg viewBox="0 0 24 24" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M5 5a2 2 0 0 1 3.008-1.728l11.997 6.998a2 2 0 0 1 .003 3.458l-12 7A2 2 0 0 1 5 19z"/></svg>)", canvas)),
      vPause(Icon::FromSVG(R"(<svg viewBox="0 0 24 24" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="14" y="3" width="5" height="18" rx="1"/><rect x="5" y="3" width="5" height="18" rx="1"/></svg>)", canvas)),
      vNext (Icon::FromSVG(R"(<svg viewBox="0 0 24 24" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M21 4v16"/><path d="M6.029 4.285A2 2 0 0 0 3 6v12a2 2 0 0 0 3.029 1.715l9.997-5.998a2 2 0 0 0 .003-3.432z"/></svg>)", canvas)),
      vPrev (Icon::FromSVG(R"(<svg viewBox="0 0 24 24" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M17.971 4.285A2 2 0 0 1 21 6v12a2 2 0 0 1-3.029 1.715l-9.997-5.998a2 2 0 0 1-.003-3.432z"/><path d="M3 20V4"/></svg>)", canvas))
{
    uControls->Pack(Dir::LEFT, 0.2_per, { 0, PADDING * 0.9f }, {{ .right = PADDING }});
    uControls->Pack(Dir::LEFT, 0.2_per, { 0, PADDING * 0.5f }, {{ .right = PADDING }});
    uControls->Pack(Dir::LEFT, 0.2_per, { 0, PADDING * 0.9f });
}

void Playbar::Draw(Canvas& canvas, const Track& track) {
    [[maybe_unused]] const auto _ = uRoot->BeginDraw(canvas);

    canvas.Fill(BACKGROUND_ALT);
    canvas.DrawRect({ 0, uRoot->rect.Size() });

    canvas.Stroke(ACCENT);
    canvas.StrokeWeight(PROGRESS_BAR_THICKNESS);
    canvas.DrawLine(0, { uRoot->rect.Width(), 0 });

    canvas.NoStroke();
    canvas.Fill(BACKGROUND);

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
        canvas.DrawRect({ 0, uControls->rect.Size() });

        Icon* icons[] = { &vPrev, &vPlay, &vNext };
        for (int i = 0; i < uControls->children.Length(); ++i) QWith$(uControls->children[i]->BeginDraw(canvas)) {
            const auto& child = uControls->children[i];
            const fv2 s = child->GetInnerRect().Size(),
                      h = s.y / icons[i]->viewBox.Size();
            canvas.DrawMesh(icons[i]->mesh, { (s.x - s.y) / 2, child->padding.top }, h, WHITE);
        }
    }

    QWith$(uControlsExtra->BeginDraw(canvas)) {
        canvas.DrawRect({ 0, uControlsExtra->rect.Size() });
    }
}
