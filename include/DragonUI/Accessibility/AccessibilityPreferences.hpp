#pragma once

#include <functional>
#include <vector>

namespace DragonOS::DragonUI {

/**
 * @brief  User accessibility preferences honoured by DragonUI.
 *
 * Supports:
 *   - High-contrast themes      (high-contrast colour scheme)
 *   - Reduced motion            (disable / minimise animations)
 *   - Large fonts / text scale  (scaled typography)
 *   - Screen-reader mode        (route richer notifications)
 *
 * The preference object is owned by AccessibilityManager.  Preference
 * changes are observable via RAII subscriptions so renderers and controls
 * can react immediately (e.g. swap to a high-contrast theme).
 */
class AccessibilityPreferences final {
public:
    AccessibilityPreferences() noexcept = default;

    AccessibilityPreferences(const AccessibilityPreferences&) = delete;
    AccessibilityPreferences& operator=(const AccessibilityPreferences&) = delete;

    // ── High contrast ───────────────────────────────────────────────────

    void SetHighContrast(bool enabled) noexcept;
    [[nodiscard]] bool IsHighContrast() const noexcept { return m_highContrast; }

    // ── Reduced motion ──────────────────────────────────────────────────

    void SetReducedMotion(bool enabled) noexcept;
    [[nodiscard]] bool IsReducedMotion() const noexcept { return m_reducedMotion; }

    // ── Large fonts / text scale ────────────────────────────────────────

    /// @brief  Text scaling factor (1.0 = normal).  Clamped to [1.0, 2.0].
    void SetTextScale(float scale) noexcept;
    [[nodiscard]] float GetTextScale() const noexcept { return m_textScale; }
    [[nodiscard]] bool IsLargeFonts() const noexcept { return m_textScale > 1.0f; }

    // ── Screen reader ───────────────────────────────────────────────────

    void SetScreenReaderActive(bool active) noexcept;
    [[nodiscard]] bool IsScreenReaderActive() const noexcept { return m_screenReader; }

    // ── Observability ───────────────────────────────────────────────────

    using ChangedCallback = std::function<void(const AccessibilityPreferences&)>;

    class ChangeListener final {
    public:
        ChangeListener() = default;
        ChangeListener(const ChangeListener&) = delete;
        ChangeListener& operator=(const ChangeListener&) = delete;
        ChangeListener(ChangeListener&& other) noexcept { *this = std::move(other); }

        ChangeListener& operator=(ChangeListener&& other) noexcept
        {
            if (this != &other)
            {
                Release();
                m_owner = other.m_owner;
                m_id = other.m_id;
                other.m_id = 0;
            }
            return *this;
        }

        ~ChangeListener() { Release(); }

        [[nodiscard]] explicit operator bool() const noexcept { return m_id != 0; }
        void Detach() noexcept { Release(); }

    private:
        friend class AccessibilityPreferences;
        ChangeListener(AccessibilityPreferences* owner, uint64_t id) noexcept
            : m_owner(owner), m_id(id) {}

        void Release() noexcept
        {
            if (m_owner && m_id != 0)
                m_owner->RemoveListener(m_id);
            m_id = 0;
        }

        AccessibilityPreferences* m_owner{};
        uint64_t m_id{};
    };

    ChangeListener AddListener(ChangedCallback cb) noexcept;

private:
    friend class ChangeListener;
    void RemoveListener(uint64_t id) noexcept;
    void Notify() noexcept;

    struct Entry { uint64_t id{}; ChangedCallback cb; };
    std::vector<Entry> m_listeners;
    uint64_t m_nextId{ 1 };

    bool   m_highContrast{};
    bool   m_reducedMotion{};
    bool   m_screenReader{};
    float  m_textScale{ 1.0f };
};

} // namespace DragonOS::DragonUI
