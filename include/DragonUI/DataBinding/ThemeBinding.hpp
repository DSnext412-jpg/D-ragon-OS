#pragma once

#include <DragonUI/DataBinding/Observable.hpp>

#include <Theme/Theme.hpp>
#include <Theme/ThemeColor.hpp>
#include <Theme/ThemeManager.hpp>

#include <memory>
#include <utility>
#include <vector>

namespace DragonOS::DragonUI {

/**
 * @brief Bridges the active theme into the data-binding system.
 *
 * Each bound semantic token becomes an Observable<ThemeColor> that updates
 * automatically whenever the theme is replaced, so view-model properties and
 * bindings can react to theme changes.
 *
 * \code
 * ThemeBinding themeBinding(themeManager);
 * auto accent = themeBinding.BindColor(Theme::SemanticColor::Accent);
 * ...
 * themeManager.SetTheme(...);              // accent->Get() refreshes
 * \endcode
 *
 * The binding owns a ThemeChangedListener token, so it unsubscribes itself on
 * destruction.  It keeps no reference to the manager after destruction.
 */
class ThemeBinding {
public:
    explicit ThemeBinding(Theme::ThemeManager& manager) noexcept
        : m_manager(&manager)
        , m_listener(manager.AddThemeChangedListener(
              [this](const Theme::Theme&)
              {
                  Refresh();
              }))
    {
    }

    ThemeBinding(const ThemeBinding&) = delete;
    ThemeBinding& operator=(const ThemeBinding&) = delete;

    /// Registers a semantic token and returns an observable color that follows
    /// the active theme.
    ObservablePtr<Theme::ThemeColor> BindColor(Theme::SemanticColor token) noexcept
    {
        auto observable = MakeObservable<Theme::ThemeColor>(m_manager->GetColor(token));
        m_bindings.emplace_back(token, observable);
        return observable;
    }

    /// Re-reads every bound color from the active theme.
    void Refresh() noexcept
    {
        for (auto& [token, observable] : m_bindings)
        {
            if (observable)
                observable->Set(m_manager->GetColor(token));
        }
    }

    [[nodiscard]] Theme::ThemeManager& GetManager() const noexcept { return *m_manager; }

private:
    Theme::ThemeManager* m_manager;
    Theme::ThemeManager::ThemeChangedListener m_listener;
    std::vector<std::pair<Theme::SemanticColor, ObservablePtr<Theme::ThemeColor>>> m_bindings;
};

} // namespace DragonOS::DragonUI
