#include <DragonUI/Controls/MenuItem.hpp>
#include <DragonUI/Controls/Menu.hpp>
#include <DragonUI/Core/RenderContext.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

UIMenuItem::UIMenuItem(std::wstring_view text) noexcept
    : m_text(text)
{
    m_focusable = false;
    SetMinSize(MinItemWidth, ItemHeight);
}

void UIMenuItem::SetText(std::wstring_view text) noexcept
{
    m_text = text;
    InvalidateLayout();
}

void UIMenuItem::SetShortcut(std::wstring_view shortcut) noexcept
{
    m_shortcut = shortcut;
    InvalidateLayout();
}

void UIMenuItem::SetChecked(bool checked) noexcept
{
    m_checked = checked;
    InvalidateVisual();
}

void UIMenuItem::SetSubmenu(std::unique_ptr<UIMenu> submenu) noexcept
{
    m_submenu = std::move(submenu);
    if (m_submenu)
    {
        m_submenu->SetParent(nullptr);
    }
    InvalidateVisual();
}

void UIMenuItem::SetHovered(bool hovered) noexcept
{
    m_hovered = hovered;
    SetControlState(hovered ? ControlState::Hover : ControlState::Normal);
    InvalidateVisual();
}

DesiredSize UIMenuItem::MeasureOverride(const LayoutSlot& available) noexcept
{
    if (m_isSeparator)
    {
        float w = (std::min)((std::max)(MinItemWidth, available.width), available.width);
        return { w, SeparatorHeight };
    }

    float textW = static_cast<float>(m_text.size()) * 8.0f;
    float scW = m_shortcut.empty() ? 0.0f : ShortcutColumnW;
    float arrowW = HasSubmenu() ? ArrowColumnW : 0.0f;
    float iconW = m_iconGlyph ? IconColumnW : 0.0f;
    float checkW = m_checked ? CheckColumnW : 0.0f;
    float totalW = PaddingH + iconW + checkW + textW + scW + arrowW + PaddingH;
    totalW = (std::min)((std::max)(totalW, MinItemWidth), MaxItemWidth);
    totalW = (std::min)(totalW, available.width);

    return { totalW, ItemHeight };
}

void UIMenuItem::Render(RenderContext& ctx) noexcept
{
    if (m_isSeparator)
    {
        auto slot = GetBounds();
        float midY = slot.y + slot.height * 0.5f;
        D2D1_RECT_F lineRect = {
            slot.x + PaddingH, midY,
            slot.x + slot.width - PaddingH, midY + 1.0f
        };
        ctx.FillRectangle(lineRect, Theme::SemanticColor::MenuSeparator);
        return;
    }

    auto slot = GetBounds();
    auto d2d = static_cast<D2D1_RECT_F>(slot);

    bool isDisabled = !IsEnabled();
    bool isHovered = (GetControlState() == ControlState::Hover) || m_hovered;

    if (isHovered && !isDisabled)
    {
        ctx.FillRectangle(d2d, Theme::SemanticColor::MenuItemHover);
    }

    float x = d2d.left + PaddingH;
    float centerY = d2d.top + slot.height * 0.5f;

    Theme::SemanticColor textColor = isDisabled
        ? Theme::SemanticColor::MenuItemTextDisabled
        : Theme::SemanticColor::MenuItemText;

    Theme::SemanticColor iconColor = isDisabled
        ? Theme::SemanticColor::MenuItemTextDisabled
        : Theme::SemanticColor::TextPrimary;

    if (m_checked)
    {
        float cx = x + CheckColumnW * 0.5f - 4.0f;
        float cy = centerY - 4.0f;
        D2D1_RECT_F checkRect = { cx, cy, cx + 8.0f, cy + 8.0f };
        ctx.FillRectangle(checkRect, isHovered
            ? Theme::SemanticColor::Accent
            : Theme::SemanticColor::MenuItemSelected);
        x += CheckColumnW;
    }

    if (m_iconGlyph)
    {
        std::wstring_view glyphStr(reinterpret_cast<const wchar_t*>(&m_iconGlyph), 1);
        auto gSize = ctx.MeasureText(glyphStr, IconColumnW);
        D2D1_RECT_F gRect = {
            x + (IconColumnW - gSize.width) * 0.5f,
            centerY - gSize.height * 0.5f,
            x + (IconColumnW - gSize.width) * 0.5f + gSize.width,
            centerY - gSize.height * 0.5f + gSize.height
        };
        ctx.DrawText(glyphStr, gRect, iconColor);
        x += IconColumnW;
    }

    if (!m_text.empty())
    {
        float maxTextW = slot.width - (x - d2d.left) - PaddingH;
        if (!m_shortcut.empty()) maxTextW -= ShortcutColumnW;
        if (HasSubmenu()) maxTextW -= ArrowColumnW;

        auto textSize = ctx.MeasureText(m_text, maxTextW);
        D2D1_RECT_F textRect = {
            x, centerY - textSize.height * 0.5f,
            x + textSize.width, centerY + textSize.height * 0.5f
        };
        ctx.DrawText(m_text, textRect, textColor);
    }

    if (!m_shortcut.empty())
    {
        float sx = d2d.right - PaddingH - ShortcutColumnW;
        if (HasSubmenu()) sx -= ArrowColumnW;
        auto scSize = ctx.MeasureText(m_shortcut, ShortcutColumnW);
        D2D1_RECT_F scRect = {
            sx, centerY - scSize.height * 0.5f,
            sx + scSize.width, centerY + scSize.height * 0.5f
        };
        ctx.DrawText(m_shortcut, scRect, Theme::SemanticColor::TextSecondary);
    }

    if (HasSubmenu())
    {
        float ax = d2d.right - PaddingH - ArrowColumnW;
        std::wstring arrow = L"\u25B6";
        auto aSize = ctx.MeasureText(arrow, ArrowColumnW);
        D2D1_RECT_F aRect = {
            ax + (ArrowColumnW - aSize.width) * 0.5f,
            centerY - aSize.height * 0.5f,
            ax + (ArrowColumnW - aSize.width) * 0.5f + aSize.width,
            centerY - aSize.height * 0.5f + aSize.height
        };
        ctx.DrawText(arrow, aRect, textColor);
    }
}

bool UIMenuItem::OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept
{
    if (m_isSeparator) return false;

    switch (type)
    {
    case EventType::MouseEnter:
        SetHovered(true);
        return true;
    case EventType::MouseLeave:
        SetHovered(false);
        return true;
    case EventType::Click:
        if (IsEnabled() && !HasSubmenu() && m_onAction)
        {
            m_onAction();
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

bool UIMenuItem::OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept
{
    if (type == EventType::KeyDown && args.key == Input::KeyCode::Return)
    {
        if (IsEnabled() && !HasSubmenu() && m_onAction)
        {
            m_onAction();
            return true;
        }
    }
    return false;
}

} // namespace
