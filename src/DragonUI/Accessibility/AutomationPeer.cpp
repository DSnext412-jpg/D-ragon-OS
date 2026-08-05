#include <DragonUI/Accessibility/AutomationPeer.hpp>

#include <DragonUI/Core/Container.hpp>
#include <DragonUI/Core/Control.hpp>
#include <DragonUI/Core/Element.hpp>
#include <DragonUI/Controls/Button.hpp>
#include <DragonUI/Controls/CheckBox.hpp>
#include <DragonUI/Controls/ContextMenu.hpp>
#include <DragonUI/Controls/GridView.hpp>
#include <DragonUI/Controls/Image.hpp>
#include <DragonUI/Controls/Label.hpp>
#include <DragonUI/Controls/ListView.hpp>
#include <DragonUI/Controls/Menu.hpp>
#include <DragonUI/Controls/MenuBar.hpp>
#include <DragonUI/Controls/MenuItem.hpp>
#include <DragonUI/Controls/PasswordBox.hpp>
#include <DragonUI/Controls/ProgressBar.hpp>
#include <DragonUI/Controls/RadioButton.hpp>
#include <DragonUI/Controls/Separator.hpp>
#include <DragonUI/Controls/StatusBar.hpp>
#include <DragonUI/Controls/TextBox.hpp>
#include <DragonUI/Controls/ToggleSwitch.hpp>
#include <DragonUI/Controls/ToolBar.hpp>
#include <DragonUI/Controls/TreeView.hpp>
#include <DragonUI/Dialogs/UIDialog.hpp>

#include <any>
#include <string>

