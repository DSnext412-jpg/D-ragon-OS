#pragma once

#include <cstdint>
#include <d2d1.h>
#include <algorithm>

namespace DragonOS::DragonUI {

enum class Visibility : uint8_t { Visible, Hidden, Collapsed };

enum class Alignment : uint8_t { Start, Center, End, Stretch };

enum class Orientation : uint8_t { Horizontal, Vertical };

struct Thickness {
    float left{};
    float top{};
    float right{};
    float bottom{};

    constexpr Thickness() = default;
    constexpr Thickness(float uniform) : left{uniform}, top{uniform}, right{uniform}, bottom{uniform} {}
    constexpr Thickness(float l, float t, float r, float b) : left{l}, top{t}, right{r}, bottom{b} {}

    [[nodiscard]] constexpr float Horizontal() const { return left + right; }
    [[nodiscard]] constexpr float Vertical() const { return top + bottom; }
};

struct DesiredSize {
    float width{};
    float height{};
};

struct LayoutSlot {
    float x{};
    float y{};
    float width{};
    float height{};

    constexpr LayoutSlot() = default;
    constexpr LayoutSlot(float x, float y, float w, float h) : x{x}, y{y}, width{w}, height{h} {}

    [[nodiscard]] explicit operator D2D1_RECT_F() const {
        return D2D1_RECT_F{x, y, x + width, y + height};
    }

    [[nodiscard]] static LayoutSlot FromD2D(const D2D1_RECT_F& r) {
        return {r.left, r.top, r.right - r.left, r.bottom - r.top};
    }

    [[nodiscard]] LayoutSlot Inset(const Thickness& t) const {
        return {
            x + t.left,
            y + t.top,
            (std::max)(0.0f, width - t.Horizontal()),
            (std::max)(0.0f, height - t.Vertical())
        };
    }
};

} // namespace
