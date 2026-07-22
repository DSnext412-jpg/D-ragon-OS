#include <Completion/CompletionEngine.hpp>

namespace DragonOS::Completion {

void CompletionEngine::AddProvider(
    std::unique_ptr<CompletionProvider> provider) noexcept
{
    if (provider)
    {
        m_providers.push_back(std::move(provider));
    }
}

void CompletionEngine::RemoveProvider(const std::wstring& name) noexcept
{
    for (auto it = m_providers.begin(); it != m_providers.end(); ++it)
    {
        if ((*it)->GetName() == name)
        {
            m_providers.erase(it);
            return;
        }
    }
}

std::vector<CompletionItem> CompletionEngine::GetCompletions(
    const std::wstring& input,
    size_t cursorPos) const noexcept
{
    std::vector<CompletionItem> results;

    for (const auto& provider : m_providers)
    {
        auto completions = provider->GetCompletions(input, cursorPos);
        results.insert(results.end(), completions.begin(), completions.end());
    }

    return results;
}

} // namespace DragonOS::Completion
