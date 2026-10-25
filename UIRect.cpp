#include "UIRect.h"

#include "App.h"
#include "glp.h"
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
        return Dirs::IsX(side) ? remaining.Width() : remaining.Height();
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

UIRef::UIRef(UIRect& r) : i(r.self) {}

UIRef::operator UIRect&() const {
    return (*UIDoc::Instance)[i];
}
UIRect& UIRef::operator*() const {
    return (*UIDoc::Instance)[i];
}
UIRect* UIRef::operator->() const {
    return &(*UIDoc::Instance)[i];
}

UIRef::operator bool() const {
    return i != EMPTY;
}

UIRef::operator unsigned() const {
    return i;
}

UIRef UIRef::Sibling() const {
    return UIDoc::Instance->links[i].sibling;
}

UIRef UIRef::Child() const {
    return UIDoc::Instance->links[i].child;
}

UIRef UIRef::Parent() const {
    return UIDoc::Instance->links[i].parent;
}

UISiblingIter UIRef::Siblings() const {
    return { Sibling() };
}

UISiblingIter UIRef::Children() const {
    return { Child() };
}

UITreeIter UIRef::ChildrenRecr() const {
    return { *this };
}

void UIRef::Kill() const {
    UIDoc::Instance->Kill(*this);
}

void UIRef::TruncChildren(u32 n) const {
    UIDoc::Instance->TruncChildren(*this, n);
}

void UIRect::SetState(int state, bool val) {
    mouseStates &= ~state;
    mouseStates |= val ? state : 0;
}

bool UIRect::GetState(int state) const {
    return (mouseStates & state) == state;
}

void UIRect::ClearMouseStates() {
    mouseStates = 0;
}

float UIRect::ScrollDelta() const {
    return UIDoc::Instance->globalScrollDelta;
}

fv2 UIRect::ClickPos() const {
    return UIDoc::Instance->globalClickPos - rect.BottomLeft();
}

const UIRef* UIRect::operator->() const {
    return &self;
}

UIRect UIRect::Root(const fRect2D& root, UIDoc& doc) {
    return { .rect = root };
}

