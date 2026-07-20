/**
 * @file    Config.hpp
 * @brief   Application-wide compile-time constants.
 *
 * All tunable values such as window dimensions, version numbers, and
 * string identifiers live here so they can be changed in one place.
 */

#pragma once

#include <cstdint>
#include <string_view>

namespace DragonOS::Config {

// ── Identity ───────────────────────────────────────────────────────────────

/// @brief  Display name of the application.
inline constexpr std::wstring_view ApplicationName = L"DragonOS";

/// @brief  Win32 window class name registered for the main window.
inline constexpr std::wstring_view WindowClassName = L"DragonOSWindow";

/// @brief  Text shown in the main window title bar.
inline constexpr std::wstring_view WindowTitle     = L"DragonOS";

// ── Version ─────────────────────────────────────────────────────────────────

inline constexpr std::uint32_t VersionMajor = 0;   ///< Major version number.
inline constexpr std::uint32_t VersionMinor = 1;   ///< Minor version number.
inline constexpr std::uint32_t VersionPatch = 0;   ///< Patch version number.

// ── Default window geometry ─────────────────────────────────────────────────

inline constexpr int DefaultWindowWidth  = 1280;   ///< Default client width.
inline constexpr int DefaultWindowHeight = 720;    ///< Default client height.

} // namespace DragonOS::Config
