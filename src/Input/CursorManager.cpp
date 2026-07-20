#include <Input/CursorManager.hpp>

#include <Windows.h>

namespace DragonOS::Input {

static HCURSOR GetWin32Cursor(CursorType type) noexcept
{
    switch (type)
    {
    case CursorType::Arrow:             return ::LoadCursorW(nullptr, IDC_ARROW);
    case CursorType::Hand:              return ::LoadCursorW(nullptr, IDC_HAND);
    case CursorType::ResizeHorizontal:  return ::LoadCursorW(nullptr, IDC_SIZEWE);
    case CursorType::ResizeVertical:    return ::LoadCursorW(nullptr, IDC_SIZENS);
    case CursorType::ResizeDiagonalNWSE: return ::LoadCursorW(nullptr, IDC_SIZENWSE);
    case CursorType::ResizeDiagonalNESW: return ::LoadCursorW(nullptr, IDC_SIZENESW);
    case CursorType::Text:              return ::LoadCursorW(nullptr, IDC_IBEAM);
    case CursorType::Wait:              return ::LoadCursorW(nullptr, IDC_WAIT);
    case CursorType::Cross:             return ::LoadCursorW(nullptr, IDC_CROSS);
    case CursorType::Move:              return ::LoadCursorW(nullptr, IDC_SIZEALL);
    default:                            return ::LoadCursorW(nullptr, IDC_ARROW);
    }
}

CursorManager::CursorManager() noexcept
{
    m_current = CursorType::Arrow;
}

void CursorManager::SetCursor(CursorType type) noexcept
{
    if (m_current == type) { return; }
    m_current = type;
    m_dirty   = true;
}

void CursorManager::Apply() noexcept
{
    if (!m_dirty) { return; }
    ::SetCursor(GetWin32Cursor(m_current));
    m_dirty = false;
}

}
