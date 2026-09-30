#pragma once
#include "UIRect.h"

struct Track {
    Texture2D cover;
    String title, artist;

    static Track Load(CStr filename);
};