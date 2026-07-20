#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace DragonOS::Apps {

enum class AppCategory : uint8_t {
    Uncategorized,
    Development,
    Graphics,
    Internet,
    Media,
    Games,
    Productivity,
    System,
    Utilities,
};

struct AppInfo final {
    uint64_t    id{ 0 };
    std::wstring name;
    std::wstring displayName;
    std::wstring iconPath;
    AppCategory  category{ AppCategory::Uncategorized };
    bool         pinned{ false };
    bool         hidden{ false };
    uint64_t     lastUsedTime{ 0 };
};

} // namespace DragonOS::Apps
