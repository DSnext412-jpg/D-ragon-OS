#include <Security/SecurityConfig.hpp>
#include <Security/UserManager.hpp>

#include <Engine/EngineContext.hpp>

#include <fstream>
#include <sstream>

#include <Windows.h>

namespace DragonOS::Security {

// ============================================================================
//  SettingSection
// ============================================================================

void SettingSection::Set(std::wstring_view key, std::wstring_view value) noexcept
{
    auto it = m_entries.find(key);
    if (it != m_entries.end() && it->second.protection != SettingProtection::Normal)
        return;
    m_entries[std::wstring{ key }] = { std::wstring{ value }, SettingProtection::Normal, false };
}

void SettingSection::SetProtected(std::wstring_view key, std::wstring_view value) noexcept
{
    m_entries[std::wstring{ key }] = { std::wstring{ value }, SettingProtection::Protected, false };
}

void SettingSection::SetReadOnly(std::wstring_view key, std::wstring_view value) noexcept
{
    m_entries[std::wstring{ key }] = { std::wstring{ value }, SettingProtection::ReadOnly, false };
}

std::wstring_view SettingSection::Get(std::wstring_view key, std::wstring_view defaultValue) const noexcept
{
    auto it = m_entries.find(key);
    return (it != m_entries.end()) ? it->second.value : defaultValue;
}

int SettingSection::GetInt(std::wstring_view key, int defaultValue) const noexcept
{
    auto it = m_entries.find(key);
    if (it == m_entries.end()) return defaultValue;
    try { return std::stoi(it->second.value); }
    catch (...) { return defaultValue; }
}

bool SettingSection::Has(std::wstring_view key) const noexcept
{
    return m_entries.contains(std::wstring{ key });
}

bool SettingSection::IsReadOnly(std::wstring_view key) const noexcept
{
    auto it = m_entries.find(key);
    return it != m_entries.end() && it->second.protection == SettingProtection::ReadOnly;
}

bool SettingSection::IsProtected(std::wstring_view key) const noexcept
{
    auto it = m_entries.find(key);
    return it != m_entries.end() && it->second.protection == SettingProtection::Protected;
}

SettingProtection SettingSection::GetProtection(std::wstring_view key) const noexcept
{
    auto it = m_entries.find(key);
    return it != m_entries.end() ? it->second.protection : SettingProtection::Normal;
}

// ============================================================================
//  SecurityConfig
// ============================================================================

bool SecurityConfig::Initialize(Engine::EngineContext& ctx) noexcept
{
    if (m_initialized) return true;
    (void)ctx;
    Load();
    m_initialized = true;
    return true;
}

void SecurityConfig::Shutdown() noexcept
{
    if (!m_initialized) return;
    Save();
    m_globalSections.clear();
    m_userSections.clear();
    m_initialized = false;
}

void SecurityConfig::Update(float /*deltaTime*/) noexcept {}
void SecurityConfig::Render(Engine::EngineContext& /*ctx*/) noexcept {}
void SecurityConfig::Resize(float /*width*/, float /*height*/) noexcept {}

SettingSection& SecurityConfig::GetGlobalSection(std::wstring_view name) noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_globalSections.find(name);
    if (it != m_globalSections.end()) return it->second;
    auto [newIt, _] = m_globalSections.emplace(
        std::wstring{ name }, SettingSection{});
    return newIt->second;
}

SettingSection& SecurityConfig::GetUserSection(std::wstring_view name, UserId uid) noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_userSections[uid].find(name);
    if (it != m_userSections[uid].end()) return it->second;
    auto [newIt, _] = m_userSections[uid].emplace(
        std::wstring{ name }, SettingSection{});
    return newIt->second;
}

std::wstring SecurityConfig::GetGlobalConfigPath() const noexcept
{
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring exePath = path;
    auto pos = exePath.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
        exePath = exePath.substr(0, pos);
    return exePath + L"\\config\\global.ini";
}

