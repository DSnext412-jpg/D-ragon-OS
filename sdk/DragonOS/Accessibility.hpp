#pragma once

#include <cstdint>
#include <string_view>

namespace dragonos::sdk {

/**
 * @brief  Accessibility & UI Automation service exposed to SDK consumers.
 *
 * Lets plugins and extensions:
 *   - query and update user accessibility preferences
 *     (high contrast, reduced motion, text scale)
 *   - raise screen-reader announcements and alerts
 *   - observe the live automation tree (node count, depth)
 *
 * All methods are noexcept and safe to call from the main thread.
 */
class IAccessibilityService {
public:
    virtual ~IAccessibilityService() noexcept = default;

    // ── Preferences ───────────────────────────────────────────────────

    virtual bool IsHighContrast() const noexcept = 0;
    virtual void SetHighContrast(bool enabled) noexcept = 0;

    virtual bool IsReducedMotion() const noexcept = 0;
    virtual void SetReducedMotion(bool enabled) noexcept = 0;

    virtual bool IsLargeFonts() const noexcept = 0;
    virtual void SetTextScale(float scale) noexcept = 0;
    virtual float GetTextScale() const noexcept = 0;

    virtual bool IsScreenReaderActive() const noexcept = 0;

    // ── Screen-reader events ──────────────────────────────────────────

    /// @brief  Surface a live-region announcement to the screen reader.
    virtual void Announce(std::wstring_view text) noexcept = 0;

    /// @brief  Raise an alert (screen readers typically interrupt).
    virtual void Alert(std::wstring_view text) noexcept = 0;

    /// @brief  Notify listeners that the automation tree structure changed.
    virtual void ReportStructuredChange() noexcept = 0;

    // ── Automation tree ───────────────────────────────────────────────

    /// @brief  Number of nodes currently in the automation tree.
    virtual uint64_t GetNodeCount() const noexcept = 0;

    /// @brief  Maximum depth of the automation tree.
    virtual uint64_t GetTreeDepth() const noexcept = 0;
};

} // namespace dragonos::sdk
