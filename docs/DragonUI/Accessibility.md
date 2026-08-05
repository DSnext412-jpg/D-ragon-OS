# DragonUI Accessibility & UI Automation

## Architecture

DragonUI Accessibility provides a full UI Automation layer that mirrors the
framework element tree with accessible objects, screen-reader notifications,
keyboard navigation, accessibility preferences and a live inspector. It is
split into four cooperating pieces:

- **Accessible objects** (`IAccessible`, `AccessibleElement`) — one automation
  node per framework element exposing name, description, role, state, value,
  bounds and parent/child links.
- **Automation peers** (`AutomationPeer` + per-control peers) — refine the
  role / state / value for each control type and generate virtual children
  (virtualized list rows, tree nodes, grid rows).
- **Central service** (`AccessibilityManager`) — owns the notification hub,
  the automation tree and the preferences; reports focus / value / structure
  changes, drives logical keyboard navigation and applies accessibility
  preferences (high-contrast theme, text scale).
- **Inspector** (`AccessibilityInspector`) — a developer tool that renders the
  automation tree and exposes live preference toggles.

## Class Diagram

```
IAccessible
    |
    +-- AccessibleElement            (base automation node)
          |
          +-- AutomationPeer         (base control peer)
                +-- ButtonPeer / LabelPeer / TextBoxPeer / PasswordBoxPeer
                +-- CheckBoxPeer / RadioButtonPeer / ToggleSwitchPeer
                +-- MenuPeer / MenuItemPeer / ProgressBarPeer
                +-- ListViewPeer / TreeViewPeer / GridViewPeer / DialogPeer
                +-- ImagePeer / SeparatorPeer / StatusBarPeer / ToolBarPeer

AccessibilityManager                 (central service)
    +-- AccessibilityPreferences      (high contrast, reduced motion, text scale)
    +-- AccessibilityNotificationHub  (screen-reader / automation events)
    +-- UIAAutomationTree             (automation tree + traversal)
```

## Accessible Object Model

### Roles (`AccessibilityRole`)

`AccessibilityRole` mirrors the roles exposed through Windows UI Automation:

`Window`, `Dialog`, `Pane`, `Label`, `Button`, `TextBox`, `PasswordBox`,
`CheckBox`, `RadioButton`, `ToggleSwitch`, `Menu`, `MenuBar`, `MenuItem`,
`Tree`, `TreeItem`, `List`, `ListItem`, `Grid`, `GridCell`, `ComboBox`,
`ProgressBar`, `Slider`, `ScrollBar`, `Separator`, `Image`, `StatusBar`,
`ToolBar`, `ToolTip`, `Hyperlink`, `Group`, `Tab`, `TabItem`, `TitleBar`,
`Header`, `Table`, `TableRow`, `Notifications`.

`ToString(AccessibilityRole)` returns the stable, human-readable role name
used by the inspector and the automation tree.

### States (`AccessibilityState`)

`AccessibilityState` is a bit-flag set composed per control by its peer:

`Enabled`, `Disabled`, `Focusable`, `Focused`, `Selected`,
`SelectedInactive`, `Checked`, `Unchecked`, `Indeterminate`, `Pressed`,
`Expanded`, `Collapsed`, `ReadOnly`, `Invalid`, `OffScreen`, `Protected`,
`Busy`, `Visible`.

Use `HasState(state, AccessibilityState::Focused)` to test individual flags.

### Base element metadata

Every `Element` now carries accessibility metadata that is picked up
automatically by its peer:

```cpp
auto button = std::make_unique<UIButton>(L"Save");
button->SetAccessibleName(L"Save document");
button->SetAccessibleDescription(L"Writes the current document to disk");
button->SetAutomationId(L"file-save");
button->SetAccessKey(L'S');
```

- `SetAccessibleName` / `GetAccessibleName` — the screen-reader label.
- `SetAccessibleDescription` / `GetAccessibleDescription` — extra context.
- `SetAutomationId` / `GetAutomationId` — stable, test-facing identifier.
- `SetAccessKey` / `GetAccessKey` — single-character access key.

When no explicit name is set, the peer falls back to the control's text
content (button text, label text, checkbox caption, etc.).

## Automation Peers

`AutomationPeerFactory(element)` builds the correct peer for a control:

```cpp
auto* peer = AutomationPeerFactory(myButton);
myButton->SetAccessibleName(L"Close");
auto* node  = peer;                       // is an IAccessible
auto  state = peer->GetAccessibleState();
auto  role  = peer->GetAccessibleRole();
```

Peers implement the automation patterns they support
(`AutomationPattern`): `Invoke`, `Value`, `Selection`, `SelectionItem`,
`ExpandCollapse`, `Toggle`, `RangeValue`, `Text`.

`AutomationPeer` implements the virtual extension points declared on
`AccessibleElement`:

- `ResolveRole()` — maps a control to its semantic role.
- `ComputeState()` — composes the control state flags.
- `ComputeValue()` — returns the value text (text box content, toggle
  position, progress percentage, checkbox state...).
- `BuildVirtualChildren()` — generates virtual nodes for virtualized list
  rows, tree nodes and grid rows without materializing framework elements.

## UI Automation Tree

`UIAAutomationTree` mirrors the framework element tree with one automation
node per element:

