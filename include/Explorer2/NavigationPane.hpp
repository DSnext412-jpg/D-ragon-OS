// ============================================================================
//  NavigationPane.hpp — Modern navigation / tree pane (Part 1).
//
//  A DragonUI UITreeView with:
//    - "Quick access" root: bookmarks + known folders,
//    - "This PC" root: logical drives with lazy per-directory loading,
//    - a Network placeholder node,
//    - an inline bookmark toolbar (pin/unpin current folder).
//
//  Node → path mapping lives in an id-keyed map so node pointers can be
//  handed out safely while entries are rebuilt.
//
//  All DragonUI controls are used exclusively.  Accessibility is handled by
//  the host's AccessibilityManager (Part 11).
// ============================================================================

#pragma once

#include <Explorer2/ExplorerTypes.hpp>
#include <DragonUI/Controls/TreeView.hpp>
#include <DragonUI/Controls/DockPanel.hpp>
#include <DragonUI/Controls/StackPanel.hpp>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace DragonOS::FileSystem { class FileSystemService; }

namespace DragonOS::Explorer2 {

class DirectoryCache;

/// Owns a UIDockPanel root and builds the navigation UI inside it.  DragonUI
/// control classes are final, so application panes wrap rather than derive.
class NavigationPane final {
public:
    using NavigateFn = std::function<void(const std::wstring&)>;

    NavigationPane(
        FileSystem::FileSystemService& fileSystem,
        DirectoryCache& cache,
        BookmarkStore& bookmarks) noexcept;

    [[nodiscard]] DragonUI::Element* GetControl() noexcept { return m_root.get(); }
    /// Transfers the root panel into the owning UI tree (assembly-time).
    [[nodiscard]] std::unique_ptr<DragonUI::Element> TakeRoot() noexcept
    {
        return std::move(m_root);
    }

    void SetOnNavigate(NavigateFn cb) noexcept { m_onNavigate = std::move(cb); }
    void SetOnPinRequested(std::function<void()> cb) noexcept { m_onPinRequested = std::move(cb); }

    /// (Re)builds the drive list under "This PC".
    void RefreshDrives() noexcept;

    /// Rebuilds the Quick access children from the bookmark store.
    void RefreshQuickAccess() noexcept;

    /// Best-effort: expands the tree along @p path and selects the deepest
    /// matching node.
    void HighlightPath(const std::wstring& path) noexcept;

private:
    void BuildRoots() noexcept;
    void LoadDirectoryChildren(DragonUI::UITreeNode& node, const std::wstring& path) noexcept;

    [[nodiscard]] DragonUI::UITreeNode* AddPathNode(
        DragonUI::UITreeNode& parent, const std::wstring& name,
        const std::wstring& path, uint32_t glyph) noexcept;

    [[nodiscard]] const std::wstring* PathOf(const DragonUI::UITreeNode& node) const noexcept;
    void StorePath(DragonUI::UITreeNode& node, const std::wstring& path);

    FileSystem::FileSystemService& m_fileSystem;
    DirectoryCache& m_cache;
    BookmarkStore& m_bookmarks;

    std::unique_ptr<DragonUI::UIDockPanel> m_root{std::make_unique<DragonUI::UIDockPanel>()};
    UITreeView* m_tree{};
    UITreeNode* m_quickAccessNode{};
    UITreeNode* m_thisPCNode{};

    Element* m_layout{};      ///< Stack panel
    Element* m_pinButton{};   ///< Button

    NavigateFn m_onNavigate;
    std::function<void()> m_onPinRequested;

    /// Node addresses are stable (exclusive unique_ptr ownership); entries
    /// are erased together with the nodes they describe.
    std::unordered_map<const UITreeNode*, std::wstring> m_nodePaths;

    // Accessibility peers are created by the host's AccessibilityManager
};
} // namespace DragonOS::Explorer2