#pragma once
#include "UIRect.h"

struct Track;

class MusicDisplay {
    UIRef uRoot, uCover, uTitle, uArtist;
public:
    static constexpr float
        WIDTH = 350.0f,
        PADDING = 20.0f,
        TITLE_HEIGHT = 50.0f,
        ARTIST_HEIGHT = 30.0f;

    MusicDisplay(UIRect& root);
    void Draw(Canvas& canvas, const Track& track);
};
