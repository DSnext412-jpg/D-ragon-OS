#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace DragonOS::History {

class History final {
public:
    explicit History(size_t maxEntries = 1000) noexcept
        : m_maxEntries{ maxEntries }
    {
    }

    void Add(const std::wstring& entry) noexcept;
    void Clear() noexcept;

    [[nodiscard]] const std::wstring& GetPrevious() noexcept;
    [[nodiscard]] const std::wstring& GetNext() noexcept;

    [[nodiscard]] const std::wstring& Get(size_t index) const noexcept;
    [[nodiscard]] size_t GetCount() const noexcept { return m_entries.size(); }
    [[nodiscard]] size_t GetMaxEntries() const noexcept { return m_maxEntries; }

    void ResetNavigation() noexcept { m_currentIndex = m_entries.size(); }

    [[nodiscard]] const std::vector<std::wstring>& GetAll() const noexcept
    {
        return m_entries;
    }

private:
    std::vector<std::wstring> m_entries;
    size_t                    m_maxEntries{ 1000 };
    size_t                    m_currentIndex{ 0 };
};

} // namespace DragonOS::History
