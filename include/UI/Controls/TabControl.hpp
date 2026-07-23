#pragma once
#include <UI/Core/UIContainer.hpp>
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace DragonOS::UI {

struct TabPage {
    std::wstring headerText;
    std::unique_ptr<UIElement> content;
};

class TabControl : public UIContainer {
public:
    TabControl() noexcept;

    int AddPage(std::wstring_view headerText, std::unique_ptr<UIElement> content) noexcept;
    bool RemovePage(int index) noexcept;
    void ClearPages() noexcept;

    void SetSelectedIndex(int index) noexcept;
    [[nodiscard]] int GetSelectedIndex() const noexcept { return m_selectedIndex; }
    [[nodiscard]] TabPage* GetPage(int index) const noexcept;
    [[nodiscard]] size_t GetPageCount() const noexcept { return m_tabs.size(); }

    void SetOnSelectionChanged(std::function<void(int)> callback) noexcept { m_onSelectionChanged = std::move(callback); }

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept override;
    void Render(UIRenderer& renderer) noexcept override;
    bool OnEvent(const UIEvent& event) noexcept override;

    static constexpr float TabHeaderHeight = 32.0f;
    static constexpr float MinTabWidth = 60.0f;

private:
    std::vector<TabPage> m_tabs;
    int m_selectedIndex{0};
    int m_hoveredTab{-1};
    std::function<void(int)> m_onSelectionChanged;
};

} // namespace
