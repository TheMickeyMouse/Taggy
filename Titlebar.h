#pragma once
#include "UIRect.h"

class App;

class Titlebar {
    UIRef uRoot, uCloseBtn, uMaximizeBtn, uMinimizeBtn;
public:
    static constexpr float HEIGHT = 40.0f;

    Titlebar(UIRect& root);
    void Draw(Canvas& canvas);
};
