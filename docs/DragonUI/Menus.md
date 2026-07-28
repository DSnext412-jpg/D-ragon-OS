# DragonUI — Menus & Toolbars

## Architecture

DragonUI provides a complete set of reusable command surface controls built on the existing DragonUI framework:

| Control        | Base Class    | Purpose                                  |
|---------------|---------------|------------------------------------------|
| `UIMenuBar`   | `Container`   | Horizontal top-level menu bar            |
| `UIMenu`      | `Container`   | Dropdown menu with items and submenus    |
| `UIContextMenu` | `Element`   | Right-click popup context menu           |
| `UIMenuItem`  | `Control`     | Single menu item (text, icon, shortcut)  |
| `UIToolBar`   | `Container`   | Toolbar with buttons, separators, dropdowns |
| `UIStatusBar` | `Container`   | Status bar with text, icons, progress    |

All controls integrate with:

- **ThemeManager** — colours via `SemanticColor` tokens
- **Renderer** — rendered via `RenderContext`
- **Input System** — mouse/keyboard event handling
- **Focus Manager** — keyboard navigation support
- **Event System** — `DragonUI::EventType` / `EventArgs`
- **Layout Engine** — `Measure` / `Arrange` / `Render` pipeline

## Usage

### Menu Bar

```cpp
#include <DragonUI/Controls/MenuBar.hpp>

auto menuBar = std::make_unique<UIMenuBar>();

// Create a File menu
auto fileMenu = std::make_unique<UIMenu>();
fileMenu->AddItem(L"New", []() { /* action */ });
fileMenu->AddItem(L"Open", []() { /* action */ });
fileMenu->AddSeparator();
fileMenu->AddItem(L"Exit", []() { /* action */ });
menuBar->AddMenu(L"File", std::move(fileMenu));

// Create an Edit menu
auto editMenu = std::make_unique<UIMenu>();
editMenu->AddItem(L"Copy", []() { /* action */ });
editMenu->AddItem(L"Paste", []() { /* action */ });
menuBar->AddMenu(L"Edit", std::move(editMenu));

// Layout and render
menuBar->Measure(availableSlot);
menuBar->Arrange(finalSlot);
menuBar->Render(ctx);
```

### Context Menu

```cpp
#include <DragonUI/Controls/ContextMenu.hpp>

auto ctxMenu = std::make_unique<UIContextMenu>();
ctxMenu->AddItem(L"Open", []() { /* action */ }, 0x1F4C4 /* icon */);
ctxMenu->AddSeparator();
ctxMenu->AddItem(L"Delete", []() { /* action */ });
ctxMenu->AddSubMenu(L"Send To", std::move(sendToMenu));

// Show at mouse position
ctxMenu->ShowAt(mouseX, mouseY);

// Dismiss
ctxMenu->Close();
```

### Toolbar

```cpp
#include <DragonUI/Controls/ToolBar.hpp>

auto toolbar = std::make_unique<UIToolBar>();
toolbar->AddButton(L"Back", backCallback, 0x25C0);
toolbar->AddButton(L"Forward", forwardCallback, 0x25B6);
toolbar->AddSeparator();
toolbar->AddButton(L"Refresh", refreshCallback, 0x21BB);

auto dropDown = std::make_unique<UIMenu>();
dropDown->AddItem(L"Option 1", []() {});
toolbar->AddDropDownButton(L"More", std::move(dropDown));
```

### Status Bar

```cpp
#include <DragonUI/Controls/StatusBar.hpp>

auto statusBar = std::make_unique<UIStatusBar>();
statusBar->SetStatusText(L"12 items");
statusBar->SetProgress(0.75f);  // 75% progress
statusBar->ClearProgress();
```

### Menu Items (Advanced)

```cpp
auto item = std::make_unique<UIMenuItem>(L"Save");
item->SetShortcut(L"Ctrl+S");
item->SetIcon(0x1F4BE);     // Unicode glyph
item->SetChecked(true);      // checkable item
item->SetEnabled(false);     // disabled state
item->SetOnAction([]() { /* callback */ });

// With submenu
auto sub = std::make_unique<UIMenu>();
sub->AddItem(L"Sub Item 1");
sub->AddItem(L"Sub Item 2");
item->SetSubmenu(std::move(sub));
```

## Command Integration

Connect menu items to the Command Framework for undo-ready architecture:

```cpp
#include <Command/CommandRegistry.hpp>

// Register a command
registry.Register(std::make_unique<MyCommand>());

// Create menu item that executes the command
fileMenu->AddItem(L"Save", [&registry]() {
    CommandContext ctx{/* ... */};
    registry.Execute(L"save", ctx);
});
```

## Keyboard Navigation

| Key | Action |
|-----|--------|
| `Alt` | Activate menu bar |
| `Left/Right` | Switch between menus |
| `Up/Down` | Navigate menu items |
| `Enter` | Activate selected item |
| `Escape` | Close menu |
| `Alt+F` → `N` | Access key navigation |

## Theme Integration

Menus automatically adapt to light/dark themes via these `SemanticColor` tokens:

| Token | Usage |
|-------|-------|
| `MenuBarBackground` | Menu bar background fill |
| `MenuBackground` | Dropdown/popup background |
| `MenuItemHover` | Hover highlight |
| `MenuItemSelected` | Checked/selected indicator |
| `MenuSeparator` | Separator line colour |
| `MenuBarText` | Menu bar item text |
| `MenuItemText` | Dropdown item text |
| `MenuItemTextDisabled` | Disabled item text |
| `MenuIconGray` | Icon tint colour |
| `ToolbarBackground` | Toolbar background fill |
| `ToolbarButtonHover` | Button hover state |
| `ToolbarButtonPressed` | Button pressed state |
| `StatusBarBackground` | Status bar background |
| `StatusBarText` | Status bar text |
| `StatusBarProgress` | Progress bar fill |
| `StatusBarProgressTrack` | Progress bar track |

Set these in `Theme::CreateDarkPalette()` and a future Light theme for full theme support.

## Best Practices

1. **Always use semantic colors** — never hardcode RGB values.
2. **Use smart pointers** — `std::unique_ptr` for ownership, raw pointers for observers.
3. **RAII** — controls clean up automatically.
4. **Command IDs** — for undo support, use `CommandRegistry` with unique command IDs.
5. **Separator usage** — use `AddSeparator()` to group related items (max 1 per group).
6. **Access keys** — prefix with `&` for Alt+Key navigation (future).
7. **Submenu depth** — keep nesting to ≤2 levels for usability.
8. **Toolbar sizing** — use `SetIconSize()` and `SetButtonSpacing()` for flexible layouts.
9. **Status bar panels** — use `SetLeftPanelContent()` / `SetRightPanelContent()` for custom content.
10. **Overflow** — for long toolbars, consider wrapping in `UIScrollViewer`.
