#pragma once

#include <DragonUI/Core/Control.hpp>
#include <functional>
#include <string>

namespace DragonOS::DragonUI {

class UIMenu;

class UIMenuItem final : public Control {
public:
    using ActionCallback = std::function<void()>;

    explicit UIMenuItem(std::wstring_view text = {}) noexcept;

    void SetText(std::wstring_view text) noexcept;
    [[nodiscard]] const std::wstring& GetText() const noexcept { return m_text; }

    void SetShortcut(std::wstring_view shortcut) noexcept;
    [[nodiscard]] const std::wstring& GetShortcut() const noexcept { return m_shortcut; }

    void SetIcon(uint32_t glyph) noexcept { m_iconGlyph = glyph; InvalidateVisual(); }
    [[nodiscard]] uint32_t GetIcon() const noexcept { return m_iconGlyph; }

    void SetChecked(bool checked) noexcept;
    [[nodiscard]] bool IsChecked() const noexcept { return m_checked; }

    void SetOnAction(ActionCallback cb) noexcept { m_onAction = std::move(cb); }

    void SetSubmenu(std::unique_ptr<UIMenu> submenu) noexcept;
    [[nodiscard]] UIMenu* GetSubmenu() const noexcept { return m_submenu.get(); }
    [[nodiscard]] bool HasSubmenu() const noexcept { return m_submenu != nullptr; }

    void SetSeparator(bool sep) noexcept { m_isSeparator = sep; InvalidateLayout(); }
    [[nodiscard]] bool IsSeparator() const noexcept { return m_isSeparator; }

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void Render(RenderContext& ctx) noexcept override;

    bool OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept override;
    bool OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept override;

    void SetHovered(bool hovered) noexcept;
    [[nodiscard]] bool IsHovered() const noexcept { return m_hovered; }

    static constexpr float MinItemWidth = 160.0f;
    static constexpr float MaxItemWidth = 400.0f;
    static constexpr float ItemHeight = 28.0f;
    static constexpr float PaddingH = 12.0f;
    static constexpr float IconColumnW = 24.0f;
    static constexpr float CheckColumnW = 20.0f;
    static constexpr float ShortcutColumnW = 60.0f;
    static constexpr float ArrowColumnW = 16.0f;
    static constexpr float SeparatorHeight = 6.0f;

private:
    std::wstring m_text;
    std::wstring m_shortcut;
    uint32_t m_iconGlyph{};
    bool m_checked{};
    bool m_isSeparator{};
    bool m_hovered{};
    ActionCallback m_onAction;
    std::unique_ptr<UIMenu> m_submenu;
};

} // namespace
