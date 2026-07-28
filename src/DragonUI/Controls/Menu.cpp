#include <DragonUI/Controls/Menu.hpp>
#include <DragonUI/Core/RenderContext.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

UIMenuItem* UIMenu::AddItem(std::wstring_view text, UIMenuItem::ActionCallback cb) noexcept
{
    auto item = std::make_unique<UIMenuItem>(text);
    item->SetOnAction(std::move(cb));
    auto* ptr = item.get();
    AddChild(std::move(item));
    RebuildItemList();
    return ptr;
}

UIMenuItem* UIMenu::AddSubMenuItem(std::wstring_view text, std::unique_ptr<UIMenu> submenu) noexcept
{
    auto item = std::make_unique<UIMenuItem>(text);
    item->SetSubmenu(std::move(submenu));
    auto* ptr = item.get();
    AddChild(std::move(item));
    RebuildItemList();
    return ptr;
}

void UIMenu::AddSeparator() noexcept
{
    auto item = std::make_unique<UIMenuItem>();
    item->SetSeparator(true);
    AddChild(std::move(item));
    RebuildItemList();
}

void UIMenu::ClearItems() noexcept
{
    m_hoveredIndex = -1;
    CloseSubmenus();
    ClearChildren();
    m_itemList.clear();
}

void UIMenu::SetPopupPosition(float x, float y) noexcept
{
    m_popupX = x;
    m_popupY = y;
}

void UIMenu::SetOpen(bool open) noexcept
{
    m_isOpen = open;
    if (!open)
    {
        m_hoveredIndex = -1;
        CloseSubmenus();
    }
}

void UIMenu::SetHoveredIndex(int index) noexcept
{
    if (m_hoveredIndex == index) return;

    if (m_hoveredIndex >= 0 && m_hoveredIndex < static_cast<int>(m_itemList.size()))
    {
        m_itemList[m_hoveredIndex]->SetHovered(false);
    }

    m_hoveredIndex = index;

    if (m_hoveredIndex >= 0 && m_hoveredIndex < static_cast<int>(m_itemList.size()))
    {
        auto* item = m_itemList[m_hoveredIndex];
        item->SetHovered(true);

        if (item->HasSubmenu())
        {
            CloseSubmenus();
            auto* submenu = item->GetSubmenu();
            auto itemBounds = item->GetBounds();
            float sx = GetBounds().x + GetBounds().width;
            float sy = GetBounds().y + itemBounds.y - GetBounds().y;
            submenu->SetPopupPosition(sx, sy);
            submenu->SetOpen(true);
            m_openSubmenu = submenu;
        }
        else
        {
            CloseSubmenus();
        }
    }
    else
    {
        CloseSubmenus();
    }
}

void UIMenu::SelectNext() noexcept
{
    int start = m_hoveredIndex;
    int idx = start;
    do {
        idx = (idx + 1) % static_cast<int>(m_itemList.size());
        if (!IsSeparatorItem(idx) && m_itemList[idx]->IsEnabled())
        {
            SetHoveredIndex(idx);
            return;
        }
    } while (idx != start);
}

void UIMenu::SelectPrevious() noexcept
{
    int start = m_hoveredIndex;
    int idx = start;
    do {
        idx = (idx - 1 + static_cast<int>(m_itemList.size())) % static_cast<int>(m_itemList.size());
        if (!IsSeparatorItem(idx) && m_itemList[idx]->IsEnabled())
        {
            SetHoveredIndex(idx);
            return;
        }
    } while (idx != start);
}

void UIMenu::ActivateHovered() noexcept
{
    if (m_hoveredIndex < 0 || m_hoveredIndex >= static_cast<int>(m_itemList.size())) return;
    auto* item = m_itemList[m_hoveredIndex];
    if (!item || !item->IsEnabled() || item->IsSeparator()) return;

    if (item->HasSubmenu())
    {
        auto* submenu = item->GetSubmenu();
        if (submenu->GetChildCount() > 0)
        {
            submenu->SetHoveredIndex(0);
        }
    }
    else if (item->IsChecked())
    {
        item->SetChecked(false);
    }
    else
    {
        if (!item->IsChecked())
        {
            item->SetChecked(true);
        }
    }
}

bool UIMenu::HasCheckableItems() const noexcept
{
    for (auto* item : m_itemList)
    {
        if (item && !item->IsSeparator() && item->IsChecked())
            return true;
    }
    return false;
}

