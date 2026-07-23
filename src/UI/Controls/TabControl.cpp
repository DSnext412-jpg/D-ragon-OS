#include <UI/Controls/TabControl.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <UI/Core/UIStyle.hpp>
#include <algorithm>

namespace DragonOS::UI {

TabControl::TabControl() noexcept
{
    SetStyle(UIStyle::DefaultTabControl());
    SetAccessibilityRole(L"tabcontrol");
}

int TabControl::AddPage(std::wstring_view headerText, std::unique_ptr<UIElement> content) noexcept
{
    auto* rawContent = content.get();
    TabPage page;
    page.headerText = headerText;
    page.content = rawContent;
    m_tabs.push_back(std::move(page));
    if (m_selectedIndex < 0) m_selectedIndex = 0;
    if (rawContent)
        UIContainer::AddChild(std::move(content));
    InvalidateLayout();
    return static_cast<int>(m_tabs.size()) - 1;
}

bool TabControl::RemovePage(int index) noexcept
{
    if (index < 0 || index >= static_cast<int>(m_tabs.size()))
        return false;

    if (m_tabs[index].content)
        RemoveChild(m_tabs[index].content.get());

    m_tabs.erase(m_tabs.begin() + index);
    if (m_selectedIndex >= static_cast<int>(m_tabs.size()))
        m_selectedIndex = static_cast<int>(m_tabs.size()) - 1;
    InvalidateLayout();
    return true;
}

void TabControl::ClearPages() noexcept
{
    ClearChildren();
    m_tabs.clear();
    m_selectedIndex = -1;
    InvalidateLayout();
}

void TabControl::SetSelectedIndex(int index) noexcept
{
    if (index < 0 || index >= static_cast<int>(m_tabs.size()) || index == m_selectedIndex)
        return;

    m_selectedIndex = index;
    if (m_onSelectionChanged)
        m_onSelectionChanged(m_selectedIndex);
    InvalidateLayout();
}

TabPage* TabControl::GetPage(int index) const noexcept
{
    if (index >= 0 && index < static_cast<int>(m_tabs.size()))
        return const_cast<TabPage*>(&m_tabs[index]);
    return nullptr;
}

D2D1_RECT_F TabControl::MeasureOverride(const D2D1_RECT_F& available) noexcept
{
    float availWidth = available.right - available.left;
    float availHeight = available.bottom - available.top;
    float totalHeight = TabHeaderHeight + availHeight * 0.8f;

    float totalWidth = (std::min)((std::max)(200.0f, availWidth), availWidth);
    totalHeight = (std::min)((std::max)(100.0f, totalHeight), availHeight);

    for (auto& tab : m_tabs)
    {
        if (tab.content)
        {
            D2D1_RECT_F contentAvail = {0, 0, availWidth, availHeight - TabHeaderHeight};
            tab.content->Measure(contentAvail);
        }
    }

    return {0, 0, totalWidth, totalHeight};
}

void TabControl::ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept
{
    SetBounds(finalRect);
    for (int i = 0; i < static_cast<int>(m_tabs.size()); ++i)
    {
        if (m_tabs[i].content)
        {
            if (i == m_selectedIndex)
            {
                D2D1_RECT_F contentRect = {
                    finalRect.left, finalRect.top + TabHeaderHeight,
                    finalRect.right, finalRect.bottom
                };
                m_tabs[i].content->SetVisibility(Visibility::Visible);
                m_tabs[i].content->Arrange(contentRect);
            }
            else
            {
                m_tabs[i].content->SetVisibility(Visibility::Collapsed);
            }
        }
    }
}

void TabControl::Render(UIRenderer& renderer) noexcept
{
    auto bounds = GetBounds();
    auto style = GetStyle();
    if (!style) return;

    float opacity = GetAnimatedOpacity() * GetOpacity();

    StyleStateColors bgColors;
    bgColors.background = Theme::SemanticColor::WindowBackground;
    renderer.FillBackground(bounds, bgColors, 0.0f, opacity);

    float currentX = bounds.left;

    for (int i = 0; i < static_cast<int>(m_tabs.size()); ++i)
    {
        float tabWidth = (std::max)(static_cast<float>(m_tabs[i].headerText.length()) * 8.0f + 24.0f, MinTabWidth);
        D2D1_RECT_F tabRect = {currentX, bounds.top, currentX + tabWidth, bounds.top + TabHeaderHeight};

        bool isSelected = (i == m_selectedIndex);
        bool isHovered = (i == m_hoveredTab);

        ElementState tabState = ElementState::Normal;
        if (isSelected) tabState = ElementState::Pressed;
        else if (isHovered) tabState = ElementState::Hover;

        const auto& tabColors = style->ResolveState(tabState);

        if (isSelected)
        {
            StyleStateColors selColors;
            selColors.background = Theme::SemanticColor::WindowBackground;
            selColors.border = Theme::SemanticColor::WindowBorder;
            renderer.FillBackground(tabRect, selColors, 0.0f, opacity);
            renderer.DrawBorder(tabRect, selColors, 1.0f, 0.0f, opacity);
        }
        else
        {
            renderer.FillBackground(tabRect, tabColors, 0.0f, opacity * 0.8f);
            renderer.DrawBorder(tabRect, tabColors, 1.0f, 0.0f, opacity * 0.5f);
        }

        D2D1_RECT_F textRect = {
            tabRect.left + 8.0f, tabRect.top + 6.0f, tabRect.right - 8.0f, tabRect.bottom - 6.0f
        };
        renderer.DrawText(m_tabs[i].headerText, textRect, tabColors);

        currentX += tabWidth;
    }

    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_tabs.size()))
    {
        if (m_tabs[m_selectedIndex].content &&
            m_tabs[m_selectedIndex].content->GetVisibility() == Visibility::Visible)
        {
            D2D1_RECT_F contentArea = {
                bounds.left, bounds.top + TabHeaderHeight, bounds.right, bounds.bottom
            };
            renderer.PushClip(contentArea);
            m_tabs[m_selectedIndex].content->Render(renderer);
            renderer.PopClip();
        }
    }
}

