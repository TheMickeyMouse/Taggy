#pragma once
#include "UIRect.h"

struct Track;

class MusicDisplay {
    UIRef uRoot, uCover, uTitle, uArtist;
public:
    static constexpr float
        PADDING = 20.0f,
        TITLE_HEIGHT = 50.0f,
        ARTIST_HEIGHT = 30.0f;
    inline static const Length WIDTH_PER = 0.4_fw;

    MusicDisplay(UIRect& root);
    void Draw(Canvas& canvas, const Track& track);
};
