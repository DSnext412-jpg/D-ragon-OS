#include <Security/CredentialManager.hpp>

#include <Engine/EngineContext.hpp>

#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>

#include <Windows.h>
#include <bcrypt.h>
#include <wincrypt.h>
#include <dpapi.h>

#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "crypt32.lib")

namespace DragonOS::Security {

namespace {

constexpr std::wstring_view CredentialFile = L"security\\credentials.dat";

std::vector<uint8_t> StringToBytes(std::wstring_view s)
{
    const int cbNeeded = WideCharToMultiByte(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0, nullptr, nullptr);
    if (cbNeeded <= 0) return {};
    std::vector<uint8_t> bytes(cbNeeded);
    WideCharToMultiByte(CP_UTF8, 0, s.data(), static_cast<int>(s.size()),
        reinterpret_cast<char*>(bytes.data()), cbNeeded, nullptr, nullptr);
    return bytes;
}

std::wstring BytesToString(const std::vector<uint8_t>& bytes)
{
    if (bytes.empty()) return {};
    const int cchNeeded = MultiByteToWideChar(CP_UTF8, 0,
        reinterpret_cast<const char*>(bytes.data()), static_cast<int>(bytes.size()), nullptr, 0);
    if (cchNeeded <= 0) return {};
    std::wstring result(cchNeeded, L'\0');
    MultiByteToWideChar(CP_UTF8, 0,
        reinterpret_cast<const char*>(bytes.data()), static_cast<int>(bytes.size()),
        &result[0], cchNeeded);
    return result;
}

std::wstring BytesToHex(const std::vector<uint8_t>& bytes)
{
    std::wostringstream oss;
    oss << std::hex << std::setfill(L'0');
    for (auto b : bytes)
        oss << std::setw(2) << b;
    return oss.str();
}

std::vector<uint8_t> HexToBytes(std::wstring_view hex)
{
    std::vector<uint8_t> bytes;
    bytes.reserve(hex.size() / 2);
    for (size_t i = 0; i + 1 < hex.size(); i += 2)
    {
        wchar_t hi = hex[i];
        wchar_t lo = hex[i + 1];
        auto hexVal = [](wchar_t c) -> uint8_t {
            if (c >= L'0' && c <= L'9') return static_cast<uint8_t>(c - L'0');
            if (c >= L'a' && c <= L'f') return static_cast<uint8_t>(c - L'a' + 10);
            if (c >= L'A' && c <= L'F') return static_cast<uint8_t>(c - L'A' + 10);
            return 0;
        };
        bytes.push_back((hexVal(hi) << 4) | hexVal(lo));
    }
    return bytes;
}

constexpr size_t SaltLength = 16;
constexpr size_t HashRounds = 10000;

} // anonymous namespace

bool CredentialManager::Initialize(Engine::EngineContext& ctx) noexcept
{
    if (m_initialized) return true;
    (void)ctx;
    LoadCredentials();
    m_initialized = true;
    return true;
}

void CredentialManager::Shutdown() noexcept
{
    if (!m_initialized) return;
    SaveCredentials();
    m_passwords.clear();
    m_tokens.clear();
    m_secrets.clear();
    m_initialized = false;
}

void CredentialManager::Update(float /*deltaTime*/) noexcept {}
void CredentialManager::Render(Engine::EngineContext& /*ctx*/) noexcept {}
void CredentialManager::Resize(float /*width*/, float /*height*/) noexcept {}

std::wstring CredentialManager::HashPassword(std::wstring_view password) noexcept
{
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
    if (!hAlg) return {};

    std::vector<uint8_t> hashBytes(32);

    auto pwBytes = StringToBytes(password);

    BCRYPT_HASH_HANDLE hHash = nullptr;
    BCryptCreateHash(hAlg, &hHash, nullptr, 0, nullptr, 0, 0);
    if (hHash)
    {
        BCryptHashData(hHash, pwBytes.data(), static_cast<ULONG>(pwBytes.size()), 0);
        BCryptFinishHash(hHash, hashBytes.data(), static_cast<ULONG>(hashBytes.size()), 0);
        BCryptDestroyHash(hHash);
    }

    BCryptCloseAlgorithmProvider(hAlg, 0);

    return BytesToHex(hashBytes);
}

std::vector<uint8_t> CredentialManager::ProtectData(const std::vector<uint8_t>& data) noexcept
{
    DATA_BLOB input;
    input.pbData = const_cast<BYTE*>(data.data());
    input.cbData = static_cast<DWORD>(data.size());

    DATA_BLOB output;
    if (!CryptProtectData(&input, nullptr, nullptr, nullptr, nullptr, 0, &output))
        return {};

    std::vector<uint8_t> result(output.pbData, output.pbData + output.cbData);
    LocalFree(output.pbData);
    return result;
}

std::vector<uint8_t> CredentialManager::UnprotectData(const std::vector<uint8_t>& data) noexcept
{
    DATA_BLOB input;
    input.pbData = const_cast<BYTE*>(data.data());
    input.cbData = static_cast<DWORD>(data.size());

    DATA_BLOB output;
    if (!CryptUnprotectData(&input, nullptr, nullptr, nullptr, nullptr, 0, &output))
        return {};

    std::vector<uint8_t> result(output.pbData, output.pbData + output.cbData);
    LocalFree(output.pbData);
    return result;
}

bool CredentialManager::StorePassword(std::wstring_view username, std::wstring_view password) noexcept
{
    auto hash = HashPassword(password);
    if (hash.empty()) return false;

    std::wstring salt;
    {
        std::vector<uint8_t> saltBytes(SaltLength);
        for (auto& b : saltBytes) b = static_cast<uint8_t>(rand() & 0xFF);
        salt = BytesToHex(saltBytes);
    }

    auto saltedHash = HashPassword(salt + std::wstring{ password });
    if (saltedHash.empty()) return false;

    PasswordEntry entry;
    entry.hash = saltedHash;
    entry.salt = salt;
    m_passwords[std::wstring{ username }] = std::move(entry);
    return SaveCredentials();
}

bool CredentialManager::VerifyPassword(std::wstring_view username, std::wstring_view password) noexcept
{
    auto it = m_passwords.find(std::wstring{ username });
    if (it == m_passwords.end()) return false;

    auto saltedHash = HashPassword(it->second.salt + std::wstring{ password });
    return saltedHash == it->second.hash;
}

bool CredentialManager::ChangePassword(std::wstring_view username, std::wstring_view oldPassword, std::wstring_view newPassword) noexcept
{
    if (!VerifyPassword(username, oldPassword)) return false;
    return StorePassword(username, newPassword);
}

bool CredentialManager::StoreToken(std::wstring_view key, std::wstring_view token) noexcept
{
    auto bytes = StringToBytes(token);
    auto encrypted = ProtectData(bytes);
    if (encrypted.empty()) return false;

    m_tokens[std::wstring{ key }] = { encrypted };
    return SaveCredentials();
}

std::wstring CredentialManager::RetrieveToken(std::wstring_view key) noexcept
{
    auto it = m_tokens.find(std::wstring{ key });
    if (it == m_tokens.end()) return {};

    auto decrypted = UnprotectData(it->second.encryptedData);
    return BytesToString(decrypted);
}

bool CredentialManager::RemoveToken(std::wstring_view key) noexcept
{
    auto it = m_tokens.find(std::wstring{ key });
    if (it == m_tokens.end()) return false;
    m_tokens.erase(it);
    return SaveCredentials();
}

bool CredentialManager::StoreSecret(std::wstring_view key, std::wstring_view secret) noexcept
{
    auto bytes = StringToBytes(secret);
    auto encrypted = ProtectData(bytes);
    if (encrypted.empty()) return false;

    m_secrets[std::wstring{ key }] = { encrypted };
    return SaveCredentials();
}

std::wstring CredentialManager::RetrieveSecret(std::wstring_view key) noexcept
{
    auto it = m_secrets.find(std::wstring{ key });
    if (it == m_secrets.end()) return {};

    auto decrypted = UnprotectData(it->second.encryptedData);
    return BytesToString(decrypted);
}

bool CredentialManager::RemoveSecret(std::wstring_view key) noexcept
{
    auto it = m_secrets.find(std::wstring{ key });
    if (it == m_secrets.end()) return false;
    m_secrets.erase(it);
    return SaveCredentials();
}

std::wstring CredentialManager::GetCredentialFilePath() const noexcept
{
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring exePath = path;
    auto pos = exePath.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
        exePath = exePath.substr(0, pos);
    return exePath + L"\\" + std::wstring{ CredentialFile };
}

bool CredentialManager::SaveCredentials() noexcept
{
    auto filePath = GetCredentialFilePath();

    // Create directory if needed
    auto dirPos = filePath.find_last_of(L"\\/");
    if (dirPos != std::wstring::npos)
    {
        std::wstring dir = filePath.substr(0, dirPos);
        CreateDirectoryW(dir.c_str(), nullptr);
    }

    std::wofstream file(filePath);
    if (!file.is_open()) return false;

    file << L"[passwords]\n";
    for (const auto& [user, entry] : m_passwords)
    {
        file << user << L"=" << entry.salt << L":" << entry.hash << L"\n";
    }

    file << L"[tokens]\n";
    for (const auto& [key, entry] : m_tokens)
    {
        file << key << L"=" << BytesToHex(entry.encryptedData) << L"\n";
    }

    file << L"[secrets]\n";
    for (const auto& [key, entry] : m_secrets)
    {
        file << key << L"=" << BytesToHex(entry.encryptedData) << L"\n";
    }

    file.close();
    return true;
}

bool CredentialManager::LoadCredentials() noexcept
{
    auto filePath = GetCredentialFilePath();
    std::wifstream file(filePath);
    if (!file.is_open()) return false;

    m_passwords.clear();
    m_tokens.clear();
    m_secrets.clear();

    std::wstring line;
    std::wstring currentSection;
    while (std::getline(file, line))
    {
        if (line.empty()) continue;
        if (line.front() == L'[')
        {
            auto end = line.find(L']');
            if (end != std::wstring::npos)
                currentSection = line.substr(1, end - 1);
            continue;
        }

        auto eq = line.find(L'=');
        if (eq == std::wstring::npos) continue;

        auto key = line.substr(0, eq);
        auto val = line.substr(eq + 1);

        if (currentSection == L"passwords")
        {
            auto colon = val.find(L':');
            if (colon != std::wstring::npos)
            {
                PasswordEntry entry;
                entry.salt = val.substr(0, colon);
                entry.hash = val.substr(colon + 1);
                m_passwords[key] = std::move(entry);
            }
        }
        else if (currentSection == L"tokens")
        {
            SecretEntry entry;
            entry.encryptedData = HexToBytes(val);
            m_tokens[key] = std::move(entry);
        }
        else if (currentSection == L"secrets")
        {
            SecretEntry entry;
            entry.encryptedData = HexToBytes(val);
            m_secrets[key] = std::move(entry);
        }
    }

    file.close();
    m_loaded = true;
    return true;
}

} // namespace DragonOS::Security
