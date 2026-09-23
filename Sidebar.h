#pragma once
#include "Icon.h"
#include "Smooth.h"
#include "UIRect.h"

class Sidebar {
    Ref<UIRect> side;

    struct Label {
        Str name;
        Icon icon;
        Smooth<bool> vActive = false;
    };
    Vec<Label> labels;

    Smooth<int> vSelectedIndex = 0;
    Smooth<bool> vHasSelected = false;
public:
    Sidebar(UIRect& side, Canvas& canvas);

    void Update(float dt);
    void Draw(Canvas& canvas);
};