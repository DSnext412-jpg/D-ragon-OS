#pragma once
#include <UI/Core/UIContainer.hpp>
#include <string>
#include <vector>
#include <functional>

namespace DragonOS::UI {

class ListView : public UIContainer {
public:
    ListView() noexcept;

    void SetItems(const std::vector<std::wstring>& items) noexcept;
    [[nodiscard]] const std::vector<std::wstring>& GetItems() const noexcept { return m_items; }

    void SetSelectedIndex(int index) noexcept;
    [[nodiscard]] int GetSelectedIndex() const noexcept { return m_selectedIndex; }
    [[nodiscard]] std::wstring GetSelectedItem() const noexcept;

    void SetOnSelectionChanged(std::function<void(int)> callback) noexcept { m_onSelectionChanged = std::move(callback); }

    void SetScrollOffset(float offset) noexcept;
    [[nodiscard]] float GetScrollOffset() const noexcept { return m_scrollOffset; }
    void EnsureVisible() noexcept;

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void Render(UIRenderer& renderer) noexcept override;
    bool OnEvent(const UIEvent& event) noexcept override;

    static constexpr float ItemHeight = 28.0f;

private:
    std::vector<std::wstring> m_items;
    int m_selectedIndex{-1};
    int m_hoveredIndex{-1};
    float m_scrollOffset{0};
    std::function<void(int)> m_onSelectionChanged;
};

} // namespace
