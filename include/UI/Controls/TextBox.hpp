#pragma once
#include <UI/Core/UIElement.hpp>
#include <string>
#include <functional>

namespace DragonOS::UI {

class TextBox : public UIElement {
public:
    TextBox() noexcept;
    explicit TextBox(std::wstring_view placeholder) noexcept;

    void SetText(std::wstring_view text) noexcept;
    [[nodiscard]] const std::wstring& GetText() const noexcept { return m_text; }
    [[nodiscard]] const std::wstring& GetPlaceholder() const noexcept { return m_placeholder; }
    void SetPlaceholder(std::wstring_view text) noexcept { m_placeholder = text; InvalidateVisual(); }

    void SetReadOnly(bool readOnly) noexcept { m_readOnly = readOnly; }
    [[nodiscard]] bool IsReadOnly() const noexcept { return m_readOnly; }

    void SetMaxLength(size_t max) noexcept { m_maxLength = max; }
    [[nodiscard]] size_t GetMaxLength() const noexcept { return m_maxLength; }

    void SetOnTextChanged(std::function<void(const std::wstring&)> cb) noexcept { m_onTextChanged = std::move(cb); }
    [[nodiscard]] const std::function<void(const std::wstring&)>& GetOnTextChanged() const noexcept { return m_onTextChanged; }

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void Render(UIRenderer& renderer) noexcept override;
    bool OnEvent(const UIEvent& event) noexcept override;

    void SetCursorPosition(size_t pos) noexcept;
    [[nodiscard]] size_t GetCursorPosition() const noexcept { return m_cursorPos; }

    static constexpr size_t npos = std::wstring::npos;

private:
    std::wstring m_text;
    std::wstring m_placeholder;
    size_t m_cursorPos{0};
    size_t m_maxLength{256};
    bool m_readOnly{false};
    std::function<void(const std::wstring&)> m_onTextChanged;
    float m_scrollOffset{0};
};

} // namespace
