#include <DragonUI/Controls/ScrollViewer.hpp>
#include <DragonUI/Core/RenderContext.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

void UIScrollViewer::SetHorizontalScrollOffset(float offset) noexcept
{
    m_hScrollOffset = offset;
    ClampScrollOffsets();
    InvalidateLayout();
}

void UIScrollViewer::SetVerticalScrollOffset(float offset) noexcept
{
    m_vScrollOffset = offset;
    ClampScrollOffsets();
    InvalidateLayout();
}

void UIScrollViewer::SetHorizontalScrollEnabled(bool enabled) noexcept
{
    if (m_hScrollEnabled != enabled)
    {
        m_hScrollEnabled = enabled;
        InvalidateLayout();
    }
}

void UIScrollViewer::SetVerticalScrollEnabled(bool enabled) noexcept
{
    if (m_vScrollEnabled != enabled)
    {
        m_vScrollEnabled = enabled;
        InvalidateLayout();
    }
}

void UIScrollViewer::ScrollBy(float dx, float dy) noexcept
{
    m_hScrollOffset += dx;
    m_vScrollOffset += dy;
    ClampScrollOffsets();
    InvalidateVisual();
}

void UIScrollViewer::ScrollTo(float x, float y) noexcept
{
    m_hScrollOffset = x;
    m_vScrollOffset = y;
    ClampScrollOffsets();
    InvalidateVisual();
}

void UIScrollViewer::ClampScrollOffsets() noexcept
{
    if (m_hScrollOffset < 0) m_hScrollOffset = 0;
    if (m_vScrollOffset < 0) m_vScrollOffset = 0;
    if (m_hScrollOffset > m_scrollableWidth) m_hScrollOffset = m_scrollableWidth;
    if (m_vScrollOffset > m_scrollableHeight) m_vScrollOffset = m_scrollableHeight;
}

DesiredSize UIScrollViewer::MeasureOverride(const LayoutSlot& available) noexcept
{
    auto content = available.Inset(m_padding);
    LayoutSlot childSlot{content.x, content.y, FLT_MAX, FLT_MAX};
    float maxChildW{};
    float maxChildH{};

    for (const auto& child : m_children)
    {
        if (child->GetVisibility() == Visibility::Collapsed) continue;
        child->Measure(childSlot);
        auto ds = child->GetDesiredSize();
        if (ds.width > maxChildW) maxChildW = ds.width;
        if (ds.height > maxChildH) maxChildH = ds.height;
    }

    m_scrollableWidth = (std::max)(0.0f, maxChildW - content.width);
    m_scrollableHeight = (std::max)(0.0f, maxChildH - content.height);
    ClampScrollOffsets();

    return {content.width, content.height};
}

void UIScrollViewer::MeasureChildren(const LayoutSlot& available) noexcept
{
}

void UIScrollViewer::ArrangeOverride(const LayoutSlot& finalSlot) noexcept
{
    for (const auto& child : m_children)
    {
        if (child->GetVisibility() == Visibility::Collapsed) continue;
        auto ds = child->GetDesiredSize();
        child->Arrange({
            finalSlot.x - m_hScrollOffset,
            finalSlot.y - m_vScrollOffset,
            (std::max)(ds.width, finalSlot.width),
            (std::max)(ds.height, finalSlot.height)
        });
    }
}

void UIScrollViewer::ArrangeChildren(const LayoutSlot& finalSlot) noexcept
{
}

void UIScrollViewer::Render(RenderContext& ctx) noexcept
{
    if (m_visibility != Visibility::Visible || m_opacity <= 0.0f)
        return;

    m_visualDirty = false;

    auto bounds = static_cast<D2D1_RECT_F>(m_bounds);
    ctx.PushClip(bounds);

    RenderChildren(ctx);

    RenderScrollBars(ctx);

    ctx.PopClip();
}

void UIScrollViewer::RenderScrollBars(RenderContext& ctx) const noexcept
{
    float scrollBarSize = 8.0f;

    if (m_scrollableHeight > 0 && m_vScrollEnabled)
    {
        float viewportH = m_bounds.height;
        float contentH = viewportH + m_scrollableHeight;
        float thumbSize = (viewportH / contentH) * viewportH;
        float thumbPos = (m_vScrollOffset / m_scrollableHeight) * (viewportH - thumbSize);

        D2D1_RECT_F track{static_cast<D2D1_RECT_F>(m_bounds)};
        track.left = track.right - scrollBarSize;
        ctx.FillRectangle(track, Theme::SemanticColor::ControlFill, 0.5f);

        D2D1_RECT_F thumb{track.left, m_bounds.y + thumbPos, track.right, m_bounds.y + thumbPos + thumbSize};
        ctx.FillRectangle(thumb, Theme::SemanticColor::Accent);
    }

    if (m_scrollableWidth > 0 && m_hScrollEnabled)
    {
        float viewportW = m_bounds.width;
        float contentW = viewportW + m_scrollableWidth;
        float thumbSize = (viewportW / contentW) * viewportW;
        float thumbPos = (m_hScrollOffset / m_scrollableWidth) * (viewportW - thumbSize);

        D2D1_RECT_F track{static_cast<D2D1_RECT_F>(m_bounds)};
        track.top = track.bottom - scrollBarSize;
        ctx.FillRectangle(track, Theme::SemanticColor::ControlFill, 0.5f);

        D2D1_RECT_F thumb{m_bounds.x + thumbPos, track.top, m_bounds.x + thumbPos + thumbSize, track.bottom};
        ctx.FillRectangle(thumb, Theme::SemanticColor::Accent);
    }
}

Element* UIScrollViewer::HitTest(float x, float y) noexcept
{
    if (!IsVisible()) return nullptr;

    auto fbounds = static_cast<D2D1_RECT_F>(m_bounds);
    if (x < fbounds.left || x > fbounds.right ||
        y < fbounds.top || y > fbounds.bottom)
        return nullptr;

    auto* hit = HitTestChildren(x, y);
    if (hit) return hit;
    return this;
}

} // namespace
