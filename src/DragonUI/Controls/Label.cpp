#include <DragonUI/Controls/Label.hpp>
#include <DragonUI/Core/RenderContext.hpp>
#include <algorithm>
#include <cmath>

namespace DragonOS::DragonUI {

UILabel::UILabel(std::wstring_view text) noexcept
    : m_text(text)
{
    m_focusable = false;
}

void UILabel::SetText(std::wstring_view text) noexcept
{
    m_text = text;
    InvalidateLayout();
}

DesiredSize UILabel::MeasureOverride(const LayoutSlot& available) noexcept
{
    if (m_text.empty())
        return {0, 0};

    if (m_autoSize)
    {
        float maxW = m_wordWrap ? available.width : 10000.0f;
        float charW = 7.0f;
        float lineH = 18.0f;
        float lines = 1.0f;

        if (m_wordWrap && maxW > 0.0f)
        {
            size_t charsPerLine = std::max(size_t(1), static_cast<size_t>(maxW / charW));
            lines = std::max(1.0f, std::ceil(static_cast<float>(m_text.size()) / static_cast<float>(charsPerLine)));
        }

        float textW = std::min(static_cast<float>(m_text.size()) * charW, maxW);
        float textH = lines * lineH;

        return {std::min(textW, available.width), textH};
    }

    return {0, 0};
}

void UILabel::Render(RenderContext& ctx) noexcept
{
    Element::Render(ctx);
    if (m_text.empty()) return;

    auto slot = GetBounds();
    auto d2d = static_cast<D2D1_RECT_F>(slot);

    auto measured = ctx.MeasureText(m_text, m_wordWrap ? slot.width : 10000.0f);

    float textX = d2d.left;
    float textY = d2d.top;

    switch (m_textAlign)
    {
    case Alignment::Center:
        textX = d2d.left + (slot.width - measured.width) * 0.5f;
        break;
    case Alignment::End:
        textX = d2d.right - measured.width;
        break;
    default:
        break;
    }

    switch (m_vAlign)
    {
    case Alignment::Center:
        textY = d2d.top + (slot.height - measured.height) * 0.5f;
        break;
    case Alignment::End:
        textY = d2d.bottom - measured.height;
        break;
    default:
        break;
    }

    D2D1_RECT_F textRect = {
        textX, textY,
        textX + measured.width, textY + measured.height
    };

    ctx.DrawText(m_text, textRect, m_textColor);
}

} // namespace
