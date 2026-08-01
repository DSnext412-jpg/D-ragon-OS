#pragma once

#include <cstdint>
#include <string>

namespace DragonOS::DragonUI {

/**
 * @brief  Convert a Unicode code point to a UTF-16 std::wstring.
 *
 * Handles the supplementary plane (surrogate pairs) so icons such as
 * U+1F4C1 (folder) render correctly as wchar_t text.
 */
[[nodiscard]] inline std::wstring CodepointToUtf16(uint32_t cp)
{
    if (cp <= 0xFFFFu)
        return std::wstring(1, static_cast<wchar_t>(cp));
    if (cp <= 0x10FFFFu)
    {
        cp -= 0x10000u;
        const wchar_t hi = static_cast<wchar_t>(0xD800u + (cp >> 10));
        const wchar_t lo = static_cast<wchar_t>(0xDC00u + (cp & 0x3FFu));
        return { hi, lo };
    }
    return std::wstring(1, L'?');
}

} // namespace DragonOS::DragonUI
