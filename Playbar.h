#pragma once
#include "Icon.h"
#include "UIRect.h"

namespace Quasi::IO {
    class IO;
}

struct Track;

class Playbar {
    Ref<UIRect> uRoot, uCover, uInfo, uControlsExtra, uControls;
    Ref<UIRect> uInfoTitle, uInfoArtist;
    Ref<UIRect> uDurationBar, uPrevBtn, uPlayBtn, uNextBtn;

    Icon vPlay, vPause, vNext, vPrev;
    Option<float> requestedMusicSeek = nullptr;
public:
    static constexpr float
        HEIGHT = 120.0f,
        PADDING = 24.0f,
        PROGRESS_BAR_THICKNESS = 6.0f;

    Playbar(UIRect& root, Canvas& canvas);

    void Update(float dt);
    void Draw(Canvas& canvas, Track& track, const IO::IO& io);
};
