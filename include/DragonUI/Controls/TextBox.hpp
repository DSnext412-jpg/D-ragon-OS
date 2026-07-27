#pragma once

#include <DragonUI/Core/Control.hpp>
#include <DragonUI/Validation/Validation.hpp>
#include <string>
#include <functional>
#include <vector>
#include <memory>

namespace DragonOS::DragonUI {

class UITextBox final : public Control {
public:
    using TextChangedCallback = std::function<void(UITextBox&)>;

    explicit UITextBox(std::wstring_view placeholder = {}) noexcept;

    void SetText(std::wstring_view text) noexcept;
    [[nodiscard]] const std::wstring& GetText() const noexcept { return m_text; }

    void SetPlaceholder(std::wstring_view text) noexcept;
    [[nodiscard]] const std::wstring& GetPlaceholder() const noexcept { return m_placeholder; }

    void SetReadOnly(bool readOnly) noexcept;
    [[nodiscard]] bool IsReadOnly() const noexcept { return m_readOnly; }

    void SetMaxLength(size_t max) noexcept;
    [[nodiscard]] size_t GetMaxLength() const noexcept { return m_maxLength; }

    void SetPasswordMode(bool pw) noexcept;
    [[nodiscard]] bool IsPasswordMode() const noexcept { return m_passwordMode; }

    void SetOnTextChanged(TextChangedCallback cb) noexcept { m_onTextChanged = std::move(cb); }

    // ── Undo / Redo ─────────────────────────────────────────────────────

    void Undo() noexcept;
    void Redo() noexcept;
    void ClearUndoHistory() noexcept;
    [[nodiscard]] bool CanUndo() const noexcept;
    [[nodiscard]] bool CanRedo() const noexcept;

    // ── Selection ───────────────────────────────────────────────────────

    void SelectAll() noexcept;
    void ClearSelection() noexcept;
    void SetSelection(size_t start, size_t end) noexcept;
    [[nodiscard]] size_t GetSelectionStart() const noexcept { return m_selStart; }
    [[nodiscard]] size_t GetSelectionEnd() const noexcept { return m_selEnd; }
    [[nodiscard]] std::wstring GetSelectedText() const noexcept;

    // ── Cursor ──────────────────────────────────────────────────────────

    void SetCursorPosition(size_t pos) noexcept;
    [[nodiscard]] size_t GetCursorPosition() const noexcept { return m_cursorPos; }

    // ── Validation ──────────────────────────────────────────────────────

    void SetValidator(std::shared_ptr<Validator> validator) noexcept;
    ValidationResult Validate() const noexcept;
    [[nodiscard]] ValidationState GetValidationState() const noexcept { return m_validationState; }
    void SetOnValidation(std::function<void(ValidationResult)> cb) noexcept { m_onValidation = std::move(cb); }

    void SetAsyncValidator(AsyncValidator validator) noexcept { m_asyncValidator = std::move(validator); }

    // ── Accessibility ───────────────────────────────────────────────────

    void SetAccessibleName(std::wstring_view name) noexcept { m_accessibleName = name; }
    [[nodiscard]] const std::wstring& GetAccessibleName() const noexcept { return m_accessibleName; }
    void SetAccessibleDescription(std::wstring_view desc) noexcept { m_accessibleDescription = desc; }
    [[nodiscard]] const std::wstring& GetAccessibleDescription() const noexcept { return m_accessibleDescription; }

    // ── Layout & Rendering ──────────────────────────────────────────────

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void Render(RenderContext& ctx) noexcept override;

    // ── Events ──────────────────────────────────────────────────────────

    bool OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept override;
    bool OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept override;
    bool OnFocusEvent(const FocusEventArgs& args) noexcept override;

private:
    void InsertChar(wchar_t ch) noexcept;
    void DeleteChar(bool forward) noexcept;
    void DeleteSelection() noexcept;
    bool HasSelection() const noexcept;
    void MoveCursorHome() noexcept;
    void MoveCursorEnd() noexcept;
    void MoveCursorLeft(bool shiftHeld = false) noexcept;
    void MoveCursorRight(bool shiftHeld = false) noexcept;
    void UpdateScrollOffset() noexcept;
    void PushUndoState() noexcept;
    float GetTextWidth(std::wstring_view text) const noexcept;
    float GetCharWidth() const noexcept;
    void CopyToClipboard() const noexcept;
    void CutToClipboard() noexcept;
    void PasteFromClipboard() noexcept;
    bool IsClipboardAvailable() const noexcept;

    struct UndoState {
        std::wstring text;
        size_t cursorPos{};
        size_t selStart{};
        size_t selEnd{};
    };

    std::wstring m_text;
    std::wstring m_placeholder;
    size_t m_cursorPos{};
    size_t m_selStart{};
    size_t m_selEnd{};
    float m_scrollOffset{};

    size_t m_maxLength{256};
    bool m_readOnly{};
    bool m_passwordMode{};

    std::wstring m_accessibleName;
    std::wstring m_accessibleDescription;

    TextChangedCallback m_onTextChanged;
    std::shared_ptr<Validator> m_validator;
    ValidationState m_validationState{ValidationState::Valid};
    std::function<void(ValidationResult)> m_onValidation;
    AsyncValidator m_asyncValidator;

    std::vector<UndoState> m_undoStack;
    std::vector<UndoState> m_redoStack;
    static constexpr size_t MaxUndoDepth = 50;

    // Cached layout / blink
    float m_blinkTimer{};
    bool m_caretVisible{true};
    float m_textWidth{};

    static constexpr float PaddingH = 8.0f;
    static constexpr float PaddingV = 4.0f;
    static constexpr float CaretWidth = 1.5f;
    static constexpr float BlinkInterval = 0.53f;
};

} // namespace
