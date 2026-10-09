#pragma once
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
    float val;
    enum Unit {
        PX,  // pixels
        PER, // percentage
        FW,  // fraction of width
        FH,  // fraction of height
    } unit;

    float Resolve(const UIRect& parent, Dir side) const;
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
    u32 i;

    operator UIRect&();
    operator const UIRect&() const;
    UIRect& operator*();
    const UIRect& operator*() const;
    UIRect* operator->();
    const UIRect* operator->() const;

    bool operator==(const UIRef& other) const = default;
};

class UIRect {
public:
    Length length;
    Sides padding, margin;
    // computed
    fRect2D rect;

    Dir side;
    bool centerChildrenX = false, centerChildrenY = false;
    // io states
    bool isHovered              : 1 = false,
         isChildrenHovered      : 1 = false,
         isMousePressed         : 1 = false,
         isChildrenMousePressed : 1 = false,
         onClick                : 1 = false,
         onChildrenClick        : 1 = false;
    fv2 clickPos;

    enum : u32 { EMPTY = ~(u32)0 };
    u32 siblingID = EMPTY, childID = EMPTY; // linked list/tree like scheme

    static UIRect Root(const fRect2D& root, UIDoc& doc);
    OptRef<UIRect> Sibling();
    OptRef<UIRect> Child();

    bool HasSiblings() const;
    bool HasChildren() const;

    UISiblingIter Siblings();
    UISiblingIter Children();

    void DrawDebug(Canvas& canvas);

    bool CheckMouseStates(const fv2& mouse, bool pressed, bool clicked);
    void ClearMouseStates(const fv2& mouse, bool pressed);

    fRect2D GetInnerRect() const;

    fv2 RelativePos(const fv2& p) const;

    UIRef Pack(Dir childSide, Length childLen, const UIRectOptions& options = {});

    // for easy drawing
    __attribute__((always_inline)) auto BeginDraw(Canvas& c) const {
        auto t = c.PushTransform();
        c.transform.pos = rect.BottomLeft();
        return t;
    }
};

struct UISiblingIter : IIterator<UIRect&, UISiblingIter> {
    using Item = UIRect&;
    OptRef<UIRect> current;

    UISiblingIter(UIRect& begin) : current(begin) {}

    UIRect& CurrentImpl() const;
    void AdvanceImpl();
    bool CanNextImpl() const;
};

class UIDoc {
public:
    Vec<UIRect> elements; // first is always root
    u32 hovered = NONE, active = NONE; // the element being held

    UIDoc(const fRect2D& root);
private:
    UIRect& operator[](u32 i);
public:
    UIRect& Root();

    u32 GetIDFor(const UIRect& u) const;
    UIRef Add(const UIRect& u);
    UIRef AddChild(u32 i, const UIRect& child);

    void SetRect(const fRect2D& rect);

    void ComputeLayout();
    void CheckMouseStates(const fv2& mouse, bool pressed, bool clicked);
    void DrawDebug(Canvas& canvas);

    inline static OptRef<UIDoc> Instance = nullptr;

    friend UIRect;
    friend UIRef;
};