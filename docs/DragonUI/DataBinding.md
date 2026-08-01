# DragonUI Data Binding & MVVM

## Architecture

DragonUI's data-binding stack lets views stay decoupled from their data.
A **view model** exposes its state through `Observable` members and its actions
through `ICommand` members; the **view** (a section or window) creates controls
and connects them to the model with a `BindingManager`. When the model changes,
bound controls update automatically.

The stack is header-only and lives in `DragonOS::DragonUI` under
`include/DragonUI/DataBinding/`. It builds on three existing pieces:

- `Validation/Validation.hpp` — the `Validator` family already used by `UITextBox`.
- `Core/VirtualItemSource.hpp` — the lazy item interface consumed by `UIListView`.
- `Theme/ThemeManager.hpp` — the active theme, now with change notification.

## Class Diagram

```
Data Binding
  |
  +-- Observable<T>            Observable value + change notification (RAII Listener)
  +-- Binding<T>               OneTime / OneWay / TwoWay synchronization
  +-- BindingManager           Owns and disposes a set of bindings
  +-- ICommand / RelayCommand  Command abstraction (Execute / CanExecute)
  +-- ViewModelBase            Base class with SetProperty helper
  +-- ObservableCollection<T>  Observable dynamic list (Add/Remove/Move/Replace/Reset)
  +-- CollectionViewSource<T>  Bridges ObservableCollection<T> to UIListView
  +-- ViewModelValidator       Per-property validation rules -> ValidationErrors
  +-- ValidationErrorTemplate  Drives a UILabel from ValidationErrors
  +-- ThemeBinding             Binds semantic theme colours reactively
```

## Observable<T>

The core notification primitive. Subscribe with `AddListener`, which returns a
RAII `Listener` token that unsubscribes on destruction — bindings never leak
callbacks. `Set` fires unconditionally; `SetIfChanged` fires only on an actual
change and is the safe entry point for two-way bindings (no cycles).

```cpp
auto name = MakeObservable<std::wstring>(L"Dragon");
auto token = name->AddListener([](const std::wstring& old, const std::wstring& value) {
    // re-read name->Get() here
});
name->Set(L"Phoenix");   // listener fires
name->SetIfChanged(L"Phoenix"); // unchanged: no notification
token.Detach();          // or let the token die
```

`Observable` requires shared ownership (use `MakeObservable`) so listeners can
hold a weak reference regardless of destruction order.

## Binding & BindingManager

A `Binding<T>` connects an `Observable<T>` source to a control property through
a getter/setter pair. Modes:

| Mode     | Behavior                                             |
|----------|------------------------------------------------------|
| OneTime  | Push source -> target exactly once at bind time      |
| OneWay   | Push source -> target on every source change         |
| TwoWay   | OneWay, plus target edits written back to the source |

`BindingManager::Bind` creates, stores, initializes and applies the binding, and
returns a `shared_ptr` the caller may keep to wire manual target events. The
target's own change event is routed back through an optional subscribe hook:

```cpp
BindingManager bindings;
bindings.Bind<std::wstring>(
    vm.DisplayName,                              // source observable
    [&] { return box.GetText(); },               // target getter
    [&](const std::wstring& v) { box.SetText(v); }, // target setter
    BindingMode::TwoWay,
    [&](Binding<std::wstring>::TargetChangedCallback onChanged) {
        box.SetOnTextChanged([onChanged](UITextBox& b) { onChanged(b.GetText()); });
    });
```

Two-way writes use `SetIfChanged`, so a target reporting an unchanged value
never causes a notification loop. Destroying the manager unsubscribes every
binding.

## Commands

`RelayCommand` wraps a callable and an optional `CanExecute` guard. The guard's
evaluation is refreshed explicitly with `NotifyCanExecuteChanged` (part of the
`ICommand` interface), which the view can forward to a button's `SetEnabled`:

```cpp
clearBtn->SetOnClick([cmd = vm.ClearEvents](UIButton&) { cmd->Execute(); });
vm.ClearEvents->SetOnCanExecuteChanged([this] {
    m_clearButton->SetEnabled(m_viewModel->ClearEvents->CanExecute());
});
```

## ObservableCollection<T> & CollectionViewSource<T>

