/**
 * @file    Wallpaper.hpp
 * @brief   Desktop wallpaper rendering with solid-colour and gradient support.
 *
 * The Wallpaper class owns the visual backdrop of the desktop.  It
 * supports solid-colour fills and vertical linear gradients, and is
 * designed for future extension to image-based wallpapers with
 * stretch, centre, fit, and tile modes.
 */

#pragma once

#include <Graphics/Color.hpp>

#include <cstdint>
#include <d2d1.h>

namespace DragonOS::Graphics { class Renderer; }
namespace DragonOS::Theme   { class ThemeManager; }

namespace DragonOS::Desktop {

/// @brief  Active wallpaper rendering mode.
enum class WallpaperType : uint8_t {
    SolidColor,   ///< Flat colour fill.
    Gradient,     ///< Vertical linear gradient.
    // Image       ///< Reserved for future bitmap-based wallpapers.
};

/**
 * @brief  Owns the wallpaper render state and gradient resources.
 *
 * Resources (gradient stop collection, gradient brush) are created
 * lazily on the first Render() call and are recreated when the
 * wallpaper type or colours change.
 */
class Wallpaper final {
public:
    Wallpaper() noexcept = default;
    ~Wallpaper() noexcept;

    Wallpaper(const Wallpaper&)            = delete;
    Wallpaper& operator=(const Wallpaper&) = delete;
    Wallpaper(Wallpaper&&)                 = delete;
    Wallpaper& operator=(Wallpaper&&)      = delete;

    // ── Rendering ─────────────────────────────────────────────────────────

    /**
     * @brief  Render the wallpaper into the given renderer.
     *
     * @param renderer  Target renderer (must be in a BeginFrame/EndFrame
     *                  pair).
     * @param width     Client-area width  in DIPs.
     * @param height    Client-area height in DIPs.
     */
    void Render(
        Graphics::Renderer& renderer,
        float               width,
        float               height) noexcept;

    /// @brief  Update gradient brush end-point when the window is resized.
    void Resize(float width, float height) noexcept;

    // ── Configuration ─────────────────────────────────────────────────────

    /// @brief  Switch to a solid-colour wallpaper.
    void SetSolidColor(const Graphics::Color& color) noexcept;

    /// @brief  Switch to a vertical gradient wallpaper.
    void SetGradient(
        const Graphics::Color& top,
        const Graphics::Color& bottom) noexcept;

    // ── Theme integration ─────────────────────────────────────────────────

    /// @brief  Attach a ThemeManager to drive wallpaper colours.
    void SetThemeManager(const Theme::ThemeManager& themeManager) noexcept;

private:
    void ReleaseResources() noexcept;
    void CreateGradientResources(Graphics::Renderer& renderer) noexcept;

    // ── State ─────────────────────────────────────────────────────────

    WallpaperType     m_type{ WallpaperType::Gradient };

    Graphics::Color   m_solidColor{ 7.0f / 255.0f, 10.0f / 255.0f, 18.0f / 255.0f };
    Graphics::Color   m_gradientTop{ 20.0f / 255.0f, 24.0f / 255.0f, 34.0f / 255.0f };
    Graphics::Color   m_gradientBottom{ 7.0f / 255.0f, 10.0f / 255.0f, 18.0f / 255.0f };

    ID2D1LinearGradientBrush* m_pGradientBrush{ nullptr };

    const Theme::ThemeManager* m_pThemeManager{ nullptr };

    float               m_width{ 0.0f };
    float               m_height{ 0.0f };
    std::uint32_t       m_lastTargetGeneration{ 0 };
};

} // namespace DragonOS::Desktop
