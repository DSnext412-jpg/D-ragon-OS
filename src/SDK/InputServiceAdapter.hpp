#pragma once

#include <DragonOS/Input.hpp>

#include <Input/InputManager.hpp>

namespace DragonOS::SDK {

class InputServiceAdapter final : public dragonos::sdk::IInputService {
public:
    explicit InputServiceAdapter(
        Input::InputManager& mgr) noexcept
        : m_inputManager{ mgr }
    {
    }

    dragonos::sdk::MouseState GetMouseState() const noexcept override;
    bool IsKeyDown(dragonos::sdk::KeyCode key) const noexcept override;
    bool WasKeyPressed(dragonos::sdk::KeyCode key) const noexcept override;
    bool WasKeyReleased(dragonos::sdk::KeyCode key) const noexcept override;
    size_t GetEventCount() const noexcept override;
    dragonos::sdk::InputEventData GetEvent(size_t index) const noexcept override;

private:
    Input::InputManager& m_inputManager;
};

} // namespace DragonOS::SDK
