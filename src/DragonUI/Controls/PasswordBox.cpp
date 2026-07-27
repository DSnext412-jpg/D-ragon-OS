#include <DragonUI/Controls/PasswordBox.hpp>
#include <DragonUI/Core/RenderContext.hpp>
#include <Windows.h>
#include <algorithm>
#include <cstring>

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

UIPasswordBox::UIPasswordBox(std::wstring_view placeholder) noexcept
    : m_placeholder(placeholder)
{
    m_focusable = true;
    SetMinSize(150.0f, 30.0f);
}

void UIPasswordBox::SetText(std::wstring_view text) noexcept
{
    m_text = text;
    m_cursorPos = std::min(m_cursorPos, m_text.size());
    InvalidateLayout();
    if (m_onTextChanged) m_onTextChanged(*this);
}

void UIPasswordBox::SetPlaceholder(std::wstring_view text) noexcept
{
    m_placeholder = text;
    InvalidateVisual();
}

void UIPasswordBox::SetMaxLength(size_t max) noexcept
{
    m_maxLength = max;
    if (m_text.size() > m_maxLength)
    {
        m_text.resize(m_maxLength);
        m_cursorPos = std::min(m_cursorPos, m_maxLength);
        if (m_onTextChanged) m_onTextChanged(*this);
    }
}

void UIPasswordBox::SetShowPassword(bool show) noexcept
{
    m_showPassword = show;
    InvalidateVisual();
}

int UIPasswordBox::GetPasswordStrength() const noexcept
{
    if (m_strengthCb)
        return m_strengthCb(m_text);
    if (m_text.empty()) return 0;
    if (m_text.size() < 4) return 1;
    if (m_text.size() < 8) return 2;
    bool hasUpper = false, hasDigit = false, hasSpecial = false;
    for (auto ch : m_text)
    {
        if (ch >= L'A' && ch <= L'Z') hasUpper = true;
        else if (ch >= L'0' && ch <= L'9') hasDigit = true;
        else if (ch < L'A' || (ch > L'Z' && ch < L'a') || (ch > L'z' && ch < L'0') || ch > L'9')
            hasSpecial = true;
    }
    int score = 2;
    if (m_text.size() >= 8) ++score;
    if (hasUpper) ++score;
    if (hasDigit) ++score;
    if (hasSpecial) ++score;
    return std::min(score, 5);
}

void UIPasswordBox::SetValidator(std::shared_ptr<Validator> validator) noexcept
{
    m_validator = std::move(validator);
}

ValidationResult UIPasswordBox::Validate() const noexcept
{
    if (m_validator)
        return m_validator->Validate(m_text);
    return {ValidationState::Valid};
}

void UIPasswordBox::ClearSecureMemory() noexcept
{
    if (!m_text.empty())
        std::fill(m_text.begin(), m_text.end(), L'\0');
    m_text.clear();
    m_text.shrink_to_fit();
}

// ── Text editing ──────────────────────────────────────────────────────

void UIPasswordBox::InsertChar(wchar_t ch) noexcept
{
    if (ch < 0x20) return;
    if (m_text.size() >= m_maxLength) return;
    if (m_cursorPos > m_text.size()) m_cursorPos = m_text.size();
    m_text.insert(m_cursorPos, 1, ch);
    ++m_cursorPos;
    InvalidateLayout();
    if (m_onTextChanged) m_onTextChanged(*this);
}

void UIPasswordBox::DeleteChar(bool forward) noexcept
{
    if (m_text.empty()) return;
    if (forward && m_cursorPos < m_text.size())
        m_text.erase(m_cursorPos, 1);
    else if (!forward && m_cursorPos > 0)
    {
        --m_cursorPos;
        m_text.erase(m_cursorPos, 1);
    }
    InvalidateLayout();
    if (m_onTextChanged) m_onTextChanged(*this);
}

void UIPasswordBox::MoveCursorLeft(bool shiftHeld) noexcept
{
    (void)shiftHeld;
    if (m_cursorPos > 0) --m_cursorPos;
    InvalidateVisual();
}

void UIPasswordBox::MoveCursorRight(bool shiftHeld) noexcept
{
    (void)shiftHeld;
    if (m_cursorPos < m_text.size()) ++m_cursorPos;
    InvalidateVisual();
}

float UIPasswordBox::GetCharWidth() const noexcept
{
    return 8.0f;
}

// ── Measure / Render ─────────────────────────────────────────────────

DesiredSize UIPasswordBox::MeasureOverride(const LayoutSlot& available) noexcept
{
    float w = std::max(150.0f, (std::min)(static_cast<float>(m_text.size()) * GetCharWidth() + PaddingH * 2.0f + EyeButtonWidth, available.width));
    return {w, 30.0f};
}

