#pragma once
#include <UI/Core/UIElement.hpp>
#include <UI/Core/UILayout.hpp>
#include <string>

namespace DragonOS::UI {

class Label : public UIElement {
public:
    Label() noexcept = default;
    explicit Label(std::wstring_view text) noexcept;

    void SetText(std::wstring_view text) noexcept { m_text = text; InvalidateVisual(); }
    [[nodiscard]] const std::wstring& GetText() const noexcept { return m_text; }

    void SetTextAlignment(Alignment align) noexcept { m_textAlign = align; InvalidateVisual(); }
    [[nodiscard]] Alignment GetTextAlignment() const noexcept { return m_textAlign; }

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void Render(UIRenderer& renderer) noexcept override;

private:
    std::wstring m_text;
    Alignment m_textAlign{Alignment::Start};
};

} // namespace
