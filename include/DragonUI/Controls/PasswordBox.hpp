#pragma once

#include <DragonUI/Controls/TextBox.hpp>
#include <functional>

namespace DragonOS::DragonUI {

using PasswordStrengthCallback = std::function<int(const std::wstring&)>;

class UIPasswordBox final : public Control {
public:
    explicit UIPasswordBox(std::wstring_view placeholder = {}) noexcept;

    void SetText(std::wstring_view text) noexcept;
    [[nodiscard]] const std::wstring& GetText() const noexcept { return m_text; }

    void SetPlaceholder(std::wstring_view text) noexcept;
    [[nodiscard]] const std::wstring& GetPlaceholder() const noexcept { return m_placeholder; }

    void SetMaxLength(size_t max) noexcept;
    [[nodiscard]] size_t GetMaxLength() const noexcept { return m_maxLength; }

    void SetShowPassword(bool show) noexcept;
    [[nodiscard]] bool IsPasswordVisible() const noexcept { return m_showPassword; }

    void SetPasswordStrengthCallback(PasswordStrengthCallback cb) noexcept { m_strengthCb = std::move(cb); }
    [[nodiscard]] int GetPasswordStrength() const noexcept;

    void SetOnTextChanged(std::function<void(UIPasswordBox&)> cb) noexcept { m_onTextChanged = std::move(cb); }

    void SetValidator(std::shared_ptr<Validator> validator) noexcept;
    ValidationResult Validate() const noexcept;

    void SetAccessibleName(std::wstring_view name) noexcept { m_accessibleName = name; }
    [[nodiscard]] const std::wstring& GetAccessibleName() const noexcept { return m_accessibleName; }
    void SetAccessibleDescription(std::wstring_view desc) noexcept { m_accessibleDescription = desc; }

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void Render(RenderContext& ctx) noexcept override;

    bool OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept override;
    bool OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept override;
    bool OnFocusEvent(const FocusEventArgs& args) noexcept override;

private:
    void InsertChar(wchar_t ch) noexcept;
    void DeleteChar(bool forward) noexcept;
    void MoveCursorLeft(bool shiftHeld = false) noexcept;
    void MoveCursorRight(bool shiftHeld = false) noexcept;
    float GetCharWidth() const noexcept;
    void ClearSecureMemory() noexcept;

    std::wstring m_text;
    std::wstring m_placeholder;
    size_t m_cursorPos{};
    size_t m_maxLength{256};
    bool m_showPassword{};

    std::wstring m_accessibleName;
    std::wstring m_accessibleDescription;

    std::function<void(UIPasswordBox&)> m_onTextChanged;
    PasswordStrengthCallback m_strengthCb;

    std::shared_ptr<Validator> m_validator;

    float m_blinkTimer{};
    bool m_caretVisible{true};

    static constexpr float PaddingH = 8.0f;
    static constexpr float PaddingV = 4.0f;
    static constexpr float CaretWidth = 1.5f;
    static constexpr float BlinkInterval = 0.53f;
    static constexpr float EyeButtonWidth = 28.0f;
};

} // namespace
