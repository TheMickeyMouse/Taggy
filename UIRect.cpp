#include "UIRect.h"
#include "Utils/Debug/Logger.h"

float& Dirs::GetBound(fRect2D& r, Dir side) {
    switch (side) {
        case Dir::TOP:   return r.min.y;
        case Dir::RIGHT: return r.max.x;
        case Dir::BTM:   return r.max.y;
        case Dir::LEFT:  return r.min.x;
    }
    Debug::QError$("bad side {}; how", (int)side);
    return r.min.x;
}

Dir Dirs::Opposite(Dir side) {
    switch (side) {
        case Dir::TOP:   return Dir::BTM;
        case Dir::RIGHT: return Dir::LEFT;
        case Dir::BTM:   return Dir::TOP;
        case Dir::LEFT:  return Dir::RIGHT;
    }
    Debug::QError$("bad side {}; how", (int)side);
    return (Dir)-1;
}

bool Dirs::IsMin(Dir side) {
    return side == Dir::TOP || side == Dir::RIGHT;
}

bool Dirs::IsX(Dir side) {
    return side == Dir::LEFT || side == Dir::RIGHT;
}

float Dirs::GetSideLen(const fRect2D& r, Dir side) {
    switch (side) {
        case Dir::TOP:   case Dir::BTM:  return r.Height();
        case Dir::RIGHT: case Dir::LEFT: return r.Width();
    }
    Debug::QError$("bad side {}; how", (int)side);
    return 0;
}

fRect2D Dirs::Cut(fRect2D& r, Dir side, float len) {
    float p;
    fRect2D slice = r;
    switch (side) {
        case Dir::TOP:   p = r.min.y + len; slice.max.y = p; r.min.y = p; break;
        case Dir::RIGHT: p = r.max.x - len; slice.min.x = p; r.max.x = p; break;
        case Dir::BTM:   p = r.max.y - len; slice.min.y = p; r.max.y = p; break;
        case Dir::LEFT:  p = r.min.x + len; slice.max.x = p; r.min.x = p; break;
    }
    return slice;
}

float Length::Resolve(const UIRect& parent, Dir side) const {
    if (unit == PX) return val;
    if (unit == FW || (unit == PER && Dirs::IsX(side))) {
        return val * (parent.rect.Width()  - 2 * parent.padding.x);
    }
    if (unit == FH || (unit == PER && !Dirs::IsX(side))) {
        return val * (parent.rect.Height() - 2 * parent.padding.y);
    }
    Debug::QError$("bad unit {}; how", (int)unit);
    return 0;
}

UIRect UIRect::Root(const fRect2D& root) {
    return { .rect = root };
}

void UIRect::ComputeLayout() {
    fRect2D remainingSpace = rect.Inset(padding);
    for (auto& child : children) {
        // calc the length in the direction of the axis `child.side`
        const float len = child->length.Resolve(*this, child->side);
        // partition `remainingSpace`
        child->rect = Dirs::Cut(remainingSpace, child->side, len);
        // recurse
        child->ComputeLayout();
    }
}

bool UIRect::CheckHover(const fv2& mouse) {
    isHovered = rect.Contains(mouse);
    if (!isHovered) {
        SetUnhovered();
        return false; // the children won't be hovered anyways
    }

    isChildrenHovered = false;
    for (auto& child : children) {
        isChildrenHovered |= child->CheckHover(mouse);
    }
    return true;
}

void UIRect::SetUnhovered() {
    isHovered = false;
    if (isChildrenHovered) {
        isChildrenHovered = false;
        for (auto& child : children) {
            child->SetUnhovered();
        }
    }
}

UIRect& UIRect::Pack(Dir childSide, Length childLen) {
    return *children.Push(Box<UIRect>::New({ childSide, childLen }));
}
