#include <UI/Controls/Menu.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <UI/Core/UIStyle.hpp>
#include <algorithm>

namespace DragonOS::UI {

Menu::Menu() noexcept
{
    SetStyle(UIStyle::DefaultMenu());
    SetAccessibilityRole(L"menubar");
}

int Menu::AddItem(std::wstring_view text, std::function<void()> action) noexcept
{
    MenuItem item;
    item.text = text;
    item.action = std::move(action);
    m_items.push_back(std::move(item));
    InvalidateLayout();
    return static_cast<int>(m_items.size()) - 1;
}

int Menu::AddSubMenu(std::wstring_view text, std::vector<MenuItem> subItems) noexcept
{
    MenuItem item;
    item.text = text;
    item.subItems = std::move(subItems);
    m_items.push_back(std::move(item));
    InvalidateLayout();
    return static_cast<int>(m_items.size()) - 1;
}

void Menu::ClearItems() noexcept
{
    m_items.clear();
    m_hoveredItem = -1;
    m_openSubMenu = -1;
    InvalidateLayout();
}

D2D1_RECT_F Menu::MeasureOverride(const D2D1_RECT_F& available) noexcept
{
    float totalWidth = 0;
    float maxHeight = MinItemHeight;

    for (const auto& item : m_items)
    {
        float itemWidth = static_cast<float>(item.text.length()) * 7.0f + ItemPaddingH * 2.0f;
        totalWidth += itemWidth;
        maxHeight = (std::max)(maxHeight, MinItemHeight);
    }

    float availWidth = available.right - available.left;
    float availHeight = available.bottom - available.top;

    totalWidth = (std::min)(totalWidth, availWidth);
    maxHeight = (std::min)(maxHeight, availHeight);

    return {0, 0, totalWidth, maxHeight};
}

void Menu::ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept
{
    SetBounds(finalRect);
}

D2D1_RECT_F Menu::GetItemRect(int index) const noexcept
{
    auto bounds = GetBounds();
    float currentX = bounds.left;
    for (int i = 0; i < index && i < static_cast<int>(m_items.size()); ++i)
    {
        currentX += static_cast<float>(m_items[i].text.length()) * 7.0f + ItemPaddingH * 2.0f;
    }

    float itemWidth = static_cast<float>(m_items[index].text.length()) * 7.0f + ItemPaddingH * 2.0f;
    return {currentX, bounds.top, currentX + itemWidth, bounds.bottom};
}

void Menu::CloseSubMenu() noexcept
{
    if (m_openSubMenu >= 0)
    {
        m_openSubMenu = -1;
        InvalidateVisual();
    }
}

void Menu::Render(UIRenderer& renderer) noexcept
{
    auto bounds = GetBounds();
    auto style = GetStyle();
    if (!style) return;

    float opacity = GetAnimatedOpacity() * GetOpacity();

    StyleStateColors barColors;
    barColors.background = Theme::SemanticColor::ExplorerToolbarBackground;
    renderer.FillBackground(bounds, barColors, 0.0f, opacity);

    for (int i = 0; i < static_cast<int>(m_items.size()); ++i)
    {
        D2D1_RECT_F itemRect = GetItemRect(i);

        ElementState itemState = ElementState::Normal;
        if (!m_items[i].isEnabled) itemState = ElementState::Disabled;
        else if (i == m_hoveredItem || i == m_openSubMenu) itemState = ElementState::Hover;

        const auto& itemColors = style->ResolveState(itemState);

        if (i == m_hoveredItem || i == m_openSubMenu)
        {
            renderer.FillBackground(itemRect, itemColors, 0.0f, opacity);
        }

        D2D1_RECT_F textRect = {
            itemRect.left + ItemPaddingH, itemRect.top + ItemPaddingV,
            itemRect.right - ItemPaddingH, itemRect.bottom - ItemPaddingV
        };
        renderer.DrawText(m_items[i].text, textRect, itemColors);
    }

    if (m_openSubMenu >= 0 && m_openSubMenu < static_cast<int>(m_items.size()))
    {
        auto& subItems = m_items[m_openSubMenu].subItems;
        if (!subItems.empty())
        {
            auto itemRect = GetItemRect(m_openSubMenu);
            float subW = 160.0f;
            float subH = static_cast<float>(subItems.size()) * MinItemHeight;
            D2D1_RECT_F subRect = {itemRect.left, itemRect.bottom, itemRect.left + subW, itemRect.bottom + subH};
            m_subMenuBounds = subRect;

            StyleStateColors popupColors;
            popupColors.background = Theme::SemanticColor::WindowBackground;
            popupColors.border = Theme::SemanticColor::WindowBorder;
            renderer.FillBackground(subRect, popupColors, 0.0f, opacity);
            renderer.DrawBorder(subRect, popupColors, 1.0f, 0.0f, opacity);

            for (int j = 0; j < static_cast<int>(subItems.size()); ++j)
            {
                D2D1_RECT_F subItemRect = {
                    subRect.left, subRect.top + j * MinItemHeight,
                    subRect.right, subRect.top + (j + 1) * MinItemHeight
                };

                if (subItems[j].isSeparator)
                {
                    StyleStateColors sepColors;
                    sepColors.background = Theme::SemanticColor::WindowBorder;
                    renderer.FillBackground({subItemRect.left + 4.0f, subItemRect.top + subItemRect.bottom * 0.5f,
                                            subItemRect.right - 4.0f, subItemRect.top + subItemRect.bottom * 0.5f + 1.0f},
                                            sepColors, 0.0f, opacity * 0.5f);
                    continue;
                }

                ElementState subState = ElementState::Normal;
                if (!subItems[j].isEnabled) subState = ElementState::Disabled;

                const auto& subColors = style->ResolveState(subState);

                if (j == m_hoveredItem && m_openSubMenu >= 0)
                {
                    StyleStateColors hoverColors;
                    hoverColors.background = Theme::SemanticColor::Hover;
                    renderer.FillBackground(subItemRect, hoverColors, 0.0f, opacity);
                }

                D2D1_RECT_F subTextRect = {
                    subItemRect.left + 8.0f, subItemRect.top + 2.0f,
                    subItemRect.right - 8.0f, subItemRect.bottom - 2.0f
                };
                renderer.DrawText(subItems[j].text, subTextRect, subColors);
            }
        }
    }
}

bool Menu::OnEvent(const UIEvent& event) noexcept
{
    auto bounds = GetBounds();

    switch (event.type)
    {
    case UIEventType::MouseMove:
    {
        if (event.y >= bounds.top && event.y <= bounds.bottom)
        {
            float currentX = bounds.left;
            for (int i = 0; i < static_cast<int>(m_items.size()); ++i)
            {
                float itemWidth = static_cast<float>(m_items[i].text.length()) * 7.0f + ItemPaddingH * 2.0f;
                if (event.x >= currentX && event.x <= currentX + itemWidth)
                {
                    if (m_hoveredItem != i)
                    {
                        m_hoveredItem = i;
                        if (m_openSubMenu >= 0 && !m_items[i].subItems.empty())
                        {
                            m_openSubMenu = i;
                        }
                        else if (m_openSubMenu >= 0)
                        {
                            CloseSubMenu();
                        }
                        InvalidateVisual();
                    }
                    return true;
                }
                currentX += itemWidth;
            }
        }
        if (m_hoveredItem != -1)
        {
            m_hoveredItem = -1;
            InvalidateVisual();
        }
        return false;
    }

    case UIEventType::MouseDown:
    {
        if (event.y >= bounds.top && event.y <= bounds.bottom)
        {
            float currentX = bounds.left;
            for (int i = 0; i < static_cast<int>(m_items.size()); ++i)
            {
                float itemWidth = static_cast<float>(m_items[i].text.length()) * 7.0f + ItemPaddingH * 2.0f;
                if (event.x >= currentX && event.x <= currentX + itemWidth)
                {
                    if (!m_items[i].subItems.empty())
                    {
                        if (m_openSubMenu == i)
                            CloseSubMenu();
                        else
                            m_openSubMenu = i;
                        InvalidateVisual();
                    }
                    else if (m_items[i].action)
                    {
                        m_items[i].action();
                        CloseSubMenu();
                    }
                    return true;
                }
                currentX += itemWidth;
            }
        }

        if (m_openSubMenu >= 0 && !m_subMenuBounds.left && !m_subMenuBounds.right)
        {
            if (event.x >= m_subMenuBounds.left && event.x <= m_subMenuBounds.right &&
                event.y >= m_subMenuBounds.top && event.y <= m_subMenuBounds.bottom)
            {
                int idx = static_cast<int>((event.y - m_subMenuBounds.top) / MinItemHeight);
                if (idx >= 0 && idx < static_cast<int>(m_items[m_openSubMenu].subItems.size()))
                {
                    auto& subItem = m_items[m_openSubMenu].subItems[idx];
                    if (!subItem.isSeparator && subItem.isEnabled && subItem.action)
                    {
                        subItem.action();
                        CloseSubMenu();
                    }
                    return true;
                }
            }
            CloseSubMenu();
            return true;
        }
        return false;
    }

    case UIEventType::MouseLeave:
        m_hoveredItem = -1;
        InvalidateVisual();
        return true;

    default:
        return false;
    }
}

} // namespace
