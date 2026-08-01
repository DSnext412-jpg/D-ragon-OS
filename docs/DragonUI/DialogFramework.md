# DragonUI Dialog Framework

## Architecture

The dialog framework builds on the DragonUI `Container`/`Control` base classes
and adds a self-contained surface that renders its own chrome (title bar,
border, close button) and manages its own input routing. A dialog works under
two hosting models:

- **WindowHost** — the standard model. Dialogs are pushed onto a
  `DialogManager` owned by the host (`WindowHost::Dialogs()`). The host drives
  update/render and routes input automatically, so the host UI is dimmed and
  blocked while a modal dialog is open.
- **Legacy render loop** — hosts that do not use `WindowHost` (e.g. the
  Explorer window) instantiate their own `DialogManager`, forward mouse/keyboard
  events explicitly, and draw the dialogs in their render pass.

All dialogs share a common base:

```
Element
  |
  +-- Container
        |
        +-- UIDialog                    (chrome, drag, resize, buttons, focus)
              |
              +-- UIMessageBox          (standard buttons + icons)
              +-- FileDialogBase        (shared browser chrome)
              |     +-- UIOpenFileDialog
              |     +-- UISaveFileDialog
              |     +-- UIFolderDialog
              +-- UIColorDialog         (RGB/HEX, preview, theme + recent swatches)
              +-- UIFontDialog          (family, size, bold/italic, live preview)
              +-- UIProgressDialog      (progress bar, status, cancel)
```

Supporting classes:

- `DialogManager` — owns the stack of open dialogs, handles modal input blocking,
  a dimming backdrop, and focus restoration when a modal dialog closes.
- `DialogResult` — the result code produced when a dialog is closed.

## UIDialog

The base class (`include/DragonUI/Dialogs/UIDialog.hpp`) is not `final` so the
specialised dialogs can inherit it.

```cpp
auto dialog = std::make_unique<DragonOS::DragonUI::UIDialog>(L"Settings");
dialog->SetSize(480.0f, 320.0f);
dialog->CenterIn(viewportW, viewportH);
dialog->AddContent(makeSomeControl());            // body content
dialog->AddButton(L"OK", DragonOS::DragonUI::DialogResult::OK);
dialog->AddButton(L"Cancel", DragonOS::DragonUI::DialogResult::Cancel);
dialog->SetOnClosed([](DragonOS::DragonUI::UIDialog&, DragonOS::DragonUI::DialogResult r) {
    /* r tells you which button was pressed */
});
manager.ShowDialog(std::move(dialog), host.GetFocusManager());
```

Key features:

- **Title bar** — `SetTitle(...)`; the dialog can be dragged by the title bar
  (`SetCanDrag`, default on).
- **Resizing** — `SetResizable(true)` enables the eight edge/corner handles;
  `SetMinSize` clamps the lower bound.
- **Close button** — shown by default; hide with `SetCloseButtonVisible(false)`.
- **Buttons** — `AddButton(text, DialogResult)` auto-lays buttons out along the
  bottom-right. `AddButton(text, ClickCallback)` accepts arbitrary actions.
  `SetDefaultButton`/`SetCancelButton` wire Enter/Escape.
- **Viewport** — `SetViewport(w, h)` tells the dialog its enclosing area so
  `CenterIn` and bounds clamping work. `DialogManager::SetViewport` fans this out
  to every open dialog.
- **Synchronous close callback** — `Close(result)` fires `m_onClosed`
  synchronously while the dialog object is still alive, so capturing raw
  pointers in the callback is safe.

`UIDialog` also exposes explicit input routing (`HandleMouseMove/Down/Up/Wheel`,
`HandleKey`, `HandleText`). `DialogManager` calls these for you; a legacy loop
can call them directly.

## DialogManager

Owns the open-dialog stack and mediates input, drawing and focus:

- `ShowDialog(unique_ptr<UIDialog>, FocusManager&)` — pushes the dialog and
  remembers the currently focused host control so it can be restored on close.
- Modal dialogs block all host input and draw a dimming backdrop; modeless
  dialogs pass input through to the host wherever the pointer is not over them.
- `HasModalOpen()` / `IsEmpty()` let a host decide whether to keep processing.
- `Render(ctx, w, h)` draws the backdrop and each dialog from the top down.
- `Update(dt, hostFocus)` advances animations and sweeps closed dialogs.

### WindowHost model

`WindowHost` owns a `DialogManager` and wires it into its input, update and
render passes. Host code only needs:

```cpp
auto box = DragonOS::DragonUI::UIMessageBox::Create(
    L"Delete", L"Delete this file permanently?",
    DragonOS::DragonUI::MessageBoxButtons::YesNo,
    DragonOS::DragonUI::MessageBoxIcon::Question);
box->SetOnClosed([this](DragonOS::DragonUI::UIDialog&, DragonOS::DragonUI::DialogResult r) {
    if (r == DragonOS::DragonUI::DialogResult::Yes) { startDelete(); }
});
Dialogs().ShowDialog(std::move(box), GetFocusManager());
```

### Legacy loop model

Hosts that render their own windows can drive the dialogs manually:

```cpp
// Per frame:
m_dialogs.SetViewport(m_pWindow->GetWidth(), m_pWindow->GetHeight());
m_dialogs.Update(dt, m_focusMgr);
// During input processing, give the dialogs first crack:
if (m_dialogs.HandleMouseDown(x, y, btn)) { return; }   // consumed by a dialog
// During render, after the rest of the UI:
m_dialogs.Render(dctx, viewportW, viewportH);
```

