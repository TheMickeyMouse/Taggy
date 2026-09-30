#pragma once
#include "Icon.h"
#include "UIRect.h"

struct Track;

class Playbar {
    Ref<UIRect> uRoot, uCover, uInfo, uControlsExtra, uControls;
    Ref<UIRect> uInfoTitle, uInfoArtist;

    Icon vPlay, vPause, vNext, vPrev;
public:
    static constexpr float
        HEIGHT = 120.0f,
        PADDING = 24.0f,
        PROGRESS_BAR_THICKNESS = 3.0f;

    Playbar(UIRect& root, Canvas& canvas);

    void Draw(Canvas& canvas, const Track& track);
};
