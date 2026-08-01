#pragma once

#include <DragonUI/Core/Control.hpp>
#include <DragonUI/Core/SelectionManager.hpp>

#include <any>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace DragonOS::DragonUI {

class UITreeView;

/**
 * @brief  A single node in a UITreeView.
 *
 * Nodes form a classic parent/child tree.  Ownership is exclusive
 * (unique_ptr) so the whole tree is freed automatically.  A node may
 * register a lazy loader that populates its children the first time it
 * is expanded, enabling deep, virtualized navigation trees.
 */
class UITreeNode final {
public:
    UITreeNode() noexcept = default;
    explicit UITreeNode(std::wstring_view text, uint32_t icon = 0) noexcept;

    UITreeNode(const UITreeNode&) = delete;
    UITreeNode& operator=(const UITreeNode&) = delete;

    // ── Text / icon ──────────────────────────────────────────────────

    [[nodiscard]] const std::wstring& GetText() const noexcept { return m_text; }
    void SetText(std::wstring_view text) noexcept;
    void SetIcon(uint32_t glyph) noexcept { m_iconGlyph = glyph; }
    [[nodiscard]] uint32_t GetIcon() const noexcept { return m_iconGlyph; }

    // ── Expansion ────────────────────────────────────────────────────

    [[nodiscard]] bool IsExpanded() const noexcept { return m_expanded; }
    void SetExpanded(bool expanded) noexcept { m_expanded = expanded; }
    [[nodiscard]] bool HasChildren() const noexcept { return !m_children.empty(); }
    [[nodiscard]] bool IsLeaf() const noexcept { return m_leaf && m_children.empty(); }
    void SetLeaf(bool leaf) noexcept { m_leaf = leaf; }

    // ── Lazy loading ─────────────────────────────────────────────────

    void SetLazyLoader(std::function<void(UITreeNode&)> loader) noexcept;
    void LoadChildrenIfNeeded() noexcept;
    [[nodiscard]] bool NeedsLazyLoad() const noexcept;

    // ── Hierarchy ────────────────────────────────────────────────────

    [[nodiscard]] int GetDepth() const noexcept;
    [[nodiscard]] UITreeNode* GetParent() const noexcept { return m_parent; }

    UITreeNode* AddChild(std::wstring_view text, uint32_t icon = 0) noexcept;
    void ClearChildren() noexcept;
    [[nodiscard]] const std::vector<std::unique_ptr<UITreeNode>>& GetChildren() const noexcept { return m_children; }

    // ── Data ─────────────────────────────────────────────────────────

    template<typename T>
    void SetUserData(T* data) noexcept { m_data = data; }
    template<typename T>
    [[nodiscard]] T* GetUserData() const noexcept {
        return std::any_cast<T>(&const_cast<std::any&>(m_data));
    }

private:
    friend class UITreeView;

    std::wstring m_text;
    uint32_t m_iconGlyph{};
    bool m_expanded{};
    bool m_leaf{};
    UITreeNode* m_parent{};
    std::vector<std::unique_ptr<UITreeNode>> m_children;
    std::function<void(UITreeNode&)> m_lazyLoader;
    std::any m_data;
};

/**
 * @brief  Virtualized tree control with nested nodes, expand/collapse,
 *         icons, selection, keyboard navigation and lazy loading.
 */
class UITreeView final : public Control {
public:
    using NodeActivatedCallback = std::function<void(UITreeView&, UITreeNode&)>;

    UITreeView() noexcept;

    // ── Structure ────────────────────────────────────────────────────

    UITreeNode* AddRootNode(std::wstring_view text, uint32_t icon = 0) noexcept;
    void ClearNodes() noexcept;
    [[nodiscard]] UITreeNode* GetRootNode() const noexcept { return m_root.get(); }

    // ── Expansion ────────────────────────────────────────────────────

    void ExpandNode(UITreeNode& node) noexcept;
    void CollapseNode(UITreeNode& node) noexcept;
    void ToggleNode(UITreeNode& node) noexcept;
    void ExpandAll() noexcept;
    void CollapseAll() noexcept;

    // ── Callbacks ────────────────────────────────────────────────────

    void SetOnNodeActivated(NodeActivatedCallback cb) noexcept { m_onNodeActivated = std::move(cb); }
    void SetOnNodeExpanded(std::function<void(UITreeNode&, bool)> cb) noexcept { m_onNodeExpanded = std::move(cb); }
    void SetOnSelectionChanged(std::function<void(const SelectionManager&)> cb) noexcept { m_onSelectionChanged = std::move(cb); }

    // ── Selection ────────────────────────────────────────────────────

    [[nodiscard]] SelectionManager& GetSelection() noexcept { return m_selection; }
    [[nodiscard]] const SelectionManager& GetSelection() const noexcept { return m_selection; }
    [[nodiscard]] UITreeNode* GetSelectedNode() const noexcept;
    void SelectNode(UITreeNode& node) noexcept;

    // ── Visible flattening ───────────────────────────────────────────

    [[nodiscard]] int64_t GetVisibleNodeCount() const noexcept { return static_cast<int64_t>(m_visible.size()); }
    [[nodiscard]] UITreeNode* GetVisibleNode(int64_t index) const noexcept;
    [[nodiscard]] int64_t GetVisibleIndex(const UITreeNode& node) const noexcept;
    void Refresh() noexcept;

    // ── Scrolling ────────────────────────────────────────────────────

    void SetScrollOffset(float offset) noexcept;
    [[nodiscard]] float GetScrollOffset() const noexcept { return m_scrollOffset; }
    void ScrollToNode(UITreeNode& node) noexcept;

    [[nodiscard]] UITreeNode* HitTestNodeAt(float x, float y) const noexcept;

    void SetRowHeight(float height) noexcept { m_rowHeight = (std::max)(16.0f, height); InvalidateLayout(); }
    [[nodiscard]] float GetRowHeight() const noexcept { return m_rowHeight; }

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void Render(RenderContext& ctx) noexcept override;
    bool OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept override;
    bool OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept override;
    bool OnFocusEvent(const FocusEventArgs& args) noexcept override;

private:
    void FlattenVisible() noexcept;
    void RenderNode(RenderContext& ctx, const LayoutSlot& slot, UITreeNode& node, int64_t index) noexcept;
    void RenderScrollBar(RenderContext& ctx) noexcept;
    void EnsureVisible(int64_t index) noexcept;
    [[nodiscard]] float GetContentHeight() const noexcept;
    void OnSelectionChanged(const SelectionManager& sm) noexcept;

    std::unique_ptr<UITreeNode> m_root;
    std::vector<UITreeNode*> m_visible;
    SelectionManager m_selection;
    NodeActivatedCallback m_onNodeActivated;
    std::function<void(UITreeNode&, bool)> m_onNodeExpanded;
    std::function<void(const SelectionManager&)> m_onSelectionChanged;

    float m_rowHeight{ 26.0f };
    float m_indentWidth{ 18.0f };
    float m_scrollOffset{};
    int64_t m_hoveredIndex{ -1 };
    bool m_visibleDirty{ true };

    static constexpr float ScrollBarWidth = 10.0f;
    static constexpr float ExpandArrowWidth = 18.0f;
    static constexpr float IconWidth = 20.0f;
};

} // namespace DragonOS::DragonUI