This is how the Explorer window hosts confirmations and the folder picker.

## UIMessageBox

`UIMessageBox::Create(title, message, buttons, icon)` returns a ready-to-show
dialog with a standard button set mapped to `DialogResult` values:

| MessageBoxButtons      | Buttons → DialogResult                       |
|------------------------|----------------------------------------------|
| `OK`                   | OK                                           |
| `OKCancel`             | OK, Cancel                                   |
| `YesNo`                | Yes, No                                      |
| `YesNoCancel`          | Yes, No, Cancel                              |
| `RetryCancel`          | Retry, Cancel                                |
| `AbortRetryIgnore`     | Abort, Retry, Ignore                         |

Icons are `None` / `Info` / `Warning` / `Error` / `Question`. See the code
example under "DialogManager" above.

## File dialogs

`FileDialogBase` (in `FileDialog.hpp`) provides the shared file-browser chrome:
an Up / Refresh / Home toolbar with the current path, a virtualized `UIListView`
of entries (folders first), and a bottom row with a filter selector plus a file
name box. Filters use the classic spec `L"Text Files (*.txt)|*.txt"`.

### UIOpenFileDialog

```cpp
auto dlg = std::make_unique<DragonOS::DragonUI::UIOpenFileDialog>(
    m_fs,                          // FileSystemService&
    L"Open File", L"C:\\", L"Text Files (*.txt)|*.txt", true);
dlg->SetOnClosed([this](DragonOS::DragonUI::UIDialog& d, DragonOS::DragonUI::DialogResult r) {
    if (r != DragonOS::DragonUI::DialogResult::OK) { return; }
    auto& dlg = static_cast<DragonOS::DragonUI::UIOpenFileDialog&>(d);
    const auto& files = dlg.GetSelectedFiles();   // multi-select
});
```

`multiSelect = true` enables multi-select; results are read via
`GetSelectedFile()` / `GetSelectedFiles()`.

### UISaveFileDialog

```cpp
auto dlg = std::make_unique<DragonOS::DragonUI::UISaveFileDialog>(
    m_fs, L"Save As", L"C:\\", L"report.txt", L"Text Files (*.txt)|*.txt");
// On OK: dlg->GetSelectedFile()
```

`defaultName` pre-fills the name box; the selected extension is applied
automatically on commit.

### UIFolderDialog

```cpp
auto dlg = std::make_unique<DragonOS::DragonUI::UIFolderDialog>(m_fs, L"Select Folder", L"C:\\");
// On OK: dlg->GetSelectedFolder()
```

## UIColorDialog

A colour picker with RGB and HEX text fields, a live preview swatch, theme
swatches and a row of recently used colours. The picked colour is available
once the dialog closes with OK.

```cpp
auto dlg = std::make_unique<DragonOS::DragonUI::UIColorDialog>(currentColor, L"Pick Color");
dlg->SetRecentColors(m_recent);
// On OK: dlg->GetSelectedColor() / dlg->GetHexString()
```

## UIFontDialog

A font selector (family list, size box, bold/italic toggles) with a live
preview rendered through DirectWrite.

```cpp
auto dlg = std::make_unique<DragonOS::DragonUI::UIFontDialog>(
    L"Select Font", L"Segoe UI", 14.0f, false, false);
// On OK: dlg->GetSelectedFamily() / GetSelectedSize() / GetSelectedBold() / GetSelectedItalic()
```

## UIProgressDialog

A modeless progress dialog the caller drives from its own update loop:

```cpp
auto dlg = std::make_unique<DragonOS::DragonUI::UIProgressDialog>(L"Copying...", false, true);
m_dialogs.ShowDialog(std::move(dlg), m_focusMgr);
// each frame:
auto* p = static_cast<DragonOS::DragonUI::UIProgressDialog*>(m_dialogs.GetTopDialog());
p->SetProgress(pct);
p->SetStatus(L"Item " + std::to_wstring(n));
if (p->IsCancelled()) { /* stop the work, then Close() */ }
```

`SetIndeterminate(true)` switches to the indeterminate (animated) style for
operations with no known end. The dialog is `modeless` by default so the host
keeps running; pass `modal = true` if the caller wants the host blocked.

## SDK Exposure

The dialog headers are reachable from plugins through the SDK umbrella header
(`sdk/DragonOS/DragonUI.hpp` → `include/DragonUI/DragonUI.hpp`), so plugins that
embed DragonUI in their own windows can use these dialogs directly.

The synchronous `dragonos::sdk::IDialogService` adapter
(`src/SDK/DialogServiceAdapter.cpp`) intentionally keeps its native Win32
implementation (`MessageBoxW`, COMMDLG) because its API is blocking and has no
render target. Applications needing in-process DragonUI dialogs use the
`DialogManager` hosting model above instead.

## Integration Map

- **WindowHost** — `WindowHost::Dialogs()` / `GetFocusManager()`; used by the
  demo shell.
- **Explorer** — own `DialogManager`/`FocusManager` members wired into
  `ProcessInput`, `Update` and `Render`; delete confirmations and the
  "Go To Folder..." picker.
- **Demo** — `DataControlsDemo` shows every dialog type with a live progress
  animation driven from its measure pass.

## Notes

- Dialogs are `final` subclasses of `UIDialog`; `UIDialog` itself is not final.
- `UIButton` / `UIStackPanel` are final — do not attempt to subclass them.
- Because `Close()` fires `m_onClosed` synchronously, capture dialog members by
  reference/pointer inside the callback only if you are certain the dialog stays
  alive (e.g. the `DialogManager` holds it until the callback returns).
