#pragma once

#include <Search/SearchProvider.hpp>
#include <Search/SearchQuery.hpp>
#include <Search/SearchResult.hpp>
#include <Search/SearchIndex.hpp>

#include <Engine/System.hpp>

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace DragonOS::Apps           { class ApplicationRegistry; }
namespace DragonOS::FileSystem     { class FileSystemService; }
namespace DragonOS::Command        { class CommandRegistry; }

namespace DragonOS::Search {

class SearchService final : public Engine::System {
public:
    SearchService() noexcept = default;
    ~SearchService() noexcept override { Shutdown(); }

    SearchService(const SearchService&) = delete;
    SearchService& operator=(const SearchService&) = delete;
    SearchService(SearchService&&) = delete;
    SearchService& operator=(SearchService&&) = delete;

    bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float width, float height) noexcept override;

    void RegisterProvider(SearchProviderPtr provider) noexcept;

    using SearchCallback = std::function<void(std::vector<SearchResult>)>;

    void SearchAsync(SearchQuery query, SearchCallback callback) noexcept;
    std::vector<SearchResult> SearchSync(const SearchQuery& query) noexcept;

    void ActivateResult(const SearchResult& result) noexcept;

    void SetApplicationRegistry(Apps::ApplicationRegistry& registry) noexcept;
    void SetFileSystemService(FileSystem::FileSystemService& fsService) noexcept;
    void SetCommandRegistry(Command::CommandRegistry& cmdRegistry) noexcept;

    SearchIndex& GetIndex() noexcept { return m_index; }
    bool IsSearching() const noexcept { return m_searching; }

    static constexpr float SearchDebounceSeconds = 0.3f;

private:
    void RunProviders(const SearchQuery& query, SearchCallback callback) noexcept;
    void RunOnActivate(const SearchResult& result) noexcept;

    std::vector<SearchProviderPtr> m_providers;
    std::mutex m_providersMutex;
    SearchIndex m_index;

    struct PendingSearch {
        SearchQuery query;
        SearchCallback callback;
    };
    std::vector<PendingSearch> m_pendingSearches;
    std::mutex m_pendingMutex;

    std::atomic<bool> m_searching{ false };

    Apps::ApplicationRegistry*  m_pAppRegistry{ nullptr };
    FileSystem::FileSystemService*  m_pFS{ nullptr };
    Command::CommandRegistry*   m_pCmdRegistry{ nullptr };

    bool m_initialized{ false };
};

} // namespace DragonOS::Search
