#include <UI/Core/UIElement.hpp>
#include <UI/Core/UIContainer.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <algorithm>

namespace DragonOS::UI {

uint64_t UIElement::s_nextId = 1;

UIElement::UIElement() noexcept
    : m_id(s_nextId++)
{
}

void UIElement::SetBounds(const D2D1_RECT_F& bounds) noexcept
{
    m_bounds = bounds;
    InvalidateLayout();
    InvalidateVisual();
}

void UIElement::SetPosition(float x, float y) noexcept
{
    float w = GetWidth();
    float h = GetHeight();
    m_bounds = {x, y, x + w, y + h};
    InvalidateLayout();
    InvalidateVisual();
}

void UIElement::SetSize(float width, float height) noexcept
{
    m_bounds.right = m_bounds.left + width;
    m_bounds.bottom = m_bounds.top + height;
    InvalidateLayout();
    InvalidateVisual();
}

void UIElement::SetMargin(float left, float top, float right, float bottom) noexcept
{
    m_margin[0] = left;
    m_margin[1] = top;
    m_margin[2] = right;
    m_margin[3] = bottom;
    InvalidateLayout();
}

void UIElement::SetPadding(float left, float top, float right, float bottom) noexcept
{
    m_padding[0] = left;
    m_padding[1] = top;
    m_padding[2] = right;
    m_padding[3] = bottom;
    InvalidateLayout();
}

D2D1_RECT_F UIElement::GetPaddingRect() const noexcept
{
    return {
        m_bounds.left + m_padding[0],
        m_bounds.top + m_padding[1],
        m_bounds.right - m_padding[2],
        m_bounds.bottom - m_padding[3]
    };
}

void UIElement::SetEnabled(bool enabled) noexcept
{
    m_enabled = enabled;
    SetState(enabled ? ElementState::Normal : ElementState::Disabled);
    InvalidateVisual();
}

void UIElement::SetState(ElementState state) noexcept
{
    m_state = state;
    InvalidateVisual();
}

void UIElement::Measure(const D2D1_RECT_F& availableSize) noexcept
{
    if (m_visibility != Visibility::Visible)
    {
        m_desiredSize = {0, 0, 0, 0};
        return;
    }

    float availW = availableSize.right - availableSize.left;
    float availH = availableSize.bottom - availableSize.top;

    D2D1_RECT_F contentAvailable = {
        0, 0,
        (std::max)(0.0f, availW - m_margin[0] - m_margin[2]),
        (std::max)(0.0f, availH - m_margin[1] - m_margin[3])
    };

    D2D1_RECT_F desired = MeasureOverride(contentAvailable);

    float desiredW = (std::clamp)(
        desired.right - desired.left,
        m_minWidth,
        m_maxWidth);
    float desiredH = (std::clamp)(
        desired.bottom - desired.top,
        m_minHeight,
        m_maxHeight);

    m_desiredSize = {
        0, 0,
        desiredW + m_margin[0] + m_margin[2],
        desiredH + m_margin[1] + m_margin[3]
    };

    m_layoutDirty = false;
}

void UIElement::Arrange(const D2D1_RECT_F& finalRect) noexcept
{
    m_bounds = finalRect;

    D2D1_RECT_F contentRect = {
        finalRect.left + m_padding[0],
        finalRect.top + m_padding[1],
        (std::max)(finalRect.left + m_padding[0], finalRect.right - m_padding[2]),
        (std::max)(finalRect.top + m_padding[1], finalRect.bottom - m_padding[3])
    };

    ArrangeOverride(contentRect);

    m_layoutDirty = false;
}

void UIElement::Render(UIRenderer& renderer) noexcept
{
    if (m_visibility != Visibility::Visible || m_opacity <= 0.0f)
        return;

    m_visualDirty = false;

    if (m_state == ElementState::Focused)
    {
        renderer.DrawFocusIndicator(m_bounds);
    }
}

UIElement* UIElement::HitTest(float x, float y) noexcept
{
    if (m_visibility != Visibility::Visible || !m_enabled)
        return nullptr;

    if (x >= m_bounds.left && x <= m_bounds.right &&
        y >= m_bounds.top && y <= m_bounds.bottom)
    {
        return this;
    }

    return nullptr;
}

bool UIElement::OnEvent(const UIEvent& event) noexcept
{
    (void)event;
    return false;
}

D2D1_RECT_F UIElement::MeasureOverride(const D2D1_RECT_F& availableSize) noexcept
{
    return availableSize;
}

void UIElement::ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept
{
    (void)finalRect;
}

void UIElement::SetRenderTransform(float offsetX, float offsetY, float scaleX, float scaleY) noexcept
{
    m_renderOffsetX = offsetX;
    m_renderOffsetY = offsetY;
    m_renderScaleX = scaleX;
    m_renderScaleY = scaleY;
    InvalidateVisual();
}

} // namespace
