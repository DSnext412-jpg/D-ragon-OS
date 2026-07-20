#pragma once

#include <Input/HitTest.hpp>

namespace DragonOS::WindowManager {

enum class SnapRegion {
    None,
    Left,
    Right,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    Full,
};

class SnapLayout final {
public:
    SnapLayout() = delete;

    static constexpr float SnapThreshold = 50.0f;

    static SnapRegion DetectSnapRegion(
        float viewportW, float viewportH,
        float windowX, float windowY,
        float windowW, float windowH,
        float threshold = SnapThreshold) noexcept;

    static Input::Bounds GetSnapBounds(
        SnapRegion region,
        float viewportW, float viewportH) noexcept;

    static bool IsValidSnap(SnapRegion region) noexcept
    {
        return region != SnapRegion::None;
    }
};

} // namespace DragonOS::WindowManager
