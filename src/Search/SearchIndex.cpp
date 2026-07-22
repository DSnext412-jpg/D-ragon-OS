#include <Search/SearchIndex.hpp>

#include <algorithm>
#include <cctype>
#include <cwctype>
#include <locale>

namespace DragonOS::Search {

static std::vector<std::wstring> Tokenize(std::wstring_view text) noexcept
{
    std::vector<std::wstring> tokens;
    std::wstring current;
    for (auto ch : text)
    {
        if (std::iswspace(ch) || ch == L'-' || ch == L'_' || ch == L'.')
        {
            if (!current.empty()) { tokens.push_back(current); current.clear(); }
        }
        else
        {
            current.push_back(static_cast<wchar_t>(std::towlower(ch)));
        }
    }
    if (!current.empty()) { tokens.push_back(current); }
    return tokens;
}

void SearchIndex::Add(const SearchResult& result) noexcept
{
    auto& entry = m_entries[result.id];
    entry.result = result;
    entry.keywords = Tokenize(result.displayName + L" " + result.description);
    m_dirty = true;
}

void SearchIndex::Remove(uint64_t id) noexcept
{
    m_entries.erase(id);
    m_dirty = true;
}

void SearchIndex::Clear() noexcept
{
    m_entries.clear();
    m_dirty = true;
}

void SearchIndex::Rebuild() noexcept
{
    for (auto& [id, entry] : m_entries)
    {
        entry.keywords = Tokenize(entry.result.displayName + L" " + entry.result.description);
    }
    m_dirty = false;
}

std::vector<SearchResult> SearchIndex::Query(
    std::wstring_view query, size_t maxResults) const noexcept
{
    if (query.empty()) { return {}; }

    std::vector<std::pair<float, const SearchResult*>> scored;
    const auto queryLower = [&]() {
        std::wstring q{ query };
        for (auto& ch : q) { ch = static_cast<wchar_t>(std::towlower(ch)); }
        return q;
    }();

    for (const auto& [id, entry] : m_entries)
    {
        const float relevance = ComputeRelevance(queryLower, entry);
        if (relevance > 0.0f)
        {
            scored.emplace_back(relevance, &entry.result);
        }
    }

    std::sort(scored.begin(), scored.end(),
        [](const auto& a, const auto& b) { return a.first > b.first; });

    std::vector<SearchResult> results;
    results.reserve((std::min)(scored.size(), maxResults));
    for (size_t i = 0; i < (std::min)(scored.size(), maxResults); ++i)
    {
        results.push_back(*scored[i].second);
        results.back().relevance = scored[i].first;
    }
    return results;
}

std::vector<SearchResult> SearchIndex::FuzzyQuery(
    std::wstring_view query, float threshold, size_t maxResults) const noexcept
{
    if (query.empty()) { return {}; }

    std::vector<std::pair<float, const SearchResult*>> scored;
    const auto queryLower = [&]() {
        std::wstring q{ query };
        for (auto& ch : q) { ch = static_cast<wchar_t>(std::towlower(ch)); }
        return q;
    }();

    for (const auto& [id, entry] : m_entries)
    {
        float relevance = ComputeRelevance(queryLower, entry);
        if (relevance < threshold)
        {
            relevance = 0.0f;
            for (const auto& kw : entry.keywords)
            {
                size_t matches = 0;
                size_t qPos = 0;
                for (size_t kPos = 0; kPos < kw.size() && qPos < queryLower.size(); ++kPos)
                {
                    if (kw[kPos] == queryLower[qPos])
                    {
                        ++matches;
                        ++qPos;
                    }
                }
                if (qPos == queryLower.size())
                {
                    relevance = static_cast<float>(matches) /
                        static_cast<float>((std::max)(kw.size(), queryLower.size()));
                    break;
                }
            }
        }

        if (relevance >= threshold)
        {
            scored.emplace_back(relevance, &entry.result);
        }
    }

    std::sort(scored.begin(), scored.end(),
        [](const auto& a, const auto& b) { return a.first > b.first; });

    std::vector<SearchResult> results;
    results.reserve((std::min)(scored.size(), maxResults));
    for (size_t i = 0; i < (std::min)(scored.size(), maxResults); ++i)
    {
        results.push_back(*scored[i].second);
        results.back().relevance = scored[i].first;
    }
    return results;
}

float SearchIndex::ComputeRelevance(
    std::wstring_view query, const IndexEntry& entry) const noexcept
{
    if (query.empty()) { return 0.0f; }

    float bestScore = 0.0f;

    const auto nameLower = [&]() {
        std::wstring n{ entry.result.displayName };
        for (auto& ch : n) { ch = static_cast<wchar_t>(std::towlower(ch)); }
        return n;
    }();

    if (nameLower == query) { return 1.0f; }

    if (nameLower.find(query) != std::wstring::npos)
    {
        bestScore = 0.8f;
    }

    for (const auto& kw : entry.keywords)
    {
        if (kw == query) { return 0.9f; }
        if (kw.find(query) != std::wstring::npos)
        {
            bestScore = (std::max)(bestScore, 0.6f);
        }
        if (query.find(kw) != std::wstring::npos)
        {
            bestScore = (std::max)(bestScore, 0.5f);
        }
    }

    const auto descLower = [&]() {
        std::wstring d{ entry.result.description };
        for (auto& ch : d) { ch = static_cast<wchar_t>(std::towlower(ch)); }
        return d;
    }();

    if (descLower.find(query) != std::wstring::npos)
    {
        bestScore = (std::max)(bestScore, 0.3f);
    }

    return bestScore;
}

bool SearchIndex::TokenMatch(const std::wstring& token, std::wstring_view query) const noexcept
{
    return token.find(query) != std::wstring::npos ||
           query.find(token) != std::wstring::npos;
}

} // namespace DragonOS::Search
