#include <UI/Controls/ContextMenu.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <UI/Core/UIStyle.hpp>
#include <algorithm>

namespace DragonOS::UI {

ContextMenu::ContextMenu() noexcept
{
    SetStyle(UIStyle::DefaultMenu());
    SetVisibility(Visibility::Collapsed);
    SetZIndex(10000);
    SetAccessibilityRole(L"contextmenu");
}

void ContextMenu::AddItem(std::wstring_view text, std::function<void()> action) noexcept
{
    ContextMenuItem item;
    item.text = text;
    item.action = std::move(action);
    m_items.push_back(std::move(item));
    InvalidateLayout();
}

void ContextMenu::AddSeparator() noexcept
{
    ContextMenuItem item;
    item.isSeparator = true;
    m_items.push_back(std::move(item));
    InvalidateLayout();
}

void ContextMenu::ClearItems() noexcept
{
    m_items.clear();
    InvalidateLayout();
}

void ContextMenu::ShowAt(float x, float y) noexcept
{
    m_isOpen = true;
    SetVisibility(Visibility::Visible);
    SetPosition(x, y);
    m_hoveredIndex = -1;
    InvalidateLayout();
}

void ContextMenu::Close() noexcept
{
    m_isOpen = false;
    SetVisibility(Visibility::Collapsed);
    m_hoveredIndex = -1;
    InvalidateVisual();
}

D2D1_RECT_F ContextMenu::MeasureOverride(const D2D1_RECT_F& available) noexcept
{
    float maxTextWidth = MinWidth;
    float totalHeight = 0;

    for (const auto& item : m_items)
    {
        if (item.isSeparator)
        {
            totalHeight += 8.0f;
        }
        else
        {
            float textWidth = static_cast<float>(item.text.length()) * 7.0f + ItemPaddingH * 2.0f;
            maxTextWidth = (std::max)(maxTextWidth, textWidth);
            totalHeight += ItemHeight;
        }
    }

    float availWidth = available.right - available.left;
    float availHeight = available.bottom - available.top;

    float width = (std::min)(maxTextWidth, availWidth);
    float height = (std::min)(totalHeight, availHeight);

    return {0, 0, width, height};
}

void ContextMenu::Render(UIRenderer& renderer) noexcept
{
    if (!m_isOpen) return;

    auto bounds = GetBounds();
    auto style = GetStyle();
    if (!style) return;

    float opacity = GetAnimatedOpacity() * GetOpacity();

    StyleStateColors popupColors;
    popupColors.background = Theme::SemanticColor::WindowBackground;
    popupColors.border = Theme::SemanticColor::WindowBorder;
    popupColors.foreground = Theme::SemanticColor::TextPrimary;

    renderer.FillBackground(bounds, popupColors, 0.0f, opacity);
    renderer.DrawBorder(bounds, popupColors, 1.0f, 0.0f, opacity);

    float currentY = bounds.top;
    int itemIndex = 0;

    for (size_t i = 0; i < m_items.size(); ++i)
    {
        const auto& item = m_items[i];

        if (item.isSeparator)
        {
            float sepY = currentY + 4.0f;
            StyleStateColors sepColors;
            sepColors.background = Theme::SemanticColor::WindowBorder;
            renderer.FillBackground({bounds.left + 8.0f, sepY, bounds.right - 8.0f, sepY + 1.0f},
                                    sepColors, 0.0f, opacity * 0.5f);
            currentY += 8.0f;
            continue;
        }

        D2D1_RECT_F itemRect = {
            bounds.left, currentY,
            bounds.right, currentY + ItemHeight
        };

        if (itemIndex == m_hoveredIndex && item.isEnabled)
        {
            StyleStateColors hoverColors;
            hoverColors.background = Theme::SemanticColor::Hover;
            renderer.FillBackground(itemRect, hoverColors, 0.0f, opacity);
        }

        ElementState textState = item.isEnabled ? ElementState::Normal : ElementState::Disabled;
        const auto& textColors = style->ResolveState(textState);

        D2D1_RECT_F textRect = {
            itemRect.left + ItemPaddingH, itemRect.top + 4.0f,
            itemRect.right - ItemPaddingH, itemRect.bottom - 4.0f
        };
        renderer.DrawText(item.text, textRect, textColors);

        currentY += ItemHeight;
        ++itemIndex;
    }
}

bool ContextMenu::OnEvent(const UIEvent& event) noexcept
{
    if (!m_isOpen) return false;

    auto bounds = GetBounds();

    switch (event.type)
    {
    case UIEventType::MouseMove:
    {
        if (event.x >= bounds.left && event.x <= bounds.right &&
            event.y >= bounds.top && event.y <= bounds.bottom)
        {
            int idx = 0;
            float currentY = bounds.top;
            for (size_t i = 0; i < m_items.size(); ++i)
            {
                if (m_items[i].isSeparator)
                {
                    currentY += 8.0f;
                    continue;
                }

                D2D1_RECT_F itemRect = {bounds.left, currentY, bounds.right, currentY + ItemHeight};
                if (event.y >= itemRect.top && event.y <= itemRect.bottom)
                {
                    if (m_hoveredIndex != idx)
                    {
                        m_hoveredIndex = idx;
                        InvalidateVisual();
                    }
                    return true;
                }
                currentY += ItemHeight;
                ++idx;
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
    case UIEventType::Click:
    {
        if (event.x >= bounds.left && event.x <= bounds.right &&
            event.y >= bounds.top && event.y <= bounds.bottom)
        {
            int idx = 0;
            float currentY = bounds.top;
            for (size_t i = 0; i < m_items.size(); ++i)
            {
                if (m_items[i].isSeparator)
                {
                    currentY += 8.0f;
                    continue;
                }

                D2D1_RECT_F itemRect = {bounds.left, currentY, bounds.right, currentY + ItemHeight};
                if (event.y >= itemRect.top && event.y <= itemRect.bottom &&
                    m_items[i].isEnabled && m_items[i].action)
                {
                    auto action = m_items[i].action;
                    Close();
                    action();
                    return true;
                }
                currentY += ItemHeight;
                ++idx;
            }
            Close();
            return true;
        }

        Close();
        return true;
    }

    default:
        return false;
    }
}

} // namespace
