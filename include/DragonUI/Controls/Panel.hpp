#pragma once

#include <DragonUI/Core/Container.hpp>

namespace DragonOS::DragonUI {

class UIPanel final : public Container {
public:
    UIPanel() noexcept = default;

    void SetBackground(Theme::SemanticColor color) noexcept { m_background = color; InvalidateVisual(); }
    [[nodiscard]] Theme::SemanticColor GetBackground() const noexcept { return m_background; }

    void SetBorderColor(Theme::SemanticColor color) noexcept { m_borderColor = color; InvalidateVisual(); }
    [[nodiscard]] Theme::SemanticColor GetBorderColor() const noexcept { return m_borderColor; }

    void SetBorderThickness(float thickness) noexcept { m_borderThickness = thickness; InvalidateVisual(); }
    [[nodiscard]] float GetBorderThickness() const noexcept { return m_borderThickness; }

    void SetCornerRadius(float radius) noexcept { m_cornerRadius = radius; InvalidateVisual(); }
    [[nodiscard]] float GetCornerRadius() const noexcept { return m_cornerRadius; }

    void Render(RenderContext& ctx) noexcept override;

private:
    Theme::SemanticColor m_background{Theme::SemanticColor::WindowBackground};
    Theme::SemanticColor m_borderColor{Theme::SemanticColor::WindowBorder};
    float m_borderThickness{};
    float m_cornerRadius{};
};

} // namespace
