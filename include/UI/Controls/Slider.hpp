#pragma once
#include <UI/Core/UIElement.hpp>
#include <functional>

namespace DragonOS::UI {

class Slider : public UIElement {
public:
    Slider() noexcept;

    void SetRange(float min, float max) noexcept { m_min = min; m_max = max; InvalidateVisual(); }
    [[nodiscard]] float GetMin() const noexcept { return m_min; }
    [[nodiscard]] float GetMax() const noexcept { return m_max; }

    void SetValue(float value) noexcept;
    [[nodiscard]] float GetValue() const noexcept { return m_value; }

    void SetStep(float step) noexcept { m_step = step; }
    [[nodiscard]] float GetStep() const noexcept { return m_step; }

    void SetOnValueChanged(std::function<void(float)> cb) noexcept { m_onValueChanged = std::move(cb); }

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void Render(UIRenderer& renderer) noexcept override;
    bool OnEvent(const UIEvent& event) noexcept override;

private:
    float ClampValue(float val) const noexcept;
    [[nodiscard]] float ValueToPosition() const noexcept;
    void PositionToValue(float x) noexcept;

    float m_min{0}, m_max{100}, m_value{50}, m_step{1};
    bool m_isDragging{false};
    std::function<void(float)> m_onValueChanged;
    static constexpr float TrackHeight = 6.0f;
    static constexpr float ThumbRadius = 8.0f;
    static constexpr float MinWidth = 100.0f;
};

} // namespace
