#include "ConfigServiceAdapter.hpp"

#include <fstream>
#include <sstream>
#include <Windows.h>

namespace DragonOS::SDK {

static std::wstring EscapeJson(const std::wstring& s)
{
    std::wstring out;
    out.reserve(s.size() + 2);
    for (auto c : s)
    {
        switch (c)
        {
        case L'"':  out += L"\\\""; break;
        case L'\\': out += L"\\\\"; break;
        case L'\n': out += L"\\n";  break;
        case L'\r': out += L"\\r";  break;
        case L'\t': out += L"\\t";  break;
        default:    out += c;       break;
        }
    }
    return out;
}

ConfigServiceAdapter::ConfigServiceAdapter(std::wstring filePath) noexcept
    : m_filePath(std::move(filePath))
{
    Load();
}

dragonos::sdk::ConfigSection& ConfigServiceAdapter::GetSection(
    std::wstring_view name) noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_sections.find(name);
    if (it != m_sections.end())
    {
        return it->second;
    }
    auto [newIt, _] = m_sections.emplace(
        std::wstring{ name },
        dragonos::sdk::ConfigSection{});
    return newIt->second;
}

bool ConfigServiceAdapter::Load() noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sections.clear();

    std::ifstream file(m_filePath);
    if (!file.is_open()) { return false; }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::string content = buffer.str();
    std::wstring currentSection;
    size_t pos = 0;
    while (pos < content.size())
    {
        auto eol = content.find('\n', pos);
        if (eol == std::string::npos) { eol = content.size(); }
        std::string line = content.substr(pos, eol - pos);
        pos = eol + 1;

        if (line.empty()) { continue; }
        if (line.front() == '[')
        {
            auto end = line.find(']');
            if (end != std::string::npos)
            {
                int len = MultiByteToWideChar(CP_UTF8, 0, line.c_str() + 1, static_cast<int>(end - 1), nullptr, 0);
                currentSection.resize(len);
                MultiByteToWideChar(CP_UTF8, 0, line.c_str() + 1, static_cast<int>(end - 1), &currentSection[0], len);
            }
        }
        else
        {
            auto eq = line.find('=');
            if (eq != std::string::npos && !currentSection.empty())
            {
                std::string keyStr = line.substr(0, eq);
                std::string valStr = line.substr(eq + 1);

                int klen = MultiByteToWideChar(CP_UTF8, 0, keyStr.c_str(), static_cast<int>(keyStr.size()), nullptr, 0);
                std::wstring key(klen, L'\0');
                MultiByteToWideChar(CP_UTF8, 0, keyStr.c_str(), static_cast<int>(keyStr.size()), &key[0], klen);

                int vlen = MultiByteToWideChar(CP_UTF8, 0, valStr.c_str(), static_cast<int>(valStr.size()), nullptr, 0);
                std::wstring val(vlen, L'\0');
                MultiByteToWideChar(CP_UTF8, 0, valStr.c_str(), static_cast<int>(valStr.size()), &val[0], vlen);

                m_sections[currentSection].Set(key, val);
            }
        }
    }

    return true;
}

bool ConfigServiceAdapter::Save() noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::wofstream file(m_filePath);
    if (!file.is_open()) { return false; }

    for (const auto& [sectionName, section] : m_sections)
    {
        file << L"[" << sectionName << L"]\n";
        for (const auto& [key, val] : section.GetAll())
        {
            file << key << L"=" << val << L"\n";
        }
        file << L"\n";
    }

    file.close();
    return true;
}

} // namespace DragonOS::SDK
