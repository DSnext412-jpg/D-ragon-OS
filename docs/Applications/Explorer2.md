# Explorer 2.0 — Modern File Explorer for DragonOS

## Overview

Explorer 2.0 is a modern file manager rebuilt exclusively on the **DragonUI 1.0 Framework**. Every pixel of UI, every interaction, and every accessibility feature uses DragonUI controls, layouts, dialogs, data binding, and automation peers. No legacy UI, no custom widget toolkit — just DragonUI.

## Architecture

Explorer 2.0 follows a **MVVM (Model-View-ViewModel)** pattern with DragonUI data binding. The component hierarchy is:

```
ExplorerWindow (root DockPanel)
├─ TabStrip (horizontal DockPanel, top)
├─ RibbonBar (toolbar with actions, search)
├─ Center Region
│   ├─ NavigationPane (tree view, left, docked)
│   ├─ CenterColumn (vertical StackPanel)
│   │   ├─ BreadcrumbBar (address bar, top)
│   │   └─ FileListPane (virtualized list, center)
│   └─ PreviewPane (optional, right docked)
└─ StatusBarPane (bottom, full width)
```

### Key Components

| Component | DragonUI Controls | Responsibilities |
|-----------|-------------------|------------------|
| **NavigationPane** | `UITreeView`, `UIDockPanel`, `UIStackPanel`, `UIButton` | Tree navigation with Quick Access, This PC (drives), Network, lazy loading, bookmarks, AutomationPeer |
| **FileListPane** | `UIListView`, `CollectionViewSource`, `UIDockPanel` | Virtualized file listing with Details/List/Tiles modes, sorting, filtering, grouping, custom row renderer, AutomationPeer |
| **RibbonBar** | `UIButton`, `UIToolBar`, `UIStackPanel`, `UITextBox`, `UIMenu` | Action toolbar with navigation, file operations, view/sort/group menus, search box, AutomationPeer |
| **BreadcrumbBar** | `UIDockPanel`, `UIStackPanel`, `UITextBox`, `UIButton` | Editable path display with history drop-down, overflow ellipsis, edit mode (F6/Ctrl+L), AutomationPeer |
| **TabStrip** | `UIButton`, `UIDockPanel`, `Container` | Tabbed document interface with drag-reorder, middle-click close, new tab, AutomationPeer |
| **PreviewPane** | `UIDockPanel`, `UIStackPanel`, `UILabel`, `UIScrollViewer` | Image/text preview with plugin provider framework (IPreviewProvider), AutomationPeer |
| **StatusBarPane** | `UIStatusBar`, `UIStackPanel`, `UILabel`, `UIProgressBar` | Item counts, selection count, disk background operations, disk free space, AutomationPeer |
| **SearchController** | N/A | Incremental deep search with time-sliced walker, quick filter, indexed search provider, AutomationPeer |
| **ContextMenuService** | `UIContextMenu`, `UIMenu`, `UIMenuItem` | Background and item context menus, plugin extensions, "Open with", AutomationPeer |

## Part Details

### Part 1 — Modern Navigation Pane

- **TreeView** with hierarchical nodes: Quick Access (bookmarks + known folders), This PC (logical drives with lazy per-directory loading), Network placeholder
- **Lazy loading**: Directory children loaded on demand via `UITreeView::SetLazyLoader`
- **Directory caching**: Shared `DirectoryCache` with TTL-based freshness
- **Bookmark store**: Persistent in-memory store with pin/unpin functionality
- **Accessibility**: Every tree node and button exposes `AutomationPeer` with proper `AutomationId` and `AccessibleName`

### Part 2 — Modern File List

- **UIListView** with three presentation modes:
  - **Details**: Column headers + rows (default)
  - **List**: Single-column list with icons
  - **Tiles**: Icon grid with two-line labels
- **Sorting**: Column-header clicks routed to `ExplorerViewModel::ToggleSort`; sort indicators updated accordingly
- **Filtering**: Incremental quick filter via `ExplorerViewModel::SetQuickFilter`; matches flow through `CollectionViewSource`
- **Grouping**: Group headers embedded in the flat `FileRow` sequence; collapse/expand via `ExplorerViewModel::ToggleGroupCollapsed`; group labels and counts rendered inline by custom item renderer
- **Accessibility**: `AutomationPeer` on list view; row/column headers exposed with descriptive names; selection changed announcements

