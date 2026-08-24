// ============================================================================
//  FileListPane.hpp — The virtualized file list (Part 2).
//
//  Wraps a DragonUI UIListView bound to the tab's ObservableCollection<FileRow>
//  through CollectionViewSource.  Provides:
//    - Details / List / Tiles presentation (custom row renderer, incl. group
//      headers drawn inline so virtualization keeps working when grouped),
//    - sortable column headers (clicks are intercepted before the control's
//      own sort and routed to the view model, keeping groups intact),
//    - selection helpers that expose only entry rows,
//    - hit-testing for context menus and activation.
//
//  All DragonUI controls are used exclusively.  Accessibility is handled by
//  the host's AccessibilityManager (Part 11).
// ============================================================================

#pragma once

#include <Explorer2/ExplorerTypes.hpp>
#include <DragonUI/Controls/ListView.hpp>
#include <DragonUI/Controls/DockPanel.hpp>
#include <DragonUI/DataBinding/CollectionViewSource.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace DragonOS::Explorer2 {

class ExplorerViewModel;

/// Owns a UIDockPanel root; DragonUI controls are final so panes wrap.
class FileListPane final {
public:
    using ActivateCallback     = std::function<void(const FileSystem::FileEntry&)>;
    using SelectionChangedFn   = std::function<void()>;
    using SortChangedFn        = std::function<void()>;

    FileListPane() noexcept;

    [[nodiscard]] DragonUI::Element* GetControl() noexcept { return m_root.get(); }
    /// Transfers the root panel into the owning UI tree (assembly-time).
    [[nodiscard]] std::unique_ptr<DragonUI::Element> TakeRoot() noexcept
    {
        return std::move(m_root);
    }

    /// Binds a view model's rows to the list.  Call again when switching tabs.
    void BindModel(ExplorerViewModel& viewModel) noexcept;

    void ApplyViewMode(ViewMode mode) noexcept;

    void SetOnActivateEntry(ActivateCallback cb) noexcept { m_onActivate = std::move(cb); }
    void SetOnSelectionChanged(SelectionChangedFn cb) noexcept { m_onSelectionChanged = std::move(cb); }
    void SetOnSortChanged(SortChangedFn cb) noexcept { m_onSortChanged = std::move(cb); }

    /// External sort change (ribbon / context menu): re-syncs column arrows.
    void NotifySortChanged() noexcept;

    // ── Selection ────────────────────────────────────────────────────────

    [[nodiscard]] std::vector<const FileSystem::FileEntry*> GetSelectedEntries() const noexcept;
    [[nodiscard]] const FileSystem::FileEntry* GetSingleSelectedEntry() const noexcept;
    void ClearSelection() noexcept;
    void SelectAllEntries() noexcept;

    // ── Hit testing ──────────────────────────────────────────────────────

    /// Entry under an absolute point, or nullptr (background / header).
    [[nodiscard]] const FileSystem::FileEntry* GetEntryAtPoint(float x, float y) const noexcept;

    /// Row under an absolute point; nullptr when none.
    [[nodiscard]] const FileRow* GetRowAtPoint(float x, float y) const noexcept;

    /// Intercepts Details-header clicks and routes them to the view model.
    /// @return true when the click was consumed as a sort action.
    bool HandleHeaderClick(float x, float y) noexcept;

    /// Keeps a renamed/moved entry visible after refresh.
    void RevealEntry(const std::wstring& fullPath) noexcept;

    void SetFocusToList() noexcept;

    [[nodiscard]] DragonUI::UIListView& GetList() noexcept { return *m_list; }

private:
    void ConfigureColumns() noexcept;
    void InstallProviders() noexcept;
    void RefreshSortIndicators() noexcept;

    [[nodiscard]] int ColumnIndexFromLocalX(float localX) const noexcept;
    [[nodiscard]] static DragonUI::SortDirection ToControlDirection(SortDir dir) noexcept;

    ExplorerViewModel* m_viewModel{};
    std::unique_ptr<DragonUI::UIDockPanel> m_root{std::make_unique<DragonUI::UIDockPanel>()};
    DragonUI::Element* m_releasedRoot{};  ///< Alias kept after TakeRoot.
    std::shared_ptr<DragonUI::CollectionViewSource<FileRow>> m_source;
    UIListView* m_list{};  // Raw pointer, owned by m_root child

    ActivateCallback   m_onActivate;
    SelectionChangedFn m_onSelectionChanged;
    SortChangedFn      m_onSortChanged;

    // Filter state
    ViewMode m_lastAppliedMode{ ViewMode::Details };
};