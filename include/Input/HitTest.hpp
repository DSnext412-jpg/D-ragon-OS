#pragma once

#include <cstdint>

namespace DragonOS::Input {

struct Point {
    float x{ 0.0f };
    float y{ 0.0f };
};

struct Bounds {
    float x{ 0.0f };
    float y{ 0.0f };
    float width{ 0.0f };
    float height{ 0.0f };

    [[nodiscard]] constexpr float Left()   const noexcept { return x; }
    [[nodiscard]] constexpr float Top()    const noexcept { return y; }
    [[nodiscard]] constexpr float Right()  const noexcept { return x + width; }
    [[nodiscard]] constexpr float Bottom() const noexcept { return y + height; }

    [[nodiscard]] constexpr bool Contains(float px, float py) const noexcept
    {
        return px >= Left() && px < Right() &&
               py >= Top()  && py < Bottom();
    }

    [[nodiscard]] constexpr bool Contains(const Point& pt) const noexcept
    {
        return Contains(pt.x, pt.y);
    }

    [[nodiscard]] constexpr Bounds Inset(float dx, float dy) const noexcept
    {
        return { x + dx, y + dy, width - dx * 2.0f, height - dy * 2.0f };
    }
};

enum class HitTestRegion : uint8_t {
    None,
    TitleBar,
    Client,
    BorderLeft,
    BorderRight,
    BorderTop,
    BorderBottom,
    BorderTopLeft,
    BorderTopRight,
    BorderBottomLeft,
    BorderBottomRight,
    CloseButton,
    MaximizeButton,
    MinimizeButton,
};

}
