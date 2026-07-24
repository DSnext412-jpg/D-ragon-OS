#pragma once

#include <DragonUI/Core/Control.hpp>

namespace DragonOS::DragonUI {

class UIProgressBar final : public Control {
public:
    UIProgressBar() noexcept = default;

    void SetValue(float value) noexcept;
    [[nodiscard]] float GetValue() const noexcept { return m_value; }

    void SetRange(float min, float max) noexcept;
    [[nodiscard]] float GetMin() const noexcept { return m_min; }
    [[nodiscard]] float GetMax() const noexcept { return m_max; }

    [[nodiscard]] float GetNormalizedValue() const noexcept;

    void SetIndeterminate(bool ind) noexcept { m_indeterminate = ind; InvalidateVisual(); }
    [[nodiscard]] bool IsIndeterminate() const noexcept { return m_indeterminate; }

    void SetAnimationTime(float t) noexcept { m_animTime = t; InvalidateVisual(); }

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void Render(RenderContext& ctx) noexcept override;

private:
    float m_value{};
    float m_min{};
    float m_max{100.0f};
    float m_animTime{};
    bool m_indeterminate{};
};

} // namespace
