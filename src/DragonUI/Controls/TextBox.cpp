#include <DragonUI/Controls/TextBox.hpp>
#include <DragonUI/Core/RenderContext.hpp>
#include <Windows.h>
#include <algorithm>
#include <cmath>

static float GetFrameTime() noexcept
{
    static LARGE_INTEGER freq = [] {
        LARGE_INTEGER f;
        QueryPerformanceFrequency(&f);
        return f;
    }();
    static LARGE_INTEGER last = [] {
        LARGE_INTEGER l;
        QueryPerformanceCounter(&l);
        return l;
    }();
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    float dt = static_cast<float>(now.QuadPart - last.QuadPart) / static_cast<float>(freq.QuadPart);
    last = now;
    return dt;
}

namespace DragonOS::DragonUI {

UITextBox::UITextBox(std::wstring_view placeholder) noexcept
    : m_placeholder(placeholder)
{
    m_focusable = true;
    SetMinSize(120.0f, 30.0f);
}

void UITextBox::SetText(std::wstring_view text) noexcept
{
    if (m_readOnly) return;
    m_text = text;
    m_cursorPos = static_cast<size_t>(std::min(m_cursorPos, m_text.size()));
    m_selStart = m_selEnd = m_cursorPos;
    InvalidateLayout();
    if (m_onTextChanged) m_onTextChanged(*this);
}

void UITextBox::SetPlaceholder(std::wstring_view text) noexcept
{
    m_placeholder = text;
    InvalidateVisual();
}

void UITextBox::SetReadOnly(bool readOnly) noexcept
{
    m_readOnly = readOnly;
    InvalidateVisual();
}

void UITextBox::SetMaxLength(size_t max) noexcept
{
    m_maxLength = max;
    if (m_text.size() > m_maxLength)
    {
        m_text.resize(m_maxLength);
        m_cursorPos = std::min(m_cursorPos, m_maxLength);
        if (m_onTextChanged) m_onTextChanged(*this);
    }
}

void UITextBox::SetPasswordMode(bool pw) noexcept
{
    m_passwordMode = pw;
    InvalidateVisual();
}

void UITextBox::SetValidator(std::shared_ptr<Validator> validator) noexcept
{
    m_validator = std::move(validator);
    if (m_validator)
        m_validationState = Validate().state;
    InvalidateVisual();
}

ValidationResult UITextBox::Validate() const noexcept
{
    if (m_validator)
        return m_validator->Validate(m_text);
    return {ValidationState::Valid};
}

// ── Selection ─────────────────────────────────────────────────────────

void UITextBox::SelectAll() noexcept
{
    m_selStart = 0;
    m_selEnd = m_text.size();
    m_cursorPos = m_text.size();
    InvalidateVisual();
}

void UITextBox::ClearSelection() noexcept
{
    m_selStart = m_selEnd = m_cursorPos;
    InvalidateVisual();
}

void UITextBox::SetSelection(size_t start, size_t end) noexcept
{
    m_selStart = std::min(start, m_text.size());
    m_selEnd = std::min(end, m_text.size());
    m_cursorPos = m_selEnd;
    InvalidateVisual();
}

std::wstring UITextBox::GetSelectedText() const noexcept
{
    if (!HasSelection()) return {};
    return m_text.substr(m_selStart, m_selEnd - m_selStart);
}

bool UITextBox::HasSelection() const noexcept
{
    return m_selStart != m_selEnd;
}

// ── Cursor ────────────────────────────────────────────────────────────

void UITextBox::SetCursorPosition(size_t pos) noexcept
{
    m_cursorPos = std::min(pos, m_text.size());
    m_selStart = m_selEnd = m_cursorPos;
    UpdateScrollOffset();
    InvalidateVisual();
}

// ── Undo / Redo ───────────────────────────────────────────────────────

void UITextBox::PushUndoState() noexcept
{
    UndoState state{m_text, m_cursorPos, m_selStart, m_selEnd};
    m_undoStack.push_back(std::move(state));
    if (m_undoStack.size() > MaxUndoDepth)
        m_undoStack.erase(m_undoStack.begin());
    m_redoStack.clear();
}

void UITextBox::Undo() noexcept
{
    if (m_undoStack.empty()) return;
    UndoState current{m_text, m_cursorPos, m_selStart, m_selEnd};
    m_redoStack.push_back(std::move(current));
    if (m_redoStack.size() > MaxUndoDepth)
        m_redoStack.erase(m_redoStack.begin());

    auto& state = m_undoStack.back();
    m_text = state.text;
    m_cursorPos = state.cursorPos;
    m_selStart = state.selStart;
    m_selEnd = state.selEnd;
    m_undoStack.pop_back();

    UpdateScrollOffset();
    InvalidateLayout();
    if (m_onTextChanged) m_onTextChanged(*this);
}

void UITextBox::Redo() noexcept
{
    if (m_redoStack.empty()) return;
    UndoState current{m_text, m_cursorPos, m_selStart, m_selEnd};
    m_undoStack.push_back(std::move(current));
    if (m_undoStack.size() > MaxUndoDepth)
        m_undoStack.erase(m_undoStack.begin());

    auto& state = m_redoStack.back();
    m_text = state.text;
    m_cursorPos = state.cursorPos;
    m_selStart = state.selStart;
    m_selEnd = state.selEnd;
    m_redoStack.pop_back();

    UpdateScrollOffset();
    InvalidateLayout();
    if (m_onTextChanged) m_onTextChanged(*this);
}

void UITextBox::ClearUndoHistory() noexcept
{
    m_undoStack.clear();
    m_redoStack.clear();
}

bool UITextBox::CanUndo() const noexcept { return !m_undoStack.empty(); }
bool UITextBox::CanRedo() const noexcept { return !m_redoStack.empty(); }

// ── Text editing ──────────────────────────────────────────────────────

void UITextBox::InsertChar(wchar_t ch) noexcept
{
    if (m_readOnly) return;
    if (ch < 0x20) return;

    PushUndoState();

    if (HasSelection()) DeleteSelection();

    if (m_text.size() >= m_maxLength) return;

    if (m_cursorPos > m_text.size()) m_cursorPos = m_text.size();
    m_text.insert(m_cursorPos, 1, ch);
    ++m_cursorPos;
    m_selStart = m_selEnd = m_cursorPos;

    m_validationState = Validate().state;
    UpdateScrollOffset();
    if (m_asyncValidator && !m_text.empty())
        m_asyncValidator(m_text, [this](ValidationResult result) {
            m_validationState = result.state;
            InvalidateVisual();
            if (m_onValidation) m_onValidation(result);
        });
    InvalidateLayout();
    if (m_onTextChanged) m_onTextChanged(*this);
}

void UITextBox::DeleteChar(bool forward) noexcept
{
    if (m_readOnly) return;

    PushUndoState();

    if (HasSelection())
    {
        DeleteSelection();
    }
    else if (!m_text.empty())
    {
        if (forward && m_cursorPos < m_text.size())
        {
            m_text.erase(m_cursorPos, 1);
        }
        else if (!forward && m_cursorPos > 0)
        {
            --m_cursorPos;
            m_text.erase(m_cursorPos, 1);
        }
    }

    m_selStart = m_selEnd = m_cursorPos;
    m_validationState = Validate().state;
    UpdateScrollOffset();
    InvalidateLayout();
    if (m_onTextChanged) m_onTextChanged(*this);
}

void UITextBox::DeleteSelection() noexcept
{
    if (!HasSelection()) return;
    size_t start = std::min(m_selStart, m_selEnd);
    size_t end = std::max(m_selStart, m_selEnd);
    m_text.erase(start, end - start);
    m_cursorPos = start;
    m_selStart = m_selEnd = start;
}

// ── Cursor movement ───────────────────────────────────────────────────

void UITextBox::MoveCursorHome() noexcept
{
    m_cursorPos = 0;
    m_selStart = m_selEnd = m_cursorPos;
    UpdateScrollOffset();
    InvalidateVisual();
}

void UITextBox::MoveCursorEnd() noexcept
{
    m_cursorPos = m_text.size();
    m_selStart = m_selEnd = m_cursorPos;
    UpdateScrollOffset();
    InvalidateVisual();
}

void UITextBox::MoveCursorLeft(bool shiftHeld) noexcept
{
    if (shiftHeld)
    {
        if (m_cursorPos > 0)
        {
            if (m_selStart == m_selEnd)
                m_selStart = m_cursorPos;
            --m_cursorPos;
            m_selEnd = m_cursorPos;
        }
    }
    else
    {
        if (m_cursorPos > 0) --m_cursorPos;
        ClearSelection();
    }
    UpdateScrollOffset();
    InvalidateVisual();
}

void UITextBox::MoveCursorRight(bool shiftHeld) noexcept
{
    if (shiftHeld)
    {
        if (m_cursorPos < m_text.size())
        {
            if (m_selStart == m_selEnd)
                m_selStart = m_cursorPos;
            ++m_cursorPos;
            m_selEnd = m_cursorPos;
        }
    }
    else
    {
        if (m_cursorPos < m_text.size()) ++m_cursorPos;
        ClearSelection();
    }
    UpdateScrollOffset();
    InvalidateVisual();
}

// ── Scrolling ─────────────────────────────────────────────────────────

void UITextBox::UpdateScrollOffset() noexcept
{
    float charW = GetCharWidth();
    float visibleW = GetWidth() - PaddingH * 2.0f;

    float cursorX = static_cast<float>(m_cursorPos) * charW;

    if (cursorX - m_scrollOffset > visibleW)
        m_scrollOffset = cursorX - visibleW + charW * 2;
    else if (cursorX - m_scrollOffset < 0)
        m_scrollOffset = cursorX - charW * 2;

    if (m_scrollOffset < 0) m_scrollOffset = 0;
    InvalidateVisual();
}

// ── Clipboard ─────────────────────────────────────────────────────────

bool UITextBox::IsClipboardAvailable() const noexcept
{
    return OpenClipboard(nullptr) != FALSE;
}

void UITextBox::CopyToClipboard() const noexcept
{
    if (!HasSelection() || !IsClipboardAvailable()) return;

    auto selected = GetSelectedText();
    if (selected.empty()) { CloseClipboard(); return; }

    EmptyClipboard();
    auto hGlobal = GlobalAlloc(GMEM_MOVEABLE, (selected.size() + 1) * sizeof(wchar_t));
    if (!hGlobal) { CloseClipboard(); return; }

    auto* dst = static_cast<wchar_t*>(GlobalLock(hGlobal));
    if (dst)
    {
        wcscpy_s(dst, selected.size() + 1, selected.c_str());
        GlobalUnlock(hGlobal);
        SetClipboardData(CF_UNICODETEXT, hGlobal);
    }
    CloseClipboard();
}

void UITextBox::CutToClipboard() noexcept
{
    if (m_readOnly || !HasSelection()) return;
    CopyToClipboard();
    PushUndoState();
    DeleteSelection();
    m_selStart = m_selEnd = m_cursorPos;
    UpdateScrollOffset();
    InvalidateLayout();
    if (m_onTextChanged) m_onTextChanged(*this);
}

void UITextBox::PasteFromClipboard() noexcept
{
    if (m_readOnly || !IsClipboardAvailable()) return;

    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (!hData) { CloseClipboard(); return; }

    auto* src = static_cast<const wchar_t*>(GlobalLock(hData));
    if (src)
    {
        PushUndoState();
        if (HasSelection()) DeleteSelection();

        std::wstring pasteText(src);
        size_t available = m_maxLength - m_text.size();
        if (pasteText.size() > available)
            pasteText.resize(available);

        if (!pasteText.empty())
        {
            if (m_cursorPos > m_text.size()) m_cursorPos = m_text.size();
            m_text.insert(m_cursorPos, pasteText);
            m_cursorPos += pasteText.size();
        }

        GlobalUnlock(hData);
    }
    CloseClipboard();

    m_selStart = m_selEnd = m_cursorPos;
    m_validationState = Validate().state;
    UpdateScrollOffset();
    InvalidateLayout();
    if (m_onTextChanged) m_onTextChanged(*this);
}

// ── Measure / Render ─────────────────────────────────────────────────

float UITextBox::GetCharWidth() const noexcept
{
    return 8.0f;
}

float UITextBox::GetTextWidth(std::wstring_view text) const noexcept
{
    return static_cast<float>(text.size()) * GetCharWidth();
}

DesiredSize UITextBox::MeasureOverride(const LayoutSlot& available) noexcept
{
    float h = 30.0f;
    m_textWidth = GetTextWidth(m_passwordMode ? std::wstring(m_text.size(), L'*') : m_text);
    float w = std::max(120.0f, (std::min)(m_textWidth + PaddingH * 2.0f, available.width));
    return {w, h};
}

void UITextBox::Render(RenderContext& ctx) noexcept
{
    Control::Render(ctx);

    auto d2d = static_cast<D2D1_RECT_F>(GetBounds());
    bool isFocused = GetControlState() == ControlState::Focused;
    bool isDisabled = GetControlState() == ControlState::Disabled;
    bool isHover = GetControlState() == ControlState::Hover;

    // Background
    Theme::SemanticColor bg = isDisabled ? Theme::SemanticColor::Disabled
        : m_validationState == ValidationState::Invalid ? Theme::SemanticColor::Error
        : Theme::SemanticColor::ControlFill;

    ctx.FillRoundedRect(d2d, bg, 4.0f, 4.0f);

    // Border
    Theme::SemanticColor border = isDisabled ? Theme::SemanticColor::Disabled
        : isFocused ? Theme::SemanticColor::ControlAccentBorder
        : m_validationState == ValidationState::Invalid ? Theme::SemanticColor::Error
        : isHover ? Theme::SemanticColor::ControlBorder
        : Theme::SemanticColor::ControlBorder;

    ctx.DrawRoundedRect(d2d, border, 4.0f, 4.0f, isFocused ? 2.0f : 1.0f);

    // Focus indicator
    if (isFocused)
        ctx.DrawRoundedRect(d2d, Theme::SemanticColor::ControlAccentBorder, 4.0f, 4.0f, 2.0f);

    // Content area
    D2D1_RECT_F contentRect = {
        d2d.left + PaddingH,
        d2d.top + PaddingV,
        d2d.right - PaddingH,
        d2d.bottom - PaddingV
    };

    // Clip for scrolling
    ctx.PushClip(d2d);

    D2D1_RECT_F textRect = {
        contentRect.left - m_scrollOffset,
        contentRect.top,
        contentRect.right - m_scrollOffset,
        contentRect.bottom
    };

    // Determine display text (password mode masks characters)
    std::wstring displayText = m_passwordMode
        ? std::wstring(m_text.size(), L'\u25CF')
        : m_text;

    // Selection background
    if (HasSelection() && isFocused && !isDisabled)
    {
        float charW = GetCharWidth();
        float selLeft = contentRect.left - m_scrollOffset + static_cast<float>(m_selStart) * charW;
        float selRight = contentRect.left - m_scrollOffset + static_cast<float>(m_selEnd) * charW;

        D2D1_RECT_F selRect = {
            selLeft, textRect.top, selRight, textRect.bottom
        };
        ctx.FillRectangle(selRect, Theme::SemanticColor::Selection);
    }

    // Text or placeholder
    if (!displayText.empty())
    {
        ctx.DrawText(displayText, textRect, isDisabled ? Theme::SemanticColor::Disabled
            : Theme::SemanticColor::ControlText);
    }
    else if (!m_placeholder.empty() && !isFocused)
    {
        ctx.DrawText(m_placeholder, textRect, Theme::SemanticColor::PlaceholderText);
    }

    // Caret (blinking)
    if (isFocused && !isDisabled && !m_readOnly)
    {
        m_blinkTimer += GetFrameTime();
        if (m_blinkTimer >= BlinkInterval)
        {
            m_blinkTimer = 0;
            m_caretVisible = !m_caretVisible;
        }

        if (m_caretVisible)
        {
            float caretX = contentRect.left - m_scrollOffset + static_cast<float>(m_cursorPos) * GetCharWidth();
            D2D1_RECT_F caretRect = {
                caretX, textRect.top,
                caretX + CaretWidth, textRect.bottom
            };
            ctx.FillRectangle(caretRect, Theme::SemanticColor::CaretColor);
        }
    }
    else
    {
        m_caretVisible = true;
        m_blinkTimer = 0;
    }

    ctx.PopClip();
}

// ── Events ────────────────────────────────────────────────────────────

bool UITextBox::OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept
{
    if (type == EventType::MouseDown)
    {
        float charW = GetCharWidth();
        float clickX = args.x - GetX() - PaddingH + m_scrollOffset;
        size_t pos = static_cast<size_t>(std::max(0.0f, clickX / charW));
        pos = std::min(pos, m_text.size());
        SetCursorPosition(pos);
        return true;
    }

    return false;
}

bool UITextBox::OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept
{
    if (type == EventType::TextInput && !m_readOnly)
    {
        if (args.character >= 0x20)
        {
            InsertChar(args.character);
            return true;
        }
        return false;
    }

    if (type != EventType::KeyDown) return false;

    switch (args.key)
    {
    case Input::KeyCode::Back:
        DeleteChar(false);
        return true;

    case Input::KeyCode::Delete:
        DeleteChar(true);
        return true;

    case Input::KeyCode::Home:
        if (args.ctrl) MoveCursorHome();
        else MoveCursorHome();
        return true;

    case Input::KeyCode::End:
        if (args.ctrl) MoveCursorEnd();
        else MoveCursorEnd();
        return true;

    case Input::KeyCode::Left:
        MoveCursorLeft(args.shift);
        return true;

    case Input::KeyCode::Right:
        MoveCursorRight(args.shift);
        return true;

    case Input::KeyCode::A:
        if (args.ctrl) { SelectAll(); return true; }
        return false;

    case Input::KeyCode::C:
        if (args.ctrl) { CopyToClipboard(); return true; }
        return false;

    case Input::KeyCode::X:
        if (args.ctrl) { CutToClipboard(); return true; }
        return false;

    case Input::KeyCode::V:
        if (args.ctrl) { PasteFromClipboard(); return true; }
        return false;

    case Input::KeyCode::Z:
        if (args.ctrl) { Undo(); return true; }
        return false;

    case Input::KeyCode::Y:
        if (args.ctrl) { Redo(); return true; }
        return false;

    default:
        return false;
    }
}

bool UITextBox::OnFocusEvent(const FocusEventArgs& args) noexcept
{
    m_caretVisible = true;
    m_blinkTimer = 0;
    InvalidateVisual();
    return false;
}

} // namespace
