/**
 * @file    RenderTarget.hpp
 * @brief   RAII wrapper for a Direct2D hardware-render target.
 *
 * Manages the lifetime of an ID2D1HwndRenderTarget created from a
 * native Win32 window handle.
 */

#pragma once

#include <d2d1.h>

namespace DragonOS::Graphics {

/**
 * @brief  Owns an ID2D1HwndRenderTarget bound to a specific HWND.
 *
 * The render target is created in the Create() call and released in
 * the destructor or on a subsequent Create() or Release() call.
 */
class RenderTarget final {
public:
    /// @brief  Constructs a null render target.
    RenderTarget() noexcept = default;

    /// @brief  Releases the underlying Direct2D render target.
    ~RenderTarget() noexcept { Release(); }

    RenderTarget(const RenderTarget&)            = delete;
    RenderTarget& operator=(const RenderTarget&) = delete;

    /// @brief  Move constructor.
    RenderTarget(RenderTarget&& other) noexcept
        : m_target{ other.m_target }
    {
        other.m_target = nullptr;
    }

    /// @brief  Move assignment.
    RenderTarget& operator=(RenderTarget&& other) noexcept
    {
        if (this != &other)
        {
            Release();
            m_target       = other.m_target;
            other.m_target = nullptr;
        }
        return *this;
    }

    /**
     * @brief  Create an HWND render target from a factory and window.
     *
     * @param factory  The Direct2D factory (must be valid).
     * @param hwnd     The native window handle.
     * @param size     Client-area dimensions in pixels.
     *
     * @return true on success, false on failure.
     */
    [[nodiscard]] bool Create(
        ID2D1Factory*    factory,
        HWND             hwnd,
        D2D1_SIZE_U      size) noexcept
    {
        Release();

        const auto props    = D2D1::RenderTargetProperties();
        const auto hwndProp = D2D1::HwndRenderTargetProperties(hwnd, size);

        const HRESULT hr = factory->CreateHwndRenderTarget(
            props, hwndProp, &m_target);

        return SUCCEEDED(hr);
    }

    /// @brief  Release the current render target (safe to call repeatedly).
    void Release() noexcept
    {
        if (m_target)
        {
            m_target->Release();
            m_target = nullptr;
        }
    }

    // ── Thin forwarding wrappers ───────────────────────────────────────

    /// @brief  Begin drawing — must be paired with EndDraw().
    void BeginDraw() noexcept { if (m_target) m_target->BeginDraw(); }

    /**
     * @brief  End drawing and present the frame.
     * @return The HRESULT from EndDraw (caller may check for errors).
     */
    HRESULT EndDraw() noexcept
    {
        return m_target ? m_target->EndDraw() : E_FAIL;
    }

    /// @brief  Clear the render target to a solid colour.
    void Clear(const D2D1_COLOR_F& color) noexcept
    {
        if (m_target) m_target->Clear(&color);
    }

    /// @brief  Resize the render target to match a new client area.
    /// @param  size  New dimensions in pixels.
    /// @return HRESULT of the resize operation.
    HRESULT Resize(D2D1_SIZE_U size) noexcept
    {
        return m_target ? m_target->Resize(&size) : E_FAIL;
    }

    // ── Factory methods ────────────────────────────────────────────────

    /// @brief  Create a solid-colour brush from this render target.
    HRESULT CreateSolidColorBrush(
        const D2D1_COLOR_F&     color,
        ID2D1SolidColorBrush**  brush) noexcept
    {
        return m_target
            ? m_target->CreateSolidColorBrush(color, brush)
            : E_FAIL;
    }

    // ── Accessors ──────────────────────────────────────────────────────

    /// @brief  Retrieve the raw Direct2D render target pointer.
    [[nodiscard]] ID2D1HwndRenderTarget* Get() const noexcept { return m_target; }

    /// @brief  Return the current size of the render target in DIPs.
    [[nodiscard]] D2D1_SIZE_F GetSize() const noexcept
    {
        return m_target ? m_target->GetSize() : D2D1_SIZE_F{};
    }

    /// @brief  Check whether a valid render target is held.
    explicit operator bool() const noexcept { return m_target != nullptr; }

private:
    ID2D1HwndRenderTarget* m_target{ nullptr };
};

} // namespace DragonOS::Graphics
