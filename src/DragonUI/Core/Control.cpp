#include <DragonUI/Core/Control.hpp>

namespace DragonOS::DragonUI {

bool Control::OnEvent(const EventArgs& args) noexcept
{
    switch (args.type)
    {
    case EventType::MouseEnter:
    case EventType::MouseLeave:
    case EventType::MouseMove:
    case EventType::MouseDown:
    case EventType::MouseUp:
    case EventType::Click:
    case EventType::DoubleClick:
        return OnMouseEvent(args.type, args.mouse);

    case EventType::KeyDown:
    case EventType::KeyUp:
    case EventType::TextInput:
        return OnKeyEvent(args.type, args.key);

    case EventType::GotFocus:
    case EventType::LostFocus:
        return OnFocusEvent(args.focus);

    default:
        return false;
    }
}

bool Control::OnMouseEvent(EventType /*type*/, const MouseEventArgs& /*args*/) noexcept
{
    return false;
}

bool Control::OnKeyEvent(EventType /*type*/, const KeyEventArgs& /*args*/) noexcept
{
    return false;
}

bool Control::OnFocusEvent(const FocusEventArgs& /*args*/) noexcept
{
    return false;
}

} // namespace
