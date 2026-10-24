#pragma once
#include "UIRect.h"
#include "GUI/Canvas.h"
#include "Utils/Math/Rect.h"

using namespace Quasi;
using namespace Math;
using namespace Graphics;

enum class Dir : u8 { TOP, RIGHT, BTM, LEFT };
namespace Dirs {
    Dir Opposite(Dir side);
    Dir Cross(Dir side);

    bool IsMin(Dir side);
    bool IsX(Dir side);
    bool IsY(Dir side);

    float& GetBound(fRect2D& r, Dir side);
    float GetSideLen(const fRect2D& r, Dir side);

    fRect2D Cut(fRect2D& r, Dir side, float len);
}

class UIRect;

struct Length {
    float val, val2 = 0;
    enum Unit : u8 {
        PX,  // pixels
        PER, // percentage
        FW,  // fraction of width
        FH,  // fraction of height
        RPER // percentage of remaining
    } unit, unit2 = PX;
    enum Op : u8 { NONE, MIN, MAX } op = NONE;

    Length(float x = 0, Unit u = PX) : val(x), unit(u) {}
    Length(float x, Unit u, float y, Unit v, Op op) : val(x), val2(y), unit(u), unit2(v), op(op) {}

    static float ResolveSingle(const UIRect& parent, const fRect2D& remaining, Dir side, float val, Unit unit);
    float Resolve(const UIRect& parent, const fRect2D& remaining, Dir side) const;

    static Length Min(const Length& a, const Length& b);
    static Length Max(const Length& a, const Length& b);
};

inline Length operator*(float x, Length::Unit u) { return Length { x, u }; }
inline Length operator""_px (long double x) { return Length { (float)x, Length::PX }; }
inline Length operator""_per(long double x) { return Length { (float)x, Length::PER }; }
inline Length operator""_fw (long double x) { return Length { (float)x, Length::FW }; }
inline Length operator""_fh (long double x) { return Length { (float)x, Length::FH }; }

// keeps aggregate initialization
struct SidesBase { float top = 0, right = 0, btm = 0, left = 0; };
struct Sides : SidesBase {
    Sides() = default;
    Sides(float x) : SidesBase(x, x, x, x) {}
    Sides(float x, float y) : SidesBase(y, x, y, x) {}
    Sides(float t, float r, float b, float l) : SidesBase(t, r, b, l) {}
    Sides(SidesBase s) : SidesBase(s) {}

    float GetSide(Dir d) const;
    float GetX() const;
    float GetY() const;

    fRect2D Shrink(const fRect2D& rect) const;
    fRect2D Expand(const fRect2D& rect) const;
};

static const fColor
    WHITE          = 0xdcddde_rgbf,
    LIGHT_GRAY     = 0xabb2bf_rgbf,
    DARK_GRAY      = 0x5c6370_rgbf,
    BACKGROUND     = 0x171a1f_rgbf,
    BACKGROUND_ALT = 0x20242b_rgbf,
    RED            = 0xc51f1f_rgbf,
    ACCENT         = 0x61afef_rgbf;

struct UIRectOptions {
    Sides padding = 0;
    Sides margin = 0;
    bool centerX = false, centerY = false;
};

class UIDoc;
struct UISiblingIter;

struct UIRef {
    enum : u32 { EMPTY = ~(u32)0, ROOT = 0 };
    u32 i = 0;

    UIRef(u32 i = 0) : i(i) {}
    UIRef(UIRect& r);

    operator UIRect&() const;
    UIRect& operator*() const;
    UIRect* operator->() const;
    explicit operator bool() const;
    explicit operator u32() const;

    UIRef Sibling() const;
    UIRef Child() const;
    UIRef Parent() const;
    UISiblingIter Siblings() const;
    UISiblingIter Children() const;

    void Kill() const;
    void TruncChildren(u32 n) const;

    bool operator==(const UIRef& other) const = default;
};

class UIRect {
public:
    fRect2D rect; // computed
    Sides padding, margin;
    Length length;
    UIRef self;

