#include "ResourceServiceAdapter.hpp"

#include <Windows.h>

#include <fstream>
#include <iterator>

namespace DragonOS::SDK {

std::vector<uint8_t> ResourceServiceAdapter::LoadBinary(
    std::wstring_view path) noexcept
{
    std::ifstream file(std::wstring{ path }, std::ios::binary);
    if (!file) { return {}; }

    return { std::istreambuf_iterator<char>(file),
             std::istreambuf_iterator<char>() };
}

std::wstring ResourceServiceAdapter::LoadText(
    std::wstring_view path) noexcept
{
    std::wifstream file(std::wstring{ path });
    if (!file) { return {}; }

    return { std::istreambuf_iterator<wchar_t>(file),
             std::istreambuf_iterator<wchar_t>() };
}

std::wstring ResourceServiceAdapter::ResolvePath(
    std::wstring_view relativePath) noexcept
{
    wchar_t modulePath[MAX_PATH];
    if (::GetModuleFileNameW(nullptr, modulePath, MAX_PATH) == 0)
    {
        return std::wstring{ relativePath };
    }

    std::wstring baseDir{ modulePath };
    auto pos = baseDir.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
    {
        baseDir.resize(pos + 1);
    }

    return baseDir + std::wstring{ relativePath };
}

bool ResourceServiceAdapter::Exists(std::wstring_view path) noexcept
{
    std::ifstream file(std::wstring{ path });
    return file.is_open();
}

} // namespace DragonOS::SDK
