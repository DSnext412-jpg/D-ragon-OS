#include <DragonUI/Controls/ToolBar.hpp>
#include <DragonUI/Core/RenderContext.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

size_t UIToolBar::AddButton(std::wstring_view text, ButtonCallback cb, uint32_t icon) noexcept
{
    size_t idx = m_buttons.size();
    ToolBarButton btn;
    btn.text = text;
    btn.iconGlyph = icon;
    btn.callback = std::move(cb);
    btn.isSeparator = false;
    m_buttons.push_back(std::move(btn));
    InvalidateLayout();
    return idx;
}

size_t UIToolBar::AddDropDownButton(std::wstring_view text, std::unique_ptr<UIMenu> menu, uint32_t icon) noexcept
{
    size_t idx = m_buttons.size();
    ToolBarButton btn;
    btn.text = text;
    btn.iconGlyph = icon;
    btn.isSeparator = false;
    btn.dropDownMenu = std::move(menu);
    m_buttons.push_back(std::move(btn));
    InvalidateLayout();
    return idx;
}

void UIToolBar::AddSeparator() noexcept
{
    ToolBarButton sep;
    sep.isSeparator = true;
    m_buttons.push_back(std::move(sep));
    InvalidateLayout();
}

void UIToolBar::ClearButtons() noexcept
{
    m_buttons.clear();
    m_hoveredIndex = -1;
    m_pressedIndex = -1;
    InvalidateLayout();
}

void UIToolBar::SetButtonEnabled(size_t index, bool enabled) noexcept
{
    if (index < m_buttons.size())
    {
        m_buttons[index].enabled = enabled;
        InvalidateVisual();
    }
}

bool UIToolBar::IsButtonEnabled(size_t index) const noexcept
{
    if (index < m_buttons.size())
        return m_buttons[index].enabled;
    return false;
}

void UIToolBar::CloseDropDown() noexcept
{
    if (m_openDropDown >= 0 && m_openDropDown < static_cast<int>(m_buttons.size()))
    {
        auto& btn = m_buttons[m_openDropDown];
        if (btn.dropDownMenu)
        {
            btn.dropDownMenu->SetOpen(false);
        }
    }
    m_openDropDown = -1;
}

DesiredSize UIToolBar::MeasureOverride(const LayoutSlot& available) noexcept
{
    float totalW = 0.0f;
    float maxH = DefaultButtonSize;

    for (auto& btn : m_buttons)
    {
        if (btn.isSeparator)
        {
            totalW += SeparatorWidth;
        }
        else
        {
            float btnW = DefaultButtonSize;
            if (!btn.text.empty())
            {
                float textW = static_cast<float>(btn.text.size()) * 7.0f;
                btnW += textW;
                if (btn.iconGlyph) btnW += 4.0f;
            }
            totalW += (std::min)(btnW, DefaultButtonSize * 2.0f);
        }
        totalW += m_spacing;
    }

    if (!m_buttons.empty()) totalW -= m_spacing;
    return {(std::min)(totalW, available.width), (std::min)(maxH, available.height)};
}

void UIToolBar::ArrangeOverride(const LayoutSlot& finalSlot) noexcept
{
    float x = finalSlot.x;
    float y = finalSlot.y;
    float h = finalSlot.height;

    for (auto& btn : m_buttons)
    {
        if (btn.isSeparator)
        {
            btn.iconGlyph = 0;
            x += SeparatorWidth + m_spacing;
            continue;
        }

        float btnW = DefaultButtonSize;
        if (!btn.text.empty())
        {
            float textW = static_cast<float>(btn.text.size()) * 7.0f;
            btnW += textW;
            if (btn.iconGlyph) btnW += 4.0f;
        }
        btnW = (std::min)(btnW, DefaultButtonSize * 2.0f);

        x += m_spacing;
    }
}

