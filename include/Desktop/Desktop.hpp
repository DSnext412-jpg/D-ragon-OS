/**
 * @file    Desktop.hpp
 * @brief   Desktop surface — owns the wallpaper and hosts icons.
 *
 * The Desktop is the root visual element of the desktop environment.
 * It renders the wallpaper and will eventually own desktop icons,
 * widget containers, and other persistent surface-level objects.
 */

#pragma once

#include <memory>

namespace DragonOS::Graphics { class Renderer; }
namespace DragonOS::Desktop { class Wallpaper; }

namespace DragonOS::Desktop {

/**
 * @brief  Manages the desktop surface and its visual children.
 *
 * Currently the only child is the Wallpaper.  Future versions will
 * also own icons, widgets, and context menus.
 */
class Desktop final {
public:
    Desktop();
    ~Desktop() noexcept;

    Desktop(const Desktop&)            = delete;
    Desktop& operator=(const Desktop&) = delete;
    Desktop(Desktop&&)                 = delete;
    Desktop& operator=(Desktop&&)      = delete;

    // ── Rendering ─────────────────────────────────────────────────────────

    /// @brief  Render the desktop background and all children.
    void Render(
        Graphics::Renderer& renderer,
        float               width,
        float               height) noexcept;

    /// @brief  Resize all children to match a new client area.
    void Resize(float width, float height) noexcept;

    // ── Future API stubs ──────────────────────────────────────────────────

    /// @brief  Add a desktop icon (reserved).
    void AddIcon() {}

    /// @brief  Remove a desktop icon (reserved).
    void RemoveIcon() {}

    /// @brief  Update the desktop wallpaper (reserved).
    void SetWallpaper() {}

    /// @brief  Access the wallpaper for configuration.
    [[nodiscard]] Wallpaper& GetWallpaper() noexcept { return *m_pWallpaper; }

private:
    std::unique_ptr<Wallpaper> m_pWallpaper;
};

} // namespace DragonOS::Desktop
