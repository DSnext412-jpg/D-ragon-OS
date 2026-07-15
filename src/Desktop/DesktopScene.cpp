/**
 * @file    DesktopScene.cpp
 * @brief   Implementation of the DesktopScene class.
 */

#include <Desktop/DesktopScene.hpp>
#include <Desktop/Desktop.hpp>
#include <Graphics/Renderer.hpp>

namespace DragonOS::Desktop {

// ============================================================================
//  Construction / Destruction
// ============================================================================

DesktopScene::DesktopScene()
    : m_pDesktop{ std::make_unique<Desktop>() }
{
}

DesktopScene::~DesktopScene() = default;

// ============================================================================
//  Rendering
// ============================================================================

void DesktopScene::Render(
    Graphics::Renderer& renderer,
    float               width,
    float               height) noexcept
{
    m_pDesktop->Render(renderer, width, height);
}

// ============================================================================
//  Resize
// ============================================================================

void DesktopScene::Resize(float width, float height) noexcept
{
    m_pDesktop->Resize(width, height);
}

// ============================================================================
//  Update (animation)
// ============================================================================

void DesktopScene::Update(float deltaTime) noexcept
{
    // Reserved for future animation-driven objects.
    (void)deltaTime;
}

} // namespace DragonOS::Desktop