### Part 3 — Ribbon / Toolbar

- **Layout**: `[Back Forward]` cluster | [File actions: New folder, Cut, Copy, Paste, Rename, Delete, Properties] | [View ⬢ Sort ⬢ Group ↓] | [Preview] [Search box]
- **Drop-down menus**: View mode, Sort by, Group by with check marks reflecting active state
- **Search box**: Bound to `SearchController`; incremental text changes routed to view model and deep search walker
- **State sync**: Enabled/disabled state of action buttons mirrors per-tab selection state
- **Accessibility**: All buttons, edit box, and menus have `AutomationPeer` and `AutomationId`

### Part 4 — Breadcrumb Navigation

- **Editable path display**: Current path as clickable segment buttons
- **Overflow handling**: Ellipsis (`...`) button shown when path exceeds visible segments; opens history menu of hidden locations
- **History drop-down**: Recent locations from navigation history
- **Inline edit mode**: Toggled by F6, Ctrl+L, or click empty space; text box appears in place of segments
- **Navigate on segment click**: Clicking a breadcrumb segment navigates to that folder
- **Accessibility**: Full `AutomationPeer` wiring; edit box focus managed through `FocusManager`

### Part 5 — Search

- **Quick filter**: Instant name-based filtering of current directory listing; delegated to `ExplorerViewModel::SetQuickFilter`
- **Deep search**: Time-sliced recursive walker with ~2 ms per-frame budget; never stalls UI; results stream into view model
- **Indexed search**: Optional `ISearchIndexProvider` hook; plugins/OS search service can contribute results; results merged and de-duplicated with deep search
- **Search modes**: Togglable search-results mode that replaces normal folder semantics in the status bar
- **Accessibility**: Status bar announces search state; match count announced

### Part 6 — Preview Pane

- **Image preview**: Rendered as large glyph preview
- **Text preview**: First bytes rendered as wrapped text in scroll viewer
- **PDF preview**: Plugin-driven via `IPreviewProvider`; third parties register providers through the Explorer plugin host
- **Default fallback**: Icon + metadata block when no specialized provider available
- **Providers**: `IPreviewProvider` interface with `CanPreview()` and `CreatePreview()`; plugins register by extension or MIME type
- **Accessibility**: Preview title and content announced via `AutomationPeer`

### Part 7 — Context Menus

- **Background menu**: Copy, Paste, New folder, Properties, Sort, View mode, Group by
- **Item menu**: Open, Properties, Rename, Delete, Copy, Cut, Paste, Open with
- **Plugin extensions**: `IExplorerContextMenuExtension`; plugins can append items to any menu based on selection context
- **"Open with"**: Extension-point for file-type handlers; registered per-file-extension
- **Legacy adapter**: `ExtensionPointManager::ContextMenuExtension` bridging
- **Accessibility**: Context menus rendered through window overlay system; menu item actions announced

### Part 8 — Tabbed Explorer

- **Multiple tabs**: Tab strip with title + close button per tab
- **Drag-reorder**: Click-and-drag to reorder tabs; model order mutates, strip rebuilds
- **Middle-click close**: ✕ button or middle-click on tab closes
- **New tab**: `+` button or Ctrl+T; activates new tab with home folder
- **Restore tabs**: Ctrl+Shift+T restores most recently closed tab from closed-stack (up to 16 entries)
- **Accessibility**: Tab titles and close buttons have `AutomationId`; focus management through `FocusManager`

### Part 9 — Status Bar

- **Left panel**: Background operation text + progress bar (loading, file operations)
- **Right panel**: Item count (`N items`), selection count (`M selected`), free disk space (per-drive TTL-cached)
- **Background operations**: Progress shown in [0,1]; -1 hides progress; text announced via accessibility
- **Accessibility**: Summary text and progress announced; disk free space announced

### Part 10 — Performance

