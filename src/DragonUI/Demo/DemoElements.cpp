#include <DragonUI/Demo/DemoElements.hpp>
#include <DragonUI/Controls/Label.hpp>
#include <DragonUI/Controls/Button.hpp>
#include <DragonUI/Controls/Separator.hpp>
#include <DragonUI/Controls/ProgressBar.hpp>
#include <DragonUI/Validation/Validation.hpp>
#include <algorithm>
#include <memory>

namespace DragonOS::DragonUI::Demo {

// ── DemoPanel ────────────────────────────────────────────────────────

DemoPanel::DemoPanel(const Thickness& windowMargin) noexcept
    : m_windowMargin(windowMargin)
{
}

void DemoPanel::Arrange(const LayoutSlot& finalSlot) noexcept
{
    auto inset = finalSlot.Inset(m_windowMargin);
    Element::Arrange(inset);
    auto content = GetContentSlot();
    ArrangeChildren(content);
}

void DemoPanel::Render(RenderContext& ctx) noexcept
{
    auto slot = GetBounds();
    auto d2d = static_cast<D2D1_RECT_F>(slot);
    ctx.FillRectangle(d2d, Theme::SemanticColor::WindowBackground);
    ctx.DrawRectangle(d2d, Theme::SemanticColor::WindowBorder, 1.0f);
    Container::Render(ctx);
}

void DemoPanel::ArrangeChildren(const LayoutSlot& finalSlot) noexcept
{
    float y = finalSlot.y;
    for (const auto& child : m_children)
    {
        if (child->GetVisibility() != Visibility::Visible) continue;

        auto ds = child->GetDesiredSize();
        auto margin = child->GetMargin();
        float childW = (std::min)(ds.width, finalSlot.width);
        float childH = (std::min)(ds.height, finalSlot.height);

        float xOff = finalSlot.x + margin.left;
        float yOff = y + margin.top;
        float w = childW - margin.Horizontal();
        float h = childH - margin.Vertical();
        if (w < 0) w = 0;
        if (h < 0) h = 0;

        LayoutSlot childSlot{xOff, yOff, w, h};
        child->Arrange(childSlot);
        y = yOff + h + margin.bottom + 4;
    }
}

// ── DemoText ─────────────────────────────────────────────────────────

DemoText::DemoText(std::wstring_view text) noexcept
    : m_text(text)
{
}

void DemoText::SetText(std::wstring_view text) noexcept
{
    m_text = text;
    InvalidateLayout();
}

DesiredSize DemoText::MeasureOverride(const LayoutSlot& available) noexcept
{
    float lineHeight = 18.0f;
    float textWidth = static_cast<float>(m_text.size()) * 8.0f;
    return {
        (std::min)(textWidth, available.width),
        lineHeight
    };
}

void DemoText::Render(RenderContext& ctx) noexcept
{
    Element::Render(ctx);
    if (m_text.empty()) return;

    auto slot = GetBounds();
    auto d2d = static_cast<D2D1_RECT_F>(slot);
    ctx.DrawText(m_text, d2d, Theme::SemanticColor::TextPrimary);
}

// ── DemoButton ───────────────────────────────────────────────────────

DemoButton::DemoButton(std::wstring_view text) noexcept
    : m_text(text)
{
    m_focusable = true;
    SetMinSize(80.0f, 32.0f);
}

void DemoButton::SetText(std::wstring_view text) noexcept
{
    m_text = text;
    InvalidateLayout();
}

DesiredSize DemoButton::MeasureOverride(const LayoutSlot& available) noexcept
{
    float textW = static_cast<float>(m_text.size()) * 8.0f + 16.0f;
    return {(std::min)(textW, available.width), 32.0f};
}

void DemoButton::Render(RenderContext& ctx) noexcept
{
    Control::Render(ctx);

    auto slot = GetBounds();
    auto d2d = static_cast<D2D1_RECT_F>(slot);

    Theme::SemanticColor bg = Theme::SemanticColor::Accent;
    Theme::SemanticColor fg = Theme::SemanticColor::TextPrimary;
    Theme::SemanticColor border = Theme::SemanticColor::WindowBorder;

    switch (GetControlState())
    {
    case ControlState::Hover:
        bg = Theme::SemanticColor::AccentHover;
        break;
    case ControlState::Pressed:
        bg = Theme::SemanticColor::AccentPressed;
        break;
    case ControlState::Focused:
        bg = Theme::SemanticColor::AccentHover;
        border = Theme::SemanticColor::Accent;
        break;
    default:
        break;
    }

    ctx.FillRectangle(d2d, bg);
    ctx.DrawRectangle(d2d, border);

    if (GetControlState() == ControlState::Focused)
        ctx.DrawRectangle(d2d, Theme::SemanticColor::Accent, 2.0f);

    if (!m_text.empty())
    {
        D2D1_RECT_F textRect = {
            d2d.left + 8, d2d.top + 6,
            d2d.right - 8, d2d.bottom - 6
        };
        ctx.DrawText(m_text, textRect, fg);
    }
}

bool DemoButton::OnMouseEvent(EventType /*type*/, const MouseEventArgs& /*args*/) noexcept
{
    return true;
}

// ── InputControlsDemo ────────────────────────────────────────────────

std::unique_ptr<Element> InputControlsDemo::Create() noexcept
{
    auto panel = std::make_unique<DemoPanel>(Thickness(40, 50, 40, 40));
    panel->SetPadding(Thickness(16));

    // Title
    auto title = std::make_unique<UILabel>(L"DragonUI Input Controls Demo");
    title->SetTextAlignment(Alignment::Center);
    title->SetMinSize(0, 28);
    panel->AddChild(std::move(title));

    auto sep1 = std::make_unique<UISeparator>();
    sep1->SetMargin(Thickness(0, 4, 0, 4));
    panel->AddChild(std::move(sep1));

    // ── TextBox ───────────────────────────────────────────────────────
    auto tbLabel = std::make_unique<UILabel>(L"TextBox:");
    tbLabel->SetMargin(Thickness(0, 4, 0, 0));
    panel->AddChild(std::move(tbLabel));

    auto textBox = std::make_unique<UITextBox>(L"Type something...");
    textBox->SetMargin(Thickness(0, 2, 0, 0));
    textBox->SetOnTextChanged([](UITextBox& tb) {
        (void)tb;
    });
    panel->AddChild(std::move(textBox));

    // ── PasswordBox ───────────────────────────────────────────────────
    auto pwLabel = std::make_unique<UILabel>(L"PasswordBox:");
    pwLabel->SetMargin(Thickness(0, 8, 0, 0));
    panel->AddChild(std::move(pwLabel));

    auto pwBox = std::make_unique<UIPasswordBox>(L"Enter password");
    pwBox->SetMargin(Thickness(0, 2, 0, 0));
    pwBox->SetPasswordStrengthCallback([](const std::wstring& pwd) -> int {
        if (pwd.empty()) return 0;
        if (pwd.size() < 4) return 1;
        if (pwd.size() < 8) return 2;
        bool hasUpper = false, hasDigit = false, hasSpecial = false;
        for (auto ch : pwd) {
            if (ch >= L'A' && ch <= L'Z') hasUpper = true;
            else if (ch >= L'0' && ch <= L'9') hasDigit = true;
            else if (ch < L'A' || ch > L'z') hasSpecial = true;
        }
        int s = 2;
        if (pwd.size() >= 8) ++s;
        if (hasUpper) ++s;
        if (hasDigit) ++s;
        if (hasSpecial) ++s;
        return std::min(s, 5);
    });
    panel->AddChild(std::move(pwBox));

    // ── CheckBoxes ────────────────────────────────────────────────────
    auto cbLabel = std::make_unique<UILabel>(L"CheckBoxes:");
    cbLabel->SetMargin(Thickness(0, 8, 0, 0));
    panel->AddChild(std::move(cbLabel));

    auto cb1 = std::make_unique<UICheckBox>(L"Option A");
    cb1->SetMargin(Thickness(0, 2, 0, 0));
    panel->AddChild(std::move(cb1));

    auto cb2 = std::make_unique<UICheckBox>(L"Option B");
    cb2->SetMargin(Thickness(0, 2, 0, 0));
    cb2->SetChecked(true);
    panel->AddChild(std::move(cb2));

    auto cbIndeterminate = std::make_unique<UICheckBox>(L"Mixed");
    cbIndeterminate->SetCheckState(CheckState::Indeterminate);
    cbIndeterminate->SetMargin(Thickness(0, 2, 0, 0));
    panel->AddChild(std::move(cbIndeterminate));

    // ── RadioButtons ──────────────────────────────────────────────────
    auto rbLabel = std::make_unique<UILabel>(L"RadioButtons:");
    rbLabel->SetMargin(Thickness(0, 8, 0, 0));
    panel->AddChild(std::move(rbLabel));

    static auto s_radioGroup = std::make_shared<RadioGroup>();

    auto rb1 = std::make_unique<UIRadioButton>(L"Choice 1");
    rb1->SetMargin(Thickness(0, 2, 0, 0));
    rb1->SetGroup(s_radioGroup.get());
    panel->AddChild(std::move(rb1));

    auto rb2 = std::make_unique<UIRadioButton>(L"Choice 2");
    rb2->SetMargin(Thickness(0, 2, 0, 0));
    rb2->SetGroup(s_radioGroup.get());
    panel->AddChild(std::move(rb2));

    auto rb3 = std::make_unique<UIRadioButton>(L"Choice 3");
    rb3->SetMargin(Thickness(0, 2, 0, 0));
    rb3->SetGroup(s_radioGroup.get());
    rb3->SetChecked(true);
    panel->AddChild(std::move(rb3));

    // ── ToggleSwitches ────────────────────────────────────────────────
    auto tsLabel = std::make_unique<UILabel>(L"ToggleSwitches:");
    tsLabel->SetMargin(Thickness(0, 8, 0, 0));
    panel->AddChild(std::move(tsLabel));

    auto ts1 = std::make_unique<UIToggleSwitch>(L"Wi-Fi");
    ts1->SetToggled(true);
    ts1->SetMargin(Thickness(0, 2, 0, 0));
    ts1->SetOnToggled([](UIToggleSwitch& ts, bool val) {
        (void)ts;
        (void)val;
    });
    panel->AddChild(std::move(ts1));

    auto ts2 = std::make_unique<UIToggleSwitch>(L"Bluetooth");
    ts2->SetMargin(Thickness(0, 2, 0, 0));
    panel->AddChild(std::move(ts2));

    // ── Validation ────────────────────────────────────────────────────
    auto sep2 = std::make_unique<UISeparator>();
    sep2->SetMargin(Thickness(0, 8, 0, 4));
    panel->AddChild(std::move(sep2));

    auto valLabel = std::make_unique<UILabel>(L"Validation demo (TextBox with required + min length):");
    valLabel->SetMinSize(0, 18);
    panel->AddChild(std::move(valLabel));

    auto valTb = std::make_unique<UITextBox>(L"Type a value...");
    valTb->SetMargin(Thickness(0, 2, 0, 0));
    auto composite = std::make_shared<CompositeValidator>();
    composite->AddValidator(std::make_shared<RequiredValidator>());
    composite->AddValidator(std::make_shared<MinLengthValidator>(3, L"Minimum 3 characters."));
    valTb->SetValidator(composite);

    auto valStatus = std::make_unique<UILabel>(L"Validation: Pending");
    valStatus->SetTextColor(Theme::SemanticColor::TextSecondary);
    valStatus->SetMinSize(0, 16);
    valStatus->SetMargin(Thickness(0, 2, 0, 0));

    valTb->SetOnTextChanged([status = valStatus.get()](UITextBox& tb) {
        auto result = tb.Validate();
        (void)tb;
        if (status)
        {
            if (result.state == ValidationState::Valid)
                status->SetText(L"Validation: Valid");
            else
                status->SetText(L"Validation: " + result.errorMessage);
        }
    });

    panel->AddChild(std::move(valTb));
    panel->AddChild(std::move(valStatus));

    // ── Theme Switch ──────────────────────────────────────────────────
    auto sep3 = std::make_unique<UISeparator>();
    sep3->SetMargin(Thickness(0, 8, 0, 4));
    panel->AddChild(std::move(sep3));

    auto themeLabel = std::make_unique<UILabel>(L"Hover / Focus / Disabled states:");
    themeLabel->SetMinSize(0, 18);
    panel->AddChild(std::move(themeLabel));

    auto disabledTb = std::make_unique<UITextBox>(L"Disabled");
    disabledTb->SetEnabled(false);
    disabledTb->SetMargin(Thickness(0, 2, 0, 0));
    panel->AddChild(std::move(disabledTb));

    auto disabledCb = std::make_unique<UICheckBox>(L"Disabled checkbox");
    disabledCb->SetEnabled(false);
    disabledCb->SetChecked(true);
    disabledCb->SetMargin(Thickness(0, 2, 0, 0));
    panel->AddChild(std::move(disabledCb));

    auto disabledTs = std::make_unique<UIToggleSwitch>(L"Disabled toggle");
    disabledTs->SetEnabled(false);
    disabledTs->SetToggled(true);
    disabledTs->SetMargin(Thickness(0, 2, 0, 0));
    panel->AddChild(std::move(disabledTs));

    return panel;
}

} // namespace
