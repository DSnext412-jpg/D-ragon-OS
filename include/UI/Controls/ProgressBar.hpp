#pragma once
#include <UI/Core/UIElement.hpp>

namespace DragonOS::UI {

class ProgressBar : public UIElement {
public:
    ProgressBar() noexcept;

    void SetValue(float value) noexcept { m_value = std::clamp(value, 0.0f, 1.0f); InvalidateVisual(); }
    [[nodiscard]] float GetValue() const noexcept { return m_value; }

    void SetIndeterminate(bool indeterminate) noexcept { m_indeterminate = indeterminate; InvalidateVisual(); }
    [[nodiscard]] bool IsIndeterminate() const noexcept { return m_indeterminate; }

    void SetShowLabel(bool show) noexcept { m_showLabel = show; InvalidateVisual(); }
    [[nodiscard]] bool GetShowLabel() const noexcept { return m_showLabel; }

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void Render(UIRenderer& renderer) noexcept override;

private:
    float m_value{0};
    bool m_indeterminate{false};
    bool m_showLabel{true};
    static constexpr float BarHeight = 8.0f;
};

} // namespace
