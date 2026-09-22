#pragma once
#include "Icon.h"
#include "UIRect.h"

class Sidebar {
    Ref<UIRect> side;
    Vec<Str> names;
    Vec<Icon> icons;
public:
    Sidebar(UIRect& side, Canvas& canvas);

    void Update();
    void Draw(Canvas& canvas) const;
};