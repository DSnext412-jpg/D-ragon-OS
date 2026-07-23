#pragma once
#include <UI/Core/UIElement.hpp>
#include <string>
#include <functional>

namespace DragonOS::UI {

class CheckBox : public UIElement {
public:
    CheckBox() noexcept;
    explicit CheckBox(std::wstring_view text) noexcept;

    void SetText(std::wstring_view text) noexcept { m_text = text; InvalidateVisual(); }
    [[nodiscard]] const std::wstring& GetText() const noexcept { return m_text; }

    void SetChecked(bool checked) noexcept;
    [[nodiscard]] bool IsChecked() const noexcept { return m_checked; }

    void SetOnCheckedChanged(std::function<void(bool)> cb) noexcept { m_onCheckedChanged = std::move(cb); }

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void Render(UIRenderer& renderer) noexcept override;
    bool OnEvent(const UIEvent& event) noexcept override;

private:
    std::wstring m_text;
    bool m_checked{false};
    std::function<void(bool)> m_onCheckedChanged;
    static constexpr float BoxSize = 18.0f;
};

} // namespace