namespace DragonOS::DragonUI {

AutomationPeer::AutomationPeer(Element* element) noexcept
    : AccessibleElement(element)
{
}

// ── Button ────────────────────────────────────────────────────────────────

ButtonPeer::ButtonPeer(UIButton* element) noexcept
    : AutomationPeer(element)
{
    SetRoleOverride(AccessibilityRole::Button);
    SetPatterns(AutomationPattern::Invoke);
}

AccessibilityState ButtonPeer::ComputeState() const noexcept
{
    AccessibilityState s = AutomationPeer::ComputeState();
    if (auto* btn = dynamic_cast<UIButton*>(m_element))
        if (btn->GetControlState() == ControlState::Pressed)
            s |= AccessibilityState::Pressed;
    return s;
}

// ── Label ────────────────────────────────────────────────────────────────

LabelPeer::LabelPeer(UILabel* element) noexcept
    : AutomationPeer(element)
{
    SetRoleOverride(AccessibilityRole::Label);
    SetPatterns(AutomationPattern::Text);
}

std::wstring_view LabelPeer::ComputeValue() const noexcept
{
    if (auto* label = dynamic_cast<UILabel*>(m_element))
        return label->GetText();
    return {};
}

// ── Text box ─────────────────────────────────────────────────────────────

TextBoxPeer::TextBoxPeer(UITextBox* element) noexcept
    : AutomationPeer(element)
{
    SetRoleOverride(AccessibilityRole::TextBox);
    SetPatterns(AutomationPattern::Value | AutomationPattern::Text);
}

AccessibilityState TextBoxPeer::ComputeState() const noexcept
{
    AccessibilityState s = AutomationPeer::ComputeState();
    if (auto* tb = dynamic_cast<UITextBox*>(m_element))
    {
        if (tb->IsReadOnly())
            s |= AccessibilityState::ReadOnly;
        if (tb->GetValidationState() != ValidationState::Valid)
            s |= AccessibilityState::Invalid;
    }
    return s;
}

std::wstring_view TextBoxPeer::ComputeValue() const noexcept
{
    if (auto* tb = dynamic_cast<UITextBox*>(m_element))
        return tb->GetText();
    return {};
}

// ── Password box ─────────────────────────────────────────────────────────

PasswordBoxPeer::PasswordBoxPeer(UIPasswordBox* element) noexcept
    : AutomationPeer(element)
{
    SetRoleOverride(AccessibilityRole::PasswordBox);
    SetPatterns(AutomationPattern::Value);
}

AccessibilityState PasswordBoxPeer::ComputeState() const noexcept
{
    AccessibilityState s = AutomationPeer::ComputeState();
    s |= AccessibilityState::Protected;
    return s;
}

std::wstring_view PasswordBoxPeer::ComputeValue() const noexcept
{
    // Masked for privacy — never expose the raw secret to automation.
    return {};
}

// ── Check box ────────────────────────────────────────────────────────────

CheckBoxPeer::CheckBoxPeer(UICheckBox* element) noexcept
    : AutomationPeer(element)
{
    SetRoleOverride(AccessibilityRole::CheckBox);
    SetPatterns(AutomationPattern::Toggle | AutomationPattern::Value);
}

AccessibilityState CheckBoxPeer::ComputeState() const noexcept
{
    AccessibilityState s = AutomationPeer::ComputeState();
    if (auto* cb = dynamic_cast<UICheckBox*>(m_element))
    {
        switch (cb->GetCheckState())
        {
        case CheckState::Checked:       s |= AccessibilityState::Checked;    break;
        case CheckState::Indeterminate: s |= AccessibilityState::Indeterminate; break;
        case CheckState::Unchecked:     s |= AccessibilityState::Unchecked;  break;
        }
    }
    return s;
}

std::wstring_view CheckBoxPeer::ComputeValue() const noexcept
{
    if (auto* cb = dynamic_cast<UICheckBox*>(m_element))
    {
        switch (cb->GetCheckState())
        {
        case CheckState::Checked:       return std::wstring_view(L"Checked");
        case CheckState::Indeterminate: return std::wstring_view(L"Indeterminate");
        case CheckState::Unchecked:     return std::wstring_view(L"Unchecked");
        }
    }
    return {};
}

// ── Radio button ─────────────────────────────────────────────────────────

RadioButtonPeer::RadioButtonPeer(UIRadioButton* element) noexcept
    : AutomationPeer(element)
{
    SetRoleOverride(AccessibilityRole::RadioButton);
    SetPatterns(AutomationPattern::Toggle | AutomationPattern::SelectionItem);
}

AccessibilityState RadioButtonPeer::ComputeState() const noexcept
{
    AccessibilityState s = AutomationPeer::ComputeState();
    if (auto* rb = dynamic_cast<UIRadioButton*>(m_element))
        s |= rb->IsChecked() ? AccessibilityState::Checked
                             : AccessibilityState::Unchecked;
    return s;
}

// ── Toggle switch ────────────────────────────────────────────────────────

ToggleSwitchPeer::ToggleSwitchPeer(UIToggleSwitch* element) noexcept
    : AutomationPeer(element)
{
    SetRoleOverride(AccessibilityRole::ToggleSwitch);
    SetPatterns(AutomationPattern::Toggle | AutomationPattern::Value);
}

AccessibilityState ToggleSwitchPeer::ComputeState() const noexcept
{
    AccessibilityState s = AutomationPeer::ComputeState();
    if (auto* sw = dynamic_cast<UIToggleSwitch*>(m_element))
        s |= sw->IsToggled() ? AccessibilityState::Checked
                             : AccessibilityState::Unchecked;
    return s;
}

std::wstring_view ToggleSwitchPeer::ComputeValue() const noexcept
{
    if (auto* sw = dynamic_cast<UIToggleSwitch*>(m_element))
        return sw->IsToggled() ? std::wstring_view(L"On") : std::wstring_view(L"Off");
    return {};
}

// ── Menus ────────────────────────────────────────────────────────────────

MenuPeer::MenuPeer(Element* element) noexcept
    : AutomationPeer(element)
{
    SetPatterns(AutomationPattern::ExpandCollapse);
}

AccessibilityRole MenuPeer::ResolveRole() const noexcept
{
    if (dynamic_cast<UIMenuBar*>(m_element) || dynamic_cast<UIContextMenu*>(m_element))
        return AccessibilityRole::MenuBar;
    return AccessibilityRole::Menu;
}

MenuItemPeer::MenuItemPeer(UIMenuItem* element) noexcept
    : AutomationPeer(element)
{
    SetRoleOverride(AccessibilityRole::MenuItem);
    SetPatterns(AutomationPattern::Invoke | AutomationPattern::ExpandCollapse);
}

AccessibilityState MenuItemPeer::ComputeState() const noexcept
{
    AccessibilityState s = AutomationPeer::ComputeState();
    if (auto* mi = dynamic_cast<UIMenuItem*>(m_element))
    {
        if (mi->IsChecked())
            s |= AccessibilityState::Checked;
        if (mi->HasSubmenu())
            s |= mi->IsHovered() ? AccessibilityState::Expanded
                                 : AccessibilityState::Collapsed;
    }
    return s;
}

std::wstring_view MenuItemPeer::ComputeValue() const noexcept
{
    if (auto* mi = dynamic_cast<UIMenuItem*>(m_element))
        return mi->IsChecked() ? std::wstring_view(L"Checked")
                               : std::wstring_view(L"Unchecked");
    return {};
}

// ── Progress bar ─────────────────────────────────────────────────────────

ProgressBarPeer::ProgressBarPeer(UIProgressBar* element) noexcept
    : AutomationPeer(element)
{
    SetRoleOverride(AccessibilityRole::ProgressBar);
    SetPatterns(AutomationPattern::RangeValue);
}

std::wstring_view ProgressBarPeer::ComputeValue() const noexcept
{
    if (auto* pb = dynamic_cast<UIProgressBar*>(m_element))
    {
        if (pb->IsIndeterminate())
            return std::wstring_view(L"Indeterminate");
        const int pct = static_cast<int>(pb->GetNormalizedValue() * 100.0f + 0.5f);
        m_valueBuffer = std::to_wstring(pct) + L"%";
        return m_valueBuffer;
    }
    return {};
}

// ── List view ────────────────────────────────────────────────────────────

ListViewPeer::ListViewPeer(UIListView* element) noexcept
    : AutomationPeer(element)
{
    SetRoleOverride(AccessibilityRole::List);
    SetPatterns(AutomationPattern::Selection);
}

AccessibilityState ListViewPeer::ComputeState() const noexcept
{
    AccessibilityState s = AutomationPeer::ComputeState();
    if (auto* lv = dynamic_cast<UIListView*>(m_element))
        if (lv->GetSelection().GetSelectedCount() > 0)
            s |= AccessibilityState::Selected;
    return s;
}

namespace {
std::wstring AnyToText(const std::any& value) noexcept
{
    if (const auto* ws = std::any_cast<std::wstring>(&value))
        return *ws;
    if (const auto* s = std::any_cast<std::string>(&value))
        return std::wstring(s->begin(), s->end());
    if (const auto* pc = std::any_cast<const char*>(&value))
        return pc ? std::wstring(*pc, *pc + std::char_traits<char>::length(*pc)) : std::wstring{};
    if (const auto* pw = std::any_cast<const wchar_t*>(&value))
        return pw ? std::wstring(*pw) : std::wstring{};
    if (const auto* pcv = std::any_cast<const wchar_t* const>(&value))
        return pcv ? std::wstring(*pcv) : std::wstring{};
    return {};
}
} // namespace

bool ListViewPeer::BuildVirtualChildren() noexcept
{
    auto* lv = dynamic_cast<UIListView*>(m_element);
    if (!lv)
        return false;

    const int64_t count = lv->GetDisplayCount();
    const int64_t start = lv->GetScrollOffset() > 0.0f
        ? static_cast<int64_t>(lv->GetScrollOffset() / lv->GetItemHeight())
        : 0;
    const int64_t end = (std::min<int64_t>)(count, start + 64);
    for (int64_t i = start; i < end; ++i)
    {
        auto row = AccessibleElement::CreateVirtual();
        row->SetRoleOverride(AccessibilityRole::ListItem);
        row->SetNameOverride(AnyToText(lv->GetItemAt(i)));
        if (lv->GetSelection().IsSelected(i))
            row->SetStateOverride(AccessibilityState::Selected);
        row->SetParent(this);
        AddChild(std::move(row));
    }
    return end > start;
}

// ── Tree view ────────────────────────────────────────────────────────────

TreeViewPeer::TreeViewPeer(UITreeView* element) noexcept
    : AutomationPeer(element)
{
    SetRoleOverride(AccessibilityRole::Tree);
    SetPatterns(AutomationPattern::Selection);
}

AccessibilityState TreeViewPeer::ComputeState() const noexcept
{
    AccessibilityState s = AutomationPeer::ComputeState();
    if (auto* tv = dynamic_cast<UITreeView*>(m_element))
        if (tv->GetSelection().GetSelectedCount() > 0)
            s |= AccessibilityState::Selected;
    return s;
}

bool TreeViewPeer::BuildVirtualChildren() noexcept
{
    auto* tv = dynamic_cast<UITreeView*>(m_element);
    if (!tv)
        return false;

    const int64_t count = tv->GetVisibleNodeCount();
    for (int64_t i = 0; i < count; ++i)
    {
        auto node = AccessibleElement::CreateVirtual();
        node->SetRoleOverride(AccessibilityRole::TreeItem);
        if (UITreeNode* tn = tv->GetVisibleNode(i))
        {
            node->SetNameOverride(tn->GetText());
            node->SetStateOverride(tn->IsExpanded() ? AccessibilityState::Expanded
                                                    : AccessibilityState::Collapsed);
            if (tv->GetSelection().IsSelected(i))
                AddState(AccessibilityState::Selected);
        }
        node->SetParent(this);
        AddChild(std::move(node));
    }
    return count > 0;
}

// ── Grid view ────────────────────────────────────────────────────────────

GridViewPeer::GridViewPeer(UIGridView* element) noexcept
    : AutomationPeer(element)
{
    SetRoleOverride(AccessibilityRole::Grid);
    SetPatterns(AutomationPattern::Selection);
}

AccessibilityState GridViewPeer::ComputeState() const noexcept
{
    AccessibilityState s = AutomationPeer::ComputeState();
    if (auto* gv = dynamic_cast<UIGridView*>(m_element))
        if (gv->GetSelection().GetSelectedCount() > 0)
            s |= AccessibilityState::Selected;
    return s;
}

bool GridViewPeer::BuildVirtualChildren() noexcept
{
    auto* gv = dynamic_cast<UIGridView*>(m_element);
    if (!gv)
        return false;

    const int64_t count = gv->GetRowCount();
    for (int64_t i = 0; i < count && i < 64; ++i)
    {
        auto row = AccessibleElement::CreateVirtual();
        row->SetRoleOverride(AccessibilityRole::ListItem);
        row->SetNameOverride(AnyToText(gv->GetRowItem(i)));
        if (gv->GetSelection().IsSelected(i))
            row->SetStateOverride(AccessibilityState::Selected);
        row->SetParent(this);
        AddChild(std::move(row));
    }
    return count > 0;
}

// ── Dialog ───────────────────────────────────────────────────────────────

DialogPeer::DialogPeer(UIDialog* element) noexcept
    : AutomationPeer(element)
{
    SetPatterns(AutomationPattern::Text);
}

AccessibilityRole DialogPeer::ResolveRole() const noexcept
{
    if (dynamic_cast<UIDialog*>(m_element))
        return AccessibilityRole::Dialog;
    return AccessibilityRole::Pane;
}

AccessibilityState DialogPeer::ComputeState() const noexcept
{
    AccessibilityState s = AutomationPeer::ComputeState();
    if (auto* dlg = dynamic_cast<UIDialog*>(m_element))
        if (dlg->IsModal())
            s |= AccessibilityState::Busy;
    return s;
}

// ── Image / separator / status / toolbar ─────────────────────────────────

ImagePeer::ImagePeer(UIImage* element) noexcept
    : AutomationPeer(element)
{
    SetRoleOverride(AccessibilityRole::Image);
    SetPatterns(AutomationPattern::Text);
}

SeparatorPeer::SeparatorPeer(UISeparator* element) noexcept
    : AutomationPeer(element)
{
    SetRoleOverride(AccessibilityRole::Separator);
    (void)element;
}

StatusBarPeer::StatusBarPeer(UIStatusBar* element) noexcept
    : AutomationPeer(element)
{
    SetRoleOverride(AccessibilityRole::StatusBar);
    SetPatterns(AutomationPattern::Text);
}

ToolBarPeer::ToolBarPeer(UIToolBar* element) noexcept
    : AutomationPeer(element)
{
    SetRoleOverride(AccessibilityRole::ToolBar);
    SetPatterns(AutomationPattern::Selection);
}

// ── Factory ──────────────────────────────────────────────────────────────

std::unique_ptr<AccessibleElement> AutomationPeerFactory(Element* element) noexcept
{
    if (!element)
        return nullptr;

    if (auto* b = dynamic_cast<UIButton*>(element))       return std::make_unique<ButtonPeer>(b);
    if (auto* l = dynamic_cast<UILabel*>(element))        return std::make_unique<LabelPeer>(l);
    if (auto* t = dynamic_cast<UITextBox*>(element))      return std::make_unique<TextBoxPeer>(t);
    if (auto* p = dynamic_cast<UIPasswordBox*>(element))  return std::make_unique<PasswordBoxPeer>(p);
    if (auto* c = dynamic_cast<UICheckBox*>(element))     return std::make_unique<CheckBoxPeer>(c);
    if (auto* r = dynamic_cast<UIRadioButton*>(element))  return std::make_unique<RadioButtonPeer>(r);
    if (auto* s = dynamic_cast<UIToggleSwitch*>(element)) return std::make_unique<ToggleSwitchPeer>(s);
    if (auto* m = dynamic_cast<UIMenuItem*>(element))     return std::make_unique<MenuItemPeer>(m);
    if (dynamic_cast<UIMenu*>(element) || dynamic_cast<UIMenuBar*>(element) ||
        dynamic_cast<UIContextMenu*>(element))            return std::make_unique<MenuPeer>(element);
    if (auto* pb = dynamic_cast<UIProgressBar*>(element)) return std::make_unique<ProgressBarPeer>(pb);
    if (auto* lv = dynamic_cast<UIListView*>(element))    return std::make_unique<ListViewPeer>(lv);
    if (auto* tv = dynamic_cast<UITreeView*>(element))    return std::make_unique<TreeViewPeer>(tv);
    if (auto* gv = dynamic_cast<UIGridView*>(element))    return std::make_unique<GridViewPeer>(gv);
    if (auto* d = dynamic_cast<UIDialog*>(element))       return std::make_unique<DialogPeer>(d);
    if (auto* img = dynamic_cast<UIImage*>(element))      return std::make_unique<ImagePeer>(img);
    if (auto* sep = dynamic_cast<UISeparator*>(element))  return std::make_unique<SeparatorPeer>(sep);
    if (auto* sb = dynamic_cast<UIStatusBar*>(element))   return std::make_unique<StatusBarPeer>(sb);
    if (auto* tb = dynamic_cast<UIToolBar*>(element))     return std::make_unique<ToolBarPeer>(tb);

    // Generic containers / elements fall back to the base peer.
    return std::make_unique<AutomationPeer>(element);
}

AccessibilityRole InferAccessibleRole(Element* element) noexcept
{
    if (!element)
        return AccessibilityRole::Unknown;

    if (dynamic_cast<UIButton*>(element))       return AccessibilityRole::Button;
    if (dynamic_cast<UILabel*>(element))        return AccessibilityRole::Label;
    if (dynamic_cast<UITextBox*>(element))      return AccessibilityRole::TextBox;
    if (dynamic_cast<UIPasswordBox*>(element))  return AccessibilityRole::PasswordBox;
    if (dynamic_cast<UICheckBox*>(element))     return AccessibilityRole::CheckBox;
    if (dynamic_cast<UIRadioButton*>(element))  return AccessibilityRole::RadioButton;
    if (dynamic_cast<UIToggleSwitch*>(element)) return AccessibilityRole::ToggleSwitch;
    if (dynamic_cast<UIMenuItem*>(element))     return AccessibilityRole::MenuItem;
    if (dynamic_cast<UIMenu*>(element))         return AccessibilityRole::Menu;
    if (dynamic_cast<UIMenuBar*>(element))      return AccessibilityRole::MenuBar;
    if (dynamic_cast<UIContextMenu*>(element))  return AccessibilityRole::MenuBar;
    if (dynamic_cast<UITreeView*>(element))     return AccessibilityRole::Tree;
    if (dynamic_cast<UIListView*>(element))     return AccessibilityRole::List;
    if (dynamic_cast<UIGridView*>(element))     return AccessibilityRole::Grid;
    if (dynamic_cast<UIProgressBar*>(element))  return AccessibilityRole::ProgressBar;
    if (dynamic_cast<UIDialog*>(element))       return AccessibilityRole::Dialog;
    if (dynamic_cast<UIImage*>(element))        return AccessibilityRole::Image;
    if (dynamic_cast<UISeparator*>(element))    return AccessibilityRole::Separator;
    if (dynamic_cast<UIStatusBar*>(element))    return AccessibilityRole::StatusBar;
    if (dynamic_cast<UIToolBar*>(element))      return AccessibilityRole::ToolBar;

    if (dynamic_cast<Container*>(element))
        return AccessibilityRole::Pane;
    return AccessibilityRole::Label;
}

} // namespace DragonOS::DragonUI