void UIPasswordBox::Render(RenderContext& ctx) noexcept
{
    Control::Render(ctx);

    auto d2d = static_cast<D2D1_RECT_F>(GetBounds());
    bool isFocused = GetControlState() == ControlState::Focused;
    bool isDisabled = GetControlState() == ControlState::Disabled;
    bool isHover = GetControlState() == ControlState::Hover;

    auto bg = isDisabled ? Theme::SemanticColor::Disabled : Theme::SemanticColor::ControlFill;
    ctx.FillRoundedRect(d2d, bg, 4.0f, 4.0f);

    auto border = isDisabled ? Theme::SemanticColor::Disabled
        : isFocused ? Theme::SemanticColor::ControlAccentBorder
        : isHover ? Theme::SemanticColor::ControlBorder
        : Theme::SemanticColor::ControlBorder;
    ctx.DrawRoundedRect(d2d, border, 4.0f, 4.0f, isFocused ? 2.0f : 1.0f);
    if (isFocused)
        ctx.DrawRoundedRect(d2d, Theme::SemanticColor::ControlAccentBorder, 4.0f, 4.0f, 2.0f);

    // Eye button area
    float eyeBtnX = d2d.right - EyeButtonWidth;
    D2D1_RECT_F eyeRect = {eyeBtnX, d2d.top, d2d.right, d2d.bottom};

    D2D1_RECT_F contentRect = {
        d2d.left + PaddingH, d2d.top + PaddingV,
        eyeBtnX - PaddingH, d2d.bottom - PaddingV
    };

    ctx.PushClip(d2d);

    // Display text
    std::wstring displayText = m_showPassword ? m_text : std::wstring(m_text.size(), L'\u25CF');

    D2D1_RECT_F textRect = {
        contentRect.left, contentRect.top,
        contentRect.right, contentRect.bottom
    };

    if (!displayText.empty())
    {
        ctx.DrawText(displayText, textRect,
            isDisabled ? Theme::SemanticColor::Disabled : Theme::SemanticColor::ControlText);
    }
    else if (!m_placeholder.empty() && !isFocused)
    {
        ctx.DrawText(m_placeholder, textRect, Theme::SemanticColor::PlaceholderText);
    }

    // Caret (blinking)
    if (isFocused && !isDisabled)
    {
        m_blinkTimer += GetFrameTime();
        if (m_blinkTimer >= BlinkInterval)
        {
            m_blinkTimer = 0;
            m_caretVisible = !m_caretVisible;
        }

        if (m_caretVisible)
        {
            float caretX = contentRect.left + static_cast<float>(m_cursorPos) * GetCharWidth();
            D2D1_RECT_F caretRect = {caretX, textRect.top, caretX + CaretWidth, textRect.bottom};
            ctx.FillRectangle(caretRect, Theme::SemanticColor::CaretColor);
        }
    }
    else
    {
        m_caretVisible = true;
        m_blinkTimer = 0;
    }

    ctx.PopClip();

    // Draw eye icon
    if (!isDisabled)
    {
        std::wstring eyeGlyph = m_showPassword ? L"\u25C9" : L"\u25CB";
        D2D1_RECT_F eyeTextRect = {
            eyeRect.left + 4, eyeRect.top + 6,
            eyeRect.right - 4, eyeRect.bottom - 6
        };
        ctx.DrawText(eyeGlyph, eyeTextRect, Theme::SemanticColor::ControlText);
    }
}

// ── Events ────────────────────────────────────────────────────────────

bool UIPasswordBox::OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept
{
    if (type == EventType::Click)
    {
        auto d2d = static_cast<D2D1_RECT_F>(GetBounds());
        float eyeBtnX = d2d.right - EyeButtonWidth;
        if (args.x >= eyeBtnX && args.x <= d2d.right)
        {
            m_showPassword = !m_showPassword;
            InvalidateVisual();
            return true;
        }
    }

    if (type == EventType::MouseDown)
    {
        float charW = GetCharWidth();
        float clickX = args.x - GetX() - PaddingH;
        size_t pos = static_cast<size_t>(std::max(0.0f, clickX / charW));
        pos = std::min(pos, m_text.size());
        m_cursorPos = pos;
        InvalidateVisual();
        return true;
    }

    return false;
}

bool UIPasswordBox::OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept
{
    if (type == EventType::TextInput)
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
        DeleteChar(false); return true;
    case Input::KeyCode::Delete:
        DeleteChar(true); return true;
    case Input::KeyCode::Left:
        MoveCursorLeft(args.shift); return true;
    case Input::KeyCode::Right:
        MoveCursorRight(args.shift); return true;
    default:
        return false;
    }
}

bool UIPasswordBox::OnFocusEvent(const FocusEventArgs&) noexcept
{
    m_caretVisible = true;
    m_blinkTimer = 0;
    InvalidateVisual();
    return false;
}

} // namespace
