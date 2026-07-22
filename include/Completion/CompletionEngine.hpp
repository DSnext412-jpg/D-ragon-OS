#pragma once

#include <Completion/CompletionProvider.hpp>

#include <memory>
#include <string>
#include <vector>

namespace DragonOS::Completion {

class CompletionEngine final {
public:
    CompletionEngine() noexcept = default;
    ~CompletionEngine() noexcept = default;

    CompletionEngine(const CompletionEngine&) = delete;
    CompletionEngine& operator=(const CompletionEngine&) = delete;
    CompletionEngine(CompletionEngine&&) = delete;
    CompletionEngine& operator=(CompletionEngine&&) = delete;

    void AddProvider(std::unique_ptr<CompletionProvider> provider) noexcept;
    void RemoveProvider(const std::wstring& name) noexcept;

    std::vector<CompletionItem> GetCompletions(
        const std::wstring& input,
        size_t cursorPos) const noexcept;

    [[nodiscard]] size_t GetProviderCount() const noexcept
    {
        return m_providers.size();
    }

private:
    std::vector<std::unique_ptr<CompletionProvider>> m_providers;
};

} // namespace DragonOS::Completion
