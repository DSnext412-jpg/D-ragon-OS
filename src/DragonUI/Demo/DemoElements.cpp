#include <DragonUI/Demo/DemoElements.hpp>
#include <algorithm>

namespace DragonOS::DragonUI::Demo {

// ── DemoPanel ────────────────────────────────────────────────────────

DemoPanel::DemoPanel(const Thickness& windowMargin) noexcept
    : m_windowMargin(windowMargin)
{
}

void DemoPanel::Arrange(const LayoutSlot& finalSlot) noexcept
{
    auto inset = finalSlot.Inset(m_windowMargin);
    Element::Arrange(inset);
    auto content = GetContentSlot();
    ArrangeChildren(content);
}

void DemoPanel::Render(RenderContext& ctx) noexcept
{
    auto slot = GetBounds();
    auto d2d = static_cast<D2D1_RECT_F>(slot);
    ctx.FillRectangle(d2d, Theme::SemanticColor::WindowBackground);
    ctx.DrawRectangle(d2d, Theme::SemanticColor::WindowBorder, 1.0f);
    Container::Render(ctx);
}

void DemoPanel::ArrangeChildren(const LayoutSlot& finalSlot) noexcept
{
    float y = finalSlot.y;
    for (const auto& child : m_children)
    {
        if (child->GetVisibility() != Visibility::Visible) continue;

        auto ds = child->GetDesiredSize();
        auto margin = child->GetMargin();
        float childW = (std::min)(ds.width, finalSlot.width);
        float childH = (std::min)(ds.height, finalSlot.height);

        float xOff = finalSlot.x + margin.left;
        float yOff = y + margin.top;
        float w = childW - margin.Horizontal();
        float h = childH - margin.Vertical();
        if (w < 0) w = 0;
        if (h < 0) h = 0;

        LayoutSlot childSlot{xOff, yOff, w, h};
        child->Arrange(childSlot);
        y = yOff + h + margin.bottom + 4;
    }
}

// ── DemoText ─────────────────────────────────────────────────────────

DemoText::DemoText(std::wstring_view text) noexcept
    : m_text(text)
{
}

void DemoText::SetText(std::wstring_view text) noexcept
{
    m_text = text;
    InvalidateLayout();
}

DesiredSize DemoText::MeasureOverride(const LayoutSlot& available) noexcept
{
    float lineHeight = 18.0f;
    float textWidth = static_cast<float>(m_text.size()) * 8.0f;
    return {
        (std::min)(textWidth, available.width),
        lineHeight
    };
}

void DemoText::Render(RenderContext& ctx) noexcept
{
    Element::Render(ctx);
    if (m_text.empty()) return;

    auto slot = GetBounds();
    auto d2d = static_cast<D2D1_RECT_F>(slot);
    ctx.DrawText(m_text, d2d, Theme::SemanticColor::TextPrimary);
}

// ── DemoButton ───────────────────────────────────────────────────────

DemoButton::DemoButton(std::wstring_view text) noexcept
    : m_text(text)
{
    m_focusable = true;
    SetMinSize(80.0f, 32.0f);
}

void DemoButton::SetText(std::wstring_view text) noexcept
{
    m_text = text;
    InvalidateLayout();
}

DesiredSize DemoButton::MeasureOverride(const LayoutSlot& available) noexcept
{
    float textW = static_cast<float>(m_text.size()) * 8.0f + 16.0f;
    return {(std::min)(textW, available.width), 32.0f};
}

void DemoButton::Render(RenderContext& ctx) noexcept
{
    Control::Render(ctx);

    auto slot = GetBounds();
    auto d2d = static_cast<D2D1_RECT_F>(slot);

    Theme::SemanticColor bg = Theme::SemanticColor::Accent;
    Theme::SemanticColor fg = Theme::SemanticColor::TextPrimary;
    Theme::SemanticColor border = Theme::SemanticColor::WindowBorder;

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
    default:
        break;
    }

    ctx.FillRectangle(d2d, bg);
    ctx.DrawRectangle(d2d, border);

    if (GetControlState() == ControlState::Focused)
        ctx.DrawRectangle(d2d, Theme::SemanticColor::Accent, 2.0f);

    if (!m_text.empty())
    {
        D2D1_RECT_F textRect = {
            d2d.left + 8, d2d.top + 6,
            d2d.right - 8, d2d.bottom - 6
        };
        ctx.DrawText(m_text, textRect, fg);
    }
}

bool DemoButton::OnMouseEvent(EventType /*type*/, const MouseEventArgs& /*args*/) noexcept
{
    return true;
}

} // namespace
