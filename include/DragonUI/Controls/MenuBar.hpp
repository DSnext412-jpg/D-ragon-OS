#pragma once

#include <DragonUI/Core/Container.hpp>
#include <DragonUI/Controls/Menu.hpp>
#include <string>

namespace DragonOS::DragonUI {

class UIMenuBar final : public Container {
public:
    UIMenuBar() noexcept = default;

    struct MenuBarEntry {
        std::wstring text;
        std::unique_ptr<UIMenu> menu;
        float measuredWidth{};
        bool hovered{};
    };

    size_t AddMenu(std::wstring_view text, std::unique_ptr<UIMenu> menu) noexcept;
    void ClearMenus() noexcept;

    void OpenMenu(size_t index) noexcept;
    void CloseMenu() noexcept;
    void CloseAll() noexcept;

    [[nodiscard]] size_t GetMenuCount() const noexcept { return m_entries.size(); }
    [[nodiscard]] bool HasOpenMenu() const noexcept { return m_openIndex != static_cast<size_t>(-1); }
    [[nodiscard]] size_t GetOpenIndex() const noexcept { return m_openIndex; }

    void SelectNext() noexcept;
    void SelectPrevious() noexcept;
    void ActivateFocused() noexcept;

    void SetTabNavigate(bool tab) noexcept { m_tabNavigate = tab; }
    void SetAltActive(bool active) noexcept;
    [[nodiscard]] bool IsAltActive() const noexcept { return m_altActive; }

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void ArrangeOverride(const LayoutSlot& finalSlot) noexcept override;
    void Render(RenderContext& ctx) noexcept override;

    bool OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept;
    bool OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept;

    [[nodiscard]] const MenuBarEntry& GetEntry(size_t index) const noexcept { return m_entries[index]; }

    static constexpr float ItemPaddingH = 10.0f;
    static constexpr float ItemPaddingV = 4.0f;
    static constexpr float MinItemHeight = 30.0f;

private:
    int HitTestItems(float px) const noexcept;

    std::vector<MenuBarEntry> m_entries;
    size_t m_openIndex{ static_cast<size_t>(-1) };
    size_t m_hoveredIndex{ static_cast<size_t>(-1) };
    bool m_altActive{};
    bool m_tabNavigate{};
};

} // namespace
