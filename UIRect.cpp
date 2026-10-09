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

Dir Dirs::Cross(Dir side) {
    switch (side) {
        case Dir::TOP:   case Dir::BTM:  return Dir::LEFT;
        case Dir::RIGHT: case Dir::LEFT: return Dir::TOP;
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

bool Dirs::IsY(Dir side) {
    return side == Dir::TOP || side == Dir::BTM;
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
        return val * (parent.rect.Width() - parent.padding.GetX());
    }
    if (unit == FH || (unit == PER && Dirs::IsY(side))) {
        return val * (parent.rect.Height() - parent.padding.GetY());
    }
    Debug::QError$("bad unit {}; how", (int)unit);
    return 0;
}

float Sides::GetSide(Dir d) const {
    switch (d) {
        case Dir::TOP:   return top;
        case Dir::RIGHT: return right;
        case Dir::BTM:   return btm;
        case Dir::LEFT:  return left;
    }
    return 0;
}

float Sides::GetX() const {
    return right + left;
}

float Sides::GetY() const {
    return top + btm;
}

fRect2D Sides::Shrink(const fRect2D& rect) const {
    return {
        { rect.min.x + left,  rect.min.y + top },
        { rect.max.x - right, rect.max.y - btm }
    };
}

fRect2D Sides::Expand(const fRect2D& rect) const {
    return {
        { rect.min.x - left,  rect.min.y - top },
        { rect.max.x + right, rect.max.y + btm }
    };
}

UIRef::operator UIRect&() {
    return (*UIDoc::Instance)[i];
}

UIRef::operator const UIRect&() const {
    return (*UIDoc::Instance)[i];
}

UIRect& UIRef::operator*() {
    return (*UIDoc::Instance)[i];
}

const UIRect& UIRef::operator*() const {
    return (*UIDoc::Instance)[i];
}

UIRect* UIRef::operator->() {
    return &(*UIDoc::Instance)[i];
}

const UIRect* UIRef::operator->() const {
    return &(*UIDoc::Instance)[i];
}

UIRect UIRect::Root(const fRect2D& root, UIDoc& doc) {
    return { .rect = root };
}

OptRef<UIRect> UIRect::Sibling() {
    return (*UIDoc::Instance)[siblingID];
}

OptRef<UIRect> UIRect::Child() {
    return (*UIDoc::Instance)[childID];
}

bool UIRect::HasSiblings() const {
    return siblingID != ~0;
}

bool UIRect::HasChildren() const {
    return childID != ~0;
}

UISiblingIter UIRect::Siblings() {
    return { Sibling() };
}

UISiblingIter UIRect::Children() {
    return { Child() };
}

void UIRect::DrawDebug(Canvas& canvas) {
    static const fColor
        DBG_MARGIN_COLOR  = 0xecc74b_rgb,
        DBG_PADDING_COLOR = 0xa9e75f_rgb,
        DBG_CENTER_COLOR  = 0x51abf5_rgb,
        DBG_CENTER_PRESSED_COLOR = 0xc678dd_rgb;
    static constexpr float DBG_FNT_SIZE = 12.0f;

    canvas.NoStroke();

    if (isMousePressed) {
        canvas.Fill(DBG_CENTER_PRESSED_COLOR.AddAlpha(onClick ? 0.4f : 0.2f));
    } else {
        canvas.Fill(DBG_CENTER_COLOR.AddAlpha(0.2f));
    }
    const fRect2D center = padding.Shrink(rect);
    canvas.DrawRect(center);

    canvas.NoFill();
    canvas.Stroke(DBG_MARGIN_COLOR);
    canvas.StrokeWeight(1);
    canvas.DrawRect(margin.Expand(rect));

    canvas.Stroke(DBG_PADDING_COLOR);
    canvas.DrawRect(rect);

    canvas.Stroke(1);
    canvas.DrawText(
        Text::Format("{}x{}", center.Width(), center.Height()),
        DBG_FNT_SIZE, center.BottomLeft(), { TextAlign::LEFT | TextAlign::VBOTTOM });
}

bool UIRect::CheckMouseStates(const fv2& mouse, bool pressed, bool clicked) {
    isHovered = rect.Contains(mouse);
    if (!isHovered) {
        ClearMouseStates(mouse, pressed);
        return false; // the children won't be hovered anyways
    }

    onClick = clicked;
    isMousePressed = isMousePressed ? pressed : clicked;
    if (pressed) clickPos = mouse - rect.BottomLeft();

    isChildrenHovered = isChildrenMousePressed = onChildrenClick = false;
    for (auto& child : Children()) {
        isChildrenHovered |= child.CheckMouseStates(mouse, pressed, clicked);
        isChildrenMousePressed |= child.isMousePressed;
        onChildrenClick |= child.onChildrenClick;
    }

    if (!isChildrenHovered)
        UIDoc::Instance->hovered = UIDoc::Instance->GetIDFor(*this);

    return true;
}

void UIRect::ClearMouseStates(const fv2& mouse, bool pressed) {
    isHovered = onClick = onChildrenClick = false;

    // we should keep the ui element 'pressed' even if the mouse leaves the hitbox
    if (isMousePressed && !isChildrenMousePressed) {
        // keep updating press state & mouse position
        isMousePressed = pressed;
        clickPos = mouse - rect.BottomLeft();
    } else if (isChildrenMousePressed) {
        goto clearChildren;
    } else {
        isMousePressed = false;
    }

    if (isChildrenHovered) {
        clearChildren:

        isChildrenHovered = false;
        for (auto& child : Children()) {
            child.ClearMouseStates(mouse, pressed);
        }
    }
}

fRect2D UIRect::GetInnerRect() const {
    return padding.Shrink(rect);
}

fv2 UIRect::RelativePos(const fv2& p) const {
    return p - rect.BottomLeft();
}

UIRef UIRect::Pack(Dir childSide, Length childLen, const UIRectOptions& options) {
    const u32 id = UIDoc::Instance->GetIDFor(*this);
    return UIDoc::Instance->AddChild(id, UIRect {
        .length = childLen,
        .padding = options.padding,
        .margin = options.margin,
        .side = childSide,
        .centerChildrenX = options.centerX,
        .centerChildrenY = options.centerY,
    });
}

UIRect& UISiblingIter::CurrentImpl() const {
    return (UIRect&)*current;
}

void UISiblingIter::AdvanceImpl() {
    current = current->Sibling();
}

bool UISiblingIter::CanNextImpl() const {
    return (bool)current;
}

UIDoc::UIDoc(const fRect2D& root) {
    elements.Push(UIRect::Root(root, *this));
    Instance = *this;
}

UIRect& UIDoc::operator[](u32 i) {
    return i != ~0 ? OptRefs::SomeRef(elements[i]) : nullptr;
}

UIRect& UIDoc::Root() {
    return elements[0];
}

u32 UIDoc::GetIDFor(const UIRect& u) const {
    return elements.AsSpan().Unaddress(&u);
}

UIRef UIDoc::Add(const UIRect& u) {
    const u32 i = elements.Length();
    elements.Push(u);
    return { i };
}

UIRef UIDoc::AddChild(u32 i, const UIRect& child) {
    const u32 childID = elements.Length();
    elements.Push(child);
    UIRect& parent = elements[i];

    if (parent.HasChildren()) {
        u32 lastChild = parent.childID, next;
        while ((next = elements[lastChild].siblingID) != UIRect::EMPTY) lastChild = next;
        elements[lastChild].siblingID = childID;
    } else {
        parent.childID = childID;
    }

    return { childID };
}

void UIDoc::SetRect(const fRect2D& rect) {
    Root().rect = rect;
}

void UIDoc::ComputeLayout() {
    Vec<u32> queue;
    queue.Push(0);

    while (auto currentID = queue.TryTake()) {
        // Debug::QInfo$("eval {}", *currentID);
        UIRect& curr = elements[*currentID];

        fRect2D remainingSpace = curr.padding.Shrink(curr.rect);
        u32 currentStackSize = queue.Length();
        for (u32 i = curr.childID; i != UIRect::EMPTY; i = elements[i].siblingID) {
            UIRect& child = elements[i];
            // calc the length in the direction of the axis `child.side`
            const float len = child.length.Resolve(curr, child.side);
            const float marginExtra = child.margin.GetSide(Dirs::Opposite(child.side));
            // partition `remainingSpace`
            child.rect = child.margin.Shrink(Dirs::Cut(remainingSpace, child.side, len + marginExtra));

            queue.Push(i);
        }
        if (queue.Length() != currentStackSize) queue.Skip(currentStackSize).Reverse();

        if ((curr.centerChildrenX || curr.centerChildrenY) && curr.HasChildren()) {
            const fv2 offset = {
                curr.centerChildrenX ? remainingSpace.Width() / 2 : 0,
                curr.centerChildrenY ? remainingSpace.Height() / 2 : 0,
            };
            for (UIRect& child : curr.Children()) {
                if ((curr.centerChildrenX && Dirs::IsX(child.side)) || (curr.centerChildrenY && Dirs::IsY(child.side))) {
                    child.rect = child.rect.Move(offset);
                }
            }
        }
    }
}

void UIDoc::CheckMouseStates(const fv2& mouse, bool pressed, bool clicked) {
    Root().CheckMouseStates(mouse, pressed, clicked);
}

void UIDoc::DrawDebug(Canvas& canvas) {
    if (hovered == UIRect::EMPTY) return;
    elements[hovered].DrawDebug(canvas);
}