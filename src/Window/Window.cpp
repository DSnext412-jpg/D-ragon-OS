/**
 * @file    Window.cpp
 * @brief   Implementation of the Window class.
 */

#include <Engine/Engine.hpp>
#include <Graphics/Color.hpp>
#include <Input/InputManager.hpp>
#include <Window/Window.hpp>

#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <exception>
#include <iterator>
#include <stdexcept>
#include <string>

// ── Debug logging (compiled out in Release builds) ──────────────────────────

#ifdef _DEBUG
#define WND_LOG(msg)  ::OutputDebugStringW(L"[DragonOS] " msg L"\n")
#define WND_LOGF(...)                                                         \
    do {                                                                      \
        wchar_t _buf[512];                                                    \
        ::swprintf_s(_buf, __VA_ARGS__);                                      \
        ::OutputDebugStringW(L"[DragonOS] ");                                 \
        ::OutputDebugStringW(_buf);                                           \
        ::OutputDebugStringW(L"\n");                                          \
    } while (false)
#else
#define WND_LOG(msg)  ((void)0)
#define WND_LOGF(...) ((void)0)
#endif

namespace DragonOS::Window {

namespace {

// ============================================================================
//  Win32 error reporting helper
// ============================================================================

[[nodiscard]] std::string FormatWin32Error(
    DWORD       errorCode,
    const char* functionName,
    const char* sourceFile,
    int         sourceLine) noexcept
{
    wchar_t systemBuf[4096]{};

    const DWORD fmtResult = ::FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        systemBuf,
        static_cast<DWORD>(std::size(systemBuf)),
        nullptr);

    if (fmtResult == 0)
    {
        ::swprintf_s(systemBuf, L"(FormatMessageW failed for error code %lu)", errorCode);
    }
    else
    {
        // Strip trailing CR/LF that FormatMessageW appends.
        auto len = wcslen(systemBuf);
        while (len > 0 && (systemBuf[len - 1] == L'\r' || systemBuf[len - 1] == L'\n' || systemBuf[len - 1] == L'.'))
        {
            systemBuf[--len] = L'\0';
        }
    }

    // Convert wide system message to narrow for the exception.
    char narrowBuf[4096]{};
    ::wcstombs_s(nullptr, narrowBuf, systemBuf, std::size(narrowBuf) - 1);

    char message[4096]{};
    ::sprintf_s(message,
        "%s failed\n"
        "Error Code: %lu\n"
        "Message:    %s\n"
        "File:       %s\n"
        "Line:       %d",
        functionName, errorCode, narrowBuf, sourceFile, sourceLine);

    return std::string{ message };
}

} // anonymous namespace

// ============================================================================
//  Construction / Destruction
// ============================================================================

Window::Window(
    HINSTANCE            hInstance,
    std::wstring         className,
    const std::wstring&  title,
    int                  width,
    int                  height)
    : m_hInstance{ hInstance }
    , m_className{ std::move(className) }
{
    // ── Validate instance handle ───────────────────────────────────────────
    if (!m_hInstance)
    {
        throw std::runtime_error{
            "Window: hInstance is null — cannot register window class."
        };
    }

    // ── Register window class ──────────────────────────────────────────────
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(WNDCLASSEXW);
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = m_hInstance;
    wc.hIcon         = ::LoadIconW(nullptr, IDI_APPLICATION);
    wc.hCursor       = ::LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(::GetStockObject(WHITE_BRUSH));
    wc.lpszClassName = m_className.c_str();

    const ATOM classAtom = ::RegisterClassExW(&wc);

    if (classAtom == 0)
    {
        const DWORD err = ::GetLastError();
        const std::string details = FormatWin32Error(
            err, "RegisterClassExW", __FILE__, __LINE__);
        throw std::runtime_error{ details };
    }

    // ── Validate registration atom ─────────────────────────────────────────
    // ATOM should be non-zero (success).  If zero we already threw above.

    // ── Pre-validation before CreateWindowExW ──────────────────────────────
    // Verify that the registered class can be found with the same parameters.
    {
        // Validate class name.
        if (m_className.empty())
        {
            ::UnregisterClassW(m_className.c_str(), m_hInstance);
            throw std::runtime_error{
                "Window: class name is empty — cannot create window."
            };
        }

        // Validate width/height.
        if (width <= 0 || height <= 0)
        {
            ::UnregisterClassW(m_className.c_str(), m_hInstance);
            throw std::runtime_error{
                "Window: invalid width or height (must be > 0)."
            };
        }

        // Validate window styles — WS_OVERLAPPEDWINDOW is a known valid style.
    }

    // ── Create native window ───────────────────────────────────────────────
    // Pass 'this' as lpParam so WndProc can retrieve the instance during
    // WM_NCCREATE and store it in the window's GWLP_USERDATA.
    m_hWnd = ::CreateWindowExW(
        0,                      // dwExStyle
        m_className.c_str(),    // lpClassName
        title.c_str(),          // lpWindowName
        WS_OVERLAPPEDWINDOW,    // dwStyle
        CW_USEDEFAULT,          // X
        CW_USEDEFAULT,          // Y
        width,                  // nWidth
        height,                 // nHeight
        nullptr,                // hWndParent
        nullptr,                // hMenu
        m_hInstance,            // hInstance
        this);                  // lpCreateParams

    if (!m_hWnd)
    {
        const DWORD err = ::GetLastError();
        const std::string details = FormatWin32Error(
            err, "CreateWindowExW", __FILE__, __LINE__);

        ::UnregisterClassW(m_className.c_str(), m_hInstance);

        throw std::runtime_error{ details };
    }

    // ── Initialise the Direct2D renderer ───────────────────────────────────
    if (!m_renderer.Initialize(m_hWnd))
    {
        ::DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
        ::UnregisterClassW(m_className.c_str(), m_hInstance);
        throw std::runtime_error{ "Window: Failed to initialise renderer." };
    }

    // ── Initialise the desktop manager ─────────────────────────────────────
    if (!m_desktopManager.Initialize())
    {
        ::DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
        ::UnregisterClassW(m_className.c_str(), m_hInstance);
        throw std::runtime_error{ "Window: Failed to initialise desktop." };
    }

    // ── Store client dimensions ────────────────────────────────────────────
    m_width  = width;
    m_height = height;
}

