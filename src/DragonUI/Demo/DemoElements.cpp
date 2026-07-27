#include <DragonUI/Demo/DemoElements.hpp>
#include <DragonUI/Controls/Label.hpp>
#include <DragonUI/Controls/Button.hpp>
#include <DragonUI/Controls/Separator.hpp>
#include <DragonUI/Controls/ProgressBar.hpp>
#include <DragonUI/Controls/Panel.hpp>
#include <DragonUI/Controls/StackPanel.hpp>
#include <DragonUI/Controls/Grid.hpp>
#include <DragonUI/Controls/DockPanel.hpp>
#include <DragonUI/Controls/WrapPanel.hpp>
#include <DragonUI/Controls/Canvas.hpp>
#include <DragonUI/Controls/ScrollViewer.hpp>
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

// ── Layout Containers Demo ───────────────────────────────────────────

namespace {

class ColorBlock : public Element {
public:
    explicit ColorBlock(Theme::SemanticColor color, std::wstring_view label = {})
        : m_color(color), m_label(label) {}
    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override {
        float w = (std::min)(m_label.empty() ? 60.0f : m_label.size() * 8.0f + 16.0f, available.width);
        float h = (std::min)(m_label.empty() ? 60.0f : 36.0f, available.height);
        return {w, h};
    }
    void Render(RenderContext& ctx) noexcept override {
        auto d2d = static_cast<D2D1_RECT_F>(GetBounds());
        ctx.FillRoundedRect(d2d, m_color, 4, 4);
        if (!m_label.empty())
            ctx.DrawText(m_label, d2d, Theme::SemanticColor::TextPrimary);
    }
private:
    Theme::SemanticColor m_color;
    std::wstring m_label;
};

class SectionHeader : public Element {
public:
    explicit SectionHeader(std::wstring_view text) : m_text(text) {}
    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override {
        float w = (std::min)(m_text.size() * 9.0f, available.width);
        return {w, 24.0f};
    }
    void Render(RenderContext& ctx) noexcept override {
        auto d2d = static_cast<D2D1_RECT_F>(GetBounds());
        ctx.DrawText(m_text, d2d, Theme::SemanticColor::TextPrimary);
    }
private:
    std::wstring m_text;
};

} // anon namespace

