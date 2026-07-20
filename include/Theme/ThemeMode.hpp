/**
 * @file    ThemeMode.hpp
 * @brief   Enumeration of available colour-scheme modes.
 */

#pragma once

namespace DragonOS::Theme {

/**
 * @brief  Identifies the active colour-scheme mode.
 *
 * Modes control which palette is active.  Dark and Light are
 * concrete modes; System defers to the OS setting (future).
 */
enum class ThemeMode {
    Dark,   ///< Dark colour scheme (default).
    Light,  ///< Light colour scheme.
    System, ///< Follow OS preference (future).
};

} // namespace DragonOS::Theme
