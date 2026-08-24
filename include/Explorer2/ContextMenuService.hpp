// ============================================================================
//  ContextMenuService.hpp — Context menus + extension points (Part 7).
//
//  Builds right-click menus for list backgrounds, single entries and multi
//  selections.  The host window owns an overlay stack: built menus are handed
//  over through a MenuSink and every open menu is routed input + rendered on
//  top of everything else until dismissed.
//
//  Extension points (Part 12 bridge):
//    - IExplorerContextMenuExtension: appends plugin items to any menu,
//    - legacy ExtensionPointManager::ContextMenuExtension adapters are
//      registered by ExplorerSystem.
//
//  Accessibility is handled by the host's AccessibilityManager (Part 11).
// ============================================================================

#pragma once

#include <Explorer2/ExplorerTypes.hpp>
#include <DragonUI/Controls/ContextMenu.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace DragonOS::ExtensionPoints { class ExtensionPointManager; }

namespace DragonOS::Explorer2 {

/// A menu factory appends items for the given selection snapshot.
class IExplorerContextMenuExtension {
public:
    virtual ~IExplorerContextMenuExtension() = default;

    /// @return true when this extension wants items in this context.
    [[nodiscard]] virtual bool AppliesTo(
        const std::vector<FileSystem::FileEntry>& selection,
        const std::wstring& currentDirectory) noexcept = 0;

    /// Append items to @p sink.  Item callbacks capture what they need.
    virtual void AppendItems(DragonUI::UIContextMenu& menu,
                             const std::vector<FileSystem::FileEntry>& selection,
                             const std::wstring& currentDirectory) noexcept = 0;
};

/// Lightweight command surface implemented by ExplorerWindow.
class IContextMenuTarget {
public:
    virtual ~IContextMenuTarget() = default;

    virtual void OpenEntry(const FileSystem::FileEntry&) = 0;
    virtual void CutSelection() = 0;
    virtual void CopySelection() = 0;
    virtual void PasteIntoCurrentFolder() = 0;
    virtual void DeleteSelection() = 0;
    virtual void RenameSelection() = 0;
    virtual void ShowProperties(const FileSystem::FileEntry*) = 0;
    virtual void NewFolder() = 0;
    virtual void RefreshView() = 0;
    virtual void SelectAll() = 0;
    virtual void SetViewMode(ViewMode) = 0;
    virtual void SetSortBy(SortKey) = 0;
    virtual void SetGroupBy(GroupMode) = 0;
};

class ContextMenuService final {
public:
    using MenuSink = std::function<void(std::unique_ptr<DragonUI::UIContextMenu>, float, float)>;

    /// A menu factory appends items for the given selection snapshot.
    struct State final {
        IContextMenuTarget* target{};
        std::wstring currentPath;
        std::vector<FileSystem::FileEntry> selection;   ///< Empty = background.
        ViewMode viewMode{ ViewMode::Details };
        SortKey sortKey{ SortKey::Name };
        GroupMode groupMode{ GroupMode::None };
        bool clipboardHasContent{ false };
    };

    /// Lightweight "Open with" entry for a specific file (supplied by the plugin host).
    struct OpenWithAction final {
        std::wstring label;
        std::function<void(const FileSystem::FileEntry&)> action;
    };
    using OpenWithProvider =
        std::function<std::vector<OpenWithAction>(const FileSystem::FileEntry&)>;

    void SetSink(MenuSink sink) noexcept { m_sink = std::move(sink); }

    /// Supplies per-file "Open with" actions (plugin-registered handlers).
    void SetOpenWithProvider(OpenWithProvider provider) noexcept
    {
        m_openWith = std::move(provider);
    }

    /// Registers a plugin extension (ownership stays with the caller/registry).
    void AddExtension(IExplorerContextMenuExtension* extension) noexcept;
    void RemoveExtension(IExplorerContextMenuExtension* extension) noexcept;

    /// Adapts OS-level ExtensionPointManager context-menu entries.
    void BindLegacyExtensions(ExtensionPoints::ExtensionPointManager* manager) noexcept
    {
        m_legacyManager = manager;
    }

    /// Builds and pops the appropriate menu at absolute screen coordinates.
    void ShowBackgroundMenu(float x, float y, const State& state) const noexcept;
    void ShowItemMenu(float x, float y, const State& state) const noexcept;

private:
    void AppendPluginItems(
        DragonUI::UIContextMenu& menu, const State& state) const noexcept;
    void AppendOpenWithItems(
        DragonUI::UIContextMenu& menu, const State& state) const noexcept;

    MenuSink m_sink;
    OpenWithProvider m_openWith;
    std::vector<IExplorerContextMenuExtension*> m_extensions;
    ExtensionPoints::ExtensionPointManager* m_legacyManager{ nullptr };
};
} // namespace DragonOS::Explorer2