void UIRect::DrawDebug(Canvas& canvas) {
    static const fColor
        DBG_MARGIN_COLOR  = 0xecc74b_rgb,
        DBG_PADDING_COLOR = 0xa9e75f_rgb,
        DBG_CENTER_COLOR  = 0x51abf5_rgb,
        DBG_CENTER_PRESSED_COLOR = 0xc678dd_rgb;
    static constexpr float DBG_FNT_SIZE = 12.0f;

    canvas.NoStroke();

    if (mouseStates & PRESSED) {
        canvas.Fill(DBG_CENTER_PRESSED_COLOR.AddAlpha(mouseStates & CLICKED ? 0.4f : 0.2f));
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

void UIRect::ComputeLayout() {
    UIDoc::Instance->ComputeLayout(self);
}

void UIRect::MarkDestroyed() {
    valid = false;
}

bool UIRect::IsValid() const {
    return valid;
}

fRect2D UIRect::GetInnerRect() const {
    return padding.Shrink(rect);
}

fv2 UIRect::RelativePos(const fv2& p) const {
    return p - rect.BottomLeft();
}

UIRef UIRect::Pack(Dir childSide, Length childLen, Str name, const UIRectOptions& options) {
    return UIDoc::Instance->AddChild(self, UIRect {
        .padding = options.padding,
        .margin = options.margin,
        .length = childLen,
        .side = childSide,
        .centerChildrenX = options.centerX,
        .centerChildrenY = options.centerY,
    }, name);
}

void UIRect::Background(Canvas& c, const fColor& color, float border) const  {
    [[maybe_unused]] const auto _ = c.PushStyles();

    c.NoStroke();
    c.Fill(color);
    if (border == 0.0f) {
        c.DrawRect({ 0, rect.Size() });
    } else {
        c.DrawRoundedRect({ 0, rect.Size() }, border);
    }
}

void UIRect::Background(Canvas& c, const fColor& color, float tr, float br, float tl, float bl) const {
    [[maybe_unused]] const auto _ = c.PushStyles();

    c.NoStroke();
    c.Fill(color);
    c.DrawVarRoundedRect({ 0, rect.Size() }, tr, br, tl, bl);
}

void UIRect::Capsule(Canvas& c, const fColor& color) const {
    [[maybe_unused]] const auto _2 = c.PushStyles();

    const float r = rect.Width() / 2;
    c.Stroke(color);
    c.StrokeWeight(r);
    c.DrawLine(r, rect.Size() - r);
}

void UIRect::Label(Canvas& c, Str text, const fColor& color, TextAlign::AlignOptions align, bool bold) const {
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

UIRef UISiblingIter::CurrentImpl() const {
    return current;
}

void UISiblingIter::AdvanceImpl() {
    current = current.Sibling();
}

bool UISiblingIter::CanNextImpl() const {
    return (bool)current;
}

UIRef UITreeIter::CurrentImpl() const {
    return current;
}

void UITreeIter::AdvanceImpl() {
    current = next;
    if (const auto child = next.Child()) next = child;
    else while (true) {
        if (const auto sibling = next.Sibling()) return (void)(next = sibling);
        if ((next = next.Parent()) == parent) return;
    }
}

bool UITreeIter::CanNextImpl() const {
     return next != parent;
}

UIDoc::UIDoc(const fRect2D& root) {
    elements.Push(UIRect::Root(root, *this));
    links.Push({});
    debugNames.Push("[root]");
    Instance = *this;
}

UIRect& UIDoc::operator[](u32 i) {
    return i != ~0 ? OptRefs::SomeRef(elements[i]) : nullptr;
}

UIRect& UIDoc::Root() {
    return elements[UIRef::ROOT];
}

UIRef UIDoc::AddChild(UIRef i, const UIRect& child, Str name) {
    const UIRef childID = AddDirect(child);

    links[(u32)childID] = { .parent = i, .sibling = UIRef::EMPTY, .child = UIRef::EMPTY };

    if (i.Child()) {
        UIRef lastChild = i.Child(), next;
        while ((next = lastChild.Sibling())) lastChild = next;
        links[(u32)lastChild].sibling = childID; // make `childID` the sibling of the last child of `i`
    } else {
        links[(u32)i].child = childID; // make `childID` the child of `i`
    }

    debugNames[(u32)childID] = name;

    return { childID };
}

UIRef UIDoc::TryReviveSlot() {
    if (!firstEmptySlot) return UIRef::EMPTY;

    const UIRef freeSlot = firstEmptySlot;
    firstEmptySlot = freeSlot.Sibling();

    return freeSlot;
}

UIRef UIDoc::AddDirect(const UIRect& child) {
    UIRef slot = TryReviveSlot();
    if (slot) {
        (*slot = child).self = slot;
    } else {
        slot = elements.Length();
        elements.Push(child).self = slot;
        links.Push({});
        debugNames.Push({});
    }
    return slot;
}

void UIDoc::JoinToDeadLinkedList(UIRef first, UIRef last) {
    links[(u32)last].sibling = firstEmptySlot;
    firstEmptySlot = first;
}

void UIDoc::Kill(UIRef i) {
    if (!i->IsValid()) return;
    i->MarkDestroyed();
    JoinToDeadLinkedList(i, i);
}

void UIDoc::KillAllAfterSibling(UIRef sib) {
    if (!sib) return;
    // pre-order tree traversal
    // R
    // ├─ A
    // │  ├─ A1
    // │  ├─ A2
    // │  │  ├── A2.1
    // │  │  └── A2.2
    // │  └─ A3
    // │     └── A3.1
    // ├─ B
    // └─ C
    // (sib is A) needs to become linked list of A -> A1 -> A2 -> A2.1 -> A2.2 -> A3 -> A3.1 -> B -> C
    const UIRef first = sib, commonParent = sib.Parent();
    UIRef last;

    // PrettyPrintStructure();
    while (true) {
        sib->MarkDestroyed();
        if (sib.Child()) {
            // venture deeper
            // remember afterwards one needs to
            //      1. join sib to sib.child as siblings
            //      2. join sib's last child (A3.1) to the next one (B)
            //      3. continue onward by moving back to sib (parent)'s sibling
            sib = sib.Child();
            continue;
        }

        if (sib.Sibling()) {
            sib = sib.Sibling(); // no reattaching needed
        } else { // end of the road; go back
            last = sib;

            UIRef outerSibling;
            do {
                if (sib.Parent() == commonParent) goto exit; // all siblings under `commonParent` explored; halt
                sib = sib.Parent();

                // perform the steps mentioned above
                outerSibling = sib.Sibling();
                links[(u32)sib].sibling = sib.Child();
            } while (!outerSibling); // need to find sibling to iterate to

            sib = outerSibling;
            links[(u32)last].sibling = sib; // attach A3.1 to B
        }
    }
    exit:
    JoinToDeadLinkedList(first, last);
}

void UIDoc::KillAllChildren(UIRef parent) {
    KillAllAfterSibling(parent.Child());
    links[(u32)parent].child = UIRef::EMPTY;
}

void UIDoc::TruncChildren(UIRef parent, u32 n) {
    if (n == 0) KillAllChildren(parent);

    UISiblingIter it = parent.Children();
    it.AdvanceBy(n - 1);
    KillAllAfterSibling((*it).Sibling());
    links[(u32)*it].sibling = UIRef::EMPTY;
}

void UIDoc::SetRect(const fRect2D& rect) {
    Root().rect = rect;
}

void UIDoc::ComputeLayout(UIRef rootNode) {
    Vec<UIRef> queue;
    queue.Push(rootNode);

    while (auto currID = queue.TryTake()) {
        UIRect& curr = **currID;

        fRect2D remainingSpace = curr.padding.Shrink(curr.rect);
        const u32 currentStackSize = queue.Length();
        for (UIRef i = currID->Child(); i; i = i.Sibling()) {
            UIRect& child = *i;
            // calc the length in the direction of the axis `child.side`
            const float len = child.length.Resolve(curr, remainingSpace, child.side);
            const float marginExtra = child.margin.GetSide(Dirs::Opposite(child.side));
            // partition `remainingSpace`
            child.rect = child.margin.Shrink(Dirs::Cut(remainingSpace, child.side, len + marginExtra));

            queue.Push(i);
        }
        if (queue.Length() != currentStackSize) queue.Skip(currentStackSize).Reverse();

        if ((curr.centerChildrenX || curr.centerChildrenY) && currID->Child()) {
            const fv2 offset = {
                curr.centerChildrenX ? remainingSpace.Width() / 2 : 0,
                curr.centerChildrenY ? remainingSpace.Height() / 2 : 0,
            };
            for (UIRef i : curr->Children()) {
                UIRect& child = *i;
                if ((curr.centerChildrenX && Dirs::IsX(child.side)) || (curr.centerChildrenY && Dirs::IsY(child.side))) {
                    child.rect = child.rect.Move(offset);
                }
            }
        }
    }
}

void UIDoc::CheckMouseStates(const fv2& mouse, bool pressed, bool clicked, float scroll) {
    // Root().CheckMouseStates(mouse, pressed, clicked, scroll);
    globalScrollDelta = scroll;
    globalClickPos = mouse;

    for (auto& e : elements) e.ClearMouseStates();

    // keep focused elements focused, even if the mouse leaves the hitbox, as long as the mouse is still pressed
    if (focusedID && pressed) {
        focusedID->SetState(UIRect::PRESSED, pressed);
    } else {
        focusedID = UIRef::EMPTY;
    }

    // next is always a child of search
    UIRef search = UIRef::ROOT, next = UIRef::ROOT;
    while (next) {
        UIRect& curr = *next;

        if (curr.rect.Contains(mouse)) { // search deeper
            curr.SetState(UIRect::HOVERED, true);
            curr.SetState(UIRect::CLICKED, clicked);
            // when ur mouse is held and hoveres over a ui elm,
            // without this check it would register as a mouse press (we dont want this)
            // should only trigger when the mouse is pressed inside the elm to begin with
            curr.SetState(UIRect::PRESSED, curr.Pressed() ? pressed : clicked);
            if (curr.mouseStates & UIRect::PRESSED) focusedID = next;

            search = next;
            next = next.Child();
        } else { // search wider
            next = next.Sibling();
        }
    }
    // search is the last element hovered; iterate backwards
    hoveredID = search;

    while ((search = search.Parent())) {
        search->SetState(UIRect::CHILDREN_INVOLVED, true);
    }
}

void UIDoc::DrawDebug(Canvas& canvas) {
    if (!hoveredID) return;
    hoveredID->DrawDebug(canvas);
}

Str UIDoc::DebugNameOf(UIRef i) const {
    return debugNames[(u32)i];
}

void UIDoc::PrettyPrintStructure() const {
    Vec<Tuple<UIRef, u32, u32>> queue;
    queue.Push({ UIRef::ROOT, 0, 0 });

    Debug::QDebug$("dumping ui structure ({} nodes total; {} bytes):", elements.Length(), elements.Length() * (sizeof(UIRect) + sizeof(Link) + sizeof(Str)));
    constexpr char INDENT_STRING[] = "│  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  ";
    while (auto process = queue.TryTake()) {
        const auto [currID, nested, childIdx] = *process;

        if (nested > 24) { Debug::QError$("very high nesting! aborting..."); return; }

        if (nested) {
            Text::Print("{}{}─ ",
                Str::Slice(INDENT_STRING, Q_STRLIT_LEN("│  ") * (nested - 1)),
                (const char*)(currID.Sibling() ? "├" : "└")
            );
        }

        const Str debugName = DebugNameOf(currID);
        if (debugName.EndsWith('#')) Text::PrintLn("{}{} [{}]", debugName, childIdx, (u32)currID);
        else Text::PrintLn("{} [{}]", debugName, (u32)currID);

        const u32 currentStackSize = queue.Length();
        u32 n = 0;
        for (UIRef i = currID.Child(); i; i = i.Sibling()) {
            queue.Push({ i, nested + 1, n++ });
        }
        if (queue.Length() != currentStackSize) queue.Skip(currentStackSize).Reverse();
    }
    Text::Print("dead list: ");
    u32 numDead = 0;
    for (UIRef dead : UISiblingIter(firstEmptySlot)) {
        Text::Print("{} -> ", (u32)dead);
        ++numDead;
    }
    Text::PrintLn("[end] ({} total).", numDead);

    Debug::QDebug$("finished ui dump");
}
