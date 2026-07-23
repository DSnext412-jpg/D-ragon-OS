#include <UI/Controls/ScrollViewer.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <UI/Core/UIStyle.hpp>
#include <algorithm>

namespace DragonOS::UI {

ScrollViewer::ScrollViewer() noexcept
{
    SetStyle(UIStyle::DefaultScrollViewer());
    SetAccessibilityRole(L"scrollbar");
    m_clipChildren = true;
}

void ScrollViewer::SetContent(std::unique_ptr<UIElement> content) noexcept
{
    ClearChildren();
    if (content)
        AddChild(std::move(content));
    InvalidateLayout();
}

UIElement* ScrollViewer::GetContent() const noexcept
{
    return GetChildCount() > 0 ? GetChildAt(0) : nullptr;
}

void ScrollViewer::SetScrollX(float x) noexcept
{
    m_scrollX = (std::max)(0.0f, (std::min)(x, m_contentSize.width - (GetBounds().right - GetBounds().left) + m_scrollBarWidth));
    InvalidateVisual();
}

void ScrollViewer::SetScrollY(float y) noexcept
{
    float viewHeight = (GetBounds().bottom - GetBounds().top);
    m_scrollY = (std::max)(0.0f, (std::min)(y, m_contentSize.height - viewHeight + m_scrollBarWidth));
    InvalidateVisual();
}

void ScrollViewer::Measure(const D2D1_RECT_F& availableSize) noexcept
{
    float availWidth = availableSize.right - availableSize.left;
    float availHeight = availableSize.bottom - availableSize.top;

    D2D1_RECT_F childAvailable = {0, 0, availWidth * 2.0f, availHeight * 2.0f};

    for (auto& child : m_children)
    {
        if (child->GetVisibility() == Visibility::Collapsed) continue;
        child->Measure(childAvailable);
        m_contentSize = {child->GetDesiredSize().right - child->GetDesiredSize().left,
                         child->GetDesiredSize().bottom - child->GetDesiredSize().top};
    }

    SetDesiredSize({0, 0, availWidth, availHeight});
    m_layoutDirty = false;
}

void ScrollViewer::Arrange(const D2D1_RECT_F& finalRect) noexcept
{
    SetBounds(finalRect);
    float viewWidth = finalRect.right - finalRect.left;
    float viewHeight = finalRect.bottom - finalRect.top;

    for (auto& child : m_children)
    {
        if (child->GetVisibility() == Visibility::Collapsed) continue;
        D2D1_RECT_F childRect = {
            -m_scrollX, -m_scrollY,
            (std::max)(m_contentSize.width, viewWidth) - m_scrollX,
            (std::max)(m_contentSize.height, viewHeight) - m_scrollY
        };
        child->Arrange(childRect);
    }
    m_layoutDirty = false;
}

void ScrollViewer::Render(UIRenderer& renderer) noexcept
{
    auto bounds = GetBounds();
    auto style = GetStyle();
    if (!style) return;

    float opacity = GetAnimatedOpacity() * GetOpacity();

    StyleStateColors bgColors;
    bgColors.background = Theme::SemanticColor::WindowBackground;
    renderer.FillBackground(bounds, bgColors, 0.0f, opacity);

    float viewWidth = bounds.right - bounds.left;
    float viewHeight = bounds.bottom - bounds.top;

    D2D1_RECT_F contentRect = {bounds.left, bounds.top, bounds.right - m_scrollBarWidth, bounds.bottom - m_scrollBarWidth};
    renderer.PushClip(contentRect);

    SortChildrenByZIndex();
    for (auto& child : m_children)
    {
        if (child->GetVisibility() == Visibility::Visible)
            child->Render(renderer);
    }

    renderer.PopClip();

    bool showHScroll = m_contentSize.width > viewWidth;
    bool showVScroll = m_contentSize.height > viewHeight;

    if (showVScroll)
    {
        D2D1_RECT_F vBarRect = {
            bounds.right - m_scrollBarWidth, bounds.top,
            bounds.right, bounds.bottom - (showHScroll ? m_scrollBarWidth : 0)
        };
        RenderScrollBar(renderer, vBarRect, m_contentSize.height, viewHeight, m_scrollY, true);
    }

    if (showHScroll)
    {
        D2D1_RECT_F hBarRect = {
            bounds.left, bounds.bottom - m_scrollBarWidth,
            bounds.right - (showVScroll ? m_scrollBarWidth : 0), bounds.bottom
        };
        RenderScrollBar(renderer, hBarRect, m_contentSize.width, viewWidth, m_scrollX, false);
    }
}

void ScrollViewer::RenderScrollBar(UIRenderer& renderer, const D2D1_RECT_F& barRect, float contentExtent, float viewportExtent, float scrollPos, bool vertical) noexcept
{
    float opacity = GetAnimatedOpacity() * GetOpacity();

    StyleStateColors trackColors;
    trackColors.background = Theme::SemanticColor::WindowBackground;
    renderer.FillBackground(barRect, trackColors, 0.0f, opacity * 0.8f);

    StyleStateColors borderColors;
    borderColors.border = Theme::SemanticColor::WindowBorder;
    renderer.DrawBorder(barRect, borderColors, 1.0f, 0.0f, opacity * 0.5f);

    float thumbSize;
    float thumbPos;
    float barLength = vertical ? (barRect.bottom - barRect.top) : (barRect.right - barRect.left);
    float viewRatio = viewportExtent / contentExtent;

    if (viewRatio >= 1.0f) return;

    float thumbLen = (std::max)(barLength * viewRatio, MinScrollBarHeight);
    float scrollRange = contentExtent - viewportExtent;
    float scrollRatio = scrollPos / scrollRange;
    float moveRange = barLength - thumbLen;

    if (vertical)
    {
        thumbPos = barRect.top + scrollRatio * moveRange;
        thumbSize = barRect.right - barRect.left;
        D2D1_RECT_F thumbRect = {barRect.left, thumbPos, barRect.right, thumbPos + thumbLen};
        StyleStateColors thumbColors;
        thumbColors.background = Theme::SemanticColor::Accent;
        renderer.FillBackground(thumbRect, thumbColors, 2.0f, opacity * 0.6f);
    }
    else
    {
        thumbPos = barRect.left + scrollRatio * moveRange;
        thumbSize = barRect.bottom - barRect.top;
        D2D1_RECT_F thumbRect = {thumbPos, barRect.top, thumbPos + thumbLen, barRect.bottom};
        StyleStateColors thumbColors;
        thumbColors.background = Theme::SemanticColor::Accent;
        renderer.FillBackground(thumbRect, thumbColors, 2.0f, opacity * 0.6f);
    }
}

bool ScrollViewer::OnEvent(const UIEvent& event) noexcept
{
    switch (event.type)
    {
    case UIEventType::Scroll:
    {
        if (event.wheelDelta != 0)
        {
            SetScrollY(m_scrollY - event.wheelDelta * 30.0f);
            return true;
        }
        return false;
    }

    case UIEventType::MouseDown:
    {
        auto bounds = GetBounds();
        float viewWidth = bounds.right - bounds.left;
        float viewHeight = bounds.bottom - bounds.top;

        bool showVScroll = m_contentSize.height > viewHeight;
        bool showHScroll = m_contentSize.width > viewWidth;

        if (showVScroll)
        {
            D2D1_RECT_F vBarRect = {
                bounds.right - m_scrollBarWidth, bounds.top,
                bounds.right, bounds.bottom - (showHScroll ? m_scrollBarWidth : 0)
            };
            if (event.x >= vBarRect.left && event.x <= vBarRect.right &&
                event.y >= vBarRect.top && event.y <= vBarRect.bottom)
            {
                float barLen = vBarRect.bottom - vBarRect.top;
                float viewRatio = viewHeight / m_contentSize.height;
                float thumbLen = (std::max)(barLen * viewRatio, MinScrollBarHeight);
                float scrollRange = m_contentSize.height - viewHeight;
                if (scrollRange > 0)
                {
                    float moveRange = barLen - thumbLen;
                    float clickRatio = (event.y - vBarRect.top) / barLen;
                    m_scrollY = clickRatio * scrollRange;
                    InvalidateVisual();
                }
                return true;
            }
        }

        if (showHScroll)
        {
            D2D1_RECT_F hBarRect = {
                bounds.left, bounds.bottom - m_scrollBarWidth,
                bounds.right - (showVScroll ? m_scrollBarWidth : 0), bounds.bottom
            };
            if (event.x >= hBarRect.left && event.x <= hBarRect.right &&
                event.y >= hBarRect.top && event.y <= hBarRect.bottom)
            {
                float barLen = hBarRect.right - hBarRect.left;
                float viewRatio = viewWidth / m_contentSize.width;
                float thumbLen = (std::max)(barLen * viewRatio, MinScrollBarHeight);
                float scrollRange = m_contentSize.width - viewWidth;
                if (scrollRange > 0)
                {
                    float moveRange = barLen - thumbLen;
                    float clickRatio = (event.x - hBarRect.left) / barLen;
                    m_scrollX = clickRatio * scrollRange;
                    InvalidateVisual();
                }
                return true;
            }
        }

        return UIContainer::OnEvent(event);
    }

    default:
        return UIContainer::OnEvent(event);
    }
}

} // namespace
