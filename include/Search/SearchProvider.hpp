#pragma once

#include <Search/SearchQuery.hpp>
#include <Search/SearchResult.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace DragonOS::Search {

class SearchProvider {
public:
    virtual ~SearchProvider() noexcept = default;

    virtual std::wstring_view GetName() const noexcept = 0;
    virtual std::vector<SearchResult> Search(const SearchQuery& query) noexcept = 0;
    virtual void OnActivate(const SearchResult& result) noexcept = 0;
};

using SearchProviderPtr = std::unique_ptr<SearchProvider>;

} // namespace DragonOS::Search
