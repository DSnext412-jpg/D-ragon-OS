#pragma once
#include <UI/Core/UIElement.hpp>
#include <string>
#include <functional>

namespace DragonOS::UI {

class ToggleSwitch : public UIElement {
public:
    ToggleSwitch() noexcept;
    explicit ToggleSwitch(std::wstring_view text) noexcept;

    void SetText(std::wstring_view text) noexcept { m_text = text; InvalidateVisual(); }
    [[nodiscard]] const std::wstring& GetText() const noexcept { return m_text; }

    void SetToggled(bool toggled) noexcept;
    [[nodiscard]] bool IsToggled() const noexcept { return m_toggled; }

    void SetOnToggled(std::function<void(bool)> cb) noexcept { m_onToggled = std::move(cb); }

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void Render(UIRenderer& renderer) noexcept override;
    bool OnEvent(const UIEvent& event) noexcept override;

private:
    std::wstring m_text;
    bool m_toggled{false};
    std::function<void(bool)> m_onToggled;
    float m_thumbOffset{0};
    static constexpr float TrackWidth = 44.0f;
    static constexpr float TrackHeight = 22.0f;
    static constexpr float ThumbSize = 18.0f;
};

} // namespace
