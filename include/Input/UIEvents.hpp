#pragma once

#include <Input/MouseButtons.hpp>
#include <Input/HitTest.hpp>

#include <cstdint>

namespace DragonOS::Input {

enum class UIEventType : uint8_t {
    None,
    MouseEnter,
    MouseExit,
    MouseMove,
    MouseDown,
    MouseUp,
    MouseClick,
    MouseDoubleClick,
    FocusGained,
    FocusLost,
};

struct UIEvent {
    UIEventType  type{ UIEventType::None };
    float        x{ 0.0f };
    float        y{ 0.0f };
    MouseButton  button{ MouseButton::Left };
    HitTestRegion region{ HitTestRegion::None };
    uint64_t     targetId{ 0 };
};

}
