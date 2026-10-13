#include "UIRect.h"

#include "App.h"
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

float Length::ResolveSingle(const UIRect& parent, const fRect2D& remaining, Dir side, float val, Unit unit) {
    if (unit == PX) return val;
    if (unit == FW || (unit == PER && Dirs::IsX(side))) {
        return val * (parent.rect.Width() - parent.padding.GetX());
    }
    if (unit == FH || (unit == PER && Dirs::IsY(side))) {
        return val * (parent.rect.Height() - parent.padding.GetY());
    }
    if (unit == RPER) {
        return Dirs::IsX(side) ? remaining.Size().x : remaining.Size().y;
    }
    Debug::QError$("bad unit {}; how", (int)unit);
    return 0;
}

float Length::Resolve(const UIRect& parent, const fRect2D& remaining, Dir side) const {
    switch (op) {
        case NONE: return ResolveSingle(parent, remaining, side, val, unit);
        case MIN:  return std::min(ResolveSingle(parent, remaining, side, val, unit), ResolveSingle(parent, remaining, side, val2, unit2));
        case MAX:  return std::max(ResolveSingle(parent, remaining, side, val, unit), ResolveSingle(parent, remaining, side, val2, unit2));
    }
    Debug::QError$("bad op {}; how", (int)op);
    return 0;
}

Length Length::Min(const Length& a, const Length& b) {
    return {
        a.val, a.unit,
        b.val, b.unit,
        MIN
    };
}

