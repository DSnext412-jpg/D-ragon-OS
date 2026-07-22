#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

namespace dragonos::sdk {

class ConfigSection {
public:
    void Set(std::wstring_view key, std::wstring_view value) noexcept
    {
        m_values[std::wstring{ key }] = std::wstring{ value };
    }

    std::wstring_view Get(std::wstring_view key, std::wstring_view defaultValue = {}) const noexcept
    {
        auto it = m_values.find(std::wstring{ key });
        return (it != m_values.end()) ? it->second : defaultValue;
    }

    int GetInt(std::wstring_view key, int defaultValue = 0) const noexcept
    {
        auto it = m_values.find(std::wstring{ key });
        if (it == m_values.end()) { return defaultValue; }
        return std::stoi(it->second);
    }

    bool Has(std::wstring_view key) const noexcept
    {
        return m_values.contains(std::wstring{ key });
    }

    const std::unordered_map<std::wstring, std::wstring>& GetAll() const noexcept
    {
        return m_values;
    }

private:
    std::unordered_map<std::wstring, std::wstring> m_values;
};

class IConfigService {
public:
    virtual ~IConfigService() noexcept = default;
    virtual ConfigSection& GetSection(std::wstring_view name) noexcept = 0;
    virtual bool Save() noexcept = 0;
    virtual bool Load() noexcept = 0;
};

} // namespace dragonos::sdk
