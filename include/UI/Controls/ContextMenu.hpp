#pragma once
#include <UI/Core/UIElement.hpp>
#include <string>
#include <vector>
#include <functional>

namespace DragonOS::UI {

struct ContextMenuItem {
    std::wstring text;
    bool isSeparator{false};
    bool isEnabled{true};
    std::function<void()> action;
};

class ContextMenu : public UIElement {
public:
    ContextMenu() noexcept;

    void AddItem(std::wstring_view text, std::function<void()> action = nullptr) noexcept;
    void AddSeparator() noexcept;
    void ClearItems() noexcept;

    void ShowAt(float x, float y) noexcept;
    void Close() noexcept;
    [[nodiscard]] bool IsOpen() const noexcept { return m_isOpen; }

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void Render(UIRenderer& renderer) noexcept override;
    bool OnEvent(const UIEvent& event) noexcept override;

    static constexpr float ItemHeight = 28.0f;
    static constexpr float ItemPaddingH = 12.0f;
    static constexpr float MinWidth = 120.0f;

private:
    std::vector<ContextMenuItem> m_items;
    bool m_isOpen{false};
    int m_hoveredIndex{-1};
};

} // namespace