Length Length::Max(const Length& a, const Length& b) {
    return {
        a.val, a.unit,
        b.val, b.unit,
        MAX
    };
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

OptRef<UIRect> UIRect::Parent() {
    return (*UIDoc::Instance)[parentID];
}

OptRef<const UIRect> UIRect::Sibling() const {
    return (*UIDoc::Instance)[siblingID];
}

OptRef<const UIRect> UIRect::Child() const {
    return (*UIDoc::Instance)[childID];
}

OptRef<const UIRect> UIRect::Parent() const {
    return (*UIDoc::Instance)[parentID];
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

void UIRect::ClearMouseStates() {
    isHovered              =
    isChildrenHovered      =
    isMousePressed         =
    isChildrenMousePressed =
    onClick                =
    onChildrenClick        = false;
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

void UIRect::Background(Canvas& c, const fColor& color, float border) {
    [[maybe_unused]] const auto _ = c.PushStyles();

    c.NoStroke();
    c.Fill(color);
    if (border == 0.0f) {
        c.DrawRect({ 0, rect.Size() });
    } else {
        c.DrawRoundedRect({ 0, rect.Size() }, border);
    }
}

void UIRect::Background(Canvas& c, const fColor& color, float tr, float br, float tl, float bl) {
    [[maybe_unused]] const auto _ = c.PushStyles();

    c.NoStroke();
    c.Fill(color);
    c.DrawVarRoundedRect({ 0, rect.Size() }, tr, br, tl, bl);
}

void UIRect::Label(Canvas& c, Str text, const fColor& color, TextAlign::AlignOptions align, bool bold) {
    [[maybe_unused]] const auto _ = BeginDraw(c);
    [[maybe_unused]] const auto _2 = c.PushStyles();

    const fv2 size = rect.Size();
    c.Stroke(color);
    const Font& font = bold ? App::GetFontBold() : App::GetFont();
    c.SetFont(font);

    // ellipsis rendering
    const auto& pointMetric = font.GetGlyphRect('.');
    const float relativeSize      = size.y * 64.0f / (float)font.FontSize(),
                pointSize         = pointMetric.advance.x,
                maxLength         = size.x / relativeSize,
                preEllipsesLength = maxLength - pointSize;
    float len = 0.0f, preLen;
    usize i = 0;
    for (; i < text.Length(); ++i) {
        const float w = font.CalcCharWidth(text[i]);
        if (len + w > preEllipsesLength) goto renderEllipses;
        len += w;
    }
    goto normalRender;

    renderEllipses:
    preLen = len - (float)(i - text.First(i).TrimEnd().Length()) * pointSize;
    for (usize j = i; j < text.Length(); ++j) {
        if (len > maxLength) { // actually render ellipses
            c.DrawText(text.First(i), size.y, 0, { align, size });

            // normal "..." takes up too much space; fake it instead
            const float r = font.CalcCharWidth('.') * relativeSize / 8.0f;
            fv2 p = {
                preLen * relativeSize + 1.5f * r,
                ((float)font.GetMetric().ascend / 64.0f - pointMetric.offset.y) * relativeSize // moves pen to '.' position
            };

            // for centering text
            p.y -= 0.5f * (size.y - (float)font.GetMetric().fontHeight * relativeSize / 64.0f) + r * 1.5f;

            c.StrokeWeight(r * 0.75f);
            c.DrawPoint(p); p.x += 2.5f * r;
            c.DrawPoint(p); p.x += 2.5f * r;
            c.DrawPoint(p);
            return;
        }
        len += font.CalcCharWidth(text[j]);
    }

    normalRender:
    c.DrawText(text, size.y, 0, { align, size });
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
    return elements[UIRect::ROOT];
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
    UIRect& newChild = elements.Push(child);
    UIRect& parent = elements[i];

    newChild.parentID = i;

    if (parent.HasChildren()) {
        u32 lastChild = parent.childID, next;
        while ((next = SiblingOf(lastChild)) != UIRect::EMPTY) lastChild = next;
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
    queue.Push(UIRect::ROOT);

    while (auto currentID = queue.TryTake()) {
        // Debug::QInfo$("eval {}", *currentID);
        UIRect& curr = elements[*currentID];

        fRect2D remainingSpace = curr.padding.Shrink(curr.rect);
        u32 currentStackSize = queue.Length();
        for (u32 i = curr.childID; i != UIRect::EMPTY; i = SiblingOf(i)) {
            UIRect& child = elements[i];
            // calc the length in the direction of the axis `child.side`
            const float len = child.length.Resolve(curr, remainingSpace, child.side);
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

void UIDoc::CheckMouseStates(const fv2& mouse, bool pressed, bool clicked, float scroll) {
    // Root().CheckMouseStates(mouse, pressed, clicked, scroll);

    for (auto& e : elements) e.ClearMouseStates();

    // keep focused elements focused, even if the mouse leaves the hitbox, as long as the mouse is still pressed
    if (focusedID != UIRect::EMPTY && pressed) {
        UIRect& focus = elements[focusedID];
        focus.isMousePressed = pressed;
        focus.clickPos = mouse - focus.rect.BottomLeft();
    } else {
        focusedID = UIRect::EMPTY;
    }

    // next is always a child of search
    u32 search = UIRect::ROOT, next = UIRect::ROOT;
    while (next != UIRect::EMPTY) {
        UIRect& curr = elements[next];

        if (curr.rect.Contains(mouse)) { // search deeper
            curr.isHovered = true;
            curr.onClick = clicked;
            // when ur mouse is held and hoveres over a ui elm,
            // without this check it would register as a mouse press (we dont want this)
            // should only trigger when the mouse is pressed inside the elm to begin with
            curr.isMousePressed = curr.isMousePressed ? pressed : clicked;
            if (curr.isMousePressed) focusedID = next;
            if (pressed) curr.clickPos = mouse - curr.rect.BottomLeft();
            curr.scrollDelta = scroll;

            search = next;
            next = curr.childID;
        } else { // search wider
            next = curr.siblingID;
        }
    }
    // search is the last element hovered; iterate backwards
    hoveredID = search;
    UIRect& hov = elements[hoveredID];

    const bool isChildPressed = hov.isMousePressed;
    while ((search = ParentOf(search)) != UIRect::EMPTY) {
        UIRect& curr = elements[search];
        curr.isChildrenHovered = true;
        curr.isChildrenMousePressed = isChildPressed;
    }
}

void UIDoc::DrawDebug(Canvas& canvas) {
    if (hoveredID == UIRect::EMPTY) return;
    elements[hoveredID].DrawDebug(canvas);
}

u32 UIDoc::SiblingOf(u32 x) {
    return elements[x].siblingID;
}

u32 UIDoc::ChildOf(u32 x) {
    return elements[x].childID;
}

u32 UIDoc::ParentOf(u32 x) {
    return elements[x].parentID;
}
