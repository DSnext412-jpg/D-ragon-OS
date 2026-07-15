/**
 * @file    Mouse.cpp
 * @brief   Implementation of the Mouse class.
 */

#include <Input/Mouse.hpp>

namespace DragonOS::Input {

// ============================================================================
//  Message-driven updates
// ============================================================================

void Mouse::OnMove(float x, float y) noexcept
{
    m_deltaX  += x - m_x;
    m_deltaY  += y - m_y;
    m_x        = x;
    m_y        = y;
}

void Mouse::OnButtonDown(MouseButton button) noexcept
{
    const auto idx = MouseButtonIndex(button);
    if (idx < MouseButtonCount)
    {
        m_rawState[idx] = true;
    }
}

void Mouse::OnButtonUp(MouseButton button) noexcept
{
    const auto idx = MouseButtonIndex(button);
    if (idx < MouseButtonCount)
    {
        m_rawState[idx] = false;
    }
}

void Mouse::OnDoubleClick(MouseButton button) noexcept
{
    const auto idx = MouseButtonIndex(button);
    if (idx < MouseButtonCount)
    {
        m_doubleClicked[idx] = true;
        m_rawState[idx]      = true;
    }
}

void Mouse::OnWheel(float delta) noexcept
{
    m_wheelDelta   += delta;
    m_wheelUpdated  = true;
}

// ============================================================================
//  Per-frame lifecycle
// ============================================================================

void Mouse::Update() noexcept
{
    for (std::size_t i = 0; i < MouseButtonCount; ++i)
    {
        m_pressed[i]       =  m_rawState[i] && !m_prevRawState[i];
        m_released[i]      = !m_rawState[i] &&  m_prevRawState[i];
        m_held[i]          =  m_rawState[i];

        m_prevRawState[i]  =  m_rawState[i];
    }

    // Accumulated deltas are reported once per frame, then reset.
    m_deltaX      = 0.0f;
    m_deltaY      = 0.0f;
    m_wheelDelta  = 0.0f;
    m_wheelUpdated = false;
}

void Mouse::Reset() noexcept
{
    m_x = m_y = 0.0f;
    m_deltaX = m_deltaY = 0.0f;
    m_wheelDelta = 0.0f;
    m_wheelUpdated = false;

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

// ============================================================================
//  Queries
// ============================================================================

bool Mouse::IsButtonPressed(MouseButton button) const noexcept
{
    const auto idx = MouseButtonIndex(button);
    return (idx < MouseButtonCount) && m_pressed[idx];
}

bool Mouse::IsButtonReleased(MouseButton button) const noexcept
{
    const auto idx = MouseButtonIndex(button);
    return (idx < MouseButtonCount) && m_released[idx];
}

bool Mouse::IsButtonHeld(MouseButton button) const noexcept
{
    const auto idx = MouseButtonIndex(button);
    return (idx < MouseButtonCount) && m_held[idx];
}

bool Mouse::IsButtonDoubleClicked(MouseButton button) const noexcept
{
    const auto idx = MouseButtonIndex(button);
    return (idx < MouseButtonCount) && m_doubleClicked[idx];
}

} // namespace DragonOS::Input
