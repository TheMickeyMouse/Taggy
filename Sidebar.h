#pragma once
#include "Icon.h"
#include "Smooth.h"
#include "UIRect.h"

class Sidebar {
    UIRef uRoot;

    struct Label {
        Str name;
        Icon icon;
        Smooth<bool> vActive = { false, Speed::FAST };
    };
    Vec<Label> labels;

    Smooth<int> vSelectedIndex = { 0, Speed::FAST };
    Smooth<bool> vHasSelected = { false, Speed::SLOW };
public:
    static constexpr float PADDING = 5.0f, INNER_WIDTH = 48.0f;

    Sidebar(UIRect& root, Canvas& canvas);

    void Update(float dt);
    void Draw(Canvas& canvas);
};