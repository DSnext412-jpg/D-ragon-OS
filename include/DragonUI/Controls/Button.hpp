#pragma once

#include <DragonUI/Core/Control.hpp>
#include <functional>
#include <string>

namespace DragonOS::DragonUI {

class UIButton final : public Control {
public:
    using ClickCallback = std::function<void(UIButton&)>;

    explicit UIButton(std::wstring_view text = {}) noexcept;

    void SetText(std::wstring_view text) noexcept;
    [[nodiscard]] const std::wstring& GetText() const noexcept { return m_text; }

    void SetIcon(wchar_t glyph) noexcept { m_iconGlyph = glyph; InvalidateLayout(); }
    [[nodiscard]] wchar_t GetIcon() const noexcept { return m_iconGlyph; }

    void SetCornerRadius(float radius) noexcept { m_cornerRadius = radius; InvalidateVisual(); }
    [[nodiscard]] float GetCornerRadius() const noexcept { return m_cornerRadius; }

    void SetOnClick(ClickCallback cb) noexcept { m_onClick = std::move(cb); }

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void Render(RenderContext& ctx) noexcept override;

    bool OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept override;
    bool OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept override;

private:
    std::wstring m_text;
    wchar_t m_iconGlyph{};
    float m_cornerRadius{4.0f};
    ClickCallback m_onClick;
};

} // namespace
