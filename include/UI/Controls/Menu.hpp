#pragma once
#include <UI/Core/UIContainer.hpp>
#include <string>
#include <vector>
#include <functional>

namespace DragonOS::UI {

struct MenuItem {
    std::wstring text;
    std::vector<MenuItem> subItems;
    bool isEnabled{true};
    bool isSeparator{false};
    std::function<void()> action;
};

class Menu : public UIContainer {
public:
    Menu() noexcept;

    int AddItem(std::wstring_view text, std::function<void()> action = nullptr) noexcept;
    int AddSubMenu(std::wstring_view text, std::vector<MenuItem> subItems) noexcept;
    void ClearItems() noexcept;

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept override;
    void Render(UIRenderer& renderer) noexcept override;
    bool OnEvent(const UIEvent& event) noexcept override;

    static constexpr float ItemPaddingH = 12.0f;
    static constexpr float ItemPaddingV = 4.0f;
    static constexpr float MinItemHeight = 28.0f;

private:
    [[nodiscard]] D2D1_RECT_F GetItemRect(int index) const noexcept;
    void CloseSubMenu() noexcept;

    std::vector<MenuItem> m_items;
    int m_hoveredItem{-1};
    int m_openSubMenu{-1};
    D2D1_RECT_F m_subMenuBounds{};
};

} // namespace
