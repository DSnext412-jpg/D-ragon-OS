#pragma once

#include <DragonUI/Accessibility/AccessibilityRole.hpp>
#include <DragonUI/Accessibility/AccessibilityState.hpp>

#include <functional>
#include <string_view>
#include <vector>

namespace DragonOS::DragonUI {

class Element;

/**
 * @brief  Types of screen-reader / automation events produced by DragonUI.
 *
 * These notifications are framework-internal telemetry.  A future
 * screen-reader bridge can subscribe to an AccessibilityManager's
 * notification hub and translate each event into UIA / MSAA raised events.
 * All events are fired with the affected element (may be null) and an
 * optional string payload.
 */
enum class AccessibilityEvent : uint8_t {
    None,
    FocusChanged,     ///< Keyboard focus moved to a new element.
    NameChanged,      ///< The accessible name of an element changed.
    HelpChanged,      ///< The accessible description changed.
    ValueChanged,     ///< The element's value / toggle / position changed.
    PropertyChanged,  ///< A generic state property changed.
    LiveRegionChanged,///< Announce a live region update (e.g. status text).
    SelectionChanged, ///< The current selection changed.
    Show,             ///< An element became visible / a window opened.
    Hide,             ///< An element became hidden / a window closed.
    StructuredChanged,///< The accessibility tree structure changed.
    Alert,            ///< Important alert requiring attention.
    Invoked,          ///< An invokable control was invoked (e.g. button clicked).
    Toggled,          ///< A toggle / check control changed state.
    Expanded,         ///< A node was expanded.
    Collapsed,        ///< A node was collapsed.
    AutomationIdChanged, ///< The automation id was assigned / changed.
};

[[nodiscard]] constexpr std::string_view ToString(AccessibilityEvent evt) noexcept
{
    using E = AccessibilityEvent;
    switch (evt)
    {
    case E::FocusChanged:       return "FocusChanged";
    case E::NameChanged:        return "NameChanged";
    case E::HelpChanged:        return "HelpChanged";
    case E::ValueChanged:       return "ValueChanged";
    case E::PropertyChanged:    return "PropertyChanged";
    case E::LiveRegionChanged:  return "LiveRegionChanged";
    case E::SelectionChanged:   return "SelectionChanged";
    case E::Show:               return "Show";
    case E::Hide:               return "Hide";
    case E::StructuredChanged:  return "StructuredChanged";
    case E::Alert:              return "Alert";
    case E::Invoked:            return "Invoked";
    case E::Toggled:            return "Toggled";
    case E::Expanded:           return "Expanded";
    case E::Collapsed:          return "Collapsed";
    case E::AutomationIdChanged:return "AutomationIdChanged";
    case E::None:               break;
    }
    return "None";
}

/**
 * @brief  A single accessibility event, ready for a screen-reader bridge.
 */
struct AccessibilityEventArgs {
    AccessibilityEvent type{ AccessibilityEvent::None };
    Element*           element{};
    AccessibilityRole  role{ AccessibilityRole::Unknown };
    AccessibilityState state{ AccessibilityState::None };
    std::wstring_view  name;
    std::wstring_view  value;
};

/**
 * @brief  Fan-out hub for accessibility listeners.
 *
 * Provides an RAII subscription token so a listener is removed automatically
 * when it goes out of scope (matching the ThemeManager listener pattern).
 */
class AccessibilityNotificationHub final {
public:
    using Listener = std::function<void(const AccessibilityEventArgs&)>;

    AccessibilityNotificationHub() noexcept = default;
    ~AccessibilityNotificationHub() noexcept { Clear(); }

    AccessibilityNotificationHub(const AccessibilityNotificationHub&) = delete;
    AccessibilityNotificationHub& operator=(const AccessibilityNotificationHub&) = delete;

    class Subscription final {
    public:
        Subscription() = default;
        Subscription(const Subscription&) = delete;
        Subscription& operator=(const Subscription&) = delete;
        Subscription(Subscription&& other) noexcept { *this = std::move(other); }

        Subscription& operator=(Subscription&& other) noexcept
        {
            if (this != &other)
            {
                Release();
                m_hub = other.m_hub;
                m_id = other.m_id;
                other.m_id = 0;
            }
            return *this;
        }

        ~Subscription() { Release(); }

        [[nodiscard]] explicit operator bool() const noexcept { return m_id != 0; }
        void Detach() noexcept { Release(); }

    private:
        friend class AccessibilityNotificationHub;
        Subscription(AccessibilityNotificationHub* hub, uint64_t id) noexcept
            : m_hub(hub), m_id(id) {}

        void Release() noexcept
        {
            if (m_hub && m_id != 0)
                m_hub->Remove(m_id);
            m_id = 0;
        }

        AccessibilityNotificationHub* m_hub{};
        uint64_t m_id{};
    };

    Subscription Subscribe(Listener listener) noexcept;
    void Publish(const AccessibilityEventArgs& args) noexcept;

    /// @brief  Convenience: publish a simple event for an element.
    void Notify(AccessibilityEvent type, Element* element,
                std::wstring_view name = {}, std::wstring_view value = {}) noexcept;

    void Clear() noexcept { m_listeners.clear(); m_nextId = 1; }

private:
    struct Entry { uint64_t id{}; Listener listener; };
    void Remove(uint64_t id) noexcept;

    std::vector<Entry> m_listeners;
    uint64_t m_nextId{ 1 };
};

} // namespace DragonOS::DragonUI