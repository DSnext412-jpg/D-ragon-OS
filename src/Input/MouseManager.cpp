#include <Input/MouseManager.hpp>

namespace DragonOS::Input {

void MouseManager::OnMove(float x, float y) noexcept
{
    m_prevX   = m_x;
    m_prevY   = m_y;
    m_deltaX  += x - m_x;
    m_deltaY  += y - m_y;
    m_x        = x;
    m_y        = y;
    m_inClient = true;

    if (!m_isDragging)
    {
        for (std::size_t i = 0; i < MouseButtonCount; ++i)
        {
            if (m_rawState[i])
            {
                const float dx = m_x - m_dragStartX;
                const float dy = m_y - m_dragStartY;
                if (dx * dx + dy * dy >= DragThreshold * DragThreshold)
                {
                    m_isDragging = true;
                    break;
                }
            }
        }
    }
}

void MouseManager::OnButtonDown(MouseButton button) noexcept
{
    const auto idx = MouseButtonIndex(button);
    if (idx < MouseButtonCount)
    {
        m_rawState[idx] = true;
        m_dragStartX    = m_x;
        m_dragStartY    = m_y;
        m_dragButton    = button;
    }
}

void MouseManager::OnButtonUp(MouseButton button) noexcept
{
    const auto idx = MouseButtonIndex(button);
    if (idx < MouseButtonCount)
    {
        m_rawState[idx] = false;
        m_isDragging    = false;
    }
}

void MouseManager::OnDoubleClick(MouseButton button) noexcept
{
    const auto idx = MouseButtonIndex(button);
    if (idx < MouseButtonCount)
    {
        m_doubleClicked[idx] = true;
        m_rawState[idx]      = true;
        m_dragStartX         = m_x;
        m_dragStartY         = m_y;
        m_dragButton         = button;
    }
}

void MouseManager::OnWheel(float delta) noexcept
{
    m_wheelDelta  += delta;
    m_wheelUpdated = true;
}

void MouseManager::OnLeave() noexcept
{
    m_inClient = false;
    m_isDragging = false;
    m_prevX = m_x;
    m_prevY = m_y;
}

void MouseManager::Update() noexcept
{
    for (std::size_t i = 0; i < MouseButtonCount; ++i)
    {
        m_pressed[i]       =  m_rawState[i] && !m_prevRawState[i];
        m_released[i]      = !m_rawState[i] &&  m_prevRawState[i];
        m_held[i]          =  m_rawState[i];
        m_prevRawState[i]  =  m_rawState[i];
    }

    m_deltaX      = 0.0f;
    m_deltaY      = 0.0f;
    m_wheelDelta  = 0.0f;
    m_wheelUpdated = false;
}

void MouseManager::Reset() noexcept
{
    m_x = m_y = 0.0f;
    m_prevX = m_prevY = 0.0f;
    m_deltaX = m_deltaY = 0.0f;
    m_wheelDelta = 0.0f;
    m_inClient = false;
    m_isDragging = false;

    for (std::size_t i = 0; i < MouseButtonCount; ++i)
    {
        m_rawState[i]      = false;
        m_prevRawState[i]  = false;
        m_pressed[i]       = false;
        m_released[i]      = false;
        m_held[i]          = false;
        m_doubleClicked[i] = false;
    }
}

bool MouseManager::IsPressed(MouseButton button) const noexcept
{
    const auto idx = MouseButtonIndex(button);
    return (idx < MouseButtonCount) && m_pressed[idx];
}

bool MouseManager::WasClicked(MouseButton button) const noexcept
{
    const auto idx = MouseButtonIndex(button);
    return (idx < MouseButtonCount) && m_pressed[idx] && !m_isDragging;
}

bool MouseManager::WasReleased(MouseButton button) const noexcept
{
    const auto idx = MouseButtonIndex(button);
    return (idx < MouseButtonCount) && m_released[idx];
}

bool MouseManager::IsHeld(MouseButton button) const noexcept
{
    const auto idx = MouseButtonIndex(button);
    return (idx < MouseButtonCount) && m_held[idx];
}

bool MouseManager::WasDoubleClicked(MouseButton button) const noexcept
{
    const auto idx = MouseButtonIndex(button);
    return (idx < MouseButtonCount) && m_doubleClicked[idx];
}

}
