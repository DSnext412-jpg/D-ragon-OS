#pragma once

#include <Engine/System.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

namespace DragonOS::Security {

class CredentialManager final : public Engine::System {
public:
    CredentialManager() noexcept = default;
    ~CredentialManager() noexcept override { Shutdown(); }

    CredentialManager(const CredentialManager&) = delete;
    CredentialManager& operator=(const CredentialManager&) = delete;
    CredentialManager(CredentialManager&&) = delete;
    CredentialManager& operator=(CredentialManager&&) = delete;

    bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float width, float height) noexcept override;

    bool StorePassword(std::wstring_view username, std::wstring_view password) noexcept;
    bool VerifyPassword(std::wstring_view username, std::wstring_view password) noexcept;
    bool ChangePassword(std::wstring_view username, std::wstring_view oldPassword, std::wstring_view newPassword) noexcept;

    bool StoreToken(std::wstring_view key, std::wstring_view token) noexcept;
    std::wstring RetrieveToken(std::wstring_view key) noexcept;
    bool RemoveToken(std::wstring_view key) noexcept;

    bool StoreSecret(std::wstring_view key, std::wstring_view secret) noexcept;
    std::wstring RetrieveSecret(std::wstring_view key) noexcept;
    bool RemoveSecret(std::wstring_view key) noexcept;

    bool SaveCredentials() noexcept;
    bool LoadCredentials() noexcept;

private:
    static std::wstring HashPassword(std::wstring_view password) noexcept;
    static std::vector<uint8_t> ProtectData(const std::vector<uint8_t>& data) noexcept;
    static std::vector<uint8_t> UnprotectData(const std::vector<uint8_t>& data) noexcept;

    std::wstring GetCredentialFilePath() const noexcept;

    struct PasswordEntry {
        std::wstring hash;
        std::wstring salt;
    };

    struct SecretEntry {
        std::vector<uint8_t> encryptedData;
    };

    std::unordered_map<std::wstring, PasswordEntry> m_passwords;
    std::unordered_map<std::wstring, SecretEntry> m_tokens;
    std::unordered_map<std::wstring, SecretEntry> m_secrets;

    bool m_loaded{ false };
    bool m_initialized{ false };
};

} // namespace DragonOS::Security
