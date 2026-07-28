#pragma once

#include <DragonUI/Core/Element.hpp>
#include <DragonUI/Controls/Menu.hpp>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace DragonOS::DragonUI {

class UIContextMenu final : public Element {
public:
    UIContextMenu() noexcept = default;

    using ItemCallback = std::function<void()>;

    uint64_t AddItem(std::wstring_view text, ItemCallback cb = {}, uint32_t icon = 0) noexcept;
    void AddSeparator() noexcept;
    uint64_t AddSubMenu(std::wstring_view text, std::unique_ptr<UIMenu> submenu, uint32_t icon = 0) noexcept;
    void ClearItems() noexcept;

    void ShowAt(float x, float y) noexcept;
    void Close() noexcept;
    [[nodiscard]] bool IsOpen() const noexcept { return m_isOpen; }

    UIMenu* GetMenu() noexcept { return m_menu.get(); }

    void Render(RenderContext& ctx) noexcept override;
    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void ArrangeOverride(const LayoutSlot& finalSlot) noexcept override;

    bool OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept;
    bool OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept;

    [[nodiscard]] UIMenuItem* FindItemById(uint64_t id) noexcept;

private:
    struct ContextItem {
        uint64_t id{};
        bool isSeparator{};
        std::wstring text;
        uint32_t icon{};
        ItemCallback callback;
        bool enabled{ true };
        bool checked{};
        std::wstring shortcut;
        std::unique_ptr<UIMenu> submenu;
    };

    uint64_t NextId() noexcept
    {
        static uint64_t s_next = 1;
        return s_next++;
    }

    void RebuildMenu() noexcept;

    std::unique_ptr<UIMenu> m_menu;
    std::vector<ContextItem> m_items;
    std::unordered_map<UIMenuItem*, uint64_t> m_itemIdMap;
    bool m_isOpen{};
    float m_showX{};
    float m_showY{};
};

} // namespace
