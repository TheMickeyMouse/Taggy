#pragma once
#include "GUI/Canvas.h"

using namespace Quasi;
using namespace Graphics;
using namespace Math;

class Icon {
public:
    UIMesh mesh;
    fRect2D viewBox;

    static Icon FromSVG(Str src, Canvas& canvas);

    static void ParseSVGHeader(Str& src, Icon& icon, Canvas& canvas);
    static void ParsePath(Str src, Canvas& canvas);
};
