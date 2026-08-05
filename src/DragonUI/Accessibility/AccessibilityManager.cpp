#include <DragonUI/Accessibility/AccessibilityManager.hpp>

#include <DragonUI/Core/Element.hpp>
#include <DragonUI/Core/Control.hpp>
#include <DragonUI/Core/FocusManager.hpp>
#include <Theme/HighContrast.hpp>

#include <algorithm>

namespace DragonOS::DragonUI {

void AccessibilityManager::Initialize(Element* root, FocusManager* focusMgr,
                                      Theme::ThemeManager* themeMgr) noexcept
{
    if (m_initialized)
        return;

    m_root = root;
    m_focusMgr = focusMgr;
    m_themeMgr = themeMgr;
    m_tree.SetRoot(root);

    m_prefListener = m_preferences.AddListener(
        [this](const AccessibilityPreferences&) { ApplyHighContrast(); });

    m_initialized = true;
}

void AccessibilityManager::Shutdown() noexcept
{
    if (!m_initialized)
        return;

    m_prefListener.Detach();
    m_tree.Clear();
    RestoreTheme();
    m_focusMgr = nullptr;
    m_themeMgr = nullptr;
    m_root = nullptr;
    m_initialized = false;
}

void AccessibilityManager::SetRoot(Element* root) noexcept
{
    m_root = root;
    m_tree.SetRoot(root);
}

// ── Screen-reader reporting ───────────────────────────────────────────────

void AccessibilityManager::ReportFocus(Element* element) noexcept
{
    if (!element)
        return;
    const auto* node = m_tree.FindByElement(element);
    AccessibilityEventArgs args;
    args.type = AccessibilityEvent::FocusChanged;
    args.element = element;
    if (node)
    {
        args.role = node->GetAccessibleRole();
        args.state = node->GetAccessibleState();
        args.name = node->GetAccessibleName();
        args.value = node->GetAccessibleValue();
    }
    m_notifications.Publish(args);
}

void AccessibilityManager::ReportValueChanged(Element* element) noexcept
{
    if (!element)
        return;
    const auto* node = m_tree.FindByElement(element);
    AccessibilityEventArgs args;
    args.type = AccessibilityEvent::ValueChanged;
    args.element = element;
    if (node)
    {
        args.role = node->GetAccessibleRole();
        args.value = node->GetAccessibleValue();
    }
    m_notifications.Publish(args);
}

void AccessibilityManager::ReportSelectionChanged(Element* container) noexcept
{
    m_notifications.Notify(AccessibilityEvent::SelectionChanged, container);
}

void AccessibilityManager::ReportStructuredChange() noexcept
{
    m_tree.Rebuild();
    m_notifications.Notify(AccessibilityEvent::StructuredChanged, m_root);
}

void AccessibilityManager::Announce(std::wstring_view text) noexcept
{
    AccessibilityEventArgs args;
    args.type = AccessibilityEvent::LiveRegionChanged;
    args.element = m_root;
    args.role = AccessibilityRole::Pane;
    args.name = text;
    m_notifications.Publish(args);
}

void AccessibilityManager::Alert(std::wstring_view text) noexcept
{
    AccessibilityEventArgs args;
    args.type = AccessibilityEvent::Alert;
    args.element = m_root;
    args.name = text;
    m_notifications.Publish(args);
}

// ── Keyboard navigation ───────────────────────────────────────────────────

bool AccessibilityManager::HandleNavigationKey(Input::KeyCode key, bool shift, bool alt,
                                               std::wstring_view accessKey) noexcept
{
    if (!m_focusMgr)
        return false;

    switch (key)
    {
    case Input::KeyCode::Tab:
        if (shift)
            m_focusMgr->FocusPrevious();
        else
            m_focusMgr->FocusNext();
        return true;

    case Input::KeyCode::Up:
        m_focusMgr->MoveFocusDirection(FocusDirection::Up);
        return true;
    case Input::KeyCode::Down:
        m_focusMgr->MoveFocusDirection(FocusDirection::Down);
        return true;
    case Input::KeyCode::Left:
        m_focusMgr->MoveFocusDirection(FocusDirection::Left);
        return true;
    case Input::KeyCode::Right:
        m_focusMgr->MoveFocusDirection(FocusDirection::Right);
        return true;

    case Input::KeyCode::Return:
        m_focusMgr->ActivateFocused();
        return true;

    default:
        break;
    }

    // Access key (Alt + letter/digit).
    if (alt && !accessKey.empty() && accessKey.size() == 1)
    {
        if (Control* hit = m_focusMgr->FindByAccessKey(accessKey.front(), m_focusMgr->GetFocused()))
        {
            m_focusMgr->SetFocus(hit);
            return true;
        }
    }
    return false;
}

// ── Accessibility preferences application ─────────────────────────────────

void AccessibilityManager::ApplyPreferences() noexcept
{
    ApplyHighContrast();
}

void AccessibilityManager::ApplyHighContrast() noexcept
{
    if (!m_themeMgr)
        return;

    if (m_preferences.IsHighContrast())
    {
        if (m_highContrastApplied)
            return;
        m_savedTheme = std::make_unique<Theme::Theme>(m_themeMgr->GetCurrentTheme());
        m_themeMgr->SetTheme(std::make_unique<Theme::Theme>(Theme::CreateHighContrastTheme()));
        m_highContrastApplied = true;
    }
    else
    {
        RestoreTheme();
    }
}

void AccessibilityManager::RestoreTheme() noexcept
{
    if (!m_themeMgr || !m_highContrastApplied)
        return;

    if (m_savedTheme && !m_savedTheme->GetName().empty())
        m_themeMgr->SetTheme(std::make_unique<Theme::Theme>(*m_savedTheme));

    m_savedTheme.reset();
    m_highContrastApplied = false;
}

void AccessibilityManager::NotifyPreferencesChanged(const AccessibilityPreferences& prefs) noexcept
{
    (void)prefs;
}

} // namespace DragonOS::DragonUI