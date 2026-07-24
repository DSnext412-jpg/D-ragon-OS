#include <DragonUI/Controls/Button.hpp>
#include <DragonUI/Core/RenderContext.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

UIButton::UIButton(std::wstring_view text) noexcept
    : m_text(text)
{
    m_focusable = true;
    SetMinSize(80.0f, 32.0f);
}

void UIButton::SetText(std::wstring_view text) noexcept
{
    m_text = text;
    InvalidateLayout();
}

DesiredSize UIButton::MeasureOverride(const LayoutSlot& available) noexcept
{
    float textW = static_cast<float>(m_text.size()) * 8.0f;
    float iconW = m_iconGlyph ? 24.0f : 0.0f;
    float totalW = textW + iconW + 24.0f;
    return {std::min(totalW, available.width), 32.0f};
}

void UIButton::Render(RenderContext& ctx) noexcept
{
    Control::Render(ctx);

    auto slot = GetBounds();
    auto d2d = static_cast<D2D1_RECT_F>(slot);

    Theme::SemanticColor bg = Theme::SemanticColor::Accent;
    Theme::SemanticColor fg = Theme::SemanticColor::TextPrimary;
    Theme::SemanticColor border = Theme::SemanticColor::Accent;

    switch (GetControlState())
    {
    case ControlState::Hover:
        bg = Theme::SemanticColor::AccentHover;
        break;
    case ControlState::Pressed:
        bg = Theme::SemanticColor::AccentPressed;
        break;
    case ControlState::Focused:
        bg = Theme::SemanticColor::AccentHover;
        border = Theme::SemanticColor::Accent;
        break;
    case ControlState::Disabled:
        bg = Theme::SemanticColor::Disabled;
        border = Theme::SemanticColor::Disabled;
        fg = Theme::SemanticColor::TextSecondary;
        break;
    default:
        break;
    }

    ctx.FillRoundedRect(d2d, bg, m_cornerRadius, m_cornerRadius);
    ctx.DrawRoundedRect(d2d, border, m_cornerRadius, m_cornerRadius);

    if (GetControlState() == ControlState::Focused)
        ctx.DrawRoundedRect(d2d, Theme::SemanticColor::Accent, m_cornerRadius, m_cornerRadius, 2.0f);

    float x = d2d.left + 8;
    float centerY = d2d.top + slot.height * 0.5f;

    if (m_iconGlyph)
    {
        std::wstring_view glyph(&m_iconGlyph, 1);
        auto gSize = ctx.MeasureText(glyph, 100);
        D2D1_RECT_F gRect = {x, centerY - gSize.height * 0.5f,
                             x + gSize.width, centerY + gSize.height * 0.5f};
        ctx.DrawText(glyph, gRect, fg);
        x += gSize.width + 4.0f;
    }

    if (!m_text.empty())
    {
        auto textSize = ctx.MeasureText(m_text, slot.width - (x - d2d.left) - 8.0f);
        D2D1_RECT_F textRect = {
            x, centerY - textSize.height * 0.5f,
            x + textSize.width, centerY + textSize.height * 0.5f
        };
        ctx.DrawText(m_text, textRect, fg);
    }
}

bool UIButton::OnMouseEvent(EventType type, const MouseEventArgs& /*args*/) noexcept
{
    if (type == EventType::Click && m_onClick)
    {
        m_onClick(*this);
        return true;
    }
    return false;
}

bool UIButton::OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept
{
    if (type == EventType::KeyDown &&
        (args.key == Input::KeyCode::Return || args.key == Input::KeyCode::Space))
    {
        if (m_onClick)
        {
            m_onClick(*this);
            return true;
        }
    }
    return false;
}

} // namespace
