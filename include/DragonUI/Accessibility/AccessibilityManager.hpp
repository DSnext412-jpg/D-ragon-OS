#pragma once

#include <DragonUI/Accessibility/AccessibilityPreferences.hpp>
#include <DragonUI/Accessibility/AccessibilityNotification.hpp>
#include <DragonUI/Accessibility/UIAAutomationTree.hpp>

#include <Input/KeyCodes.hpp>
#include <Theme/ThemeManager.hpp>

#include <memory>
#include <string>

namespace DragonOS::DragonUI {

class Element;
class Container;
class FocusManager;

/**
 * @brief  Central accessibility service for a DragonUI host.
 *
 * Owns:
 *   - AccessibilityPreferences  (high contrast, reduced motion, large fonts)
 *   - AccessibilityNotificationHub (screen-reader / automation events)
 *   - UIAAutomationTree         (automation tree + traversal)
 *
 * Responsibilities:
 *   - report focus / value / structure changes to screen-reader listeners
 *   - drive logical keyboard navigation (arrow keys, access keys)
 *   - apply user accessibility preferences (high-contrast theme, text scale)
 *   - provide live announcements for live regions and alerts
 */
class AccessibilityManager final {
public:
    AccessibilityManager() noexcept = default;
    ~AccessibilityManager() noexcept { Shutdown(); }

    AccessibilityManager(const AccessibilityManager&) = delete;
    AccessibilityManager& operator=(const AccessibilityManager&) = delete;

    // ── Lifecycle ─────────────────────────────────────────────────────

    /// @brief  Attach the manager to a host and its services.
    void Initialize(Element* root, FocusManager* focusMgr, Theme::ThemeManager* themeMgr) noexcept;

    /// @brief  Detach services and restore any accessibility theme changes.
    void Shutdown() noexcept;

    [[nodiscard]] bool IsInitialized() const noexcept { return m_initialized; }

    // ── Host wiring ────────────────────────────────────────────────────

    void SetRoot(Element* root) noexcept;
    [[nodiscard]] Element* GetRoot() const noexcept { return m_root; }
    void SetFocusManager(FocusManager* focusMgr) noexcept { m_focusMgr = focusMgr; }
    void SetThemeManager(Theme::ThemeManager* themeMgr) noexcept { m_themeMgr = themeMgr; }

    // ── Owned services ─────────────────────────────────────────────────

    [[nodiscard]] AccessibilityPreferences& Preferences() noexcept { return m_preferences; }
    [[nodiscard]] const AccessibilityPreferences& Preferences() const noexcept { return m_preferences; }

    [[nodiscard]] AccessibilityNotificationHub& Notifications() noexcept { return m_notifications; }
    [[nodiscard]] const AccessibilityNotificationHub& Notifications() const noexcept { return m_notifications; }

    [[nodiscard]] UIAAutomationTree& Tree() noexcept { return m_tree; }
    [[nodiscard]] const UIAAutomationTree& Tree() const noexcept { return m_tree; }

    // ── Screen-reader reporting ────────────────────────────────────────

    /// @brief  Announce that keyboard focus moved to @p element.
    void ReportFocus(Element* element) noexcept;

    /// @brief  Announce that an element's value changed (text, toggle...).
    void ReportValueChanged(Element* element) noexcept;

    /// @brief  Announce a selection change on a container.
    void ReportSelectionChanged(Element* container) noexcept;

    /// @brief  Announce that the accessibility tree structure changed.
    void ReportStructuredChange() noexcept;

    /// @brief  Speak / surface a live-region announcement.
    void Announce(std::wstring_view text) noexcept;

    /// @brief  Raise an alert (screen readers typically interrupt).
    void Alert(std::wstring_view text) noexcept;

    // ── Keyboard navigation (Part 3) ───────────────────────────────────

    /// @brief  Handle arrow keys / access-key style logical navigation.
    /// @return true when the key was consumed for accessibility navigation.
    [[nodiscard]] bool HandleNavigationKey(Input::KeyCode key, bool shift, bool alt,
                                           std::wstring_view accessKey) noexcept;

    // ── Accessibility preferences application (Part 5) ─────────────────

    /// @brief  Apply the user's preference state (theme / scale) to the host.
    void ApplyPreferences() noexcept;

private:
    void ApplyHighContrast() noexcept;
    void RestoreTheme() noexcept;
    void NotifyPreferencesChanged(const AccessibilityPreferences& prefs) noexcept;

    Element* m_root{};
    FocusManager* m_focusMgr{};
    Theme::ThemeManager* m_themeMgr{};
    bool m_initialized{};

    AccessibilityPreferences m_preferences;
    AccessibilityNotificationHub m_notifications;
    UIAAutomationTree m_tree;

    // High-contrast theme restoration state.
    std::unique_ptr<Theme::Theme> m_savedTheme;
    bool m_highContrastApplied{};

    AccessibilityPreferences::ChangeListener m_prefListener;
};

} // namespace DragonOS::DragonUI