bool TabControl::OnEvent(const UIEvent& event) noexcept
{
    auto bounds = GetBounds();

    switch (event.type)
    {
    case UIEventType::MouseMove:
    {
        if (event.y >= bounds.top && event.y <= bounds.top + TabHeaderHeight)
        {
            float currentX = bounds.left;
            for (int i = 0; i < static_cast<int>(m_tabs.size()); ++i)
            {
                float tabWidth = (std::max)(static_cast<float>(m_tabs[i].headerText.length()) * 8.0f + 24.0f, MinTabWidth);
                if (event.x >= currentX && event.x <= currentX + tabWidth)
                {
                    if (m_hoveredTab != i)
                    {
                        m_hoveredTab = i;
                        InvalidateVisual();
                    }
                    return true;
                }
                currentX += tabWidth;
            }
        }
        if (m_hoveredTab != -1)
        {
            m_hoveredTab = -1;
            InvalidateVisual();
        }
        return false;
    }

    case UIEventType::MouseDown:
    {
        if (event.y >= bounds.top && event.y <= bounds.top + TabHeaderHeight)
        {
            float currentX = bounds.left;
            for (int i = 0; i < static_cast<int>(m_tabs.size()); ++i)
            {
                float tabWidth = (std::max)(static_cast<float>(m_tabs[i].headerText.length()) * 8.0f + 24.0f, MinTabWidth);
                if (event.x >= currentX && event.x <= currentX + tabWidth)
                {
                    SetSelectedIndex(i);
                    return true;
                }
                currentX += tabWidth;
            }
        }
        return false;
    }

    default:
        return false;
    }
}

} // namespace
