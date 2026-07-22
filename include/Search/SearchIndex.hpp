#pragma once

#include <Search/SearchResult.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace DragonOS::Search {

class SearchIndex final {
public:
    SearchIndex() noexcept = default;
    ~SearchIndex() noexcept = default;

    SearchIndex(const SearchIndex&) = delete;
    SearchIndex& operator=(const SearchIndex&) = delete;
    SearchIndex(SearchIndex&&) = delete;
    SearchIndex& operator=(SearchIndex&&) = delete;

    void Add(const SearchResult& result) noexcept;
    void Remove(uint64_t id) noexcept;
    void Clear() noexcept;
    void Rebuild() noexcept;

    std::vector<SearchResult> Query(std::wstring_view query, size_t maxResults = 10) const noexcept;
    std::vector<SearchResult> FuzzyQuery(std::wstring_view query, float threshold = 0.3f, size_t maxResults = 10) const noexcept;

    size_t GetCount() const noexcept { return m_entries.size(); }
    bool IsDirty() const noexcept { return m_dirty; }

private:
    struct IndexEntry {
        SearchResult result;
        std::vector<std::wstring> keywords;
    };

    float ComputeRelevance(std::wstring_view query, const IndexEntry& entry) const noexcept;
    bool TokenMatch(const std::wstring& token, std::wstring_view query) const noexcept;

    std::unordered_map<uint64_t, IndexEntry> m_entries;
    bool m_dirty{ false };
};

} // namespace DragonOS::Search
