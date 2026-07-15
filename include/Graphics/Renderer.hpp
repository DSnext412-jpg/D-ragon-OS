/**
 * @file    Renderer.hpp
 * @brief   High-level rendering orchestrator for DragonOS.
 *
 * Owns the Direct2D, DirectWrite, and WIC factories together with the
 * HWND render target.  Provides a simple begin-frame / draw / end-frame
 * workflow that the Window class calls during WM_PAINT.
 */

#pragma once

#include <Graphics/Color.hpp>
#include <Graphics/Brush.hpp>
#include <Graphics/RenderTarget.hpp>

#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>

#include <string>
#include <string_view>
#include <vector>

namespace DragonOS::Graphics {

/**
 * @brief  Manages the Direct2D rendering pipeline.
 *
 * Responsibilities:
 *   - Initialise Direct2D, DirectWrite, and WIC factories.
 *   - Create and resize the HWND render target.
 *   - Begin / End frame (BeginDraw / EndDraw).
 *   - Clear, fill rectangles, draw rectangles, draw lines, draw text.
 *   - Cache solid-colour brushes to avoid per-frame allocations.
 */
class Renderer final {
public:
    Renderer() noexcept = default;
    ~Renderer() noexcept { Shutdown(); }

    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&)                 = delete;
    Renderer& operator=(Renderer&&)      = delete;

    // ── Lifecycle ──────────────────────────────────────────────────────

    /**
     * @brief  Initialise COM, create all factories, and build the
     *         HWND render target.
     *
     * @param hwnd  The native window to render into.
     * @return true on success, false on failure.
     */
    [[nodiscard]] bool Initialize(HWND hwnd) noexcept;

    /// @brief  Release every DirectX / COM resource.
    void Shutdown() noexcept;

    /// @brief  Resize the render target to match a new client area.
    /// @param width   New client width  in pixels.
    /// @param height  New client height in pixels.
    void Resize(UINT width, UINT height) noexcept;

    // ── Frame workflow ─────────────────────────────────────────────────

    /// @brief  Begin a new frame.  Must be called before any draw calls.
    void BeginFrame() noexcept;

    /// @brief  End the current frame and present to the screen.
    void EndFrame() noexcept;

    /// @brief  Clear the entire render target to a solid colour.
    void Clear(const Color& color) noexcept;

    // ── Drawing primitives ─────────────────────────────────────────────

    /// @brief  Fill a rectangle with a solid colour.
    void FillRectangle(
        const D2D1_RECT_F& rect,
        const Color&       color,
        float              opacity = 1.0f) noexcept;

    /// @brief  Draw a rectangular outline.
    void DrawRectangle(
        const D2D1_RECT_F& rect,
        const Color&       color,
        float              strokeWidth = 1.0f) noexcept;

    /// @brief  Draw a straight line.
    void DrawLine(
        D2D1_POINT_2F      p0,
        D2D1_POINT_2F      p1,
        const Color&       color,
        float              strokeWidth = 1.0f) noexcept;

    /// @brief  Draw a string of text at the given layout rectangle.
    void DrawText(
        std::wstring_view       text,
        const D2D1_RECT_F&      layoutRect,
        const Color&            color) noexcept;

    /// @brief  Retrieve a cached brush for the given colour.
    [[nodiscard]] ID2D1SolidColorBrush* GetBrush(const Color& color) noexcept;

    /// @brief  Direct access to the underlying D2D render target.
    /// @return The HWND render target, or nullptr if not initialised.
    [[nodiscard]] ID2D1HwndRenderTarget* GetRenderTarget() const noexcept;

    /// @brief  Replace the default text format (font family + size).
    /// @param family  Font family name (e.g. L"Segoe UI").
    /// @param size    Font size in DIPs.
    /// @note  Safe to call before Initialize() (stores values for later).
    void SetDefaultTextFormat(std::wstring_view family, float size) noexcept;

private:
    // ── Factory pointers (raw COM; managed manually) ───────────────────

    ID2D1Factory*        m_pD2DFactory{ nullptr };
    IDWriteFactory*      m_pDWriteFactory{ nullptr };
    IWICImagingFactory*  m_pWICFactory{ nullptr };
    IDWriteTextFormat*   m_pTextFormat{ nullptr };

    // ── Render target ──────────────────────────────────────────────────

    RenderTarget m_renderTarget;

    // ── Brush cache ────────────────────────────────────────────────────

    struct CachedBrush final {
        D2D1_COLOR_F           color;
        ID2D1SolidColorBrush*  brush;
    };

    std::vector<CachedBrush> m_brushCache;

    // ── Typography overrides (settable before/after Initialize) ────────

    std::wstring m_fontFamily{ L"Segoe UI" };
    float        m_fontSize{ 14.0f };

    // ── State ──────────────────────────────────────────────────────────

    bool m_initialized{ false };
};

} // namespace DragonOS::Graphics
