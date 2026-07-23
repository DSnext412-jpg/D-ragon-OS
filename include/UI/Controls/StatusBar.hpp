#pragma once
#include <UI/Core/UIElement.hpp>
#include <string>

namespace DragonOS::UI {

class StatusBar : public UIElement {
public:
    StatusBar() noexcept;

    void SetText(std::wstring_view text) noexcept;
    [[nodiscard]] const std::wstring& GetText() const noexcept { return m_text; }

    void SetRightText(std::wstring_view text) noexcept;
    [[nodiscard]] const std::wstring& GetRightText() const noexcept { return m_rightText; }

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void Render(UIRenderer& renderer) noexcept override;

    static constexpr float MinHeight = 24.0f;

private:
    std::wstring m_text;
    std::wstring m_rightText;
};

} // namespace
