#include <DragonUI/Dialogs/UIProgressDialog.hpp>
#include <DragonUI/Core/RenderContext.hpp>

namespace DragonOS::DragonUI {

UIProgressDialog::UIProgressDialog(
    std::wstring_view title,
    bool modal,
    bool cancellable) noexcept
    : UIDialog(title, modal)
{
    SetResizable(false);
    SetCanDrag(true);
    SetSize(360.0f, 160.0f);

    auto content = std::make_unique<UIStackPanel>(Orientation::Vertical);
    content->SetSpacing(14.0f);

    auto statusLabel = std::make_unique<UILabel>(L"Working...");
    statusLabel->SetWordWrap(false);
    statusLabel->SetTextColor(Theme::SemanticColor::TextPrimary);
    m_statusLabel = statusLabel.get();
    content->AddChild(std::move(statusLabel));

    auto progress = std::make_unique<UIProgressBar>();
    progress->SetRange(0.0f, 100.0f);
    progress->SetValue(0.0f);
    progress->SetMinSize(0.0f, 18.0f);
    m_progress = progress.get();
    content->AddChild(std::move(progress));

    AddContent(std::move(content));

    if (cancellable)
    {
        SetCancelButton(AddButton(L"Cancel", [this](UIButton&) noexcept {
            m_cancelled = true;
        }));
    }
}

void UIProgressDialog::SetProgress(float value) noexcept
{
    if (m_progress)
        m_progress->SetValue((value < 0.0f) ? 0.0f : value);
}

float UIProgressDialog::GetProgress() const noexcept
{
    return m_progress ? m_progress->GetValue() : 0.0f;
}

void UIProgressDialog::SetStatus(std::wstring_view text) noexcept
{
    m_status = text;
    if (m_statusLabel)
        m_statusLabel->SetText(text);
}

void UIProgressDialog::SetIndeterminate(bool indeterminate) noexcept
{
    if (m_progress)
        m_progress->SetIndeterminate(indeterminate);
}

} // namespace DragonOS::DragonUI