- **Directory caching**: `DirectoryCache` LRU with TTL (5 s default); shared across all windows/tabs
- **Lazy loading**: Tree directory children loaded on demand; no upfront enumeration
- **Load pipeline**: Frame-aware request scheduling; at most one enumeration in-flight per window; stale requests dropped
- **Large folder optimization**: Time-sliced deep search walker (~2 ms/frame); cache-first directory enumeration
- **Lazy loading**: Both tree children and file list rows loaded on demand via virtualization
- **Accessibility**: No performance regression; accessibility queries are O(1) against control state

### Part 11 — Accessibility

**Every control exposes an `AutomationPeer`** with the following pattern:

```cpp
// In control construction:
m_control->SetAutomationId(L"Explorer2/Panel/Subpart");
m_peer = AutomationPeer(*m_control);
```

**Automation roles and states** wired for key controls:

| Control | AutomationId Pattern | Key States |
|---------|---------------------|------------|
| `ExplorerWindow` | `L"Explorer2"` | Focused, structured change |
| `NavigationPane` | `L"Explorer2/Navigation"` | Expanded/collapsed nodes, selection |
| `UITreeView` | `L"Explorer2/Navigation/Tree"` | Item selection, focused item |
| `FileListPane` | `L"Explorer2/FileList"` | Multi-select, focused row, column headers |
| `UIListView` | `L"Explorer2/FileList/Items"` | Row selection, column sort arrows |
| `RibbonBar` | `L"Explorer2/Ribbon"` | Button enabled/disabled, search focus |
| `UIButton` (nav) | `L"Explorer2/Ribbon/Back"` | Pressed, enabled |
| `BreadcrumbBar` | `L"Explorer2/Breadcrumb"` | Edit mode, focus |
| `UITextBox` (breadcrumbs) | `L"Explorer2/Breadcrumb/EditBox"` | Text selection, caret position |
| `TabStrip` | `L"Explorer2/Tabs"` | Tab focus, active tab, draggable |
| `StatusBarPane` | `L"Explorer2/StatusBar"` | Summary text, progress, disk free |
| `ContextMenuService` | `L"Explorer2/ContextMenu"` | Visible, item count, highlighted |

**Announcement patterns**:
- `m_accessibility.Announce(L"N items loaded")` — after directory load
- `m_accessibility.ReportFocus(control)` — focus changes
- `m_accessibility.ReportSelectionChanged(listView)` — selection changes
- `m_accessibility.ReportStructuredChange()` — dialogs, model updates
- `m_accessibility.Alert(L"Operation failed")` — errors

### Part 12 — SDK: Extension Points for Plugins

Explorer 2.0 exposes the following extension points for third-party plugins:

#### Plugin Interface (`IExplorerPlugin`)

```cpp
class IExplorerPlugin {
public:
    virtual ~IExplorerPlugin() = default;
    virtual const wchar_t* GetName() const noexcept = 0;
    virtual void OnRegistered(IExplorerPluginHost& host) = 0;
    virtual void OnUnregistered() {}
};
```

#### Plugin Host (`IExplorerPluginHost`)

```cpp
class IExplorerPluginHost {
public:
    virtual void AddContextMenuExtension(IExplorerPlugin* owner, IExplorerContextMenuExtension* extension) = 0;
    virtual void AddPreviewProvider(IExplorerPlugin* owner, IPreviewProvider* provider) = 0;
    virtual void AddOpenWithHandler(IExplorerPlugin* owner, const std::wstring& dotExtension,
                                    const std::wstring& handlerLabel,
                                    std::function<void(const FileSystem::FileEntry&)> action) = 0;
    virtual void Announce(const std::wstring& text) = 0;
};
```

#### Context Menu Extensions (`IExplorerContextMenuExtension`)

```cpp
class IExplorerContextMenuExtension {
public:
    virtual bool AppliesTo(const std::vector<FileSystem::FileEntry>& selection,
                          const std::wstring& currentDirectory) noexcept = 0;
    virtual void AppendItems(DragonUI::UIContextMenu& menu,
                             const std::vector<FileSystem::FileEntry>& selection,
                             const std::wstring& currentDirectory) noexcept = 0;
};
```

#### Preview Providers (`IPreviewProvider`)

```cpp
class IPreviewProvider {
public:
    virtual bool CanPreview(const FileSystem::FileEntry& entry) noexcept = 0;
    virtual std::unique_ptr<DragonUI::Element> CreatePreview(
        const FileSystem::FileEntry& entry) noexcept = 0;
};
```

