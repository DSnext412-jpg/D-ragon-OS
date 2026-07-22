#include <Search/SearchService.hpp>

#include <Apps/ApplicationRegistry.hpp>
#include <FileSystem/FileSystemService.hpp>
#include <Command/CommandRegistry.hpp>
#include <Command/Command.hpp>

#include <algorithm>
#include <cctype>
#include <cwctype>

namespace DragonOS::Search {

bool SearchService::Initialize(Engine::EngineContext& /*ctx*/) noexcept
{
    if (m_initialized) { return true; }
    m_initialized = true;
    return true;
}

void SearchService::Shutdown() noexcept
{
    if (!m_initialized) { return; }

    {
        std::lock_guard<std::mutex> lock(m_providersMutex);
        m_providers.clear();
    }
    {
        std::lock_guard<std::mutex> lock(m_pendingMutex);
        m_pendingSearches.clear();
    }
    m_index.Clear();
    m_initialized = false;
}

void SearchService::Update(float deltaTime) noexcept
{
    if (!m_initialized) { return; }

    std::vector<PendingSearch> pending;
    {
        std::lock_guard<std::mutex> lock(m_pendingMutex);
        pending.swap(m_pendingSearches);
    }

    for (auto& ps : pending)
    {
        RunProviders(ps.query, std::move(ps.callback));
    }

    if (m_index.IsDirty())
    {
        m_index.Rebuild();
    }
}

void SearchService::Render(Engine::EngineContext& /*ctx*/) noexcept
{
}

void SearchService::Resize(float /*width*/, float /*height*/) noexcept
{
}

void SearchService::RegisterProvider(SearchProviderPtr provider) noexcept
{
    if (!provider) { return; }
    std::lock_guard<std::mutex> lock(m_providersMutex);
    m_providers.push_back(std::move(provider));
}

void SearchService::SearchAsync(SearchQuery query, SearchCallback callback) noexcept
{
    if (!m_initialized) { return; }

    std::lock_guard<std::mutex> lock(m_pendingMutex);
    m_pendingSearches.push_back({ std::move(query), std::move(callback) });
}

std::vector<SearchResult> SearchService::SearchSync(const SearchQuery& query) noexcept
{
    std::vector<SearchResult> allResults;
    std::lock_guard<std::mutex> lock(m_providersMutex);

    for (auto& provider : m_providers)
    {
        auto results = provider->Search(query);
        allResults.insert(allResults.end(),
            std::make_move_iterator(results.begin()),
            std::make_move_iterator(results.end()));
    }

    auto indexResults = m_index.Query(query.text, query.maxResultsPerCategory);
    allResults.insert(allResults.end(),
        std::make_move_iterator(indexResults.begin()),
        std::make_move_iterator(indexResults.end()));

    std::sort(allResults.begin(), allResults.end(),
        [](const auto& a, const auto& b) { return a.relevance > b.relevance; });

    return allResults;
}

void SearchService::ActivateResult(const SearchResult& result) noexcept
{
    RunOnActivate(result);
}

void SearchService::SetApplicationRegistry(
    Apps::ApplicationRegistry& registry) noexcept
{
    m_pAppRegistry = &registry;
}

void SearchService::SetFileSystemService(
    FileSystem::FileSystemService& fsService) noexcept
{
    m_pFS = &fsService;
}

void SearchService::SetCommandRegistry(
    Command::CommandRegistry& cmdRegistry) noexcept
{
    m_pCmdRegistry = &cmdRegistry;
}

void SearchService::RunProviders(
    const SearchQuery& query, SearchCallback callback) noexcept
{
    m_searching = true;

    std::vector<SearchResult> results;

    if (query.searchFiles && m_pFS)
    {
        auto drives = m_pFS->GetLogicalDrives();
        for (const auto& drive : drives)
        {
            if (drive.find(query.text) != std::wstring::npos)
            {
                auto info = m_pFS->GetFileInfo(drive);
                SearchResult r;
                r.displayName = info.name.empty() ? drive : info.name;
                r.description = L"Drive";
                r.category = SearchResultCategory::File;
                r.actionData = drive;
                r.relevance = 0.7f;
                results.push_back(std::move(r));
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(m_providersMutex);
        for (auto& provider : m_providers)
        {
            auto providerResults = provider->Search(query);
            results.insert(results.end(),
                std::make_move_iterator(providerResults.begin()),
                std::make_move_iterator(providerResults.end()));
        }
    }

    auto indexResults = m_index.Query(query.text, query.maxResultsPerCategory);
    results.insert(results.end(),
        std::make_move_iterator(indexResults.begin()),
        std::make_move_iterator(indexResults.end()));

    std::sort(results.begin(), results.end(),
        [](const auto& a, const auto& b) { return a.relevance > b.relevance; });

    if (results.size() > query.maxResultsPerCategory * 4)
    {
        results.resize(query.maxResultsPerCategory * 4);
    }

    if (callback)
    {
        callback(std::move(results));
    }

    m_searching = false;
}

void SearchService::RunOnActivate(const SearchResult& result) noexcept
{
    {
        std::lock_guard<std::mutex> lock(m_providersMutex);
        for (auto& provider : m_providers)
        {
            provider->OnActivate(result);
        }
    }
}

} // namespace DragonOS::Search
