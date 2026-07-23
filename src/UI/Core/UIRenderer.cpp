#include <UI/Core/UIRenderer.hpp>
#include <algorithm>

namespace DragonOS::UI {

Graphics::Color UIRenderer::ResolveColor(Theme::SemanticColor token) const noexcept
{
    const auto& tc = m_theme->GetColor(token);
    return Graphics::Color(tc.r, tc.g, tc.b, tc.a);
}

void UIRenderer::FillBackground(
    const D2D1_RECT_F& rect,
    const StyleStateColors& colors,
    float /*cornerRadius*/,
    float opacity) noexcept
{
    auto color = ResolveColor(colors.background);
    if (color.a * opacity <= 0.0f)
        return;
    m_renderer->FillRectangle(rect, color, opacity);
}

void UIRenderer::DrawBorder(
    const D2D1_RECT_F& rect,
    const StyleStateColors& colors,
    float thickness,
    float /*cornerRadius*/,
    float opacity) noexcept
{
    auto color = ResolveColor(colors.border);
    if (color.a * opacity <= 0.0f || thickness <= 0.0f)
        return;
    m_renderer->DrawRectangle(rect, color, thickness);
}

void UIRenderer::DrawText(
    std::wstring_view text,
    const D2D1_RECT_F& layoutRect,
    const StyleStateColors& colors) noexcept
{
    auto color = ResolveColor(colors.foreground);
    m_renderer->DrawText(text, layoutRect, color);
}

void UIRenderer::DrawIcon(
    const D2D1_RECT_F& rect,
    wchar_t glyph,
    const StyleStateColors& colors,
    float /*fontSize*/) noexcept
{
    auto color = ResolveColor(colors.foreground);
    std::wstring str(1, glyph);
    m_renderer->DrawText(str, rect, color);
}

void UIRenderer::FillGauge(
    const D2D1_RECT_F& rect,
    float progress,
    const StyleStateColors& colors,
    float /*cornerRadius*/) noexcept
{
    auto color = ResolveColor(colors.background);
    if (color.a <= 0.0f)
        return;

    progress = (std::clamp)(progress, 0.0f, 1.0f);
    float totalWidth = rect.right - rect.left;
    float filledWidth = totalWidth * progress;

    D2D1_RECT_F filledRect = {
        rect.left,
        rect.top,
        rect.left + filledWidth,
        rect.bottom
    };

    m_renderer->FillRectangle(filledRect, color);
}

void UIRenderer::DrawFocusIndicator(const D2D1_RECT_F& rect) noexcept
{
    auto accent = ResolveColor(Theme::SemanticColor::Accent);
    if (accent.a <= 0.0f)
        return;

    D2D1_RECT_F indicatorRect = {
        rect.left - 3.0f,
        rect.top - 3.0f,
        rect.right + 3.0f,
        rect.bottom + 3.0f
    };

    m_renderer->DrawRectangle(indicatorRect, accent, 2.0f);
}

void UIRenderer::PushClip(const D2D1_RECT_F& clipRect) noexcept
{
    auto* target = m_renderer->GetRenderTarget();
    if (target)
    {
        target->PushAxisAlignedClip(clipRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    }
}

void UIRenderer::PopClip() noexcept
{
    auto* target = m_renderer->GetRenderTarget();
    if (target)
    {
        target->PopAxisAlignedClip();
    }
}

} // namespace
