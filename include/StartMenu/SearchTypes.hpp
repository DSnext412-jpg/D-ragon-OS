#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <functional>

namespace DragonOS::StartMenu {

// ── SearchResult ─────────────────────────────────────────────────────────

struct SearchResult final {
    uint64_t    appId{ 0 };
    std::wstring displayName;
    std::wstring category;
    float        relevance{ 0.0f };
};

// ── SearchProvider (abstract) ────────────────────────────────────────────

class SearchProvider {
public:
    virtual ~SearchProvider() noexcept = default;

    virtual std::vector<SearchResult> Search(std::wstring_view query) noexcept = 0;
    virtual std::wstring_view GetName() const noexcept = 0;
};

// ── SearchIndex (future) ─────────────────────────────────────────────────

class SearchIndex final {
public:
    SearchIndex() noexcept = default;
    ~SearchIndex() noexcept = default;

    void Build() noexcept {}
    void Query(std::wstring_view /*query*/, std::vector<SearchResult>& /*results*/) const noexcept {}

private:
    // Future: inverted index, trie, etc.
};

// ── SearchController (future) ────────────────────────────────────────────

class SearchController final {
public:
    SearchController() noexcept = default;
    ~SearchController() noexcept = default;

    void AddProvider(std::unique_ptr<SearchProvider> provider) noexcept
    {
        m_providers.push_back(std::move(provider));
    }

    std::vector<SearchResult> ExecuteSearch(std::wstring_view query) noexcept
    {
        std::vector<SearchResult> results;
        for (auto& provider : m_providers)
        {
            auto providerResults = provider->Search(query);
            results.insert(results.end(),
                std::make_move_iterator(providerResults.begin()),
                std::make_move_iterator(providerResults.end()));
        }
        return results;
    }

private:
    std::vector<std::unique_ptr<SearchProvider>> m_providers;
};

// ── SearchBox (visual component placeholder) ─────────────────────────────

struct SearchBox final {
    std::wstring text;
    bool         isFocused{ false };
    bool         isActive{ false };
    float        animProgress{ 0.0f };
};

} // namespace DragonOS::StartMenu
