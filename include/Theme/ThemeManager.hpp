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

#include <memory>

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
    std::unique_ptr<Theme> m_pCurrentTheme;
    bool m_initialized{ false };
};

} // namespace DragonOS::Theme
