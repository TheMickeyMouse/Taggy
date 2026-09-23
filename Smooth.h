#pragma once
#include "Utils/Math/Color.h"

using namespace Quasi;
using namespace Math;;

namespace Anim {
    inline float Lerp(float a, float b, float t) { return a + (b - a) * t; }
    template <class T> T Lerp(const T& a, const T& b, float t) { return a.Lerp(b, t); }

    template <class T> auto FloatifyVal() {
        if constexpr (Numeric<T>) return float {};
        else if constexpr (requires (T x) { { Vector { x } } -> SameAs<T>; }) return Vector<float, T::Dim> {};
        else if constexpr (requires (T x) { { IColor { x } } -> SameAs<T>; }) return IColor<float, T::Dim == 4> {};
        else return nullptr;
    }

    template <class T> using Floatify = decltype(FloatifyVal<T>());
}

template <class T>
struct Smooth {
    using F = Anim::Floatify<T>;

    T target;
    F displayValue;
    float smoothness = 1.0f;

    Smooth(T x) : target(x), displayValue(x) {}

    void Update(float dt) {
        displayValue = Anim::Lerp(displayValue, (F)target, smoothness * dt);
        if (std::abs(displayValue - target) <= f32s::DELTA) {
            displayValue = target;
        }
    }

    void Jump() { displayValue = (F)target; }

    bool IsStable() const { return displayValue == target; }
};
