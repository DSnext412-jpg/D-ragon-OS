/**
 * @file    Keyboard.cpp
 * @brief   Implementation of the Keyboard class.
 */

#include <Input/Keyboard.hpp>

namespace DragonOS::Input {

// ============================================================================
//  Message-driven updates
// ============================================================================

void Keyboard::OnKeyDown(KeyCode key) noexcept
{
    const auto idx = KeyCodeValue(key);
    if (idx < KeyCodeCount)
    {
        m_rawState[idx] = true;
    }
}

void Keyboard::OnKeyUp(KeyCode key) noexcept
{
    const auto idx = KeyCodeValue(key);
    if (idx < KeyCodeCount)
    {
        m_rawState[idx] = false;
    }
}

// ============================================================================
//  Per-frame lifecycle
// ============================================================================

void Keyboard::Update() noexcept
{
    for (std::size_t i = 0; i < KeyCodeCount; ++i)
    {
        m_pressed[i]       =  m_rawState[i] && !m_prevRawState[i];
        m_released[i]      = !m_rawState[i] &&  m_prevRawState[i];
        m_held[i]          =  m_rawState[i];

        m_prevRawState[i]  =  m_rawState[i];
    }

    UpdateModifiers();
}

void Keyboard::Reset() noexcept
{
    for (std::size_t i = 0; i < KeyCodeCount; ++i)
    {
        m_rawState[i]      = false;
        m_prevRawState[i]  = false;
        m_pressed[i]       = false;
        m_released[i]      = false;
        m_held[i]          = false;
    }

    m_ctrlDown  = false;
    m_shiftDown = false;
    m_altDown   = false;
    m_winDown   = false;
}

// ============================================================================
//  Modifier helpers
// ============================================================================

void Keyboard::UpdateModifiers() noexcept
{
    m_ctrlDown  = m_rawState[KeyCodeValue(KeyCode::LControl)] ||
                  m_rawState[KeyCodeValue(KeyCode::RControl)] ||
                  m_rawState[KeyCodeValue(KeyCode::Control)];

    m_shiftDown = m_rawState[KeyCodeValue(KeyCode::LShift)] ||
                  m_rawState[KeyCodeValue(KeyCode::RShift)] ||
                  m_rawState[KeyCodeValue(KeyCode::Shift)];

    m_altDown   = m_rawState[KeyCodeValue(KeyCode::LAlt)] ||
                  m_rawState[KeyCodeValue(KeyCode::RAlt)] ||
                  m_rawState[KeyCodeValue(KeyCode::Alt)];

    m_winDown   = m_rawState[KeyCodeValue(KeyCode::LWin)] ||
                  m_rawState[KeyCodeValue(KeyCode::RWin)];
}

// ============================================================================
//  Queries
// ============================================================================

bool Keyboard::IsKeyPressed(KeyCode key) const noexcept
{
    const auto idx = KeyCodeValue(key);
    return (idx < KeyCodeCount) && m_pressed[idx];
}

bool Keyboard::IsKeyReleased(KeyCode key) const noexcept
{
    const auto idx = KeyCodeValue(key);
    return (idx < KeyCodeCount) && m_released[idx];
}

bool Keyboard::IsKeyHeld(KeyCode key) const noexcept
{
    const auto idx = KeyCodeValue(key);
    return (idx < KeyCodeCount) && m_held[idx];
}

} // namespace DragonOS::Input
