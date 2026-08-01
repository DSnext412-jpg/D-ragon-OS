/**
 * @file    ThemeManager.hpp
 * @brief   Central coordinator for theme lifecycle and lookup.
 *
 * ThemeManager is the ONLY entry point for UI appearance data.
 * No subsystem should ever hardcode a colour, font size, or metric.
 *
 * Registered with the Engine as a managed System so every module
 * can obtain theme data through EngineContext.
 */

#pragma once

#include <Theme/Theme.hpp>
#include <Theme/ThemeColor.hpp>
#include <Theme/ThemeMetrics.hpp>
#include <Theme/ThemeMode.hpp>
#include <Theme/ThemePalette.hpp>
#include <Theme/ThemeShadow.hpp>
#include <Theme/ThemeTypography.hpp>

#include <Engine/System.hpp>

#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace DragonOS::Theme {

/**
 * @brief  Owns and serves the current theme to all subsystems.
 *
 * ## Lifecycle
 *   Initialize()  — creates the built-in DragonOS Dark theme.
 *   Shutdown()    — destroys the current theme.
 *   Update()      — future: hot-reload, transition animations.
 *   Render()      — no-op (theme data is not a visual layer).
 *   Resize()      — no-op.
 *
 * ## Lookup
 *   GetColor(token)       → ThemeColor from the active palette.
 *   GetMetric()           → ThemeMetrics constants.
 *   GetTypography()       → Font face / size definitions.
 *   GetShadow()           → Shadow definitions.
 *
 * ## Theme switching
 *   SetTheme(theme)       — replace the active theme.
 *   GetCurrentTheme()     — retrieve the active theme.
 */
class ThemeManager final : public Engine::System {
public:
    ThemeManager() noexcept = default;
    ~ThemeManager() noexcept { Shutdown(); }

    ThemeManager(const ThemeManager&)            = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;
    ThemeManager(ThemeManager&&)                 = delete;
    ThemeManager& operator=(ThemeManager&&)      = delete;

    // ── Engine::System ───────────────────────────────────────────────────

    /**
     * @brief  Create the built-in DragonOS Dark theme and set it as active.
     *
     * TODO (later phase):  Load user/custom themes on startup.
     *
     * @param ctx  Engine context (unused by ThemeManager itself).
     *
     * @return true on success.
     */
    [[nodiscard]] bool Initialize(Engine::EngineContext& ctx) noexcept override;

    /// @brief  Destroy the current theme and release state.
    void Shutdown() noexcept override;

    /**
     * @brief  Per-frame update.
     *
     * TODO:  Process theme transitions, hot-reload detection.
     *
     * @param deltaTime  Seconds elapsed since the previous frame.
     */
    void Update(float deltaTime) noexcept override;

    /// @brief  No-op — theme data is not a renderable layer.
    void Render(Engine::EngineContext& /*ctx*/) noexcept override {}

    /// @brief  No-op — theme data does not depend on viewport size.
    void Resize(float /*width*/, float /*height*/) noexcept override {}

    // ── Theme management ─────────────────────────────────────────────────

    /// @brief  Return the currently active theme.
    [[nodiscard]] const Theme& GetCurrentTheme() const noexcept;

    /**
     * @brief  Replace the active theme.
     *
     * @param theme  The new theme to use.
     */
    void SetTheme(std::unique_ptr<Theme> theme) noexcept;

    // ── Theme change notification ─────────────────────────────────────────

    using ThemeChangedCallback = std::function<void(const Theme& newTheme)>;

    /**
     * @brief  RAII subscription to theme changes.
     *
     * Holds a pointer to its ThemeManager; the subscription is removed when
     * the token is destroyed (or move-assigned over).  Keep the manager alive
     * while any token is in scope.
     */
    class ThemeChangedListener {
    public:
        ThemeChangedListener() = default;
        ThemeChangedListener(const ThemeChangedListener&) = delete;
        ThemeChangedListener& operator=(const ThemeChangedListener&) = delete;

        ThemeChangedListener(ThemeChangedListener&& other) noexcept { *this = std::move(other); }

        ThemeChangedListener& operator=(ThemeChangedListener&& other) noexcept
        {
            if (this != &other)
            {
                Release();
                m_manager = other.m_manager;
                m_id = other.m_id;
                other.m_id = 0;
            }
            return *this;
        }

        ~ThemeChangedListener() { Release(); }

        [[nodiscard]] explicit operator bool() const noexcept { return m_id != 0; }
        void Detach() noexcept { Release(); }

    private:
        friend class ThemeManager;

        ThemeChangedListener(ThemeManager* manager, uint64_t id) noexcept
            : m_manager(manager)
            , m_id(id)
        {
        }

        void Release() noexcept
        {
            if (m_manager && m_id != 0)
            {
                m_manager->RemoveThemeChangedListener(m_id);
                m_id = 0;
            }
        }

        ThemeManager* m_manager{};
        uint64_t m_id{};
    };

    /**
     * @brief  Register a callback invoked whenever the active theme is
     *         replaced (including the theme created by Initialize).
     *
     * @param cb  The callback; receives the newly active theme.
     *
     * @return A RAII token that removes the subscription when destroyed.
     */
    ThemeChangedListener AddThemeChangedListener(ThemeChangedCallback cb) noexcept;

    /// @brief  Convenience single-slot callback; replaced on every call.
    void SetOnThemeChanged(ThemeChangedCallback cb) noexcept { m_onThemeChanged = std::move(cb); }

    // ── Convenience lookups (forwarded to the active theme) ──────────────

    /**
     * @brief  Look up a colour by semantic token from the active palette.
     *
     * @param token  The semantic colour to retrieve.
     *
     * @return The colour value, or ThemeColor{} if no theme is active.
     */
    [[nodiscard]] const ThemeColor& GetColor(SemanticColor token) const noexcept;

    /// @brief  Retrieve the active metrics constants.
    [[nodiscard]] ThemeMetrics GetMetrics() const noexcept;

    /// @brief  Retrieve the active typography definitions.
    [[nodiscard]] const ThemeTypography& GetTypography() const noexcept;

    /**
     * @brief  Retrieve a shadow definition by category.
     *
     * @note  Returns a default ShadowDef if no theme is active.
     *
     * @return The active shadow structure.
     */
    [[nodiscard]] const ThemeShadow& GetShadow() const noexcept;

private:
    friend class ThemeChangedListener;

    void RemoveThemeChangedListener(uint64_t id) noexcept;

    void NotifyThemeChanged() noexcept;

    std::unique_ptr<Theme> m_pCurrentTheme;
    ThemeChangedCallback m_onThemeChanged;
    struct ThemeListenerEntry {
        uint64_t id{};
        ThemeChangedCallback cb;
    };
    std::vector<ThemeListenerEntry> m_themeListeners;
    uint64_t m_nextListenerId{1};
    bool m_initialized{ false };
};

} // namespace DragonOS::Theme