void UIToolBar::Render(RenderContext& ctx) noexcept
{
    auto d2d = static_cast<D2D1_RECT_F>(GetBounds());
    ctx.FillRectangle(d2d, Theme::SemanticColor::ToolbarBackground);

    float x = GetBounds().x + m_spacing;
    float y = GetBounds().y;
    float h = GetBounds().height;

    for (int i = 0; i < static_cast<int>(m_buttons.size()); ++i)
    {
        auto& btn = m_buttons[i];

        if (btn.isSeparator)
        {
            float midY = y + h * 0.5f;
            D2D1_RECT_F sepRect = {
                x, midY - 6.0f,
                x + 1.0f, midY + 6.0f
            };
            ctx.FillRectangle(sepRect, Theme::SemanticColor::ControlBorder);
            x += SeparatorWidth;
            continue;
        }

        float btnW = DefaultButtonSize;
        if (!btn.text.empty())
        {
            float textW = static_cast<float>(btn.text.size()) * 7.0f;
            btnW += textW;
            if (btn.iconGlyph) btnW += 4.0f;
        }
        btnW = (std::min)(btnW, DefaultButtonSize * 2.0f);

        D2D1_RECT_F btnRect = {x, y, x + btnW, y + h};

        bool isHovered = (i == m_hoveredIndex);
        bool isPressed = (i == m_pressedIndex);
        bool isDisabled = !btn.enabled;

        if (isPressed && !isDisabled)
        {
            ctx.FillRoundedRect(btnRect, Theme::SemanticColor::ToolbarButtonPressed,
                                CornerRadius, CornerRadius);
        }
        else if (isHovered && !isDisabled)
        {
            ctx.FillRoundedRect(btnRect, Theme::SemanticColor::ToolbarButtonHover,
                                CornerRadius, CornerRadius);
        }

        float cx = x + btnW * 0.5f;

        if (btn.iconGlyph)
        {
            std::wstring_view glyphStr(reinterpret_cast<const wchar_t*>(&btn.iconGlyph), 1);
            auto gSize = ctx.MeasureText(glyphStr, m_iconSize);
            float gy = y + (h - gSize.height) * 0.5f;

            if (!btn.text.empty())
            {
                float totalW = m_iconSize + 4.0f + static_cast<float>(btn.text.size()) * 7.0f;
                float startX = cx - totalW * 0.5f;
                D2D1_RECT_F gRect = {
                    startX, gy,
                    startX + gSize.width, gy + gSize.height
                };
                ctx.DrawText(glyphStr, gRect,
                    isDisabled ? Theme::SemanticColor::MenuItemTextDisabled
                               : Theme::SemanticColor::TextPrimary);

                D2D1_RECT_F tRect = {
                    startX + m_iconSize + 4.0f, y + (h - gSize.height) * 0.5f,
                    startX + totalW, y + (h - gSize.height) * 0.5f + gSize.height
                };
                ctx.DrawText(btn.text, tRect,
                    isDisabled ? Theme::SemanticColor::MenuItemTextDisabled
                               : Theme::SemanticColor::TextPrimary);
            }
            else
            {
                D2D1_RECT_F gRect = {
                    cx - gSize.width * 0.5f, gy,
                    cx + gSize.width * 0.5f, gy + gSize.height
                };
                ctx.DrawText(glyphStr, gRect,
                    isDisabled ? Theme::SemanticColor::MenuItemTextDisabled
                               : Theme::SemanticColor::TextPrimary);
            }
        }
        else if (!btn.text.empty())
        {
            auto textSize = ctx.MeasureText(btn.text, btnW - 8.0f);
            D2D1_RECT_F tRect = {
                x + (btnW - textSize.width) * 0.5f,
                y + (h - textSize.height) * 0.5f,
                x + (btnW - textSize.width) * 0.5f + textSize.width,
                y + (h - textSize.height) * 0.5f + textSize.height
            };
            ctx.DrawText(btn.text, tRect,
                isDisabled ? Theme::SemanticColor::MenuItemTextDisabled
                           : Theme::SemanticColor::TextPrimary);
        }

        if (btn.dropDownMenu)
        {
            std::wstring arrow = L"\u25BC";
            float ax = x + btnW - 10.0f;
            auto aSize = ctx.MeasureText(arrow, 10.0f);
            D2D1_RECT_F aRect = {
                ax, y + (h - aSize.height) * 0.5f,
                ax + aSize.width, y + (h - aSize.height) * 0.5f + aSize.height
            };
            ctx.DrawText(arrow, aRect, Theme::SemanticColor::TextSecondary);
        }

        x += btnW + m_spacing;
    }

    for (int i = 0; i < static_cast<int>(m_buttons.size()); ++i)
    {
        auto& btn = m_buttons[i];
        if (btn.dropDownMenu && btn.dropDownMenu->IsOpen())
        {
            btn.dropDownMenu->Render(ctx);
        }
    }
}

bool UIToolBar::OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept
{
    switch (type)
    {
    case EventType::MouseMove:
    {
        int idx = HitTest(args.x, args.y);
        m_hoveredIndex = idx;
        InvalidateVisual();
        return idx >= 0;
    }
    case EventType::Click:
    {
        int idx = HitTest(args.x, args.y);
        if (idx >= 0 && idx < static_cast<int>(m_buttons.size()))
        {
            auto& btn = m_buttons[idx];
            if (!btn.enabled) return true;

            if (btn.dropDownMenu)
            {
                if (m_openDropDown == idx)
                {
                    CloseDropDown();
                }
                else
                {
                    CloseDropDown();
                    btn.dropDownMenu->SetPopupPosition(args.x, args.y);
                    btn.dropDownMenu->SetOpen(true);
                    m_openDropDown = idx;
                }
            }
            else if (btn.callback)
            {
                btn.callback();
            }
            return true;
        }
        break;
    }
    case EventType::MouseLeave:
        m_hoveredIndex = -1;
        InvalidateVisual();
        return true;
    default:
        break;
    }
    return false;
}

int UIToolBar::HitTest(float px, float py) const noexcept
{
    float x = GetBounds().x + m_spacing;
    float y = GetBounds().y;
    float h = GetBounds().height;

    for (int i = 0; i < static_cast<int>(m_buttons.size()); ++i)
    {
        const auto& btn = m_buttons[i];

        if (btn.isSeparator)
        {
            x += SeparatorWidth;
            continue;
        }

        float btnW = DefaultButtonSize;
        if (!btn.text.empty())
        {
            float textW = static_cast<float>(btn.text.size()) * 7.0f;
            btnW += textW;
            if (btn.iconGlyph) btnW += 4.0f;
        }
        btnW = (std::min)(btnW, DefaultButtonSize * 2.0f);

        if (px >= x && px < x + btnW && py >= y && py < y + h)
        {
            return i;
        }

        x += btnW + m_spacing;
    }

    return -1;
}

} // namespace
