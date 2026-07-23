#pragma once

#include <Security/SecurityTypes.hpp>
#include <Engine/System.hpp>

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

namespace DragonOS::Security {

class UserManager;

enum class SettingScope : uint8_t {
    Global = 0,
    PerUser = 1,
};

enum class SettingProtection : uint8_t {
    Normal = 0,
    Protected = 1,
    ReadOnly = 2,
};

struct SettingEntry final {
    std::wstring value;
    SettingProtection protection{ SettingProtection::Normal };
    bool isDefault{ true };
};

class SettingSection final {
public:
    void Set(std::wstring_view key, std::wstring_view value) noexcept;
    void SetProtected(std::wstring_view key, std::wstring_view value) noexcept;
    void SetReadOnly(std::wstring_view key, std::wstring_view value) noexcept;

    std::wstring_view Get(std::wstring_view key, std::wstring_view defaultValue = {}) const noexcept;
    int GetInt(std::wstring_view key, int defaultValue = 0) const noexcept;
    bool Has(std::wstring_view key) const noexcept;

    bool IsReadOnly(std::wstring_view key) const noexcept;
    bool IsProtected(std::wstring_view key) const noexcept;

    SettingProtection GetProtection(std::wstring_view key) const noexcept;

    const std::map<std::wstring, SettingEntry, std::less<>>& GetAll() const noexcept
    {
        return m_entries;
    }

private:
    std::map<std::wstring, SettingEntry, std::less<>> m_entries;
};

class SecurityConfig final : public Engine::System {
public:
    SecurityConfig() noexcept = default;
    ~SecurityConfig() noexcept override { Shutdown(); }

    SecurityConfig(const SecurityConfig&) = delete;
    SecurityConfig& operator=(const SecurityConfig&) = delete;
    SecurityConfig(SecurityConfig&&) = delete;
    SecurityConfig& operator=(SecurityConfig&&) = delete;

    bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float width, float height) noexcept override;

    SettingSection& GetGlobalSection(std::wstring_view name) noexcept;
    SettingSection& GetUserSection(std::wstring_view name, UserId uid) noexcept;

    bool Save() noexcept;
    bool Load() noexcept;

    void SetUserManager(UserManager& um) noexcept { m_pUserMgr = &um; }

private:
    std::wstring GetGlobalConfigPath() const noexcept;
    std::wstring GetUserConfigPath(UserId uid) const noexcept;

    std::map<std::wstring, SettingSection, std::less<>> m_globalSections;
    std::map<UserId, std::map<std::wstring, SettingSection, std::less<>>> m_userSections;

    UserManager* m_pUserMgr{ nullptr };
    mutable std::mutex m_mutex;
    bool m_initialized{ false };
};

} // namespace DragonOS::Security