    Dir side;
    bool centerChildrenX = false, centerChildrenY = false;
    // if false, then this object is destroyed
    bool valid = false;

    enum MouseStates {
        NONE = 0,
        HOVERED = 1,
        PRESSED = 2,
        CLICKED = 4,
        CHILDREN_INVOLVED = 8,
    };
    int mouseStates = NONE;

    void SetState(int state, bool val);
    bool GetState(int state) const;
    void ClearMouseStates();
    float ScrollDelta() const;
    fv2 ClickPos() const;

    const UIRef* operator->() const;

    static UIRect Root(const fRect2D& root, UIDoc& doc);

    void DrawDebug(Canvas& canvas);

    void ComputeLayout();

    void MarkDestroyed();
    bool IsValid() const;

    template <usize N>
    Array<UIRef, N> Grab();

    fRect2D GetInnerRect() const;
    fv2 RelativePos(const fv2& p) const;

    UIRef Pack(Dir childSide, Length childLen, Str name, const UIRectOptions& options = {});

    // for easy drawing
    __attribute__((always_inline)) auto BeginDraw(Canvas& c) const {
        auto t = c.PushTransform();
        c.transform.pos += rect.BottomLeft() - self.Parent()->rect.BottomLeft();
        return t;
    }

    void Background(Canvas& c, const fColor& color, float border = 0);
    void Background(Canvas& c, const fColor& color, float tr, float br, float tl, float bl);
    void Label(Canvas& c, Str text, const fColor& color, TextAlign::AlignOptions align = TextAlign::LEFT, bool bold = false);
};

struct UISiblingIter : IIterator<const UIRef, UISiblingIter> {
    using Item = const UIRef;
    UIRef current;

    UISiblingIter(UIRef begin) : current(begin) {}

    UIRef CurrentImpl() const;
    void AdvanceImpl();
    bool CanNextImpl() const;
};

template <usize N> Array<UIRef, N> UIRect::Grab() {
    return self.Children().CollectFirstN<N>();
}

class UIDoc {
public:
    // `elements` are stored continuously, where the first element [0] is the root node
    // each node points to its parent, sibling and child (each may be null)
    // deleted elements have `valid` set to false and point to the next deleted elements in `siblingID`.
    // new elements should replace deleted elements, otherwise push to the back of the array.

    Vec<UIRect> elements; // first is always root

    struct Link { UIRef parent = UIRef::EMPTY, sibling = UIRef::EMPTY, child = UIRef::EMPTY; };
    Vec<Link> links;

    Vec<Str> debugNames;

    UIRef hoveredID = UIRef::EMPTY; // the smallest element hovered
    UIRef focusedID = UIRef::EMPTY; // the smallest element pressed
    UIRef firstEmptySlot = UIRef::EMPTY;

    float globalScrollDelta = 0;
    fv2 globalClickPos;

    UIDoc(const fRect2D& root);
private:
    UIRect& operator[](u32 i);
public:
    UIRect& Root();

    UIRef AddChild(UIRef i, const UIRect& child, Str name);

    UIRef TryReviveSlot(); // reserves a slot for putting a new element
    UIRef AddDirect(const UIRect& child);

    void JoinToDeadLinkedList(UIRef first, UIRef last);
    void Kill(UIRef i); // adds i to the `destroyed` linked list
    void KillAllAfterSibling(UIRef sib);
    void KillAllChildren(UIRef parent);
    void TruncChildren(UIRef parent, u32 n);

    void SetRect(const fRect2D& rect);

    void ComputeLayout(UIRef rootNode = UIRef::ROOT);
    void CheckMouseStates(const fv2& mouse, bool pressed, bool clicked, float scroll);
    void DrawDebug(Canvas& canvas);

    Str DebugNameOf(UIRef i) const;
    void PrettyPrintStructure() const;

    inline static OptRef<UIDoc> Instance = nullptr;

    friend UIRect;
    friend UIRef;
};