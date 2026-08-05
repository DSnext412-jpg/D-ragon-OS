#pragma once

#include <DragonUI/Core/Control.hpp>
#include <string>
#include <functional>

namespace DragonOS::DragonUI {

class UIToggleSwitch final : public Control {
public:
    using ToggledCallback = std::function<void(UIToggleSwitch&, bool)>;

    explicit UIToggleSwitch(std::wstring_view text = {}) noexcept;

    void SetText(std::wstring_view text) noexcept;
    [[nodiscard]] const std::wstring& GetText() const noexcept { return m_text; }

    void SetToggled(bool toggled) noexcept;
    [[nodiscard]] bool IsToggled() const noexcept { return m_toggled; }

    void SetOnToggled(ToggledCallback cb) noexcept { m_onToggled = std::move(cb); }

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void Render(RenderContext& ctx) noexcept override;

    bool OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept override;
    bool OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept override;

private:
    void Toggle() noexcept;

    std::wstring m_text;
    bool m_toggled{};
    ToggledCallback m_onToggled;

    // Animation
    float m_thumbOffset{};
    float m_animTarget{};
    float m_bgAnim{};

    static constexpr float TrackWidth = 44.0f;
    static constexpr float TrackHeight = 22.0f;
    static constexpr float ThumbSize = 18.0f;
    static constexpr float ThumbPadding = 2.0f;
    static constexpr float Spacing = 8.0f;
    static constexpr float CornerRadius = 11.0f;
    static constexpr float ThumbCornerRadius = 9.0f;
};

} // namespace