#### Registration

Plugins are registered through `ExplorerSystem::RegisterPlugin(std::unique_ptr<IExplorerPlugin>)`. The `PluginHost` bridges calls to the `ExplorerWindow` surface:

- `AddContextMenuExtension` → `ExplorerWindow::AddContextMenuExtension`
- `AddPreviewProvider` → `ExplorerWindow::AddPreviewProvider`
- `AddOpenWithHandler` → `ExplorerWindow::AddOpenWithHandler`
- `Announce` → `ExplorerWindow::Announce`

The `DRAGONOS_EXPLORER_EXPORT_PLUGIN` macro simplifies plugin shipping:

```cpp
#define DRAGONOS_EXPLORER_EXPORT_PLUGIN(PluginClass)
extern "C" __declspec(dllexport) DragonOS::Explorer2::IExplorerPlugin*
DragonOSExplorerCreatePlugin()
{
    return new (std::nothrow) PluginClass();
}
```

### Part 13 — Validation

**Explorer 2.0 must exclusively use DragonUI. No legacy UI permitted.**

**Verification checklist**:

- [x] All controls instantiated from DragonUI header classes (`DragonUI::UIButton`, `DragonUI::UITreeView`, etc.)
- [x] No `#include` of legacy UI headers (e.g., no `Button.h`, `ListView.h` from non-DragonUI paths)
- [x] Every top-level control has `SetAutomationId()` and `SetAccessibleName()`
- [x] Every control has `AutomationPeer` wired (via `= AutomationPeer(*control)` pattern)
- [x] Data binding uses `DragonUI::CollectionViewSource` and `DragonUI::ObservableCollection`
- [x] Dialogs use `DragonUI::UIDialog` / `DragonUI::UIMessageBox`
- [x] Drag-drop and input events route through DragonUI `FocusManager` and `DialogManager`
- [x] Theme applied via `DragonUI::ThemeManager` semantic colors
- [x] No Win32 window-class-based controls (e.g., `hwnd`-based buttons, combos)
- [x] Custom painting uses `DragonUI::RenderContext` only
- [x] Accessibility roles/states from `DragonUI::AccessibilityRole` / `DragonUI::AccessibilityState`
- [x] C++20 compliance: no exceptions used in public API, `noexcept` where appropriate, smart pointer ownership

**Build & test**:
- Compile with `/std:c++20` and `-W4`-level warnings
- Link against `DragonUI.lib`, `SDK.lib`, `FileSystem.lib`
- Run accessibility inspection: `Inspect.exe` should reveal AutomationId/Name on every element
- Run performance benchmark: directory load < 16 ms on 10,000-entry folder
- Run plugin sanity: register a sample context-menu extension; verify items appear in right-click menu

### Part 14 — Production-Quality C++20 Status

**Code standards enforced**:

1. **C++20** — Language features: `auto` type deduction, smart pointers, range-based for, `noexcept` specifiers, `constexpr` where applicable
2. **No raw pointer leaks** — All DragonUI objects owned by `std::unique_ptr` or parent container; `TakeRoot()` transfers ownership explicitly
3. **`noexcept` on destructors** — All Explorer2 final classes have `noexcept` destructors
4. **Exception safety** — `ShowDialog`, `BeginDeleteWithConfirm`, and file operations use RAII; `ShowDialog` always succeeds or fails gracefully
5. **Const-correctness** — View model getters are `const`; query methods do not modify state
6. **Thread safety** — Directory cache and load pipeline are single-threaded (UI thread); async loading via `LoadPipeline` posts to UI thread via callback
7. **Minimal includes** — Each `.cpp` includes only needed headers; precompiled headers where available
8. **Documentation completeness** — All public classes documented with `/** ... */` comments matching Doxygen style

**Output**: Production-quality Explorer 2.0 binary that:
- Renders a fully functional modern file explorer
- Supports all 13 functional parts (navigation, file list, ribbon, breadcrumbs, search, preview, contexts, tabs, status, performance, accessibility, plugins)
- Exclusively uses DragonUI with no legacy fallback
- Is accessible via UI Automation (Inspect.exe reveals full tree)
- Can be extended via the SDK plugin API