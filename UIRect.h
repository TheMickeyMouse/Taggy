#pragma once
#include "GUI/Canvas.h"
#include "Utils/Math/Rect.h"

using namespace Quasi;
using namespace Math;
using namespace Graphics;

enum class Dir { TOP, RIGHT, BTM, LEFT };
namespace Dirs {
    Dir Opposite(Dir side);
    bool IsMin(Dir side);
    bool IsX(Dir side);
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

class UIRect {
public:
    Dir side;
    Length length;
    Sides padding, margin;
    bool centerChildren = false;
    Vec<Box<UIRect>> children;

    // computed
    fRect2D rect;
    bool isHovered = false, isChildrenHovered = false;

    static UIRect Root(const fRect2D& root);
    void ComputeLayout();
    bool CheckHover(const fv2& mouse);
    void SetUnhovered();

    bool DrawDebug(Canvas& canvas) const;

    fRect2D GetInnerRect() const;

    UIRect& Pack(Dir childSide, Length childLen, Sides pad = 0, Sides marg = 0, bool center = false);

    // for easy drawing
    __attribute__((always_inline)) auto BeginDraw(Canvas& c) const {
        auto raii = Tuple { c.PushTransform(), c.PushStyles() };
        c.transform.pos = rect.BottomLeft();
        return raii;
    }
};

