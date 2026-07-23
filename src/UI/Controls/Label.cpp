#include <UI/Controls/Label.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <UI/Core/UIStyle.hpp>
#include <algorithm>

namespace DragonOS::UI {

Label::Label(std::wstring_view text) noexcept
    : m_text(text)
{
    SetStyle(UIStyle::DefaultLabel());
}

D2D1_RECT_F Label::MeasureOverride(const D2D1_RECT_F& available) noexcept
{
    auto style = GetStyle();
    float fontSize = style ? style->fontSize : 14.0f;
    float paddingL = style ? style->paddingLeft : 0.0f;
    float paddingR = style ? style->paddingRight : 0.0f;
    float paddingT = style ? style->paddingTop : 0.0f;
    float paddingB = style ? style->paddingBottom : 0.0f;

    float textWidth = static_cast<float>(m_text.length()) * 7.0f * (fontSize / 14.0f);
    float textHeight = fontSize * 1.2f;

    float totalWidth = (std::min)(textWidth + paddingL + paddingR, available.right - available.left);
    float totalHeight = (std::min)(textHeight + paddingT + paddingB, available.bottom - available.top);

    return D2D1_RECT_F{0, 0, totalWidth, totalHeight};
}

void Label::Render(UIRenderer& renderer) noexcept
{
    auto bounds = GetBounds();
    auto style = GetStyle();
    if (!style) return;

    const auto& stateColors = style->ResolveState(GetState());

    D2D1_RECT_F textRect = {
        bounds.left + style->paddingLeft,
        bounds.top + style->paddingTop,
        bounds.right - style->paddingRight,
        bounds.bottom - style->paddingBottom
    };

    if (!m_text.empty())
    {
        renderer.DrawText(m_text, textRect, stateColors);
    }
}

} // namespace
