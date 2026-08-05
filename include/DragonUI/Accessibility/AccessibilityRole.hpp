#pragma once

#include <cstdint>
#include <string_view>

namespace DragonOS::DragonUI {

/**
 * @brief  Semantic role of an accessible UI element.
 *
 * Mirrors the roles exposed through Windows UI Automation so that a future
 * screen-reader / automation bridge can map a DragonUI element directly to
 * an MSAA or UIA control type.
 */
enum class AccessibilityRole : uint8_t {
    Unknown,
    Window,        ///< Top-level application window.
    Dialog,        ///< Modal / modeless dialog surface.
    Pane,          ///< Generic container / group of controls.
    Label,         ///< Static descriptive text.
    Button,        ///< Push button that can be invoked.
    TextBox,       ///< Editable single-line text field.
    PasswordBox,   ///< Masked text field.
    CheckBox,      ///< Two/three state check box.
    RadioButton,   ///< Mutually exclusive option.
    ToggleSwitch,  ///< On/off switch.
    Menu,          ///< Menu container.
    MenuBar,       ///< Top-level menu bar.
    MenuItem,      ///< Single menu entry.
    Tree,          ///< Tree container.
    TreeItem,      ///< Single tree node.
    List,          ///< List container.
    ListItem,      ///< Single list row / cell.
    Grid,          ///< Grid / data grid container.
    GridCell,      ///< Single grid cell.
    ComboBox,      ///< Drop-down selector.
    ProgressBar,   ///< Progress indicator.
    Slider,        ///< Range slider.
    ScrollBar,     ///< Scroll bar.
    Separator,     ///< Visual divider.
    Image,         ///< Static image / icon.
    StatusBar,     ///< Status bar.
    ToolBar,       ///< Toolbar container.
    ToolTip,       ///< Tooltip.
    Hyperlink,     ///< Navigable link.
    Group,         ///< Group box.
    Tab,           ///< Tab page.
    TabItem,       ///< Single tab.
    TitleBar,      ///< Window title bar.
    Header,        ///< Column header.
    Table,         ///< Tabular data.
    TableRow,      ///< Data row.
    Notifications, ///< Notification / toast surface.
};

/// @brief  Stable, human-readable role name (used by the inspector & tree).
[[nodiscard]] constexpr std::string_view ToString(AccessibilityRole role) noexcept
{
    using R = AccessibilityRole;
    switch (role)
    {
    case R::Window:        return "Window";
    case R::Dialog:        return "Dialog";
    case R::Pane:          return "Pane";
    case R::Label:         return "Label";
    case R::Button:        return "Button";
    case R::TextBox:       return "TextBox";
    case R::PasswordBox:   return "PasswordBox";
    case R::CheckBox:      return "CheckBox";
    case R::RadioButton:   return "RadioButton";
    case R::ToggleSwitch:  return "ToggleSwitch";
    case R::Menu:          return "Menu";
    case R::MenuBar:       return "MenuBar";
    case R::MenuItem:      return "MenuItem";
    case R::Tree:          return "Tree";
    case R::TreeItem:      return "TreeItem";
    case R::List:          return "List";
    case R::ListItem:      return "ListItem";
    case R::Grid:          return "Grid";
    case R::GridCell:      return "GridCell";
    case R::ComboBox:      return "ComboBox";
    case R::ProgressBar:   return "ProgressBar";
    case R::Slider:        return "Slider";
    case R::ScrollBar:     return "ScrollBar";
    case R::Separator:     return "Separator";
    case R::Image:         return "Image";
    case R::StatusBar:     return "StatusBar";
    case R::ToolBar:       return "ToolBar";
    case R::ToolTip:       return "ToolTip";
    case R::Hyperlink:     return "Hyperlink";
    case R::Group:         return "Group";
    case R::Tab:           return "Tab";
    case R::TabItem:       return "TabItem";
    case R::TitleBar:      return "TitleBar";
    case R::Header:        return "Header";
    case R::Table:         return "Table";
    case R::TableRow:      return "TableRow";
    case R::Notifications: return "Notifications";
    case R::Unknown:       break;
    }
    return "Unknown";
}

} // namespace DragonOS::DragonUI
