/**
 * @file    Wallpaper.cpp
 * @brief   Implementation of the Wallpaper class.
 */

#include <Desktop/Wallpaper.hpp>
#include <Graphics/Renderer.hpp>
#include <Theme/ThemeManager.hpp>

namespace DragonOS::Desktop {

/// @brief  Convert a Theme::ThemeColor to Graphics::Color (identical layout).
static Graphics::Color ToGfxColor(const Theme::ThemeColor& tc) noexcept
{
    return { tc.r, tc.g, tc.b, tc.a };
}

// ============================================================================
//  Destruction
// ============================================================================

Wallpaper::~Wallpaper() noexcept
{
    ReleaseResources();
}

// ============================================================================
//  Resource management
// ============================================================================

void Wallpaper::ReleaseResources() noexcept
{
    if (m_pGradientBrush)
    {
        m_pGradientBrush->Release();
        m_pGradientBrush = nullptr;
    }
}

void Wallpaper::CreateGradientResources(
    Graphics::Renderer& renderer) noexcept
{
    ReleaseResources();

    auto* target = renderer.GetRenderTarget();
    if (!target) { return; }

    // ── Gradient stop collection ─────────────────────────────────────────
    const D2D1_GRADIENT_STOP stops[2] = {
        {
            0.0f,   // position
            D2D1_COLOR_F{
                m_gradientTop.r,
                m_gradientTop.g,
                m_gradientTop.b,
                m_gradientTop.a
            }
        },
        {
            1.0f,   // position
            D2D1_COLOR_F{
                m_gradientBottom.r,
                m_gradientBottom.g,
                m_gradientBottom.b,
                m_gradientBottom.a
            }
        }
    };

    ID2D1GradientStopCollection* collection = nullptr;
    HRESULT hr = target->CreateGradientStopCollection(
        stops, 2, &collection);
    if (FAILED(hr)) { return; }

    // ── Linear gradient brush ────────────────────────────────────────────
    hr = target->CreateLinearGradientBrush(
        D2D1::LinearGradientBrushProperties(
            D2D1::Point2F(0.0f, 0.0f),          // start point (top)
            D2D1::Point2F(0.0f, m_height)),     // end point   (bottom)
        collection,
        &m_pGradientBrush);

    collection->Release();

    if (FAILED(hr))
    {
        m_pGradientBrush = nullptr;
    }
}

// ============================================================================
//  Rendering
// ============================================================================

void Wallpaper::Render(
    Graphics::Renderer& renderer,
    float               width,
    float               height) noexcept
{
    if (width <= 0.0f || height <= 0.0f) { return; }

    if (m_type == WallpaperType::Gradient)
    {
        // ── Lazy resource creation / recreation after device loss ────────
        const std::uint32_t gen = renderer.GetTargetGeneration();
        if (!m_pGradientBrush || gen != m_lastTargetGeneration)
        {
            ReleaseResources();
            m_width  = width;
            m_height = height;
            m_lastTargetGeneration = gen;
            CreateGradientResources(renderer);
        }

        if (m_pGradientBrush)
        {
            // Update the end point to match the current height.
            if (height != m_height)
            {
                m_pGradientBrush->SetEndPoint(
                    D2D1::Point2F(0.0f, height));
            }

            m_width  = width;
            m_height = height;

            auto* target = renderer.GetRenderTarget();
            if (target)
            {
                const D2D1_RECT_F rect{ 0.0f, 0.0f, width, height };
                target->FillRectangle(rect, m_pGradientBrush);
            }
        }
    }
    else
    {
        // ── Solid colour — use the renderer's Clear ──────────────────────
        renderer.Clear(m_solidColor);
    }
}

// ============================================================================
//  Resize
// ============================================================================

void Wallpaper::Resize(float width, float height) noexcept
{
    m_width  = width;
    m_height = height;

    if (m_type == WallpaperType::Gradient && m_pGradientBrush)
    {
        m_pGradientBrush->SetEndPoint(
            D2D1::Point2F(0.0f, height));
    }
}

// ============================================================================
//  Configuration
// ============================================================================

void Wallpaper::SetThemeManager(const Theme::ThemeManager& themeManager) noexcept
{
    m_pThemeManager = &themeManager;

    // Pull colour defaults from the theme.
    m_solidColor     = ToGfxColor(themeManager.GetColor(Theme::SemanticColor::DesktopBackground));
    m_gradientTop    = ToGfxColor(themeManager.GetColor(Theme::SemanticColor::DesktopGradientTop));
    m_gradientBottom = ToGfxColor(themeManager.GetColor(Theme::SemanticColor::DesktopGradientBottom));
}

void Wallpaper::SetSolidColor(const Graphics::Color& color) noexcept
{
    m_type       = WallpaperType::SolidColor;
    m_solidColor = color;
    ReleaseResources();
}

void Wallpaper::SetGradient(
    const Graphics::Color& top,
    const Graphics::Color& bottom) noexcept
{
    m_type           = WallpaperType::Gradient;
    m_gradientTop    = top;
    m_gradientBottom = bottom;
    ReleaseResources();
}

} // namespace DragonOS::Desktop
