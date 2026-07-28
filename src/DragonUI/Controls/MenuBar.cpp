#include <DragonUI/Controls/MenuBar.hpp>
#include <DragonUI/Core/RenderContext.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

size_t UIMenuBar::AddMenu(std::wstring_view text, std::unique_ptr<UIMenu> menu) noexcept
{
    size_t idx = m_entries.size();
    MenuBarEntry entry;
    entry.text = text;
    entry.menu = std::move(menu);

    auto& e = m_entries.emplace_back(std::move(entry));
    auto textSize = static_cast<float>(text.size()) * 8.0f;
    e.measuredWidth = textSize + ItemPaddingH * 2.0f;

    InvalidateLayout();
    return idx;
}

void UIMenuBar::ClearMenus() noexcept
{
    m_entries.clear();
    m_openIndex = static_cast<size_t>(-1);
    m_hoveredIndex = static_cast<size_t>(-1);
    InvalidateLayout();
}

void UIMenuBar::OpenMenu(size_t index) noexcept
{
    if (index >= m_entries.size()) return;

    CloseAll();
    m_openIndex = index;
    m_altActive = true;

    auto& entry = m_entries[index];
    if (entry.menu)
    {
        auto slot = entry.menu->MeasureOverride({0, 0, 400, 800});
        float px = GetBounds().x;
        for (size_t i = 0; i < index; ++i)
        {
            px += m_entries[i].measuredWidth;
        }
        float py = GetBounds().y + GetBounds().height;
        entry.menu->SetPopupPosition(px, py);
        entry.menu->SetBounds({px, py, slot.width, slot.height});
        entry.menu->SetOpen(true);

        if (entry.menu->GetChildCount() > 0)
        {
            entry.menu->SetHoveredIndex(0);
        }
    }
}

void UIMenuBar::CloseMenu() noexcept
{
    if (m_openIndex < m_entries.size())
    {
        auto& entry = m_entries[m_openIndex];
        if (entry.menu)
        {
            entry.menu->SetOpen(false);
            entry.menu->CloseSubmenus();
        }
    }
    m_openIndex = static_cast<size_t>(-1);
    m_altActive = false;
}

void UIMenuBar::CloseAll() noexcept
{
    CloseMenu();
    m_hoveredIndex = static_cast<size_t>(-1);
    m_altActive = false;
}

void UIMenuBar::SelectNext() noexcept
{
    if (m_openIndex == static_cast<size_t>(-1))
    {
        if (!m_entries.empty()) OpenMenu(0);
        return;
    }

    size_t next = (m_openIndex + 1) % m_entries.size();
    CloseMenu();
    OpenMenu(next);
}

void UIMenuBar::SelectPrevious() noexcept
{
    if (m_openIndex == static_cast<size_t>(-1))
    {
        if (!m_entries.empty()) OpenMenu(0);
        return;
    }

    size_t prev = (m_openIndex - 1 + m_entries.size()) % m_entries.size();
    CloseMenu();
    OpenMenu(prev);
}

void UIMenuBar::ActivateFocused() noexcept
{
    if (m_hoveredIndex < m_entries.size())
    {
        OpenMenu(m_hoveredIndex);
    }
}

void UIMenuBar::SetAltActive(bool active) noexcept
{
    m_altActive = active;
    if (!active)
    {
        CloseAll();
    }
    InvalidateVisual();
}

DesiredSize UIMenuBar::MeasureOverride(const LayoutSlot& available) noexcept
{
    float totalW = 0.0f;
    for (auto& entry : m_entries)
    {
        totalW += entry.measuredWidth;
    }
    float h = (std::max)(MinItemHeight, available.height);
    return {(std::min)(totalW, available.width), h};
}

void UIMenuBar::ArrangeOverride(const LayoutSlot& finalSlot) noexcept
{
    float x = finalSlot.x;
    for (auto& entry : m_entries)
    {
        entry.menu->Arrange({
            x, finalSlot.y,
            entry.measuredWidth, finalSlot.height
        });
        x += entry.measuredWidth;
    }
}

