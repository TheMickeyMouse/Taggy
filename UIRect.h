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

class UIRect {
public:
    Dir side;
    Length length;
    fv2 padding;
    Vec<Box<UIRect>> children;

    // computed
    fRect2D rect;
    bool isHovered = false, isChildrenHovered = false;

    static UIRect Root(const fRect2D& root);
    void ComputeLayout();
    bool CheckHover(const fv2& mouse);
    void SetUnhovered();

    UIRect& Pack(Dir childSide, Length childLen);

    // for easy drawing
    __attribute__((always_inline)) auto BeginDraw(Canvas& c) const {
        auto raii = Tuple { c.PushTransform(), c.PushStyles() };
        c.transform.pos = rect.BottomLeft();
        return raii;
    }
};

