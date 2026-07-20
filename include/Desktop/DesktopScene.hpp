/**
 * @file    DesktopScene.hpp
 * @brief   Scene graph root for all renderable desktop objects.
 *
 * DesktopScene is the top-level container that DesktopManager
 * delegates to.  It owns every object that appears on the desktop:
 * the Desktop background, and — in future — floating panels,
 * popups, and overlay layers.
 */

#pragma once

#include <memory>

namespace DragonOS::Graphics { class Renderer; }
namespace DragonOS::Desktop { class Desktop; }

namespace DragonOS::Desktop {

/**
 * @brief  Root of the desktop scene graph.
 *
 * Maintains the ordered list of renderable objects.  Objects are
 * rendered in the order they are added; the Desktop background is
 * always first.
 */
class DesktopScene final {
public:
    DesktopScene();
    ~DesktopScene() noexcept;

    DesktopScene(const DesktopScene&)            = delete;
    DesktopScene& operator=(const DesktopScene&) = delete;
    DesktopScene(DesktopScene&&)                 = delete;
    DesktopScene& operator=(DesktopScene&&)      = delete;

    // ── Lifecycle ─────────────────────────────────────────────────────────

    /// @brief  Render every object in the scene.
    void Render(
        Graphics::Renderer& renderer,
        float               width,
        float               height) noexcept;

    /// @brief  Propagate resize to every scene object.
    void Resize(float width, float height) noexcept;

    /**
     * @brief  Advance animation state.
     * @param deltaTime  Time since the last frame, in seconds.
     */
    void Update(float deltaTime) noexcept;

    /// @brief  Access the desktop background.
    [[nodiscard]] Desktop& GetDesktop() noexcept { return *m_pDesktop; }

private:
    std::unique_ptr<Desktop> m_pDesktop;
};

} // namespace DragonOS::Desktop
