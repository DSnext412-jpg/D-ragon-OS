#pragma once
#include <UI/Core/UIContainer.hpp>
#include <string>
#include <vector>
#include <functional>

namespace DragonOS::UI {

class ComboBox : public UIContainer {
public:
    ComboBox() noexcept;

    void SetItems(const std::vector<std::wstring>& items) noexcept;
    [[nodiscard]] const std::vector<std::wstring>& GetItems() const noexcept { return m_items; }

    void SetSelectedIndex(int index) noexcept;
    [[nodiscard]] int GetSelectedIndex() const noexcept { return m_selectedIndex; }
    [[nodiscard]] std::wstring GetSelectedItem() const noexcept;

    void SetOnSelectionChanged(std::function<void(int)> callback) noexcept { m_onSelectionChanged = std::move(callback); }

    [[nodiscard]] bool IsOpen() const noexcept { return m_isOpen; }
    void SetOpen(bool open) noexcept;

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept override;
    void Render(UIRenderer& renderer) noexcept override;
    bool OnEvent(const UIEvent& event) noexcept override;

private:
    [[nodiscard]] D2D1_RECT_F GetHeaderRect() const noexcept;
    [[nodiscard]] D2D1_RECT_F GetPopupRect() const noexcept;
    [[nodiscard]] D2D1_RECT_F GetItemRect(int index) const noexcept;

    std::vector<std::wstring> m_items;
    int m_selectedIndex{-1};
    int m_hoveredIndex{-1};
    bool m_isOpen{false};
    std::function<void(int)> m_onSelectionChanged;
    D2D1_RECT_F m_popupBounds{};
    static constexpr float MinHeaderHeight = 32.0f;
    static constexpr float MaxPopupHeight = 200.0f;
};

} // namespace
