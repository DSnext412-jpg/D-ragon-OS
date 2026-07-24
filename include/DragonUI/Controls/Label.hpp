#pragma once

#include <DragonUI/Core/Control.hpp>
#include <string>

namespace DragonOS::DragonUI {

class UILabel final : public Control {
public:
    explicit UILabel(std::wstring_view text = {}) noexcept;

    void SetText(std::wstring_view text) noexcept;
    [[nodiscard]] const std::wstring& GetText() const noexcept { return m_text; }

    void SetTextAlignment(Alignment align) noexcept { m_textAlign = align; InvalidateVisual(); }
    [[nodiscard]] Alignment GetTextAlignment() const noexcept { return m_textAlign; }

    void SetVerticalAlignment(Alignment align) noexcept { m_vAlign = align; InvalidateVisual(); }
    [[nodiscard]] Alignment GetVerticalAlignment() const noexcept { return m_vAlign; }

    void SetWordWrap(bool wrap) noexcept { m_wordWrap = wrap; InvalidateLayout(); }
    [[nodiscard]] bool GetWordWrap() const noexcept { return m_wordWrap; }

    void SetTextColor(Theme::SemanticColor color) noexcept { m_textColor = color; InvalidateVisual(); }
    [[nodiscard]] Theme::SemanticColor GetTextColor() const noexcept { return m_textColor; }

    void SetAutoSize(bool autoSize) noexcept { m_autoSize = autoSize; InvalidateLayout(); }
    [[nodiscard]] bool GetAutoSize() const noexcept { return m_autoSize; }

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void Render(RenderContext& ctx) noexcept override;

private:
    std::wstring m_text;
    Alignment m_textAlign{Alignment::Start};
    Alignment m_vAlign{Alignment::Center};
    Theme::SemanticColor m_textColor{Theme::SemanticColor::TextPrimary};
    bool m_wordWrap{true};
    bool m_autoSize{true};
};

} // namespace
