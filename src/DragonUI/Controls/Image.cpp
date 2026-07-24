#include <DragonUI/Controls/Image.hpp>
#include <DragonUI/Core/RenderContext.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

void UIImage::SetGlyph(wchar_t glyph, float fontSize) noexcept
{
    m_glyph = glyph;
    m_glyphSize = fontSize;
    InvalidateLayout();
}

DesiredSize UIImage::MeasureOverride(const LayoutSlot& available) noexcept
{
    if (!m_glyph)
        return {0, 0};

    float size = std::min(m_glyphSize, std::min(available.width, available.height));
    return {size, size};
}

void UIImage::Render(RenderContext& ctx) noexcept
{
    Element::Render(ctx);
    if (!m_glyph) return;

    auto slot = GetBounds();
    float w = slot.width;
    float h = slot.height;

    float drawSize;
    float dx, dy;

    switch (m_stretch)
    {
    case ImageStretch::None:
        drawSize = std::min(w, h);
        dx = slot.x + (w - drawSize) * 0.5f;
        dy = slot.y + (h - drawSize) * 0.5f;
        break;

    case ImageStretch::Fill:
        drawSize = std::min(w, h);
        dx = slot.x;
        dy = slot.y;
        break;

    case ImageStretch::Uniform:
    {
        if (w > h)
        {
            drawSize = h;
            dx = slot.x + (w - drawSize) * 0.5f;
            dy = slot.y;
        }
        else
        {
            drawSize = w;
            dx = slot.x;
            dy = slot.y + (h - drawSize) * 0.5f;
        }
        break;
    }

    case ImageStretch::UniformToFill:
        drawSize = std::max(w, h);
        dx = slot.x + (w - drawSize) * 0.5f;
        dy = slot.y + (h - drawSize) * 0.5f;
        break;

    default:
        return;
    }

    std::wstring_view glyph(&m_glyph, 1);
    auto gSize = ctx.MeasureText(glyph, drawSize);

    float scale = (gSize.width > 0 && gSize.height > 0)
        ? std::min(drawSize / gSize.width, drawSize / gSize.height) : 1.0f;

    float gw = gSize.width * scale;
    float gh = gSize.height * scale;
    float gx = dx + (drawSize - gw) * 0.5f;
    float gy = dy + (drawSize - gh) * 0.5f;

    D2D1_RECT_F iconRect = {gx, gy, gx + gw, gy + gh};
    ctx.DrawText(glyph, iconRect, m_color);
}

} // namespace
