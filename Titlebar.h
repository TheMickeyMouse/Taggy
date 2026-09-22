#pragma once
#include "UIRect.h"

class Titlebar {
    Ref<UIRect> bar, closeBtn, maximizeBtn, minimizeBtn;
public:
    Titlebar(UIRect& titlebar);

    void Draw(Canvas& canvas);
};