`ObservableCollection<T>` is the MVVM counterpart of a `std::vector`. It raises
a `Change` event (Add / Remove / Replace / Move / Reset) for every mutation, and
keeps vector-compatible aliases (`size()`, `empty()`, `clear()`, `operator[]`,
range-for) so existing code can migrate without churn.

`CollectionViewSource<T>::Create(collection)` adapts the collection to the
`VirtualItemSource` interface and refreshes a bound `UIListView` automatically:

```cpp
auto items = MakeObservableCollection<std::wstring>();
auto source = CollectionViewSource<std::wstring>::Create(items);
listView->SetItemSource(source);
source->SetListView(listView.get());
items->Add(L"New item");        // the list view refreshes itself
items->Assign(std::move(newItems)); // bulk replace in one Reset change
```

The list view owns the source it was given, so the raw `SetListView` handle
never dangles.

## Validation

`ViewModelValidator` runs `PropertyValidationRule<T>` (or the string-based
`Validator` family through `MakeValidatorRule`) over observable properties and
writes results into a `ValidationErrors` instance. Rules can auto-revalidate
when their observable changes. The view renders errors with
`ValidationErrorTemplate`, which drives a `UILabel`'s text, visibility and
`SemanticColor::Error` colour:

```cpp
validator.AddRule(MakeValidatorRule(L"Name", std::make_shared<RequiredValidator>()), name);
validator.AddRule(MakeValidationRule<std::wstring>(L"Name",
    [](const std::wstring& v) -> ValidationResult {
        if (v.size() < 3) return {ValidationState::Invalid, L"At least 3 characters."};
        return {ValidationState::Valid};
    }), name);

ValidationErrorTemplate nameError;
nameError.SetLabel(nameErrorLabel);
nameError.SetErrorGetter([&]() {
    auto* msg = validator.GetErrors().GetError(L"Name");
    return msg ? *msg : std::wstring{};
});
validator.SetOnErrorsChanged([&]() { nameError.Refresh(); });
```

The existing `UITextBox::SetValidator`/`Validate()` can be used in parallel so
the field itself shows the error styling.

## Theme Binding

`ThemeManager` now raises theme-change notifications (`AddThemeChangedListener`
with a RAII token, or the single-slot `SetOnThemeChanged`) whenever the active
theme is replaced. `ThemeBinding` turns that into observable colours:

```cpp
ThemeBinding themeBinding(themeManager);
auto accent = themeBinding.BindColor(Theme::SemanticColor::Accent);
accent->SetOnChanged([](const ThemeColor&, const ThemeColor& v) { /* update UI */ });
```

`ThemeBinding` unsubscribes itself on destruction and keeps no reference to the
manager afterwards.

## Integration

- **Settings — "MVVM" tab**: `MvvmDemoSection` (`DragonUI/Demo/MvvmDemo.hpp`)
  hosts a `DemoViewModel` wired with two-way text and toggle bindings, a live
  one-way greeting label, command buttons with `CanExecute`-driven enablement,
  an `ObservableCollection` feeding a `UIListView`, name-field validation with
  an error template, and a theme-bound accent readout. It is driven through a
  `WindowHost` exactly like the Network section.
- **Explorer**: `ExplorerWindow::m_entries` is now an
  `ObservableCollection<FileSystem::FileEntry>` bound to the file list through
  `CollectionViewSource`. `LoadDirectory` replaces the collection with
  `Assign`, which refreshes the `UIListView` automatically (the previous manual
  `Refresh()` call in `UpdateFileListSource` was removed).

## SDK

Everything is exposed through `include/DragonUI/DragonUI.hpp`, which the SDK
header `sdk/DragonOS/DragonUI.hpp` includes. Include
`<DragonOS/DragonOS.hpp>` and use `namespace DragonOS::DragonUI`.

## Notes

- `Observable<T>` is single-owner by design; bind a property from exactly one
  place unless you need multiple listeners, in which case `AddListener` is
  multicast.
- `SetIfChanged` and `ViewModelBase::SetProperty` require `operator==` on the
  observed type.
- Bindings, validators and error templates must be destroyed before the
  controls they reference. Keeping them as members of the same view/section
  makes this automatic.