DesiredSize UIMenu::MeasureOverride(const LayoutSlot& available) noexcept
{
    float maxW = MinWidth;
    float totalH = 0.0f;

    for (auto* item : m_itemList)
    {
        if (!item) continue;
        item->Measure({ 0, 0, available.width, available.height });
        auto ds = item->GetDesiredSize();
        if (ds.width > maxW) maxW = ds.width;
        totalH += ds.height;
    }

    maxW = (std::min)((std::max)(maxW, MinWidth), MaxWidth);

    for (auto* item : m_itemList)
    {
        if (!item) continue;
        item->Measure({ 0, 0, maxW, available.height });
    }

    totalH += BorderThickness * 2.0f;
    return { maxW, totalH };
}

void UIMenu::ArrangeOverride(const LayoutSlot& finalSlot) noexcept
{
    float y = finalSlot.y + BorderThickness;
    for (auto* item : m_itemList)
    {
        if (!item) continue;
        float itemH = item->GetDesiredSize().height;
        item->Arrange({ finalSlot.x + BorderThickness, y,
                        finalSlot.width - BorderThickness * 2.0f, itemH });
        y += itemH;
    }
}

void UIMenu::Render(RenderContext& ctx) noexcept
{
    if (!m_isOpen) return;

    auto d2d = static_cast<D2D1_RECT_F>(GetBounds());

    ctx.FillRoundedRect(d2d, Theme::SemanticColor::MenuBackground,
                         CornerRadius, CornerRadius);
    ctx.DrawRoundedRect(d2d, Theme::SemanticColor::MenuSeparator,
                        CornerRadius, CornerRadius, BorderThickness);

    RenderChildren(ctx);

    if (m_openSubmenu && m_openSubmenu->IsOpen())
    {
        m_openSubmenu->Render(ctx);
    }
}

bool UIMenu::OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept
{
    if (!m_isOpen) return false;

    if (type == EventType::MouseMove)
    {
        for (size_t i = 0; i < m_itemList.size(); ++i)
        {
            auto* item = m_itemList[i];
            if (!item || item->IsSeparator()) continue;
            auto bounds = item->GetBounds();
            if (args.x >= bounds.x && args.x <= bounds.x + bounds.width &&
                args.y >= bounds.y && args.y <= bounds.y + bounds.height)
            {
                SetHoveredIndex(static_cast<int>(i));
                return true;
            }
        }
    }

    if (type == EventType::Click)
    {
        for (size_t i = 0; i < m_itemList.size(); ++i)
        {
            auto* item = m_itemList[i];
            if (!item || item->IsSeparator() || !item->IsEnabled()) continue;
            auto bounds = item->GetBounds();
            if (args.x >= bounds.x && args.x <= bounds.x + bounds.width &&
                args.y >= bounds.y && args.y <= bounds.y + bounds.height)
            {
                if (item->HasSubmenu()) return true;
                item->OnMouseEvent(type, args);
                return true;
            }
        }
    }

    return false;
}

bool UIMenu::OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept
{
    if (!m_isOpen) return false;
    if (type != EventType::KeyDown) return false;

    switch (args.key)
    {
    case Input::KeyCode::Down:
        SelectNext();
        return true;
    case Input::KeyCode::Up:
        SelectPrevious();
        return true;
    case Input::KeyCode::Return:
        ActivateHovered();
        return true;
    case Input::KeyCode::Escape:
        SetOpen(false);
        return true;
    case Input::KeyCode::Right:
        if (m_hoveredIndex >= 0 && m_hoveredIndex < static_cast<int>(m_itemList.size()))
        {
            auto* item = m_itemList[m_hoveredIndex];
            if (item && item->HasSubmenu())
            {
                auto* sub = item->GetSubmenu();
                if (sub && sub->GetChildCount() > 0)
                {
                    sub->SetHoveredIndex(0);
                    return true;
                }
            }
        }
        break;
    case Input::KeyCode::Left:
        SetOpen(false);
        return true;
    default:
        break;
    }
    return false;
}

void UIMenu::CloseSubmenus() noexcept
{
    if (m_openSubmenu)
    {
        m_openSubmenu->SetOpen(false);
        m_openSubmenu = nullptr;
    }
    for (auto* item : m_itemList)
    {
        if (!item) continue;
        auto* sub = item->GetSubmenu();
        if (sub) sub->SetOpen(false);
    }
}

bool UIMenu::IsSeparatorItem(int index) const noexcept
{
    if (index < 0 || index >= static_cast<int>(m_itemList.size())) return true;
    auto* item = m_itemList[index];
    return !item || item->IsSeparator();
}

void UIMenu::RebuildItemList() noexcept
{
    m_itemList.clear();
    for (auto& child : GetChildren())
    {
        auto* item = dynamic_cast<UIMenuItem*>(child.get());
        if (item)
        {
            m_itemList.push_back(item);
        }
    }
}

} // namespace
