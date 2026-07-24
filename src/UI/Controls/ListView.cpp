#include <UI/Controls/ListView.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <UI/Core/UIStyle.hpp>
#include <algorithm>

namespace DragonOS::UI {

ListView::ListView() noexcept
{
    SetFocusable(true);
    SetStyle(UIStyle::DefaultListView());
    SetAccessibilityRole(L"list");
}

void ListView::SetItems(const std::vector<std::wstring>& items) noexcept
{
    m_items = items;
    if (m_selectedIndex >= static_cast<int>(m_items.size()))
        m_selectedIndex = m_items.empty() ? -1 : 0;
    InvalidateLayout();
}

void ListView::SetSelectedIndex(int index) noexcept
{
    if (index < -1 || index >= static_cast<int>(m_items.size()))
        return;
    if (m_selectedIndex != index)
    {
        m_selectedIndex = index;
        if (m_onSelectionChanged)
            m_onSelectionChanged(m_selectedIndex);
        InvalidateVisual();
    }
}

std::wstring ListView::GetSelectedItem() const noexcept
{
    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_items.size()))
        return m_items[m_selectedIndex];
    return {};
}

void ListView::EnsureVisible() noexcept
{
    if (m_selectedIndex < 0) return;
    float itemTop = m_selectedIndex * ItemHeight;
    float itemBottom = itemTop + ItemHeight;
    float viewTop = m_scrollOffset;
    float viewBottom = m_scrollOffset + GetHeight();
    if (itemTop < viewTop)
        SetScrollOffset(itemTop);
    else if (itemBottom > viewBottom)
        SetScrollOffset(itemBottom - GetHeight());
}

void ListView::SetScrollOffset(float offset) noexcept
{
    float maxOffset = (std::max)(0.0f, static_cast<float>(m_items.size()) * ItemHeight - GetHeight());
    m_scrollOffset = (std::max)(0.0f, (std::min)(offset, maxOffset));
    InvalidateVisual();
}

D2D1_RECT_F ListView::MeasureOverride(const D2D1_RECT_F& available) noexcept
{
    float totalHeight = static_cast<float>(m_items.size()) * ItemHeight;
    float maxTextWidth = 100.0f;
    for (const auto& item : m_items)
    {
        float w = static_cast<float>(item.length()) * 7.0f;
        maxTextWidth = (std::max)(maxTextWidth, w);
    }

    float width = (std::min)(maxTextWidth + 16.0f, available.right - available.left);
    float height = (std::min)(totalHeight + 4.0f, available.bottom - available.top);
    return {0, 0, width, height};
}

void ListView::Render(UIRenderer& renderer) noexcept
{
    auto bounds = GetBounds();
    auto style = GetStyle();
    if (!style) return;

    float opacity = GetAnimatedOpacity() * GetOpacity();

    StyleStateColors bgColors;
    bgColors.background = Theme::SemanticColor::WindowBackground;
    renderer.FillBackground(bounds, bgColors, 0.0f, opacity);

    renderer.PushClip(bounds);

    float visibleStart = m_scrollOffset;
    float visibleEnd = m_scrollOffset + (bounds.bottom - bounds.top);
    int startIdx = static_cast<int>(visibleStart / ItemHeight);
    int endIdx = static_cast<int>(visibleEnd / ItemHeight) + 1;
    startIdx = (std::max)(0, startIdx);
    endIdx = (std::min)(endIdx, static_cast<int>(m_items.size()));

    for (int i = startIdx; i < endIdx; ++i)
    {
        float y = bounds.top + i * ItemHeight - m_scrollOffset;
        D2D1_RECT_F itemRect = {bounds.left, y, bounds.right, y + ItemHeight};

        if (i == m_selectedIndex)
        {
            StyleStateColors selColors;
            selColors.background = Theme::SemanticColor::Selection;
            renderer.FillBackground(itemRect, selColors, 0.0f, opacity);
        }
        else if (i == m_hoveredIndex)
        {
            StyleStateColors hoverColors;
            hoverColors.background = Theme::SemanticColor::Hover;
            renderer.FillBackground(itemRect, hoverColors, 0.0f, opacity);
        }

        D2D1_RECT_F textRect = {
            itemRect.left + 6.0f, itemRect.top + 2.0f,
            itemRect.right - 6.0f, itemRect.bottom - 2.0f
        };

        const auto& stateColors = style->ResolveState(i == m_selectedIndex ? ElementState::Focused : ElementState::Normal);
        renderer.DrawText(m_items[i], textRect, stateColors);
    }

    if (GetState() == ElementState::Focused && m_selectedIndex >= 0)
    {
        float selY = bounds.top + m_selectedIndex * ItemHeight - m_scrollOffset;
        D2D1_RECT_F focusRect = {bounds.left, selY, bounds.right, selY + ItemHeight};
        renderer.DrawFocusIndicator(focusRect);
    }

    renderer.PopClip();
}

