#pragma once
#include <UI/Core/UIContainer.hpp>
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace DragonOS::UI {

struct TreeViewItem {
    std::wstring text;
    std::vector<std::unique_ptr<TreeViewItem>> children;
    bool isExpanded{false};
    int depth{0};
    TreeViewItem* parent{nullptr};
};

class TreeView : public UIContainer {
public:
    TreeView() noexcept;

    int AddItem(int parentIndex, std::wstring_view text) noexcept;
    bool RemoveItem(int index) noexcept;
    void ClearItems() noexcept;

    void Expand(int index) noexcept;
    void Collapse(int index) noexcept;
    [[nodiscard]] bool IsExpanded(int index) const noexcept;

    void SetSelectedIndex(int index) noexcept;
    [[nodiscard]] int GetSelectedIndex() const noexcept { return m_selectedIndex; }
    [[nodiscard]] TreeViewItem* GetItem(int index) const noexcept;

    void SetOnSelectionChanged(std::function<void(int)> callback) noexcept { m_onSelectionChanged = std::move(callback); }

    void Render(UIRenderer& renderer) noexcept override;
    bool OnEvent(const UIEvent& event) noexcept override;

    static constexpr float ItemHeight = 28.0f;
    static constexpr float IndentWidth = 20.0f;

private:
    void FlatternTree() noexcept;
    void FlattenChildren(TreeViewItem* parent) noexcept;
    TreeViewItem* FindItem(int index, std::vector<std::unique_ptr<TreeViewItem>>& items, int& current) noexcept;

    std::vector<std::unique_ptr<TreeViewItem>> m_rootItems;
    std::vector<TreeViewItem*> m_flatItems;
    int m_selectedIndex{-1};
    int m_hoveredIndex{-1};
    std::function<void(int)> m_onSelectionChanged;
};

} // namespace
