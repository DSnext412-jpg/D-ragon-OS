#pragma once

// DragonOS SDK — DragonUI Framework
//
// This header exposes the official DragonUI controls to SDK consumers.
// Applications include <DragonOS/DragonOS.hpp> which pulls in this header.
//
// All controls live in the DragonOS::DragonUI namespace.
//
// Usage:
//   #include <DragonOS/DragonOS.hpp>
//   using namespace DragonOS::DragonUI;
//
// Available controls:
//   Input Controls:
//     UITextBox        — Single-line text input with clipboard, undo/redo, validation
//     UIPasswordBox    — Password input with show/hide toggle, strength callback
//     UICheckBox       — Three-state check box (checked, unchecked, indeterminate)
//     UIRadioButton    — Radio button for single-selection groups
//     UIToggleSwitch   — Animated on/off toggle with smooth transition
//
//   Menu Controls:
//     UIMenuBar        — Horizontal top-level menu bar with Alt navigation
//     UIMenu           — Dropdown menu with nested submenus, separators, icons
//     UIContextMenu    — Right-click context menu with dismiss on outside click
//     UIMenuItem       — Individual menu item with text, icon, shortcut, check
//
//   Toolbar & StatusBar:
//     UIToolBar        — Toolbar with buttons, separators, drop-down buttons
//     UIStatusBar      — Status bar with text, icons, progress indicator
//
//   Data Presentation Controls:
//     UIListView       — Virtualized list with Details/List/Tile modes, icons,
//                        single & multi selection, sorting, filtering, templates
//     UITreeView       — Nested tree with expand/collapse, lazy loading, icons,
//                        selection and keyboard navigation
//     UIGridView       — Virtualized data grid with columns, resizable headers,
//                        per-column sorting, selection and alternating rows
//
//   Selection & Virtualization:
//     SelectionManager — Reusable selection engine (single/multi/range, Ctrl/Shift)
//     VirtualItemSource— Abstract source for large data sets (tens of thousands)
//     VirtualViewport  — Math viewport; only visible rows are ever rendered
//     ItemTemplate     — Reusable, data-binding-ready item templates
//
//   Data Binding (MVVM):
//     Observable<T>           — Observable value with change notification (RAII listeners)
//     Binding<T> / BindingManager — OneTime / OneWay / TwoWay synchronization
//     ICommand / RelayCommand — Command abstraction for MVVM
//     ViewModelBase           — Base class for view models (SetProperty helper)
//     ObservableCollection<T> — Observable dynamic list with change events
//     CollectionViewSource<T> — Bridges an ObservableCollection to a UIListView
//     ViewModelValidator      — Validation rules over observable properties
//     ValidationErrorTemplate — Renders validation errors in the view
//     ThemeBinding            — Binds semantic theme colours reactively
//
// These controls integrate with ThemeManager, Input System, Focus Manager,
// Layout Engine, Event System, and the Direct2D Renderer.

#include <DragonUI/DragonUI.hpp>
