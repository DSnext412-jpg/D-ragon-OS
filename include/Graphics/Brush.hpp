/**
 * @file    Brush.hpp
 * @brief   RAII wrapper for a Direct2D solid-colour brush.
 *
 * Brushes are created from an ID2D1RenderTarget and cached by the
 * Renderer so they are never recreated every frame.
 */

#pragma once

#include <Graphics/Color.hpp>

#include <d2d1.h>

namespace DragonOS::Graphics {

/**
 * @brief  Owns a single ID2D1SolidColorBrush with move semantics.
 *
 * A Brush in the null state (default-constructed or moved-from) holds
 * no resource.  Call Create() to attach a brush, or move-assign from
 * a valid instance.
 */
class Brush final {
public:
    /// @brief  Constructs a null brush.
    Brush() noexcept = default;

    /// @brief  Releases the underlying Direct2D brush.
    ~Brush() noexcept { Release(); }

    Brush(const Brush&)            = delete;
    Brush& operator=(const Brush&) = delete;

    /// @brief  Transfers ownership of the Direct2D brush.
    Brush(Brush&& other) noexcept
        : m_brush{ other.m_brush }
    {
        other.m_brush = nullptr;
    }

    /// @brief  Transfers ownership and releases any previously held brush.
    Brush& operator=(Brush&& other) noexcept
    {
        if (this != &other)
        {
            Release();
            m_brush       = other.m_brush;
            other.m_brush = nullptr;
        }
        return *this;
    }

    /// @brief  Attach an existing, already-created brush pointer.
    /// @param  brush  A fully created ID2D1SolidColorBrush with refcount ≥ 1.
    void Attach(ID2D1SolidColorBrush* brush) noexcept
    {
        Release();
        m_brush = brush;
    }

    /// @brief  Detach and return the raw pointer without releasing it.
    /// @return The previously held brush pointer (caller assumes ownership).
    [[nodiscard]] ID2D1SolidColorBrush* Detach() noexcept
    {
        auto* ptr = m_brush;
        m_brush   = nullptr;
        return ptr;
    }

    /// @brief  Create a solid-colour brush from a render target.
    /// @param  target  The Direct2D render target that will own the brush.
    /// @param  color   Brush colour.
    /// @return true on success, false if creation failed.
    [[nodiscard]] bool Create(
        ID2D1RenderTarget* target,
        const Color&       color) noexcept
    {
        Release();

        const D2D1_COLOR_F d2d{ color.r, color.g, color.b, color.a };
        const HRESULT      hr = target->CreateSolidColorBrush(d2d, &m_brush);
        return SUCCEEDED(hr);
    }

    /// @brief  Access the raw Direct2D brush pointer.
    [[nodiscard]] ID2D1SolidColorBrush* Get() const noexcept { return m_brush; }

    /// @brief  Check whether the brush holds a valid resource.
    explicit operator bool() const noexcept { return m_brush != nullptr; }

private:
    void Release() noexcept
    {
        if (m_brush)
        {
            m_brush->Release();
            m_brush = nullptr;
        }
    }

    ID2D1SolidColorBrush* m_brush{ nullptr };
};

} // namespace DragonOS::Graphics
