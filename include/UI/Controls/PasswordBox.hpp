#pragma once
#include <UI/Core/UIElement.hpp>
#include <string>

namespace DragonOS::UI {

class PasswordBox : public UIElement {
public:
    PasswordBox() noexcept;

    void SetPassword(std::wstring_view password) noexcept;
    [[nodiscard]] const std::wstring& GetPassword() const noexcept { return m_password; }

    void SetPlaceholder(std::wstring_view text) noexcept { m_placeholder = text; InvalidateVisual(); }
    [[nodiscard]] const std::wstring& GetPlaceholder() const noexcept { return m_placeholder; }

    void SetMaxLength(size_t max) noexcept { m_maxLength = max; }

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void Render(UIRenderer& renderer) noexcept override;
    bool OnEvent(const UIEvent& event) noexcept override;

private:
    std::wstring m_password;
    std::wstring m_placeholder;
    size_t m_cursorPos{0};
    size_t m_maxLength{128};
};

} // namespace
