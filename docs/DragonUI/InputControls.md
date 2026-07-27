# DragonUI Input Controls

## Architecture

DragonUI Input Controls extend the `DragonOS::DragonUI::Control` base class and integrate with:

- **ThemeManager** — All colours are semantic tokens; no hardcoded values.
- **Input System** — Keyboard, mouse, clipboard, and text input via `InputManager`.
- **Focus Manager** — Tab navigation, focus rectangles, programmatic focus.
- **Layout Engine** — Standard `Element::Measure` / `Arrange` pipeline.
- **Event System** — `EventType::KeyDown`, `TextInput`, `Click`, `GotFocus`, etc.
- **Renderer** — Direct2D via `RenderContext`.

## Class Diagram

```
Element
  |
  +-- Control
        |
        +-- UITextBox        (single-line text input)
        +-- UIPasswordBox     (password input with show/hide)
        +-- UICheckBox        (3-state check box)
        +-- UIRadioButton     (radio button, uses RadioGroup)
        +-- UIToggleSwitch    (animated on/off toggle)
```

Supporting classes:

- `RadioGroup` — Groups `UIRadioButton` instances for exclusive selection.
- `Validator` — Base validation class.
- `RequiredValidator`, `MinLengthValidator`, `MaxLengthValidator`, `RegexValidator` — Concrete validators.
- `CompositeValidator` — Chains multiple validators.
- `ValidationState`, `ValidationResult` — Validation result types.

## Semantic Color Tokens

New tokens added to `ThemePalette`:

| Token | Purpose |
|-------|---------|
| `ControlFill` | Input control background |
| `ControlBorder` | Input control border |
| `ControlText` | Text inside controls |
| `PlaceholderText` | Placeholder / watermark text |
| `CaretColor` | Text cursor |
| `ControlAccentFill` | Active fill (toggle on, checkbox) |
| `ControlAccentBorder` | Active / focused border |

## Usage Examples

### UITextBox

```cpp
#include <DragonOS/DragonOS.hpp>
using namespace DragonOS::DragonUI;

auto textBox = std::make_unique<UITextBox>(L"Enter your name...");
textBox->SetMaxLength(50);
textBox->SetOnTextChanged([](UITextBox& tb) {
    // Handle text change
});

// Validation
auto validator = std::make_shared<CompositeValidator>();
validator->AddValidator(std::make_shared<RequiredValidator>());
validator->AddValidator(std::make_shared<MinLengthValidator>(3));
textBox->SetValidator(validator);

// Undo/Redo
textBox->Undo();
textBox->Redo();

// Clipboard
textBox->SelectAll();
textBox->CopyToClipboard(); // (via Ctrl+C)
textBox->CutToClipboard();  // (via Ctrl+X)
textBox->PasteFromClipboard(); // (via Ctrl+V)

// Read-only mode
textBox->SetReadOnly(true);

// Password mode (characters shown as bullets)
textBox->SetPasswordMode(true);
```

### UIPasswordBox

```cpp
auto pwBox = std::make_unique<UIPasswordBox>(L"Enter password");
pwBox->SetMaxLength(128);

// Password strength callback (returns 0-5)
pwBox->SetPasswordStrengthCallback([](const std::wstring& pwd) -> int {
    if (pwd.empty()) return 0;
    if (pwd.size() < 4) return 1;
    if (pwd.size() < 8) return 2;
    // ... custom strength logic
    return 5;
});

// Show/hide password toggle button (built-in)
pwBox->SetShowPassword(true);
```

### UICheckBox

```cpp
auto cb = std::make_unique<UICheckBox>(L"Enable feature");
cb->SetChecked(true);
cb->SetCheckState(CheckState::Indeterminate);
cb->SetOnCheckedChanged([](UICheckBox& cb, CheckState state) {
    if (state == CheckState::Checked) { /* ... */ }
});
```

### UIRadioButton

```cpp
auto group = std::make_shared<RadioGroup>();

auto rb1 = std::make_unique<UIRadioButton>(L"Option 1");
rb1->SetGroup(group.get());

auto rb2 = std::make_unique<UIRadioButton>(L"Option 2");
rb2->SetGroup(group.get());

group->Select(rb1.get());  // Programmatic selection
```

### UIToggleSwitch

```cpp
auto ts = std::make_unique<UIToggleSwitch>(L"Wi-Fi");
ts->SetToggled(true);
ts->SetOnToggled([](UIToggleSwitch& ts, bool val) {
    // val == true when toggled on
});
```

## Event Flow

### Text Input (UITextBox / UIPasswordBox)

1. User presses a key → `InputManager` receives `WM_CHAR` / `WM_KEYDOWN`.
2. `DragonUISystem::Update` polls `InputManager::GetEvents()`.
3. Events dispatched to `WindowHost::OnKeyDown` / `OnTextInput`.
4. `WindowHost` sends `EventArgs` to the focused control.
5. `UITextBox::OnKeyEvent` handles `TextInput`, `KeyDown` (Backspace, Delete, arrows, etc.).
6. Text is inserted/deleted → `InvalidateLayout()` triggers re-render.

### Mouse Click (CheckBox, RadioButton, ToggleSwitch)

1. User clicks → `InputManager` receives `WM_LBUTTONDOWN` / `WM_LBUTTONUP`.
2. `WindowHost::OnMouseDown` → hit test → focus change.
3. `WindowHost::OnMouseUp` → Click event to control.
4. Control toggles its state and fires callback.

### Focus Navigation

1. User presses Tab → `WindowHost::OnKeyDown` with `KeyCode::Tab`.
2. `FocusManager::FocusNext()` / `FocusPrevious()` cycles through tab order.
3. Old control gets `LostFocus`, new control gets `GotFocus`.
4. Focus rectangle drawn during `Render()`.

## Best Practices

1. **Always use semantic colours** — Never hardcode RGB values. All colours come from `ThemeManager::GetColor(token)`.

2. **Validator ownership** — Validators are `std::shared_ptr`. Share them between controls if they share rules.

3. **Password security** — `UIPasswordBox` stores text in a `std::wstring`. For production, consider secure allocators.

4. **Undo history** — `UITextBox` stores up to 50 undo states. Call `ClearUndoHistory()` to free memory when the dialog closes.

5. **Performance** — Controls only invalidate their own visual/layout state. The `WindowHost` only re-renders dirty regions.

6. **Accessibility** — Every control has `SetAccessibleName` and `SetAccessibleDescription` for future screen reader support.

7. **Keyboard operation** — All interactive controls support keyboard activation (Space / Enter for buttons, tabs for focus navigation).

## SDK Integration

All controls are available through the DragonOS SDK:

```cpp
#include <DragonOS/DragonOS.hpp>

// Controls live in DragonOS::DragonUI namespace
auto box = std::make_unique<DragonOS::DragonUI::UITextBox>();
```

No internal engine headers need to be included by application code.
