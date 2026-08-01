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
    NotifyThemeChanged();
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
    NotifyThemeChanged();
}

// ── Theme change notification ────────────────────────────────────────────

ThemeManager::ThemeChangedListener ThemeManager::AddThemeChangedListener(ThemeChangedCallback cb) noexcept
{
    auto id = m_nextListenerId++;
    m_themeListeners.push_back(ThemeListenerEntry{id, std::move(cb)});
    return ThemeChangedListener{this, id};
}

void ThemeManager::RemoveThemeChangedListener(uint64_t id) noexcept
{
    for (auto it = m_themeListeners.begin(); it != m_themeListeners.end(); ++it)
    {
        if (it->id == id)
        {
            m_themeListeners.erase(it);
            return;
        }
    }
}

void ThemeManager::NotifyThemeChanged() noexcept
{
    if (!m_pCurrentTheme)
        return;
    if (m_onThemeChanged)
        m_onThemeChanged(*m_pCurrentTheme);
    std::vector<ThemeChangedCallback> callbacks;
    callbacks.reserve(m_themeListeners.size());
    for (const auto& entry : m_themeListeners)
        callbacks.push_back(entry.cb);
    for (const auto& cb : callbacks)
    {
        if (cb)
            cb(*m_pCurrentTheme);
    }
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
