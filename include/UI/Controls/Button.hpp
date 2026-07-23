#pragma once
#include <UI/Core/UIElement.hpp>
#include <string>
#include <functional>

namespace DragonOS::UI {

class Button : public UIElement {
public:
    Button() noexcept;
    explicit Button(std::wstring_view text) noexcept;

    void SetText(std::wstring_view text) noexcept { m_text = text; InvalidateVisual(); }
    [[nodiscard]] const std::wstring& GetText() const noexcept { return m_text; }

    void SetIcon(wchar_t glyph) noexcept { m_iconGlyph = glyph; InvalidateVisual(); }
    [[nodiscard]] wchar_t GetIcon() const noexcept { return m_iconGlyph; }

    void SetOnClick(std::function<void()> callback) noexcept { m_onClick = std::move(callback); }

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept override;
    void Render(UIRenderer& renderer) noexcept override;
    bool OnEvent(const UIEvent& event) noexcept override;

private:
    std::wstring m_text;
    wchar_t m_iconGlyph{0};
    std::function<void()> m_onClick;
};

} // namespace