```cpp
auto& tree = host.GetAccessibility().Tree();

tree.SetRoot(host.GetRootElement());
tree.Rebuild();

AccessibleElement* root = tree.GetRoot();
size_t nodes  = tree.GetNodeCount();   // 0 when empty
size_t depth  = tree.GetDepth();

// Lookups
AccessibleElement* byId    = tree.FindByAutomationId(L"0/2/5");
AccessibleElement* byElem  = tree.FindByElement(someControl);
AccessibleElement* atPoint = tree.FindByPoint(320.0f, 200.0f);

// Traversal
tree.Enumerate([&](AccessibleElement* node, size_t depth) {
    auto role = node->GetAccessibleRole();
});

// Visible subset (for snapshots)
auto visible = tree.CollectVisible();
```

Automation ids use depth-first paths (e.g. `"0/2/5"`), mirroring UIA's
RuntimeId concept. The tree is rebuilt lazily via `Rebuild()` after
structural changes; `CollectVisible()` returns the currently rendered nodes.

## Central Service (`AccessibilityManager`)

Every `WindowHost` owns an `AccessibilityManager`. It is initialised
automatically with the window root, focus manager and theme manager.

### Screen-reader reporting

```cpp
auto& acc = host.GetAccessibility();

acc.ReportFocus(element);              // focus moved
acc.ReportValueChanged(textBox);       // text / value changed
acc.ReportSelectionChanged(listView);  // selection changed
acc.ReportStructuredChange();          // tree structure changed

acc.Announce(L"Download finished");    // live-region announcement
acc.Alert(L"Low disk space");          // interrupt-style alert
```

`Element` exposes virtual `GetAccessibleRole()`, `GetAccessibleState()` and
`GetAccessibleValue()` hooks that the peers use as a starting point when
composing the automation node, so the tree is always derived from the live
framework elements.

### Notifications

`AccessibilityNotificationHub` is exposed as `acc.Notifications()`:

```cpp
auto sub = acc.Notifications().Subscribe(
    [](const AccessibilityEventArgs& args) {
        if (args.type == AccessibilityEvent::FocusChanged) { /* ... */ }
        auto* target = args.element;   // may be null
        auto  name   = args.name;
        auto  value  = args.value;
    });

// Convenience: publish a simple event
acc.Notifications().Notify(AccessibilityEvent::ValueChanged, textBox,
                           L"Name", L"New value");
```

Events are `AccessibilityEvent` values (`FocusChanged`, `ValueChanged`,
`SelectionChanged`, `Show`, `Hide`, `StructuredChanged`, `Alert`, `Invoked`,
`Toggled`, `Expanded`, `Collapsed`, ...). Notifications are RAII — the
subscription auto-unsubscribes when the returned `Subscription` is destroyed.

### Preferences

`AccessibilityPreferences` is exposed as `acc.Preferences()`:

```cpp
auto& prefs = acc.Preferences();

prefs.SetHighContrast(true);        // swaps in the high-contrast theme
prefs.SetReducedMotion(true);       // minimises animations
prefs.SetTextScale(1.25f);          // large fonts (clamped to [1, 2])
prefs.SetScreenReaderActive(true);  // richer notifications
```

The manager listens to preference changes and, when high contrast is enabled,
saves the current theme, applies `Theme::CreateHighContrastTheme()`, and
restores the saved theme when the preference is disabled.

## Keyboard Navigation

`FocusManager` (wired through `WindowHost` and the accessibility manager)
supports:

- **Tab / Shift+Tab** — cycle the tab order.
- **Arrow keys** — logical navigation (Up/Down/Left/Right) via
  `MoveFocusDirection(FocusDirection)`.
- **Enter** — `ActivateFocused()` invokes the focused control.
- **Access keys** — `Alt + letter` focuses the control with that access key
  (`FindByAccessKey`).

`AccessibilityManager::HandleNavigationKey` returns `true` when a key was
consumed for accessibility navigation, so the host's normal key handling
falls back cleanly.

## Accessibility Inspector

Press **Ctrl+Shift+A** in a DragonUI host to toggle the inspector. It shows
the automation tree (role, name, automation id, state, value, bounds), the
current keyboard focus, and live toggles for high contrast, reduced motion
and large fonts so preferences can be validated on the spot.

## SDK Integration

The accessibility service is exposed to plugins through
`dragonos::sdk::IAccessibilityService` (see `sdk/DragonOS/Accessibility.hpp`):

```cpp
#include <DragonOS/DragonOS.hpp>

auto* acc = context.GetAccessibilityService();
if (!acc) { /* not available in this host */ }

acc->SetHighContrast(true);
acc->Announce(L"Settings applied");

uint64_t nodeCount = acc->GetNodeCount();
```

The service wraps the live `AccessibilityManager` behind an
`AccessibilityServiceAdapter` and is registered with the plugin manager
after DragonUI initialises.

## Best Practices

1. **Always set an accessible name** — buttons and inputs get a fallback from
   their text, but labels, images and icon-only buttons should use
   `SetAccessibleName` explicitly.

2. **Use stable automation ids** — ids are used by the tree index and by
   automation tests; keep them stable across rebuilds.

3. **Report changes** — call `ReportValueChanged` / `ReportSelectionChanged`
   whenever control state changes not driven by focus (slider drags,
   background progress updates).

4. **Virtualize lazily** — `BuildVirtualChildren` only runs when the tree is
   rebuilt; keep the virtual child generation bounded (the list peer caps
   virtual rows at 64).

5. **Preference changes are observable** — prefer reacting to the
   `ChangeListener` rather than polling; the manager already handles
   high-contrast theme swaps for you.

6. **Never hardcode colours** — the high-contrast theme and standard themes
   all use semantic tokens; your controls should too.