bool ListView::OnEvent(const UIEvent& event) noexcept
{
    if (!IsEnabled()) return false;

    auto bounds = GetBounds();

    switch (event.type)
    {
    case UIEventType::MouseEnter:
        SetState(ElementState::Hover);
        return true;

    case UIEventType::MouseLeave:
        SetState(ElementState::Normal);
        m_hoveredIndex = -1;
        InvalidateVisual();
        return true;

    case UIEventType::MouseMove:
    {
        if (event.x >= bounds.left && event.x <= bounds.right &&
            event.y >= bounds.top && event.y <= bounds.bottom)
        {
            int idx = static_cast<int>((event.y - bounds.top + m_scrollOffset) / ItemHeight);
            if (idx >= 0 && idx < static_cast<int>(m_items.size()))
            {
                if (m_hoveredIndex != idx)
                {
                    m_hoveredIndex = idx;
                    InvalidateVisual();
                }
                return true;
            }
        }
        if (m_hoveredIndex != -1)
        {
            m_hoveredIndex = -1;
            InvalidateVisual();
        }
        return false;
    }

    case UIEventType::MouseDown:
    {
        int idx = static_cast<int>((event.y - bounds.top + m_scrollOffset) / ItemHeight);
        if (idx >= 0 && idx < static_cast<int>(m_items.size()))
        {
            SetSelectedIndex(idx);
            return true;
        }
        return false;
    }

    case UIEventType::Scroll:
    {
        SetScrollOffset(m_scrollOffset - event.wheelDelta * ItemHeight * 0.5f);
        return true;
    }

    case UIEventType::KeyDown:
        switch (event.key)
        {
        case Input::KeyCode::Down:
            SetSelectedIndex((std::min)(m_selectedIndex + 1, static_cast<int>(m_items.size()) - 1));
            EnsureVisible();
            return true;
        case Input::KeyCode::Up:
            SetSelectedIndex((std::max)(m_selectedIndex - 1, 0));
            return true;
        case Input::KeyCode::Home:
            SetSelectedIndex(0);
            m_scrollOffset = 0;
            InvalidateVisual();
            return true;
        case Input::KeyCode::End:
            SetSelectedIndex(static_cast<int>(m_items.size()) - 1);
            m_scrollOffset = (std::max)(0.0f, static_cast<float>(m_items.size()) * ItemHeight - GetHeight());
            InvalidateVisual();
            return true;
        case Input::KeyCode::PageUp:
            SetScrollOffset(m_scrollOffset - GetHeight());
            return true;
        case Input::KeyCode::PageDown:
            SetScrollOffset(m_scrollOffset + GetHeight());
            return true;
        default:
            return false;
        }

    case UIEventType::FocusGained:
        SetState(ElementState::Focused);
        return true;

    case UIEventType::FocusLost:
        SetState(ElementState::Normal);
        return true;

    default:
        return false;
    }
}

} // namespace
