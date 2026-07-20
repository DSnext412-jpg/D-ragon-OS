#include <WindowManager/SnapLayout.hpp>

#include <algorithm>

namespace DragonOS::WindowManager {

SnapRegion SnapLayout::DetectSnapRegion(
    float viewportW, float viewportH,
    float windowX, float windowY,
    float windowW, float windowH,
    float threshold) noexcept
{
    if (viewportW <= 0.0f || viewportH <= 0.0f)
    {
        return SnapRegion::None;
    }

    const float leftDist   = windowX;
    const float rightDist  = viewportW - (windowX + windowW);
    const float topDist    = windowY;
    const float bottomDist = viewportH - (windowY + windowH);

    const bool nearLeft   = leftDist <= threshold;
    const bool nearRight  = rightDist <= threshold;
    const bool nearTop    = topDist <= threshold;
    const bool nearBottom = bottomDist <= threshold;

    const float midY = viewportH * 0.5f;

    if (nearTop && leftDist <= threshold * 0.5f && rightDist <= threshold * 0.5f)
    {
        return SnapRegion::Full;
    }

    if (nearTop && nearLeft)
    {
        return (windowY < midY) ? SnapRegion::TopLeft : SnapRegion::BottomLeft;
    }
    if (nearTop && nearRight)
    {
        return (windowY < midY) ? SnapRegion::TopRight : SnapRegion::BottomRight;
    }
    if (nearBottom && nearLeft)
    {
        return (windowY + windowH * 0.5f < midY) ? SnapRegion::TopLeft : SnapRegion::BottomLeft;
    }
    if (nearBottom && nearRight)
    {
        return (windowY + windowH * 0.5f < midY) ? SnapRegion::TopRight : SnapRegion::BottomRight;
    }

    if (nearLeft)  return SnapRegion::Left;
    if (nearRight) return SnapRegion::Right;

    return SnapRegion::None;
}

Input::Bounds SnapLayout::GetSnapBounds(
    SnapRegion region,
    float viewportW, float viewportH) noexcept
{
    switch (region)
    {
    case SnapRegion::Left:
        return { 0.0f, 0.0f, viewportW * 0.5f, viewportH };
    case SnapRegion::Right:
        return { viewportW * 0.5f, 0.0f, viewportW * 0.5f, viewportH };
    case SnapRegion::TopLeft:
        return { 0.0f, 0.0f, viewportW * 0.5f, viewportH * 0.5f };
    case SnapRegion::TopRight:
        return { viewportW * 0.5f, 0.0f, viewportW * 0.5f, viewportH * 0.5f };
    case SnapRegion::BottomLeft:
        return { 0.0f, viewportH * 0.5f, viewportW * 0.5f, viewportH * 0.5f };
    case SnapRegion::BottomRight:
        return { viewportW * 0.5f, viewportH * 0.5f, viewportW * 0.5f, viewportH * 0.5f };
    case SnapRegion::Full:
        return { 0.0f, 0.0f, viewportW, viewportH };
    default:
        return {};
    }
}

} // namespace DragonOS::WindowManager