Window::~Window() noexcept
{
    Close();

    if (!m_className.empty())
    {
        ::UnregisterClassW(m_className.c_str(), m_hInstance);
    }
}

// ============================================================================
//  Public interface
// ============================================================================

void Window::Show(int nCmdShow) noexcept
{
    if (m_hWnd)
    {
        ::ShowWindow(m_hWnd, nCmdShow);
    }
}

void Window::Update() noexcept
{
    if (m_hWnd)
    {
        ::UpdateWindow(m_hWnd);
    }
}

void Window::Close() noexcept
{
    if (m_hWnd && ::IsWindow(m_hWnd))
    {
        ::DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
}

HWND Window::Handle() const noexcept
{
    return m_hWnd;
}

// ============================================================================
//  Static window procedure
// ============================================================================

LRESULT CALLBACK Window::WndProc(
    HWND   hWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam) noexcept
{
    // On the very first message (WM_NCCREATE), attach the Window instance
    // to the window's user-data slot so subsequent calls can find it.
    // Note: m_hWnd is still nullptr at this point because CreateWindowExW
    // has not yet returned the HWND.  We must use the 'hWnd' parameter
    // for DefWindowProcW and NOT forward through HandleMessage (which
    // would call DefWindowProcW(m_hWnd, ...) with a null handle).
    if (uMsg == WM_NCCREATE)
    {
        auto* pcs  = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto* pWin = static_cast<Window*>(pcs->lpCreateParams);

        ::SetWindowLongPtrW(hWnd, GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(pWin));

        return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    // Retrieve the instance pointer and forward the message.
    auto* pWindow = reinterpret_cast<Window*>(
        ::GetWindowLongPtrW(hWnd, GWLP_USERDATA));

    if (pWindow)
    {
        return pWindow->HandleMessage(uMsg, wParam, lParam);
    }

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

// ============================================================================
//  Instance message dispatcher
// ============================================================================

LRESULT Window::HandleMessage(
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam) noexcept
{
    switch (uMsg)
    {
    // ── Core window messages ─────────────────────────────────────────────
    case WM_CLOSE:       OnClose();                                return 0;
    case WM_DESTROY:     OnDestroy();                              return 0;
    case WM_PAINT:       OnPaint();                                return 0;
    case WM_SIZE:        OnResize(wParam, lParam);                 return 0;
    case WM_MOVE:        OnMove(lParam);
                         return ::DefWindowProcW(m_hWnd, uMsg, wParam, lParam);
    case WM_ACTIVATE:    OnActivate(wParam);
                         return ::DefWindowProcW(m_hWnd, uMsg, wParam, lParam);
    case WM_SETFOCUS:    OnSetFocus();
                         return ::DefWindowProcW(m_hWnd, uMsg, wParam, lParam);
    case WM_KILLFOCUS:   OnKillFocus();
                         return ::DefWindowProcW(m_hWnd, uMsg, wParam, lParam);
    // WM_DPICHANGED: the suggested rect in lParam must be applied
    // explicitly; DefWindowProcW does not do it automatically.
    case WM_DPICHANGED:  OnDpiChanged(wParam, lParam);           return 0;

    // ── Mouse tracking ───────────────────────────────────────────────────
    case WM_MOUSELEAVE:  OnMouseLeave();                          return 0;

    // ── Cursor routing ───────────────────────────────────────────────────
    case WM_SETCURSOR:   return OnSetCursor(m_hWnd, wParam, lParam);

    // ── Input messages → forwarded to InputManager ──────────────────────
    case WM_MOUSEMOVE:
    {
        // Request mouse-leave notification so we know when the cursor
        // exits the client area (enables hover-exit detection).
        TRACKMOUSEEVENT tme{};
        tme.cbSize    = sizeof(tme);
        tme.dwFlags   = TME_LEAVE;
        tme.hwndTrack = m_hWnd;
        ::TrackMouseEvent(&tme);

        if (m_pInputManager)
        {
            m_pInputManager->HandleWin32Message(uMsg, wParam, lParam);
        }
        return 0;
    }

    case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN: case WM_XBUTTONUP: case WM_XBUTTONDBLCLK:
    case WM_MOUSEWHEEL:
    case WM_KEYDOWN:  case WM_KEYUP:
    case WM_SYSKEYDOWN: case WM_SYSKEYUP:
    case WM_CHAR:
        if (m_pInputManager)
        {
            m_pInputManager->HandleWin32Message(uMsg, wParam, lParam);
        }
        return 0;

    default:
        return ::DefWindowProcW(m_hWnd, uMsg, wParam, lParam);
    }
}

// ============================================================================
//  Message handlers
// ============================================================================

void Window::OnClose() noexcept
{
    ::DestroyWindow(m_hWnd);
    m_hWnd = nullptr;
}

void Window::OnDestroy() noexcept
{
    ::PostQuitMessage(0);
}

void Window::OnPaint() noexcept
{
    PAINTSTRUCT ps{};
    ::BeginPaint(m_hWnd, &ps);

    if (m_pEngine)
    {
        m_pEngine->Render();
    }

    ::EndPaint(m_hWnd, &ps);
}

void Window::OnResize(WPARAM wParam, LPARAM lParam) noexcept
{
    const UINT cx = LOWORD(lParam);
    const UINT cy = HIWORD(lParam);

    // ── Track minimise / restore state ──────────────────────────────────────
    switch (wParam)
    {
    case SIZE_MINIMIZED:
        m_isMinimized = true;
        WND_LOG(L"Window minimised");
        break;

    case SIZE_MAXIMIZED:
    case SIZE_RESTORED:
    {
        const bool wasMinimized = m_isMinimized;
        m_isMinimized = false;

        if (cx > 0 && cy > 0)
        {
            m_renderer.Resize(cx, cy);

            if (m_pEngine)
            {
                m_pEngine->Resize(
                    static_cast<float>(cx),
                    static_cast<float>(cy));
            }

            m_width  = cx;
            m_height = cy;
        }

        if (wasMinimized)
        {
            WND_LOGF(L"Window restored (%s)",
                     (wParam == SIZE_MAXIMIZED) ? L"maximised" : L"normal");
            // Force a redraw after restore.
            ::InvalidateRect(m_hWnd, nullptr, FALSE);
        }
        break;
    }
    }
}

void Window::OnMove(LPARAM lParam) noexcept
{
    const auto x = static_cast<int>(static_cast<short>(LOWORD(lParam)));
    const auto y = static_cast<int>(static_cast<short>(HIWORD(lParam)));
    WND_LOGF(L"Window moved to (%d, %d)", x, y);
    (void)x; (void)y;
}

void Window::OnActivate(WPARAM wParam) noexcept
{
    const bool active = (LOWORD(wParam) != WA_INACTIVE);
    m_isActive = active;

    WND_LOGF(L"Window %s", active ? L"activated" : L"deactivated");
}

void Window::OnSetFocus() noexcept
{
    WND_LOG(L"Window gained focus");
}

void Window::OnKillFocus() noexcept
{
    WND_LOG(L"Window lost focus");
}

void Window::OnDpiChanged(WPARAM wParam, LPARAM lParam) noexcept
{
    const auto dpi = HIWORD(wParam);
    WND_LOGF(L"DPI changed to %d", dpi);
    (void)dpi;

    auto* rect = reinterpret_cast<RECT*>(lParam);
    ::SetWindowPos(m_hWnd, nullptr,
                   rect->left, rect->top,
                   rect->right - rect->left,
                   rect->bottom - rect->top,
                   SWP_NOZORDER | SWP_NOACTIVATE);
}

void Window::OnMouseLeave() noexcept
{
    if (m_pInputManager)
    {
        m_pInputManager->OnMouseLeave();
    }
}

LRESULT Window::OnSetCursor(HWND hWnd, WPARAM wParam, LPARAM lParam) noexcept
{
    // Allow the InputManager's CursorManager to control the cursor.
    // We pass through to DefWindowProcW for default cursor handling
    // when not in the client area (HTNOWHERE, etc.).
    if (LOWORD(lParam) == HTCLIENT)
    {
        if (m_pInputManager)
        {
            m_pInputManager->GetCursorManager().Apply();
        }
        return TRUE;
    }

    return ::DefWindowProcW(hWnd, WM_SETCURSOR, wParam, lParam);
}

} // namespace DragonOS::Window
