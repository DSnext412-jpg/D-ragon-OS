#pragma once

#include <DragonUI/Core/Element.hpp>
#include <string>
#include <d2d1.h>

namespace DragonOS::DragonUI {

enum class ImageStretch : uint8_t {
    None,
    Fill,
    Uniform,
    UniformToFill,
};

class UIImage final : public Element {
public:
    UIImage() noexcept = default;

    void SetGlyph(wchar_t glyph, float fontSize = 48.0f) noexcept;
    [[nodiscard]] wchar_t GetGlyph() const noexcept { return m_glyph; }
    [[nodiscard]] float GetGlyphSize() const noexcept { return m_glyphSize; }

    void SetStretch(ImageStretch stretch) noexcept { m_stretch = stretch; InvalidateLayout(); }
    [[nodiscard]] ImageStretch GetStretch() const noexcept { return m_stretch; }

    void SetColor(Theme::SemanticColor color) noexcept { m_color = color; InvalidateVisual(); }
    [[nodiscard]] Theme::SemanticColor GetColor() const noexcept { return m_color; }

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void Render(RenderContext& ctx) noexcept override;

private:
    wchar_t m_glyph{};
    float m_glyphSize{48.0f};
    ImageStretch m_stretch{ImageStretch::Uniform};
    Theme::SemanticColor m_color{Theme::SemanticColor::TextPrimary};
};

} // namespace
