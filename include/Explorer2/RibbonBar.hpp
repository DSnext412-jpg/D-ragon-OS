// ============================================================================
//  RibbonBar.hpp — Modern ribbon / toolbar (Part 3).
//
//  Layout: [Back Forward Up Refresh] | [New Folder Cut Copy Paste Rename
//  Delete | Properties | View options | Sort options | Group options | Preview]
//  ... [Search box]
//
//  Drop-down menus are rebuilt on demand so check marks always reflect the
//  active tab's state.  The window supplies callbacks through Actions.
//  All DragonUI controls are used exclusively.  Accessibility is handled by
//  the host's AccessibilityManager (Part 11).
// ============================================================================

#pragma once

#include <Explorer2/ExplorerTypes.hpp>
#include <DragonUI/Controls/Button.hpp>
#include <DragonUI/Controls/DockPanel.hpp>
#include <DragonUI/Controls/Menu.hpp>
#include <DragonUI/Controls/StackPanel.hpp>
#include <DragonUI/Controls/ToolBar.hpp>
#include <DragonUI/Controls/TextBox.hpp>

#include <functional>
#include <memory>
#include <string>

namespace DragonOS::Explorer2 {

/// Owns a UIDockPanel root; DragonUI controls are final so panes wrap.
class RibbonBar final {
public:
    struct Actions final {
        std::function<void()> onBack;
        std::function<void()> onForward;
        std::function<void()> onUp;
        std::function<void()> onRefresh;
        std::function<void()> onNewFolder;
        std::function<void()> onCut;
        std::function<void()> onCopy;
        std::function<void()> onPaste;
        std::function<void()> onRename;
        std::function<void()> onDelete;
        std::function<void()> onProperties;
        std::function<void(ViewMode)> onViewMode;
        std::function<void(SortKey)> onSortBy;      // Toggles direction when same key.
        std::function<void(GroupMode)> onGroupBy;
        std::function<void()> onTogglePreview;

        std::function<bool()> canGoBack;
        std::function<bool()> canGoForward;
        std::function<ViewMode()> currentViewMode;
        std::function<SortKey()> currentSortKey;
        std::function<SortDir()> currentSortDir;
        std::function<GroupMode()> currentGroupMode;
        std::function<bool()> previewVisible;
        std::function<bool()> hasSelection;
        std::function<bool()> clipboardHasContent;
    };

    RibbonBar() noexcept;

    [[nodiscard]] DragonUI::Element* GetControl() noexcept { return m_root.get(); }
    /// Transfers the root panel into the owning UI tree (assembly-time).
    [[nodiscard]] std::unique_ptr<DragonUI::Element> TakeRoot() noexcept
    {
        return std::move(m_root);
    }

    void SetActions(Actions actions) noexcept;
    void BindSearch(std::function<void(const std::wstring&)> onChanged,
                    std::function<void()> onFocusRequest) noexcept;

    /// Refreshes enabled/disabled + menu state (call per tab switch / change).
    void SyncState() noexcept;

    /// Rebuilds drop-down menus so check marks reflect the current state
    /// (call after view/sort/group changes that bypass the ribbon).
    void RefreshMenus() noexcept;

    void SetSearchText(const std::wstring& text) noexcept { m_searchBox->SetText(text); }
    [[nodiscard]] DragonUI::Element* GetSearchBox() noexcept { return m_searchBox.get(); }

private:
    using MenuPtr = std::unique_ptr<DragonUI::UIMenu>;

    [[nodiscard]] MenuPtr BuildViewMenu() const noexcept;
    [[nodiscard]] MenuPtr BuildSortMenu() const noexcept;
    [[nodiscard]] MenuPtr BuildGroupMenu() const noexcept;
    [[nodiscard]] MenuPtr BuildHistorySubMenu(bool forward) const noexcept;

    void RebuildActionButtons() noexcept;

    Actions m_actions{};

    std::unique_ptr<DragonUI::Element> m_root{std::make_unique<DragonUI::UIDockPanel>()};
    Element* m_releasedRoot{};  ///< Alias kept after TakeRoot.

    // Navigation cluster
    Element* m_backButton{};
    Element* m_forwardButton{};
    Element* m_navHost{};

    // Action toolbar (fill)
    Element* m_toolBar{};

    // Search box
    std::unique_ptr<DragonUI::Element> m_searchBox;  // UITextBox wrapped in Element

    // Action button indices (for state sync)
    size_t m_indexNewFolder{ 0 };
    size_t m_indexCut{ 0 };
    size_t m_indexCopy{ 0 };
    size_t m_indexPaste{ 0 };
    size_t m_indexRename{ 0 };
    size_t m_indexDelete{ 0 };
    size_t m_indexProperties{ 0 };

    // Accessibility - peers created by host's AccessibilityManager
};
} // namespace DragonOS::Explorer2