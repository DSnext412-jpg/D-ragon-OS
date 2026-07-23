#include <UI/Core/UIStyle.hpp>
#include <UI/Core/UIElement.hpp>

namespace DragonOS::UI {

const StyleStateColors& UIStyle::ResolveState(ElementState state) const noexcept
{
    switch (state)
    {
    case ElementState::Normal:   return normal;
    case ElementState::Hover:    return hover;
    case ElementState::Pressed:  return pressed;
    case ElementState::Focused:  return focused;
    case ElementState::Disabled: return disabled;
    default:                     return normal;
    }
}

std::shared_ptr<UIStyle> UIStyle::DefaultButton() noexcept
{
    auto style = std::make_shared<UIStyle>("Button");

    style->normal.background = Theme::SemanticColor::Accent;
    style->normal.foreground = Theme::SemanticColor::TextPrimary;
    style->normal.border     = Theme::SemanticColor::Transparent;

    style->hover.background = Theme::SemanticColor::AccentHover;
    style->hover.foreground = Theme::SemanticColor::TextPrimary;
    style->hover.border     = Theme::SemanticColor::Transparent;

    style->pressed.background = Theme::SemanticColor::AccentPressed;
    style->pressed.foreground = Theme::SemanticColor::TextPrimary;
    style->pressed.border     = Theme::SemanticColor::Transparent;

    style->focused = style->normal;

    style->disabled.background = Theme::SemanticColor::Disabled;
    style->disabled.foreground = Theme::SemanticColor::TextSecondary;
    style->disabled.border     = Theme::SemanticColor::Transparent;

    style->cornerRadius    = 4.0f;
    style->borderThickness = 1.0f;
    style->paddingLeft     = 16.0f;
    style->paddingTop      = 6.0f;
    style->paddingRight    = 16.0f;
    style->paddingBottom   = 6.0f;
    style->fontSize        = 14.0f;
    style->fontBold        = false;

    return style;
}

std::shared_ptr<UIStyle> UIStyle::DefaultLabel() noexcept
{
    auto style = std::make_shared<UIStyle>("Label");

    style->normal.background = Theme::SemanticColor::Transparent;
    style->normal.foreground = Theme::SemanticColor::TextPrimary;
    style->normal.border     = Theme::SemanticColor::Transparent;

    style->hover   = style->normal;
    style->pressed = style->normal;
    style->focused = style->normal;

    style->disabled.background = Theme::SemanticColor::Transparent;
    style->disabled.foreground = Theme::SemanticColor::TextSecondary;
    style->disabled.border     = Theme::SemanticColor::Transparent;

    style->cornerRadius    = 0.0f;
    style->borderThickness = 0.0f;
    style->paddingLeft     = 0.0f;
    style->paddingTop      = 0.0f;
    style->paddingRight    = 0.0f;
    style->paddingBottom   = 0.0f;
    style->fontSize        = 14.0f;

    return style;
}

std::shared_ptr<UIStyle> UIStyle::DefaultTextBox() noexcept
{
    auto style = std::make_shared<UIStyle>("TextBox");

    style->normal.background = Theme::SemanticColor::WindowBackground;
    style->normal.foreground = Theme::SemanticColor::TextPrimary;
    style->normal.border     = Theme::SemanticColor::WindowBorder;

    style->hover   = style->normal;
    style->pressed = style->normal;

    style->focused.background = Theme::SemanticColor::WindowBackground;
    style->focused.foreground = Theme::SemanticColor::TextPrimary;
    style->focused.border     = Theme::SemanticColor::Accent;

    style->disabled.background = Theme::SemanticColor::Disabled;
    style->disabled.foreground = Theme::SemanticColor::TextSecondary;
    style->disabled.border     = Theme::SemanticColor::WindowBorder;

    style->cornerRadius    = 4.0f;
    style->borderThickness = 1.0f;
    style->paddingLeft     = 8.0f;
    style->paddingTop      = 4.0f;
    style->paddingRight    = 8.0f;
    style->paddingBottom   = 4.0f;
    style->fontSize        = 14.0f;

    return style;
}

std::shared_ptr<UIStyle> UIStyle::DefaultCheckBox() noexcept
{
    auto style = std::make_shared<UIStyle>("CheckBox");

    style->normal.background = Theme::SemanticColor::Transparent;
    style->normal.foreground = Theme::SemanticColor::TextPrimary;
    style->normal.border     = Theme::SemanticColor::WindowBorder;

    style->hover.background = Theme::SemanticColor::Hover;
    style->hover.foreground = Theme::SemanticColor::TextPrimary;
    style->hover.border     = Theme::SemanticColor::Accent;

    style->pressed.background = Theme::SemanticColor::AccentPressed;
    style->pressed.foreground = Theme::SemanticColor::TextPrimary;
    style->pressed.border     = Theme::SemanticColor::Accent;

    style->focused                    = style->hover;
    style->focused.border             = Theme::SemanticColor::Accent;

    style->disabled.background = Theme::SemanticColor::Transparent;
    style->disabled.foreground = Theme::SemanticColor::TextSecondary;
    style->disabled.border     = Theme::SemanticColor::Disabled;

    style->cornerRadius    = 3.0f;
    style->borderThickness = 1.5f;
    style->paddingLeft     = 4.0f;
    style->paddingTop      = 4.0f;
    style->paddingRight    = 4.0f;
    style->paddingBottom   = 4.0f;
    style->fontSize        = 14.0f;

    return style;
}

std::shared_ptr<UIStyle> UIStyle::DefaultToggleSwitch() noexcept
{
    auto style = std::make_shared<UIStyle>("ToggleSwitch");

    style->normal.background = Theme::SemanticColor::WindowBorder;
    style->normal.foreground = Theme::SemanticColor::TextPrimary;
    style->normal.border     = Theme::SemanticColor::Transparent;

    style->hover.background = Theme::SemanticColor::AccentHover;
    style->hover.foreground = Theme::SemanticColor::TextPrimary;
    style->hover.border     = Theme::SemanticColor::Transparent;

    style->pressed.background = Theme::SemanticColor::AccentPressed;
    style->pressed.foreground = Theme::SemanticColor::TextPrimary;
    style->pressed.border     = Theme::SemanticColor::Transparent;

    style->focused                    = style->normal;
    style->focused.border             = Theme::SemanticColor::Accent;

    style->disabled.background = Theme::SemanticColor::Disabled;
    style->disabled.foreground = Theme::SemanticColor::TextSecondary;
    style->disabled.border     = Theme::SemanticColor::Transparent;

    style->cornerRadius    = 10.0f;
    style->borderThickness = 0.0f;
    style->paddingLeft     = 0.0f;
    style->paddingTop      = 0.0f;
    style->paddingRight    = 0.0f;
    style->paddingBottom   = 0.0f;
    style->fontSize        = 14.0f;

    return style;
}

std::shared_ptr<UIStyle> UIStyle::DefaultSlider() noexcept
{
    auto style = std::make_shared<UIStyle>("Slider");

    style->normal.background = Theme::SemanticColor::Accent;
    style->normal.foreground = Theme::SemanticColor::TextPrimary;
    style->normal.border     = Theme::SemanticColor::WindowBorder;

    style->hover.background = Theme::SemanticColor::AccentHover;
    style->hover.foreground = Theme::SemanticColor::TextPrimary;
    style->hover.border     = Theme::SemanticColor::Accent;

    style->pressed.background = Theme::SemanticColor::AccentPressed;
    style->pressed.foreground = Theme::SemanticColor::TextPrimary;
    style->pressed.border     = Theme::SemanticColor::AccentPressed;

    style->focused = style->normal;

    style->disabled.background = Theme::SemanticColor::Disabled;
    style->disabled.foreground = Theme::SemanticColor::TextSecondary;
    style->disabled.border     = Theme::SemanticColor::Disabled;

    style->cornerRadius    = 2.0f;
    style->borderThickness = 0.0f;
    style->paddingLeft     = 0.0f;
    style->paddingTop      = 0.0f;
    style->paddingRight    = 0.0f;
    style->paddingBottom   = 0.0f;
    style->fontSize        = 14.0f;

    return style;
}

std::shared_ptr<UIStyle> UIStyle::DefaultProgressBar() noexcept
{
    auto style = std::make_shared<UIStyle>("ProgressBar");

    style->normal.background = Theme::SemanticColor::Accent;
    style->normal.foreground = Theme::SemanticColor::TextPrimary;
    style->normal.border     = Theme::SemanticColor::WindowBorder;

    style->hover   = style->normal;
    style->pressed = style->normal;
    style->focused = style->normal;

    style->disabled.background = Theme::SemanticColor::Disabled;
    style->disabled.foreground = Theme::SemanticColor::TextSecondary;
    style->disabled.border     = Theme::SemanticColor::Disabled;

    style->cornerRadius    = 2.0f;
    style->borderThickness = 0.0f;
    style->paddingLeft     = 0.0f;
    style->paddingTop      = 0.0f;
    style->paddingRight    = 0.0f;
    style->paddingBottom   = 0.0f;
    style->fontSize        = 12.0f;

    return style;
}

std::shared_ptr<UIStyle> UIStyle::DefaultComboBox() noexcept
{
    auto style = std::make_shared<UIStyle>("ComboBox");

    style->normal.background = Theme::SemanticColor::WindowBackground;
    style->normal.foreground = Theme::SemanticColor::TextPrimary;
    style->normal.border     = Theme::SemanticColor::WindowBorder;

    style->hover.background = Theme::SemanticColor::Hover;
    style->hover.foreground = Theme::SemanticColor::TextPrimary;
    style->hover.border     = Theme::SemanticColor::Accent;

    style->pressed.background = Theme::SemanticColor::AccentPressed;
    style->pressed.foreground = Theme::SemanticColor::TextPrimary;
    style->pressed.border     = Theme::SemanticColor::Accent;

    style->focused.background = Theme::SemanticColor::WindowBackground;
    style->focused.foreground = Theme::SemanticColor::TextPrimary;
    style->focused.border     = Theme::SemanticColor::Accent;

    style->disabled.background = Theme::SemanticColor::Disabled;
    style->disabled.foreground = Theme::SemanticColor::TextSecondary;
    style->disabled.border     = Theme::SemanticColor::Disabled;

    style->cornerRadius    = 4.0f;
    style->borderThickness = 1.0f;
    style->paddingLeft     = 8.0f;
    style->paddingTop      = 4.0f;
    style->paddingRight    = 8.0f;
    style->paddingBottom   = 4.0f;
    style->fontSize        = 14.0f;

    return style;
}

std::shared_ptr<UIStyle> UIStyle::DefaultListView() noexcept
{
    auto style = std::make_shared<UIStyle>("ListView");

    style->normal.background = Theme::SemanticColor::WindowBackground;
    style->normal.foreground = Theme::SemanticColor::TextPrimary;
    style->normal.border     = Theme::SemanticColor::WindowBorder;

    style->hover.background = Theme::SemanticColor::Hover;
    style->hover.foreground = Theme::SemanticColor::TextPrimary;
    style->hover.border     = Theme::SemanticColor::WindowBorder;

    style->pressed.background = Theme::SemanticColor::Selection;
    style->pressed.foreground = Theme::SemanticColor::TextPrimary;
    style->pressed.border     = Theme::SemanticColor::WindowBorder;

    style->focused.background = Theme::SemanticColor::WindowBackground;
    style->focused.foreground = Theme::SemanticColor::TextPrimary;
    style->focused.border     = Theme::SemanticColor::Accent;

    style->disabled.background = Theme::SemanticColor::Disabled;
    style->disabled.foreground = Theme::SemanticColor::TextSecondary;
    style->disabled.border     = Theme::SemanticColor::Disabled;

    style->cornerRadius    = 0.0f;
    style->borderThickness = 1.0f;
    style->paddingLeft     = 0.0f;
    style->paddingTop      = 0.0f;
    style->paddingRight    = 0.0f;
    style->paddingBottom   = 0.0f;
    style->fontSize        = 14.0f;

    return style;
}

std::shared_ptr<UIStyle> UIStyle::DefaultScrollViewer() noexcept
{
    auto style = std::make_shared<UIStyle>("ScrollViewer");

    style->normal.background = Theme::SemanticColor::Transparent;
    style->normal.foreground = Theme::SemanticColor::WindowBorder;
    style->normal.border     = Theme::SemanticColor::Transparent;

    style->hover.background = Theme::SemanticColor::Hover;
    style->hover.foreground = Theme::SemanticColor::WindowBorder;
    style->hover.border     = Theme::SemanticColor::Transparent;

    style->pressed.background = Theme::SemanticColor::AccentPressed;
    style->pressed.foreground = Theme::SemanticColor::Accent;
    style->pressed.border     = Theme::SemanticColor::Transparent;

    style->focused = style->normal;

    style->disabled.background = Theme::SemanticColor::Transparent;
    style->disabled.foreground = Theme::SemanticColor::Disabled;
    style->disabled.border     = Theme::SemanticColor::Transparent;

    style->cornerRadius    = 4.0f;
    style->borderThickness = 0.0f;
    style->paddingLeft     = 0.0f;
    style->paddingTop      = 0.0f;
    style->paddingRight    = 0.0f;
    style->paddingBottom   = 0.0f;
    style->fontSize        = 14.0f;

    return style;
}

std::shared_ptr<UIStyle> UIStyle::DefaultTabControl() noexcept
{
    auto style = std::make_shared<UIStyle>("TabControl");

    style->normal.background = Theme::SemanticColor::WindowBackground;
    style->normal.foreground = Theme::SemanticColor::TextPrimary;
    style->normal.border     = Theme::SemanticColor::WindowBorder;

    style->hover.background = Theme::SemanticColor::Hover;
    style->hover.foreground = Theme::SemanticColor::TextPrimary;
    style->hover.border     = Theme::SemanticColor::WindowBorder;

    style->pressed.background = Theme::SemanticColor::AccentPressed;
    style->pressed.foreground = Theme::SemanticColor::TextPrimary;
    style->pressed.border     = Theme::SemanticColor::Accent;

    style->focused.background = Theme::SemanticColor::WindowBackground;
    style->focused.foreground = Theme::SemanticColor::TextPrimary;
    style->focused.border     = Theme::SemanticColor::Accent;

    style->disabled.background = Theme::SemanticColor::Disabled;
    style->disabled.foreground = Theme::SemanticColor::TextSecondary;
    style->disabled.border     = Theme::SemanticColor::Disabled;

    style->cornerRadius    = 0.0f;
    style->borderThickness = 1.0f;
    style->paddingLeft     = 12.0f;
    style->paddingTop      = 6.0f;
    style->paddingRight    = 12.0f;
    style->paddingBottom   = 6.0f;
    style->fontSize        = 14.0f;

    return style;
}

std::shared_ptr<UIStyle> UIStyle::DefaultMenu() noexcept
{
    auto style = std::make_shared<UIStyle>("Menu");

    style->normal.background = Theme::SemanticColor::WindowBackground;
    style->normal.foreground = Theme::SemanticColor::TextPrimary;
    style->normal.border     = Theme::SemanticColor::WindowBorder;

    style->hover.background = Theme::SemanticColor::Hover;
    style->hover.foreground = Theme::SemanticColor::TextPrimary;
    style->hover.border     = Theme::SemanticColor::Transparent;

    style->pressed.background = Theme::SemanticColor::Selection;
    style->pressed.foreground = Theme::SemanticColor::TextPrimary;
    style->pressed.border     = Theme::SemanticColor::Transparent;

    style->focused = style->hover;

    style->disabled.background = Theme::SemanticColor::Transparent;
    style->disabled.foreground = Theme::SemanticColor::TextSecondary;
    style->disabled.border     = Theme::SemanticColor::Transparent;

    style->cornerRadius    = 4.0f;
    style->borderThickness = 1.0f;
    style->paddingLeft     = 12.0f;
    style->paddingTop      = 6.0f;
    style->paddingRight    = 12.0f;
    style->paddingBottom   = 6.0f;
    style->fontSize        = 14.0f;

    return style;
}

std::shared_ptr<UIStyle> UIStyle::DefaultToolbar() noexcept
{
    auto style = std::make_shared<UIStyle>("Toolbar");

    style->normal.background = Theme::SemanticColor::Transparent;
    style->normal.foreground = Theme::SemanticColor::TextPrimary;
    style->normal.border     = Theme::SemanticColor::Transparent;

    style->hover.background = Theme::SemanticColor::Hover;
    style->hover.foreground = Theme::SemanticColor::TextPrimary;
    style->hover.border     = Theme::SemanticColor::Transparent;

    style->pressed.background = Theme::SemanticColor::AccentPressed;
    style->pressed.foreground = Theme::SemanticColor::TextPrimary;
    style->pressed.border     = Theme::SemanticColor::Transparent;

    style->focused = style->hover;

    style->disabled.background = Theme::SemanticColor::Transparent;
    style->disabled.foreground = Theme::SemanticColor::TextSecondary;
    style->disabled.border     = Theme::SemanticColor::Transparent;

    style->cornerRadius    = 0.0f;
    style->borderThickness = 0.0f;
    style->paddingLeft     = 4.0f;
    style->paddingTop      = 4.0f;
    style->paddingRight    = 4.0f;
    style->paddingBottom   = 4.0f;
    style->fontSize        = 14.0f;

    return style;
}

std::shared_ptr<UIStyle> UIStyle::DefaultStatusBar() noexcept
{
    auto style = std::make_shared<UIStyle>("StatusBar");

    style->normal.background = Theme::SemanticColor::WindowBackground;
    style->normal.foreground = Theme::SemanticColor::TextSecondary;
    style->normal.border     = Theme::SemanticColor::WindowBorder;

    style->hover   = style->normal;
    style->pressed = style->normal;
    style->focused = style->normal;

    style->disabled = style->normal;

    style->cornerRadius    = 0.0f;
    style->borderThickness = 1.0f;
    style->paddingLeft     = 8.0f;
    style->paddingTop      = 4.0f;
    style->paddingRight    = 8.0f;
    style->paddingBottom   = 4.0f;
    style->fontSize        = 12.0f;

    return style;
}

} // namespace
