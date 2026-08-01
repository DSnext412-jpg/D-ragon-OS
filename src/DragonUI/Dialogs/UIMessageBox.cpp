#include <DragonUI/Dialogs/UIMessageBox.hpp>
#include <DragonUI/Core/RenderContext.hpp>

#include <algorithm>
#include <utility>

namespace DragonOS::DragonUI {

namespace {

/**
 * @brief  Small circular icon badge drawn from a glyph and a semantic
 *         colour.  Used by UIMessageBox to show a coloured icon.
 */
class IconBadge final : public Control {
public:
    IconBadge(wchar_t glyph, Theme::SemanticColor color) noexcept
        : m_glyph(glyph), m_color(color)
    {
        SetMinSize(44.0f, 44.0f);
        SetMaxSize(44.0f, 44.0f);
    }

    DesiredSize MeasureOverride(const LayoutSlot& /*available*/) noexcept override
    {
        return {44.0f, 44.0f};
    }

    void Render(RenderContext& ctx) noexcept override
    {
        Element::Render(ctx);
        auto slot = GetBounds();
        auto d2d = static_cast<D2D1_RECT_F>(slot);
        const float r = (std::min)(slot.width, slot.height) * 0.5f;
        const float cx = d2d.left + slot.width * 0.5f;
        const float cy = d2d.top + slot.height * 0.5f;

        D2D1_RECT_F badge{cx - r, cy - r, cx + r, cy + r};
        ctx.FillRoundedRect(badge, m_color, r, r);

        std::wstring_view glyph(&m_glyph, 1);
        auto ts = ctx.MeasureText(glyph, 100.0f);
        D2D1_RECT_F textRect{
            cx - ts.width * 0.5f,
            cy - ts.height * 0.5f,
            cx + ts.width * 0.5f,
            cy + ts.height * 0.5f};
        ctx.DrawText(glyph, textRect, Theme::SemanticColor::WindowTitle);
    }

private:
    wchar_t m_glyph{};
    Theme::SemanticColor m_color{Theme::SemanticColor::Accent};
};

} // namespace

std::unique_ptr<UIMessageBox> UIMessageBox::Create(
    std::wstring_view title,
    std::wstring_view message,
    MessageBoxButtons buttons,
    MessageBoxIcon icon,
    bool modal) noexcept
{
    auto mb = std::unique_ptr<UIMessageBox>(new UIMessageBox());
    mb->SetTitle(title);
    mb->SetModal(modal);
    mb->SetResizable(false);
    mb->SetCanDrag(true);
    mb->m_message = message;

    // ── Icon + message row ─────────────────────────────────────────────
    auto row = std::make_unique<UIStackPanel>(Orientation::Horizontal);
    row->SetSpacing(16.0f);
    row->SetVerticalAlignment(Alignment::Center);

    if (icon != MessageBoxIcon::None)
    {
        wchar_t glyph = L'?';
        Theme::SemanticColor color = Theme::SemanticColor::Accent;
        switch (icon)
        {
        case MessageBoxIcon::Info:
            glyph = L'i';
            color = Theme::SemanticColor::Accent;
            break;
        case MessageBoxIcon::Warning:
            glyph = L'!';
            color = Theme::SemanticColor::Warning;
            break;
        case MessageBoxIcon::Error:
            glyph = L'X';
            color = Theme::SemanticColor::Error;
            break;
        case MessageBoxIcon::Question:
            glyph = L'?';
            color = Theme::SemanticColor::Accent;
            break;
        default:
            break;
        }
        row->AddChild(std::make_unique<IconBadge>(glyph, color));
    }

    auto msgLabel = std::make_unique<UILabel>(message);
    msgLabel->SetWordWrap(true);
    msgLabel->SetVerticalAlignment(Alignment::Start);
    row->AddChild(std::move(msgLabel));

    mb->AddContent(std::move(row));

    // ── Buttons ────────────────────────────────────────────────────────
    UIButton* defaultBtn = nullptr;
    UIButton* cancelBtn = nullptr;

    switch (buttons)
    {
    case MessageBoxButtons::OK:
        defaultBtn = mb->AddButton(L"OK", DialogResult::OK);
        cancelBtn = defaultBtn;
        break;
    case MessageBoxButtons::OKCancel:
        defaultBtn = mb->AddButton(L"OK", DialogResult::OK);
        cancelBtn = mb->AddButton(L"Cancel", DialogResult::Cancel);
        break;
    case MessageBoxButtons::YesNo:
        defaultBtn = mb->AddButton(L"Yes", DialogResult::Yes);
        cancelBtn = mb->AddButton(L"No", DialogResult::No);
        break;
    case MessageBoxButtons::YesNoCancel:
        defaultBtn = mb->AddButton(L"Yes", DialogResult::Yes);
        mb->AddButton(L"No", DialogResult::No);
        cancelBtn = mb->AddButton(L"Cancel", DialogResult::Cancel);
        break;
    case MessageBoxButtons::RetryCancel:
        defaultBtn = mb->AddButton(L"Retry", DialogResult::Retry);
        cancelBtn = mb->AddButton(L"Cancel", DialogResult::Cancel);
        break;
    case MessageBoxButtons::AbortRetryIgnore:
        mb->AddButton(L"Abort", DialogResult::Abort);
        defaultBtn = mb->AddButton(L"Retry", DialogResult::Retry);
        cancelBtn = mb->AddButton(L"Ignore", DialogResult::Ignore);
        break;
    }

    mb->SetDefaultButton(defaultBtn);
    mb->SetCancelButton(cancelBtn);

    // ── Size to content ────────────────────────────────────────────────
    const float textW = static_cast<float>(message.size()) * 7.5f;
    const float iconW = icon != MessageBoxIcon::None ? 60.0f : 0.0f;
    const float width = (std::clamp)(textW + iconW + 90.0f, 320.0f, 520.0f);
    const float height = message.size() > 120 ? 240.0f : 190.0f;
    mb->SetSize(width, height);

    return mb;
}

} // namespace DragonOS::DragonUI