std::wstring SecurityConfig::GetUserConfigPath(UserId uid) const noexcept
{
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring exePath = path;
    auto pos = exePath.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
        exePath = exePath.substr(0, pos);
    return exePath + L"\\config\\user_" + std::to_wstring(uid) + L".ini";
}

bool SecurityConfig::Save() noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);

    // Create config directory
    {
        wchar_t path[MAX_PATH];
        GetModuleFileNameW(nullptr, path, MAX_PATH);
        std::wstring exePath = path;
        auto pos = exePath.find_last_of(L"\\/");
        if (pos != std::wstring::npos)
        {
            CreateDirectoryW((exePath.substr(0, pos) + L"\\config").c_str(), nullptr);
        }
    }

    // Save global settings
    {
        std::wofstream file(GetGlobalConfigPath());
        if (!file.is_open()) return false;

        for (const auto& [sectionName, section] : m_globalSections)
        {
            file << L"[" << sectionName << L"]\n";
            for (const auto& [key, entry] : section.GetAll())
            {
                wchar_t prefix = L'=';
                if (entry.protection == SettingProtection::Protected) prefix = L'!';
                else if (entry.protection == SettingProtection::ReadOnly) prefix = L'+';
                file << prefix << key << L"=" << entry.value << L"\n";
            }
            file << L"\n";
        }
        file.close();
    }

    // Save per-user settings
    for (const auto& [uid, sections] : m_userSections)
    {
        std::wofstream file(GetUserConfigPath(uid));
        if (!file.is_open()) continue;

        for (const auto& [sectionName, section] : sections)
        {
            file << L"[" << sectionName << L"]\n";
            for (const auto& [key, entry] : section.GetAll())
            {
                wchar_t prefix = L'=';
                if (entry.protection == SettingProtection::Protected) prefix = L'!';
                else if (entry.protection == SettingProtection::ReadOnly) prefix = L'+';
                file << prefix << key << L"=" << entry.value << L"\n";
            }
            file << L"\n";
        }
        file.close();
    }

    return true;
}

bool SecurityConfig::Load() noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto parseFile = [](const std::wstring& filePath,
        std::map<std::wstring, SettingSection, std::less<>>& sections)
    {
        std::wifstream file(filePath);
        if (!file.is_open()) return;

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

            SettingProtection protection = SettingProtection::Normal;
            size_t start = 0;
            if (line.front() == L'!') { protection = SettingProtection::Protected; start = 1; }
            else if (line.front() == L'+') { protection = SettingProtection::ReadOnly; start = 1; }

            auto eq = line.find(L'=', start);
            if (eq == std::wstring::npos) continue;

            auto key = line.substr(start, eq - start);
            auto val = line.substr(eq + 1);

            SettingEntry entry;
            entry.value = val;
            entry.protection = protection;
            entry.isDefault = false;
            sections[currentSection].GetAll(); // ensure section exists
            sections[currentSection].SetProtected(key, val);
            // Override protection
            auto& entries = const_cast<std::map<std::wstring, SettingEntry, std::less<>>&>(
                sections[currentSection].GetAll());
            auto it = entries.find(key);
            if (it != entries.end())
                it->second.protection = protection;
        }
        file.close();
    };

    parseFile(GetGlobalConfigPath(), m_globalSections);

    // Scan for user config files
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring exePath = path;
    auto pos = exePath.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
        exePath = exePath.substr(0, pos);

    std::wstring configDir = exePath + L"\\config";
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW((configDir + L"\\user_*.ini").c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do {
            std::wstring name = findData.cFileName;
            // Parse user ID from filename user_XXXX.ini
            if (name.size() > 8 && name.substr(0, 5) == L"user_")
            {
                auto dot = name.find(L'.');
                if (dot != std::wstring::npos)
                {
                    std::wstring uidStr = name.substr(5, dot - 5);
                    try {
                        UserId uid = static_cast<UserId>(std::stoul(uidStr));
                        parseFile(configDir + L"\\" + name, m_userSections[uid]);
                    } catch (...) {}
                }
            }
        } while (FindNextFileW(hFind, &findData));
        FindClose(hFind);
    }

    return true;
}

} // namespace DragonOS::Security
