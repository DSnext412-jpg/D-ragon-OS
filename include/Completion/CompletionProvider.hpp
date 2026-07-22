#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace DragonOS::Completion {

struct CompletionItem final {
    std::wstring displayText;
    std::wstring insertionText;
    std::wstring description;
};

class CompletionProvider {
public:
    virtual ~CompletionProvider() noexcept = default;

    virtual std::vector<CompletionItem> GetCompletions(
        const std::wstring& input,
        size_t cursorPos) const noexcept = 0;

    [[nodiscard]] virtual const std::wstring& GetName() const noexcept = 0;

protected:
    CompletionProvider() noexcept = default;
};

} // namespace DragonOS::Completion
