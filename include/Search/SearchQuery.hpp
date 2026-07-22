#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace DragonOS::Search {

struct SearchQuery final {
    std::wstring text;
    std::vector<std::wstring> tokens;
    bool searchApps{ true };
    bool searchFiles{ true };
    bool searchSettings{ true };
    bool searchCommands{ true };
    size_t maxResultsPerCategory{ 10 };
    float minRelevance{ 0.1f };
};

} // namespace DragonOS::Search
