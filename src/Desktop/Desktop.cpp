/**
 * @file    Desktop.cpp
 * @brief   Implementation of the Desktop class.
 */

#include <Desktop/Desktop.hpp>
#include <Desktop/Wallpaper.hpp>
#include <Graphics/Renderer.hpp>

namespace DragonOS::Desktop {

// ============================================================================
//  Construction / Destruction
// ============================================================================

Desktop::Desktop()
    : m_pWallpaper{ std::make_unique<Wallpaper>() }
{
}

Desktop::~Desktop() noexcept
{
    m_pWallpaper.reset();
}

// ============================================================================
//  Rendering
// ============================================================================

void Desktop::Render(
    Graphics::Renderer& renderer,
    float               width,
    float               height) noexcept
{
    m_pWallpaper->Render(renderer, width, height);
}

// ============================================================================
//  Resize
// ============================================================================

void Desktop::Resize(float width, float height) noexcept
{
    m_pWallpaper->Resize(width, height);
}

} // namespace DragonOS::Desktop