std::unique_ptr<Element> LayoutDemo::Create() noexcept
{
    // Root: vertical StackPanel with sections
    auto root = std::make_unique<UIStackPanel>(Orientation::Vertical);
    root->SetPadding(Thickness(16));
    root->SetSpacing(16);

    auto title = std::make_unique<UILabel>(L"DragonUI Layout Containers Demo");
    title->SetTextAlignment(Alignment::Center);
    title->SetMinSize(0, 28);
    root->AddChild(std::move(title));

    // ── 1. StackPanel ─────────────────────────────────────────────────
    {
        auto section = std::make_unique<UIPanel>();
        section->SetPadding(Thickness(12));
        section->SetBackground(Theme::SemanticColor::ControlFill);
        section->SetBorderThickness(1);
        section->SetCornerRadius(4);

        auto header = std::make_unique<SectionHeader>(L"1. StackPanel (Vertical)");
        section->AddChild(std::move(header));

        auto stack = std::make_unique<UIStackPanel>(Orientation::Vertical);
        stack->SetSpacing(4);
        stack->SetMargin(Thickness(0, 8, 0, 0));
        stack->AddChild(std::make_unique<ColorBlock>(Theme::SemanticColor::Accent, L"Item 1"));
        stack->AddChild(std::make_unique<ColorBlock>(Theme::SemanticColor::AccentHover, L"Item 2"));
        stack->AddChild(std::make_unique<ColorBlock>(Theme::SemanticColor::AccentPressed, L"Item 3"));
        section->AddChild(std::move(stack));
        root->AddChild(std::move(section));
    }

    // ── 2. Grid ───────────────────────────────────────────────────────
    {
        auto section = std::make_unique<UIPanel>();
        section->SetPadding(Thickness(12));
        section->SetBackground(Theme::SemanticColor::ControlFill);
        section->SetBorderThickness(1);
        section->SetCornerRadius(4);

        auto header = std::make_unique<SectionHeader>(L"2. Grid (2x2)");
        section->AddChild(std::move(header));

        auto grid = std::make_unique<UIGrid>();
        grid->SetMargin(Thickness(0, 8, 0, 0));
        grid->AddRow(GridLength::Star(1));
        grid->AddRow(GridLength::Star(1));
        grid->AddColumn(GridLength::Star(1));
        grid->AddColumn(GridLength::Star(1));
        grid->SetColumnSpacing(8);
        grid->SetRowSpacing(8);

        auto c1 = std::make_unique<ColorBlock>(Theme::SemanticColor::Selection, L"0,0");
        auto c2 = std::make_unique<ColorBlock>(Theme::SemanticColor::Accent, L"0,1");
        auto c3 = std::make_unique<ColorBlock>(Theme::SemanticColor::AccentHover, L"1,0");
        auto c4 = std::make_unique<ColorBlock>(Theme::SemanticColor::AccentPressed, L"1,1");

        grid->SetChildPosition(*c1, 0, 0);
        grid->SetChildPosition(*c2, 0, 1);
        grid->SetChildPosition(*c3, 1, 0);
        grid->SetChildPosition(*c4, 1, 1);

        grid->AddChild(std::move(c1));
        grid->AddChild(std::move(c2));
        grid->AddChild(std::move(c3));
        grid->AddChild(std::move(c4));

        section->AddChild(std::move(grid));
        root->AddChild(std::move(section));
    }

    // ── 3. DockPanel ──────────────────────────────────────────────────
    {
        auto section = std::make_unique<UIPanel>();
        section->SetPadding(Thickness(12));
        section->SetBackground(Theme::SemanticColor::ControlFill);
        section->SetBorderThickness(1);
        section->SetCornerRadius(4);

        auto header = std::make_unique<SectionHeader>(L"3. DockPanel");
        section->AddChild(std::move(header));

        auto dock = std::make_unique<UIDockPanel>();
        dock->SetMargin(Thickness(0, 8, 0, 0));
        dock->SetMinSize(200, 120);
        dock->SetClipChildren(true);

        auto top = std::make_unique<ColorBlock>(Theme::SemanticColor::Accent, L"Top");
        dock->SetChildDock(*top, Dock::Top);
        auto left = std::make_unique<ColorBlock>(Theme::SemanticColor::AccentHover, L"Left");
        dock->SetChildDock(*left, Dock::Left);
        auto right = std::make_unique<ColorBlock>(Theme::SemanticColor::AccentPressed, L"Right");
        dock->SetChildDock(*right, Dock::Right);
        auto bottom = std::make_unique<ColorBlock>(Theme::SemanticColor::Selection, L"Bottom");
        dock->SetChildDock(*bottom, Dock::Bottom);
        auto fill = std::make_unique<ColorBlock>(Theme::SemanticColor::ControlFill, L"Fill");

        dock->AddChild(std::move(top));
        dock->AddChild(std::move(left));
        dock->AddChild(std::move(right));
        dock->AddChild(std::move(bottom));
        dock->AddChild(std::move(fill));

        section->AddChild(std::move(dock));
        root->AddChild(std::move(section));
    }

    // ── 4. WrapPanel ──────────────────────────────────────────────────
    {
        auto section = std::make_unique<UIPanel>();
        section->SetPadding(Thickness(12));
        section->SetBackground(Theme::SemanticColor::ControlFill);
        section->SetBorderThickness(1);
        section->SetCornerRadius(4);

        auto header = std::make_unique<SectionHeader>(L"4. WrapPanel");
        section->AddChild(std::move(header));

        auto wrap = std::make_unique<UIWrapPanel>(Orientation::Horizontal);
        wrap->SetMargin(Thickness(0, 8, 0, 0));
        wrap->SetHorizontalSpacing(8);
        wrap->SetVerticalSpacing(8);
        wrap->SetItemWidth(70);
        wrap->SetItemHeight(40);

        const Theme::SemanticColor colors[] = {
            Theme::SemanticColor::Accent, Theme::SemanticColor::AccentHover,
            Theme::SemanticColor::AccentPressed, Theme::SemanticColor::Selection,
            Theme::SemanticColor::Hover, Theme::SemanticColor::ControlAccentFill,
            Theme::SemanticColor::Error, Theme::SemanticColor::Warning,
            Theme::SemanticColor::Success, Theme::SemanticColor::NotificationInfo
        };
        for (auto c : colors)
            wrap->AddChild(std::make_unique<ColorBlock>(c));

        section->AddChild(std::move(wrap));
        root->AddChild(std::move(section));
    }

    // ── 5. Canvas ─────────────────────────────────────────────────────
    {
        auto section = std::make_unique<UIPanel>();
        section->SetPadding(Thickness(12));
        section->SetBackground(Theme::SemanticColor::ControlFill);
        section->SetBorderThickness(1);
        section->SetCornerRadius(4);

        auto header = std::make_unique<SectionHeader>(L"5. Canvas (absolute)");
        section->AddChild(std::move(header));

        auto canvas = std::make_unique<UICanvas>();
        canvas->SetMargin(Thickness(0, 8, 0, 0));
        canvas->SetMinSize(200, 100);
        canvas->SetClipChildren(true);

        auto bg = std::make_unique<ColorBlock>(Theme::SemanticColor::ControlBorder, L"");
        canvas->SetChildPosition(*bg, 0, 0);
        canvas->AddChild(std::move(bg));

        auto dot1 = std::make_unique<ColorBlock>(Theme::SemanticColor::Accent, L"A");
        dot1->SetMinSize(30, 30);
        canvas->SetChildPosition(*dot1, 20, 20);
        canvas->AddChild(std::move(dot1));

        auto dot2 = std::make_unique<ColorBlock>(Theme::SemanticColor::AccentHover, L"B");
        dot2->SetMinSize(30, 30);
        canvas->SetChildPosition(*dot2, 80, 40);
        canvas->AddChild(std::move(dot2));

        auto dot3 = std::make_unique<ColorBlock>(Theme::SemanticColor::AccentPressed, L"C");
        dot3->SetMinSize(30, 30);
        canvas->SetChildPosition(*dot3, 140, 20);
        canvas->AddChild(std::move(dot3));

        section->AddChild(std::move(canvas));
        root->AddChild(std::move(section));
    }

    // ── 6. ScrollViewer ───────────────────────────────────────────────
    {
        auto section = std::make_unique<UIPanel>();
        section->SetPadding(Thickness(12));
        section->SetBackground(Theme::SemanticColor::ControlFill);
        section->SetBorderThickness(1);
        section->SetCornerRadius(4);

        auto header = std::make_unique<SectionHeader>(L"6. ScrollViewer");
        section->AddChild(std::move(header));

        auto sv = std::make_unique<UIScrollViewer>();
        sv->SetMargin(Thickness(0, 8, 0, 0));
        sv->SetMinSize(200, 120);

        auto inner = std::make_unique<UIStackPanel>(Orientation::Vertical);
        inner->SetSpacing(4);
        inner->SetPadding(Thickness(8));
        for (int i = 1; i <= 20; ++i)
        {
            wchar_t buf[32];
            swprintf_s(buf, L"Scrollable Item %d", i);
            inner->AddChild(std::make_unique<ColorBlock>(
                i % 2 == 0 ? Theme::SemanticColor::Accent : Theme::SemanticColor::AccentHover,
                buf));
        }
        sv->AddChild(std::move(inner));
        section->AddChild(std::move(sv));
        root->AddChild(std::move(section));
    }

    return root;
}

} // namespace
