/**
 * @file    Renderer.cpp
 * @brief   Implementation of the Renderer class.
 */

#include <Graphics/Renderer.hpp>

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace DragonOS::Graphics {

// ============================================================================
//  Lifecycle
// ============================================================================

bool Renderer::Initialize(HWND hwnd) noexcept
{
    // ── Guard against double initialisation ────────────────────────────────
    if (m_initialized) { return true; }

    HRESULT hr = S_OK;

    // ── 1. COM ─────────────────────────────────────────────────────────────
    hr = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) { return false; }

    // ── 2. Direct2D factory ────────────────────────────────────────────────
    hr = ::D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &m_pD2DFactory);

    if (FAILED(hr)) { ::CoUninitialize(); return false; }

    // ── 3. DirectWrite factory ─────────────────────────────────────────────
    hr = ::DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&m_pDWriteFactory));

    if (FAILED(hr)) { Shutdown(); return false; }

    // ── 4. WIC factory ─────────────────────────────────────────────────────
    hr = ::CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_pWICFactory));

    if (FAILED(hr)) { Shutdown(); return false; }

    // ── 5. Default text format (family and size from theme-aware members) ──
    hr = m_pDWriteFactory->CreateTextFormat(
        m_fontFamily.c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        m_fontSize,
        L"en-US",
        &m_pTextFormat);

    if (FAILED(hr)) { m_pTextFormat = nullptr; }   // non-fatal

    // ── 6. Cache HWND for later target recreation ──────────────────────────
    m_hwnd = hwnd;

    // ── 7. HWND render target ──────────────────────────────────────────────
    RECT rc{};
    ::GetClientRect(hwnd, &rc);

    if (!m_renderTarget.Create(
            m_pD2DFactory,
            hwnd,
            D2D1::SizeU(
                static_cast<UINT32>(rc.right - rc.left),
                static_cast<UINT32>(rc.bottom - rc.top))))
    {
        Shutdown();
        return false;
    }

    m_initialized = true;
    return true;
}

void Renderer::Shutdown() noexcept
{
    // ── Release cached brushes ─────────────────────────────────────────────
    for (auto& cb : m_brushCache)
    {
        if (cb.brush) { cb.brush->Release(); }
    }
    m_brushCache.clear();

    // ── Release render target ──────────────────────────────────────────────
    m_renderTarget.Release();

    // ── Release factories ──────────────────────────────────────────────────
    if (m_pTextFormat)   { m_pTextFormat->Release();   m_pTextFormat   = nullptr; }
    if (m_pWICFactory)   { m_pWICFactory->Release();   m_pWICFactory   = nullptr; }
    if (m_pDWriteFactory){ m_pDWriteFactory->Release(); m_pDWriteFactory = nullptr; }
    if (m_pD2DFactory)   { m_pD2DFactory->Release();    m_pD2DFactory   = nullptr; }

    // ── HWND cache ─────────────────────────────────────────────────────────
    m_hwnd = nullptr;

    // ── COM ────────────────────────────────────────────────────────────────
    ::CoUninitialize();

    m_initialized = false;
}

void Renderer::Resize(UINT width, UINT height) noexcept
{
    if (!m_initialized) { return; }

    if (width == 0 || height == 0) { return; }

    const HRESULT hr = m_renderTarget.Resize(D2D1::SizeU(width, height));

    if (hr == D2DERR_RECREATE_TARGET)
    {
        // Target was lost — recreate via the same path as EndFrame.
        m_brushCache.clear();
        m_renderTarget.Release();

        if (m_hwnd && m_pD2DFactory)
        {
            if (m_renderTarget.Create(
                    m_pD2DFactory,
                    m_hwnd,
                    D2D1::SizeU(width, height)))
            {
                ++m_targetGeneration;
            }
        }
    }
}

// ============================================================================
//  Frame workflow
// ============================================================================

void Renderer::BeginFrame() noexcept
{
    if (!m_initialized) { return; }
    m_renderTarget.BeginDraw();
}

