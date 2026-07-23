#include <UI/Core/UIEvent.hpp>

namespace DragonOS::UI {

bool UIEvent::IsMouseEvent() const noexcept
{
    switch (type)
    {
    case UIEventType::MouseEnter:
    case UIEventType::MouseLeave:
    case UIEventType::MouseMove:
    case UIEventType::MouseDown:
    case UIEventType::MouseUp:
    case UIEventType::Click:
    case UIEventType::DoubleClick:
    case UIEventType::Scroll:
        return true;
    default:
        return false;
    }
}

bool UIEvent::IsKeyboardEvent() const noexcept
{
    switch (type)
    {
    case UIEventType::KeyDown:
    case UIEventType::KeyUp:
    case UIEventType::Char:
        return true;
    default:
        return false;
    }
}

bool UIEvent::IsFocusEvent() const noexcept
{
    switch (type)
    {
    case UIEventType::FocusGained:
    case UIEventType::FocusLost:
        return true;
    default:
        return false;
    }
}

} // namespace
