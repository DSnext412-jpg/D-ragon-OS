#pragma once

#include <DragonOS/Config.hpp>

#include <map>
#include <mutex>
#include <string>

namespace DragonOS::SDK {

class ConfigServiceAdapter final : public dragonos::sdk::IConfigService {
public:
    ConfigServiceAdapter() noexcept = default;
    explicit ConfigServiceAdapter(std::wstring filePath) noexcept;

    dragonos::sdk::ConfigSection& GetSection(
        std::wstring_view name) noexcept override;
    bool Load() noexcept override;
    bool Save() noexcept override;

private:
    std::wstring m_filePath{ L"plugins_config.json" };
    std::map<std::wstring, dragonos::sdk::ConfigSection, std::less<>> m_sections;
    mutable std::mutex m_mutex;
};

} // namespace DragonOS::SDK
