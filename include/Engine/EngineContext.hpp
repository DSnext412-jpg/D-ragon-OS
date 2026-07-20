/**
 * @file    EngineContext.hpp
 * @brief   Shared state container passed to every System.
 *
 * EngineContext holds non-owning pointers to core services that
 * systems need to access (renderer, window, viewport dimensions).
 * This avoids global variables and keeps dependencies explicit.
 */

#pragma once

namespace DragonOS::Graphics { class Renderer; }
namespace DragonOS::Input   { class InputManager; }

#include <Theme/ThemeManager.hpp>

namespace DragonOS::Engine {

/**
 * @brief  Provides shared references to engine-wide services.
 *
 * Systems receive a reference to the context during Initialize() and
 * Render() so they can access the renderer and other shared state
 * without holding direct ownership or using globals.
 */
class EngineContext final {
public:
    // ── Renderer ─────────────────────────────────────────────────────────

    /// @brief  Set the active renderer (called during engine init).
    void SetRenderer(Graphics::Renderer& renderer) noexcept
    {
        m_pRenderer = &renderer;
    }

    /// @brief  Retrieve the current renderer (may be nullptr).
    [[nodiscard]] Graphics::Renderer* GetRenderer() const noexcept
    {
        return m_pRenderer;
    }

    // ── Viewport ─────────────────────────────────────────────────────────

    /// @brief  Set the current viewport dimensions.
    void SetViewport(float width, float height) noexcept
    {
        m_viewportWidth  = width;
        m_viewportHeight = height;
    }

    [[nodiscard]] float GetViewportWidth()  const noexcept { return m_viewportWidth; }
    [[nodiscard]] float GetViewportHeight() const noexcept { return m_viewportHeight; }

    // ── Theme ─────────────────────────────────────────────────────────────

    /// @brief  Set the active ThemeManager (called during engine init).
    void SetThemeManager(Theme::ThemeManager& themeManager) noexcept
    {
        m_pThemeManager = &themeManager;
    }

    /// @brief  Retrieve the active ThemeManager, or nullptr.
    [[nodiscard]] Theme::ThemeManager* GetThemeManager() const noexcept
    {
        return m_pThemeManager;
    }

    /**
     * @brief  Convenience access to the current theme.
     *
     * @note  Requires a ThemeManager to be set (programmer error otherwise).
     */
    [[nodiscard]] const Theme::Theme& GetCurrentTheme() const noexcept
    {
        return m_pThemeManager->GetCurrentTheme();
    }

    // ── Input ──────────────────────────────────────────────────────────────

    /// @brief  Set the active InputManager (called during engine init).
    void SetInputManager(Input::InputManager& inputManager) noexcept
    {
        m_pInputManager = &inputManager;
    }

    /// @brief  Retrieve the active InputManager, or nullptr.
    [[nodiscard]] Input::InputManager* GetInputManager() const noexcept
    {
        return m_pInputManager;
    }

private:
    Graphics::Renderer*   m_pRenderer{ nullptr };
    Theme::ThemeManager*  m_pThemeManager{ nullptr };
    Input::InputManager*  m_pInputManager{ nullptr };
    float                 m_viewportWidth{ 0.0f };
    float                 m_viewportHeight{ 0.0f };
};

} // namespace DragonOS::Engine
