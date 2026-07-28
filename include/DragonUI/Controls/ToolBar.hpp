#pragma once

#include <DragonUI/Core/Container.hpp>
#include <DragonUI/Controls/Menu.hpp>
#include <functional>
#include <string>
#include <vector>

namespace DragonOS::DragonUI {

class UIToolBar final : public Container {
public:
    UIToolBar() noexcept = default;

    using ButtonCallback = std::function<void()>;

    struct ToolBarButton {
        std::wstring text;
        uint32_t iconGlyph{};
        std::wstring tooltip;
        ButtonCallback callback;
        bool enabled{ true };
        bool isSeparator{};
        std::unique_ptr<UIMenu> dropDownMenu;
    };

    size_t AddButton(std::wstring_view text, ButtonCallback cb, uint32_t icon = 0) noexcept;
    size_t AddDropDownButton(std::wstring_view text, std::unique_ptr<UIMenu> menu, uint32_t icon = 0) noexcept;
    void AddSeparator() noexcept;
    void ClearButtons() noexcept;

    void SetButtonEnabled(size_t index, bool enabled) noexcept;
    [[nodiscard]] bool IsButtonEnabled(size_t index) const noexcept;

    void SetIconSize(float size) noexcept { m_iconSize = size; InvalidateLayout(); }
    [[nodiscard]] float GetIconSize() const noexcept { return m_iconSize; }

    void SetButtonSpacing(float spacing) noexcept { m_spacing = spacing; InvalidateLayout(); }
    [[nodiscard]] float GetButtonSpacing() const noexcept { return m_spacing; }

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void ArrangeOverride(const LayoutSlot& finalSlot) noexcept override;
    void Render(RenderContext& ctx) noexcept override;

    bool OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept;

    void CloseDropDown() noexcept;

    static constexpr float DefaultIconSize = 16.0f;
    static constexpr float DefaultButtonSize = 28.0f;
    static constexpr float DefaultSpacing = 2.0f;
    static constexpr float SeparatorWidth = 6.0f;
    static constexpr float CornerRadius = 4.0f;

private:
    int HitTest(float px, float py) const noexcept;

    std::vector<ToolBarButton> m_buttons;
    float m_iconSize{ DefaultIconSize };
    float m_spacing{ DefaultSpacing };
    int m_hoveredIndex{ -1 };
    int m_pressedIndex{ -1 };
    int m_openDropDown{ -1 };
};

} // namespace
