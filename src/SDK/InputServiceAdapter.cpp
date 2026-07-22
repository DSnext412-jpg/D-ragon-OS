#include "InputServiceAdapter.hpp"

#include <Input/Keyboard.hpp>
#include <Input/Mouse.hpp>

namespace DragonOS::SDK {

dragonos::sdk::MouseState InputServiceAdapter::GetMouseState() const noexcept
{
    dragonos::sdk::MouseState state;
    state.x = m_inputManager.GetMouseX();
    state.y = m_inputManager.GetMouseY();
    state.deltaX = m_inputManager.GetMouseDeltaX();
    state.deltaY = m_inputManager.GetMouseDeltaY();
    state.wheelDelta = m_inputManager.GetWheelDelta();
    state.leftPressed = m_inputManager.IsMousePressed(DragonOS::Input::MouseButton::Left);
    state.rightPressed = m_inputManager.IsMousePressed(DragonOS::Input::MouseButton::Right);
    state.leftClicked = m_inputManager.IsMousePressed(DragonOS::Input::MouseButton::Left);
    state.rightClicked = m_inputManager.IsMousePressed(DragonOS::Input::MouseButton::Right);
    return state;
}

bool InputServiceAdapter::IsKeyDown(
    dragonos::sdk::KeyCode key) const noexcept
{
    return m_inputManager.GetKeyboard().IsKeyHeld(
        static_cast<DragonOS::Input::KeyCode>(key));
}

bool InputServiceAdapter::WasKeyPressed(
    dragonos::sdk::KeyCode key) const noexcept
{
    return m_inputManager.IsKeyPressed(
        static_cast<DragonOS::Input::KeyCode>(key));
}

bool InputServiceAdapter::WasKeyReleased(
    dragonos::sdk::KeyCode key) const noexcept
{
    return m_inputManager.IsKeyReleased(
        static_cast<DragonOS::Input::KeyCode>(key));
}

size_t InputServiceAdapter::GetEventCount() const noexcept
{
    return m_inputManager.GetEvents().size();
}

dragonos::sdk::InputEventData InputServiceAdapter::GetEvent(size_t index) const noexcept
{
    const auto& internal = m_inputManager.GetEvents()[index];
    dragonos::sdk::InputEventData result;
    switch (internal.type)
    {
    case DragonOS::Input::EventType::KeyDown:
        result.type = dragonos::sdk::InputEventType::KeyDown;
        result.key = static_cast<dragonos::sdk::KeyCode>(internal.data.key.key);
        break;
    case DragonOS::Input::EventType::KeyUp:
        result.type = dragonos::sdk::InputEventType::KeyUp;
        result.key = static_cast<dragonos::sdk::KeyCode>(internal.data.key.key);
        break;
    case DragonOS::Input::EventType::MouseMove:
        result.type = dragonos::sdk::InputEventType::MouseMove;
        result.mouseX = internal.data.mouseMove.x;
        result.mouseY = internal.data.mouseMove.y;
        break;
    case DragonOS::Input::EventType::MouseDown:
        result.type = dragonos::sdk::InputEventType::MouseDown;
        result.mouseX = internal.data.mouseButton.x;
        result.mouseY = internal.data.mouseButton.y;
        break;
    case DragonOS::Input::EventType::MouseUp:
        result.type = dragonos::sdk::InputEventType::MouseUp;
        result.mouseX = internal.data.mouseButton.x;
        result.mouseY = internal.data.mouseButton.y;
        break;
    case DragonOS::Input::EventType::MouseWheel:
        result.type = dragonos::sdk::InputEventType::MouseWheel;
        result.mouseX = internal.data.mouseWheel.x;
        result.mouseY = internal.data.mouseWheel.y;
        break;
    case DragonOS::Input::EventType::CharacterInput:
        result.type = dragonos::sdk::InputEventType::Char;
        result.character = internal.data.character.character;
        break;
    default:
        break;
    }
    return result;
}

} // namespace DragonOS::SDK
