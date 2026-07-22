#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace DragonOS::Search {

enum class SearchResultCategory : uint8_t {
    Application,
    File,
    Settings,
    Command,
    Unknown,
};

struct SearchResult final {
    uint64_t id{ 0 };
    std::wstring displayName;
    std::wstring description;
    std::wstring iconHint;
    std::wstring actionData;      ///< Contextual data for the action (path, app id, etc.)
    SearchResultCategory category{ SearchResultCategory::Unknown };
    float relevance{ 0.0f };
};

} // namespace DragonOS::Search
