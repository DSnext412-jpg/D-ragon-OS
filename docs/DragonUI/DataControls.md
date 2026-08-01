# DragonUI Data Controls

## Architecture

DragonUI Data Controls present large, structured datasets. They extend the
`DragonOS::DragonUI::Control` base class and share a common set of reusable
building blocks:

- **SelectionManager** — A shared single/multi selection engine.
- **VirtualItemSource** — A lazily-queried item source (count + item).
- **VirtualViewport** — Pure viewport math (visible rows/columns, clamped scroll).
- **ItemTemplate** — Type-safe item renderers with visual-state metadata.

All rendering uses the theme-token `RenderContext` (no hardcoded colours) and
icons are `uint32_t` Unicode codepoints converted to UTF-16 via `Glyph.hpp`.

## Class Diagram

```
Element
  |
  +-- Control
        |
        +-- UIListView     (Details / List / Tile, columns, sorting, filtering)
        +-- UITreeView     (nested nodes, lazy loading, expand/collapse)
        +-- UIGridView     (sortable columns, resizable headers, alternating rows)
```

Supporting classes:

- `SelectionManager` — Selection engine with `None` / `Single` / `Multi` modes.
- `VirtualItemSource` — Pure virtual `GetCount()` / `GetItem(index)` adapter.
- `VirtualViewport` — Scroll + visible-range math shared by List/Grid views.
- `ItemTemplate<T>` / `MakeErasedItemRenderer(T)` — Type-safe item renderers.
- `UITreeNode` — Tree node with exclusive ownership, lazy loaders, user data.
- `Glyph` — `CodepointToUtf16(uint32_t)` for supplementary-plane icons.

## Selection Model

`SelectionManager` is shared by all three controls, so the semantics are
identical everywhere:

| Gesture                  | Result                                  |
|--------------------------|-----------------------------------------|
| Click                    | Select only the clicked item            |
| Ctrl+Click               | Toggle the clicked item                 |
| Shift+Click              | Select range from anchor to item        |
| Ctrl+Shift+Click         | Extend the current selection by range   |
| Ctrl+A                   | Select all                              |
| Arrow keys / Home / End  | Move the current item (Shift extends)   |

`SelectionMode::None` disables selection entirely; `Single` limits to one item.

## Virtualization

Only items intersecting the viewport are fetched and rendered:

1. The control owns a `VirtualItemSource`; the source materialises items on
   demand (`GetItem`), so the backing store can be huge (e.g. 50,000 entries).
2. `VirtualViewport` maps scroll offsets to visible row/column ranges and
   clamps the scrollbar.
3. `UIListView` additionally keeps a display-index → source-index vector so
   sorting and filtering do not mutate the source.

## UIListView

A virtualized list supporting three modes:

- **Details** — Column headers, per-column `getText`, sortable columns, and
  draggable column dividers.
- **List** — Single column with icons.
- **Tile** — Grid of icon tiles with labels.

```cpp
auto source = std::make_shared<MyItemSource>();   // : VirtualItemSource
DragonOS::DragonUI::UIListView list;
list.SetItemSource(source);
list.SetMode(DragonOS::DragonUI::ListViewMode::Details);
list.SetPrimaryTextProvider([](const std::any& item) -> std::wstring {
    return std::any_cast<MyItem>(&item)->name;
});
auto nameCol = DragonOS::DragonUI::UIListView::Column{};
nameCol.title  = L"Name";
nameCol.width  = 200.0f;
nameCol.getText = [](const std::any& item) -> std::wstring { /* ... */ };
list.AddColumn(std::move(nameCol));
list.SetOnItemActivated([](DragonOS::DragonUI::UIListView&, int64_t index) {
    /* open the item */
});
```

Selection is exposed through `GetSelection()`:

```cpp
list.GetSelection().SetMode(DragonOS::DragonUI::SelectionMode::Multi);
const auto& sel = list.GetSelection().GetSelectedIndices(); // vector<int64_t>
```

## UITreeView

A virtualized tree of `UITreeNode`s. Nodes own their children via `unique_ptr`
and support lazy loaders that populate children on first expansion — ideal for
filesystem navigation.

```cpp
DragonOS::DragonUI::UITreeView tree;
auto* root = tree.AddRootNode(L"Quick Access", 0x1F4C1);
root->SetExpanded(true);
auto* docs = root->AddChild(L"Documents", 0x1F4C4);
docs->SetLazyLoader([](DragonOS::DragonUI::UITreeNode& node) {
    /* populate node.AddChild(...) */
});
tree.SetOnNodeActivated([](DragonOS::DragonUI::UITreeView&, DragonOS::DragonUI::UITreeNode& node) {
    /* navigate */
});
```

Arbitrary payloads can be attached with `SetUserData<T>` / `GetUserData<T>`.
Visible rows are flattened per frame; only visible nodes are drawn. Left/Right
arrows collapse/expand, Enter activates.

## UIGridView

A column-based grid with resizable headers, per-column sorting, alternating row
colours and double-click cell activation. Rows are virtualized; columns are
configured per-column with optional `getText`, `getColor`, `comparer` and
`sortable`.

## Explorer Integration

The Explorer window (`src/Explorer/ExplorerWindow.cpp`) uses `UIListView` for the
file list (Details/List view modes) and `UITreeView` for the navigation pane:

- `ExplorerFileSource` adapts the current directory's `FileEntry` vector to
  `VirtualItemSource`.
- Name / Size / Date Modified columns render file metadata; icons come from
  `GetEntryIcon`.
- Double-click (or Enter) on a folder navigates into it; activating a tree node
  navigates to its stored path.
- Grid view mode intentionally keeps the legacy renderer.

## Performance Notes

- Never iterate the full source in render paths; rely on `VirtualViewport`
  visible-range queries.
- Prefer lazy loaders for trees with deep or slow-to-enumerate branches.
- Sorting/filtering operates on the display-index vector, not the source, so
  `Refresh()` remains cheap for large datasets.
