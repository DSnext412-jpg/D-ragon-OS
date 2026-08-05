#pragma once

#include <Theme/Theme.hpp>

namespace DragonOS::Theme {

/**
 * @brief  Construct a high-contrast theme.
 *
 * Uses a near-black background with high-luminance text and strongly
 * differentiated accent / selection colours to maximise legibility for
 * low-vision users.  Applied by AccessibilityManager when the user enables
 * the high-contrast accessibility preference.
 */
[[nodiscard]] Theme CreateHighContrastTheme() noexcept;

} // namespace DragonOS::Theme