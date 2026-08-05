#include <DragonUI/Accessibility/AccessibilityInspector.hpp>
#include <DragonUI/Accessibility/AccessibilityManager.hpp>

#include <DragonUI/Core/Element.hpp>
#include <DragonUI/Controls/Panel.hpp>
#include <DragonUI/Controls/Label.hpp>
#include <DragonUI/Controls/Button.hpp>
#include <DragonUI/Controls/StackPanel.hpp>
#include <DragonUI/Controls/ScrollViewer.hpp>
#include <DragonUI/Controls/Separator.hpp>

#include <string>

namespace DragonOS::DragonUI {

namespace {

std::wstring ToWide(std::string_view view)
{
    return std::wstring(view.begin(), view.end());
}

} // namespace

AccessibilityInspector::AccessibilityInspector(AccessibilityManager& manager) noexcept
    : m_manager(manager)
{
}

AccessibilityInspector::~AccessibilityInspector() noexcept = default;

// ── Helpers ───────────────────────────────────────────────────────────────

std::wstring AccessibilityInspector::FormatState(AccessibilityState state) const noexcept
{
    std::wstring out;
    struct Flag { AccessibilityState bit; const wchar_t* name; };
    static constexpr Flag flags[] = {
        { AccessibilityState::Enabled,            L"Enabled" },
        { AccessibilityState::Disabled,           L"Disabled" },
        { AccessibilityState::Focusable,          L"Focusable" },
        { AccessibilityState::Focused,            L"Focused" },
        { AccessibilityState::Selected,           L"Selected" },
        { AccessibilityState::Checked,            L"Checked" },
        { AccessibilityState::Unchecked,          L"Unchecked" },
        { AccessibilityState::Indeterminate,      L"Indeterminate" },
        { AccessibilityState::Pressed,            L"Pressed" },
        { AccessibilityState::Expanded,           L"Expanded" },
        { AccessibilityState::Collapsed,          L"Collapsed" },
        { AccessibilityState::ReadOnly,           L"ReadOnly" },
        { AccessibilityState::Invalid,            L"Invalid" },
        { AccessibilityState::Protected,          L"Protected" },
        { AccessibilityState::Busy,               L"Busy" },
        { AccessibilityState::Visible,            L"Visible" },
    };
    for (const auto& f : flags)
    {
        if (HasState(state, f.bit))
        {
            if (!out.empty())
                out += L", ";
            out += f.name;
        }
    }
    return out.empty() ? L"None" : out;
}

std::wstring AccessibilityInspector::FormatBounds(const LayoutSlot& bounds) const noexcept
{
    return L"(x" + std::to_wstring(static_cast<int>(bounds.x)) +
           L",y" + std::to_wstring(static_cast<int>(bounds.y)) +
           L" " + std::to_wstring(static_cast<int>(bounds.width)) +
           L"x" + std::to_wstring(static_cast<int>(bounds.height)) + L")";
}

// ── View construction ─────────────────────────────────────────────────────

std::unique_ptr<Element> AccessibilityInspector::CreateView() noexcept
{
    auto panel = std::make_unique<UIPanel>();
    panel->SetBackground(Theme::SemanticColor::WindowBackground);
    panel->SetBorderColor(Theme::SemanticColor::WindowBorder);
    panel->SetBorderThickness(1.0f);
    panel->SetCornerRadius(4.0f);
    panel->SetPadding(10.0f);

    auto stack = std::make_unique<UIStackPanel>(Orientation::Vertical);
    stack->SetSpacing(6.0f);
    stack->SetHorizontalAlignment(Alignment::Stretch);

    auto title = std::make_unique<UILabel>(L"Accessibility Inspector");
    title->SetTextColor(Theme::SemanticColor::WindowTitle);
    stack->AddChild(std::move(title));

    auto statusLabel = std::make_unique<UILabel>(L"Nodes: 0    Depth: 0");
    m_statusLabel = statusLabel.get();
    statusLabel->SetTextColor(Theme::SemanticColor::TextSecondary);
    stack->AddChild(std::move(statusLabel));

    auto focusLabel = std::make_unique<UILabel>(L"Focus: none");
    m_focusLabel = focusLabel.get();
    focusLabel->SetTextColor(Theme::SemanticColor::TextPrimary);
    stack->AddChild(std::move(focusLabel));

    stack->AddChild(std::make_unique<UISeparator>());

    // ── Control row ────────────────────────────────────────────────────
    auto controls = std::make_unique<UIStackPanel>(Orientation::Horizontal);
    controls->SetSpacing(6.0f);

    auto refresh = std::make_unique<UIButton>(L"Refresh");
    refresh->SetAccessibleName(L"Refresh accessibility tree");
    refresh->SetOnClick([this](UIButton&) { Refresh(); });
    controls->AddChild(std::move(refresh));

    auto hc = std::make_unique<UIButton>(L"High Contrast: Off");
    m_hcButton = hc.get();
    hc->SetAccessibleName(L"Toggle high contrast theme");
    hc->SetOnClick([this](UIButton&) {
        m_manager.Preferences().SetHighContrast(!m_manager.Preferences().IsHighContrast());
        Refresh();
    });
    controls->AddChild(std::move(hc));

    auto motion = std::make_unique<UIButton>(L"Reduced Motion: Off");
    m_motionButton = motion.get();
    motion->SetAccessibleName(L"Toggle reduced motion");
    motion->SetOnClick([this](UIButton&) {
        m_manager.Preferences().SetReducedMotion(!m_manager.Preferences().IsReducedMotion());
        Refresh();
    });
    controls->AddChild(std::move(motion));

    auto fonts = std::make_unique<UIButton>(L"Large Fonts: Off");
    m_fontsButton = fonts.get();
    fonts->SetAccessibleName(L"Toggle large fonts");
    fonts->SetOnClick([this](UIButton&) {
        m_manager.Preferences().SetTextScale(
            m_manager.Preferences().IsLargeFonts() ? 1.0f : 1.35f);
        Refresh();
    });
    controls->AddChild(std::move(fonts));

    stack->AddChild(std::move(controls));

    // ── Tree area ──────────────────────────────────────────────────────
    auto scroll = std::make_unique<UIScrollViewer>();
    scroll->SetHorizontalScrollOffset(0.0f);
    scroll->SetHorizontalScrollEnabled(true);

    auto treePanel = std::make_unique<UIStackPanel>(Orientation::Vertical);
    treePanel->SetSpacing(0.0f);
    treePanel->SetHorizontalAlignment(Alignment::Stretch);
    m_treePanel = treePanel.get();
    scroll->AddChild(std::move(treePanel));
    stack->AddChild(std::move(scroll));

    panel->AddChild(std::move(stack));
    m_view = panel.get();
    Refresh();
    return panel;
}

// ── Refresh ───────────────────────────────────────────────────────────────

void AccessibilityInspector::Refresh() noexcept
{
    if (!m_view)
        return;

    if (m_hcButton)
        m_hcButton->SetText(m_manager.Preferences().IsHighContrast()
            ? L"High Contrast: On" : L"High Contrast: Off");
    if (m_motionButton)
        m_motionButton->SetText(m_manager.Preferences().IsReducedMotion()
            ? L"Reduced Motion: On" : L"Reduced Motion: Off");
    if (m_fontsButton)
        m_fontsButton->SetText(m_manager.Preferences().IsLargeFonts()
            ? L"Large Fonts: On" : L"Large Fonts: Off");

    if (m_focusLabel)
    {
        std::wstring text = L"Focus: none";
        if (m_focusedElement)
        {
            const auto* node = m_manager.Tree().FindByElement(m_focusedElement);
            std::wstring name = node ? std::wstring(node->GetAccessibleName()) : std::wstring();
            if (name.empty())
                name = L"<unnamed>";
            text = L"Focus: " + name;
        }
        m_focusLabel->SetText(text);
    }

    RebuildTreeRows();
}

void AccessibilityInspector::RebuildTreeRows() noexcept
{
    if (!m_treePanel)
        return;

    m_treePanel->ClearChildren();
    m_manager.Tree().Rebuild();

    if (m_statusLabel)
        m_statusLabel->SetText(
            L"Nodes: " + std::to_wstring(m_manager.Tree().GetNodeCount()) +
            L"    Depth: " + std::to_wstring(m_manager.Tree().GetDepth()));

    AppendNodeRow(m_manager.Tree().GetRoot(), 0);
}

void AccessibilityInspector::AppendNodeRow(AccessibleElement* node, size_t indent) noexcept
{
    if (!node)
        return;

    std::wstring text(indent, L' ');
    text += L"[" + ToWide(ToString(node->GetAccessibleRole())) + L"]";

    const std::wstring_view name = node->GetAccessibleName();
    if (!name.empty())
        text += L" \"" + std::wstring(name) + L"\"";

    const std::wstring_view value = node->GetAccessibleValue();
    if (!value.empty())
        text += L" = " + std::wstring(value);

    const std::wstring_view automationId = node->GetAutomationId();
    if (!automationId.empty())
        text += L"  (id " + std::wstring(automationId) + L")";

    text += L"  [" + FormatState(node->GetAccessibleState()) + L"]";
    text += L"  " + FormatBounds(node->GetAccessibleBounds());

    auto row = std::make_unique<UILabel>(text);
    row->SetWordWrap(false);
    if (HasState(node->GetAccessibleState(), AccessibilityState::Focused))
        row->SetTextColor(Theme::SemanticColor::Accent);
    else
        row->SetTextColor(Theme::SemanticColor::TextPrimary);
    m_treePanel->AddChild(std::move(row));

    for (size_t i = 0; i < node->GetChildCount(); ++i)
    {
        auto* child = static_cast<AccessibleElement*>(node->GetChildAt(i));
        if (child)
            AppendNodeRow(child, indent + 3);
    }
}

} // namespace DragonOS::DragonUI