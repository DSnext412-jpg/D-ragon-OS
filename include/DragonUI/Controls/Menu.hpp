#pragma once

#include <DragonUI/Core/Container.hpp>
#include <DragonUI/Controls/MenuItem.hpp>
#include <string>

namespace DragonOS::DragonUI {

class UIMenu final : public Container {
public:
    UIMenu() noexcept = default;

    UIMenuItem* AddItem(std::wstring_view text, UIMenuItem::ActionCallback cb = {}) noexcept;
    UIMenuItem* AddSubMenuItem(std::wstring_view text, std::unique_ptr<UIMenu> submenu) noexcept;
    void AddSeparator() noexcept;
    void ClearItems() noexcept;

    void SetPopupPosition(float x, float y) noexcept;
    [[nodiscard]] float GetPopupX() const noexcept { return m_popupX; }
    [[nodiscard]] float GetPopupY() const noexcept { return m_popupY; }

    void SetOpen(bool open) noexcept;
    [[nodiscard]] bool IsOpen() const noexcept { return m_isOpen; }

    [[nodiscard]] int GetHoveredIndex() const noexcept { return m_hoveredIndex; }
    void SetHoveredIndex(int index) noexcept;

    void SelectNext() noexcept;
    void SelectPrevious() noexcept;
    void ActivateHovered() noexcept;

    [[nodiscard]] bool HasCheckableItems() const noexcept;

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void ArrangeOverride(const LayoutSlot& finalSlot) noexcept override;
    void Render(RenderContext& ctx) noexcept override;

    bool OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept;
    bool OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept;

    void CloseSubmenus() noexcept;

    static constexpr float MinWidth = 180.0f;
    static constexpr float MaxWidth = 400.0f;
    static constexpr float BorderThickness = 1.0f;
    static constexpr float CornerRadius = 6.0f;
    static constexpr float ShadowOffset = 4.0f;

private:
    bool IsSeparatorItem(int index) const noexcept;
    void RebuildItemList() noexcept;

    float m_popupX{};
    float m_popupY{};
    bool m_isOpen{};
    int m_hoveredIndex{ -1 };
    std::vector<UIMenuItem*> m_itemList;
    UIMenu* m_openSubmenu{};
};

} // namespace