void Renderer::EndFrame() noexcept
{
    if (!m_initialized) { return; }

    const HRESULT hr = m_renderTarget.EndDraw();

    if (hr == D2DERR_RECREATE_TARGET)
    {
        // The Direct3D device was lost — release the old target and
        // all cached brushes (they are tied to the target), then
        // recreate from the factory + cached HWND.
        m_brushCache.clear();
        m_renderTarget.Release();

        if (m_hwnd && m_pD2DFactory)
        {
            RECT rc{};
            ::GetClientRect(m_hwnd, &rc);
            if (m_renderTarget.Create(
                    m_pD2DFactory,
                    m_hwnd,
                    D2D1::SizeU(
                        static_cast<UINT32>(rc.right - rc.left),
                        static_cast<UINT32>(rc.bottom - rc.top))))
            {
                ++m_targetGeneration;
            }
        }
    }
}

void Renderer::Clear(const Color& color) noexcept
{
    if (!m_initialized) { return; }

    const D2D1_COLOR_F d2d{ color.r, color.g, color.b, color.a };
    m_renderTarget.Clear(d2d);
}

// ============================================================================
//  Drawing primitives
// ============================================================================

void Renderer::FillRectangle(
    const D2D1_RECT_F& rect,
    const Color&       color,
    float              opacity) noexcept
{
    if (!m_initialized) { return; }

    auto* brush = GetBrush(color);
    if (!brush) { return; }

    brush->SetOpacity(opacity);
    m_renderTarget.Get()->FillRectangle(rect, brush);
}

void Renderer::DrawRectangle(
    const D2D1_RECT_F& rect,
    const Color&       color,
    float              strokeWidth) noexcept
{
    if (!m_initialized) { return; }

    auto* brush = GetBrush(color);
    if (!brush) { return; }

    m_renderTarget.Get()->DrawRectangle(rect, brush, strokeWidth);
}

void Renderer::DrawLine(
    D2D1_POINT_2F      p0,
    D2D1_POINT_2F      p1,
    const Color&       color,
    float              strokeWidth) noexcept
{
    if (!m_initialized) { return; }

    auto* brush = GetBrush(color);
    if (!brush) { return; }

    m_renderTarget.Get()->DrawLine(p0, p1, brush, strokeWidth);
}

void Renderer::DrawText(
    std::wstring_view       text,
    const D2D1_RECT_F&      layoutRect,
    const Color&            color) noexcept
{
    if (!m_initialized || !m_pTextFormat) { return; }

    auto* brush = GetBrush(color);
    if (!brush) { return; }

    m_renderTarget.Get()->DrawText(
        text.data(),
        static_cast<UINT32>(text.size()),
        m_pTextFormat,
        layoutRect,
        brush);
}

// ============================================================================
//  Brush cache
// ============================================================================

ID2D1SolidColorBrush* Renderer::GetBrush(const Color& color) noexcept
{
    if (!m_initialized) { return nullptr; }

    const D2D1_COLOR_F target{ color.r, color.g, color.b, color.a };

    // ── Linear search for a matching cached brush ─────────────────────────
    for (auto& cb : m_brushCache)
    {
        if (std::memcmp(&cb.color, &target, sizeof(D2D1_COLOR_F)) == 0)
        {
            return cb.brush;
        }
    }

    // ── Not found; create a new one ───────────────────────────────────────
    ID2D1SolidColorBrush* newBrush = nullptr;
    const HRESULT         hr = m_renderTarget.CreateSolidColorBrush(
        target, &newBrush);

    if (FAILED(hr)) { return nullptr; }

    m_brushCache.push_back({ target, newBrush });
    return newBrush;
}

// ============================================================================
//  Render-target accessor
// ============================================================================

ID2D1HwndRenderTarget* Renderer::GetRenderTarget() const noexcept
{
    return m_renderTarget.Get();
}

void Renderer::SetDefaultTextFormat(std::wstring_view family, float size) noexcept
{
    m_fontFamily = family;
    m_fontSize   = size;

    // Recreate the text format immediately if already initialised.
    if (m_initialized && m_pDWriteFactory)
    {
        if (m_pTextFormat)
        {
            m_pTextFormat->Release();
            m_pTextFormat = nullptr;
        }

        HRESULT hr = m_pDWriteFactory->CreateTextFormat(
            m_fontFamily.c_str(),
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            m_fontSize,
            L"en-US",
            &m_pTextFormat);

        if (FAILED(hr)) { m_pTextFormat = nullptr; }
    }
}

} // namespace DragonOS::Graphics
