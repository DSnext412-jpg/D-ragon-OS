/**
 * @file    ThemeManager.cpp
 * @brief   ThemeManager implementation.
 */

#include <Theme/ThemeManager.hpp>

namespace DragonOS::Theme {

// ── Free function declared in Theme.cpp ─────────────────────────────────
Theme CreateDefaultDarkTheme() noexcept;

// ── Engine::System ──────────────────────────────────────────────────────

bool ThemeManager::Initialize(Engine::EngineContext& /*ctx*/) noexcept
{
    if (m_initialized)
        return true;

    m_pCurrentTheme = std::make_unique<Theme>(CreateDefaultDarkTheme());
    m_initialized = true;
    return true;
}

void ThemeManager::Shutdown() noexcept
{
    m_pCurrentTheme.reset();
    m_initialized = false;
}

void ThemeManager::Update(float /*deltaTime*/) noexcept
{
    // TODO:  Theme transitions, hot-reload detection, animation driver.
}

// ── Theme management ────────────────────────────────────────────────────

const Theme& ThemeManager::GetCurrentTheme() const noexcept
{
    return *m_pCurrentTheme;
}

void ThemeManager::SetTheme(std::unique_ptr<Theme> theme) noexcept
{
    m_pCurrentTheme = std::move(theme);
}

// ── Convenience lookups ─────────────────────────────────────────────────

const ThemeColor& ThemeManager::GetColor(SemanticColor token) const noexcept
{
    if (m_pCurrentTheme)
        return m_pCurrentTheme->GetPalette().Get(token);

    // Fallback — transparent sentinel (avoid UB on null deref).
    static constexpr ThemeColor s_fallback{};
    return s_fallback;
}

ThemeMetrics ThemeManager::GetMetrics() const noexcept
{
    return ThemeMetrics{};
}

const ThemeTypography& ThemeManager::GetTypography() const noexcept
{
    if (m_pCurrentTheme)
        return m_pCurrentTheme->GetTypography();

    static constexpr ThemeTypography s_fallback{};
    return s_fallback;
}

const ThemeShadow& ThemeManager::GetShadow() const noexcept
{
    if (m_pCurrentTheme)
        return m_pCurrentTheme->GetShadow();

    static constexpr ThemeShadow s_fallback{};
    return s_fallback;
}

} // namespace DragonOS::Theme