void UIMenuBar::Render(RenderContext& ctx) noexcept
{
    auto d2d = static_cast<D2D1_RECT_F>(GetBounds());
    ctx.FillRectangle(d2d, Theme::SemanticColor::MenuBarBackground);

    float x = GetBounds().x;
    for (size_t i = 0; i < m_entries.size(); ++i)
    {
        auto& entry = m_entries[i];
        float w = entry.measuredWidth;
        D2D1_RECT_F itemRect = {x, GetBounds().y, x + w, GetBounds().y + GetBounds().height};

        bool isHovered = (i == m_hoveredIndex);
        bool isOpen = (i == m_openIndex);

        if (isOpen)
        {
            ctx.FillRectangle(itemRect, Theme::SemanticColor::MenuBackground);
        }
        else if (isHovered && m_altActive)
        {
            ctx.FillRectangle(itemRect, Theme::SemanticColor::MenuItemHover);
        }

        if (!entry.text.empty())
        {
            auto textSize = ctx.MeasureText(entry.text, w - ItemPaddingH * 2.0f);
            D2D1_RECT_F textRect = {
                x + (w - textSize.width) * 0.5f,
                GetBounds().y + (GetBounds().height - textSize.height) * 0.5f,
                x + (w - textSize.width) * 0.5f + textSize.width,
                GetBounds().y + (GetBounds().height - textSize.height) * 0.5f + textSize.height
            };
            ctx.DrawText(entry.text, textRect, Theme::SemanticColor::MenuBarText);
        }

        if (m_altActive && isOpen)
        {
            D2D1_RECT_F underline = {
                x + 4.0f, GetBounds().y + GetBounds().height - 2.0f,
                x + w - 4.0f, GetBounds().y + GetBounds().height
            };
            ctx.FillRectangle(underline, Theme::SemanticColor::Accent);
        }

        x += w;
    }

    for (auto& entry : m_entries)
    {
        if (entry.menu && entry.menu->IsOpen())
        {
            entry.menu->Render(ctx);
        }
    }
}

bool UIMenuBar::OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept
{
    switch (type)
    {
    case EventType::MouseMove:
    {
        int idx = HitTestItems(args.x);
        if (idx >= 0)
        {
            m_hoveredIndex = static_cast<size_t>(idx);
            InvalidateVisual();
        }
        return true;
    }
    case EventType::Click:
    {
        int idx = HitTestItems(args.x);
        if (idx >= 0 && static_cast<size_t>(idx) < m_entries.size())
        {
            if (m_openIndex == static_cast<size_t>(idx))
            {
                CloseAll();
            }
            else
            {
                OpenMenu(static_cast<size_t>(idx));
            }
            return true;
        }
        break;
    }
    default:
        break;
    }
    return false;
}

bool UIMenuBar::OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept
{
    if (type != EventType::KeyDown) return false;

    switch (args.key)
    {
    case Input::KeyCode::Left:
        if (HasOpenMenu())
        {
            SelectPrevious();
            return true;
        }
        break;
    case Input::KeyCode::Right:
        if (HasOpenMenu())
        {
            SelectNext();
            return true;
        }
        break;
    case Input::KeyCode::Down:
        if (HasOpenMenu())
        {
            auto& entry = m_entries[m_openIndex];
            if (entry.menu && entry.menu->GetChildCount() > 0)
            {
                entry.menu->SelectNext();
                return true;
            }
        }
        break;
    case Input::KeyCode::Return:
        if (HasOpenMenu())
        {
            auto& entry = m_entries[m_openIndex];
            if (entry.menu)
            {
                entry.menu->ActivateHovered();
                return true;
            }
        }
        break;
    case Input::KeyCode::Escape:
        if (HasOpenMenu())
        {
            CloseAll();
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

int UIMenuBar::HitTestItems(float px) const noexcept
{
    float x = GetBounds().x;
    for (size_t i = 0; i < m_entries.size(); ++i)
    {
        if (px >= x && px < x + m_entries[i].measuredWidth)
        {
            return static_cast<int>(i);
        }
        x += m_entries[i].measuredWidth;
    }
    return -1;
}

} // namespace
