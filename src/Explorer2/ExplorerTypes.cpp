#include <Explorer2/ExplorerTypes.hpp>

#include <algorithm>
#include <cwctype>

namespace DragonOS::Explorer2 {

// ============================================================================
//  Shared helpers
// ============================================================================

int CompareNoCase(std::wstring_view a, std::wstring_view b) noexcept
{
    const size_t count = (std::min)(a.size(), b.size());
    for (size_t i = 0; i < count; ++i)
    {
        const wchar_t ca = static_cast<wchar_t>(std::towlower(a[i]));
        const wchar_t cb = static_cast<wchar_t>(std::towlower(b[i]));
        if (ca != cb)
        {
            return ca < cb ? -1 : 1;
        }
    }
    if (a.size() == b.size()) { return 0; }
    return a.size() < b.size() ? -1 : 1;
}

bool NaturalLess(const std::wstring& a, const std::wstring& b) noexcept
{
    size_t i = 0;
    size_t j = 0;
    while (i < a.size() && j < b.size())
    {
        const wchar_t ca = a[i];
        const wchar_t cb = b[j];

        const bool aDigit = std::iswdigit(ca) != 0;
        const bool bDigit = std::iswdigit(cb) != 0;

        if (aDigit && bDigit)
        {
            // Compare the full numeric runs.
            size_t iEnd = i;
            while (iEnd < a.size() && std::iswdigit(a[iEnd])) { ++iEnd; }
            size_t jEnd = j;
            while (jEnd < b.size() && std::iswdigit(b[jEnd])) { ++jEnd; }

            // Skip leading zeros.
            size_t iStart = i;
            while (iStart + 1 < iEnd && a[iStart] == L'0') { ++iStart; }
            size_t jStart = j;
            while (jStart + 1 < jEnd && b[jStart] == L'0') { ++jStart; }

            const size_t aLen = iEnd - iStart;
            const size_t bLen = jEnd - jStart;
            if (aLen != bLen) { return aLen < bLen; }

            const int cmp = a.compare(iStart, aLen, b, jStart, bLen);
            if (cmp != 0) { return cmp < 0; }

            i = iEnd;
            j = jEnd;
            continue;
        }

        const wchar_t la = static_cast<wchar_t>(std::towlower(ca));
        const wchar_t lb = static_cast<wchar_t>(std::towlower(cb));
        if (la != lb) { return la < lb; }
        ++i;
        ++j;
    }
    return (a.size() - i) < (b.size() - j);
}

std::wstring GetTypeDisplayName(const FileSystem::FileEntry& entry) noexcept
{
    using FileSystem::FileEntry;
    if (entry.IsDirectory())
    {
        return L"File folder";
    }

    const size_t dot = entry.name.rfind(L'.');
    if (dot == std::wstring::npos || dot + 1 >= entry.name.size())
    {
        return L"File";
    }

    std::wstring ext = entry.name.substr(dot + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) {
        return static_cast<wchar_t>(std::towupper(c));
    });

    if (ext == L"TXT" || ext == L"LOG" || ext == L"MD") { return ext + L" File"; }
    if (ext == L"PNG" || ext == L"JPG" || ext == L"JPEG" ||
        ext == L"BMP" || ext == L"GIF" || ext == L"WEBP")
    {
        return ext + L" Image";
    }
    if (ext == L"PDF") { return L"PDF Document"; }
    if (ext == L"WAV" || ext == L"MP3" || ext == L"OGG" || ext == L"FLAC") { return ext + L" Audio"; }
    if (ext == L"MP4" || ext == L"AVI" || ext == L"MKV") { return ext + L" Video"; }
    if (ext == L"ZIP" || ext == L"7Z" || ext == L"RAR" || ext == L"TAR") { return ext + L" Archive"; }
    if (ext == L"EXE" || ext == L"DLL" || ext == L"SYS") { return L"Application"; }
    if (ext == L"HPP" || ext == L"H" || ext == L"CPP" || ext == L"C" || ext == L"MM")
    {
        return L"C/C++ Source";
    }
    if (ext == L"JSON" || ext == L"XML" || ext == L"INI" || ext == L"CFG") { return ext + L" Configuration"; }
    return ext + L" File";
}

std::vector<std::wstring> SplitPath(const std::wstring& path) noexcept
{
    std::vector<std::wstring> segments;
    std::wstring current;
    for (const wchar_t c : path)
    {
        if (c == L'\\' || c == L'/')
        {
            if (!current.empty())
            {
                segments.push_back(current);
                current.clear();
            }
            continue;
        }
        current.push_back(c);
    }
    if (!current.empty())
    {
        segments.push_back(current);
    }
    return segments;
}

std::wstring JoinPathSegments(const std::vector<std::wstring>& segments, size_t count) noexcept
{
    if (segments.empty() || count == 0)
    {
        return {};
    }
    count = (std::min)(count, segments.size());

    std::wstring result = segments[0];
    // Keep the drive-colon form ("C:") as-is; join everything after with '\'.
    for (size_t i = 1; i < count; ++i)
    {
        if (!result.empty() && result.back() != L':')
        {
            result.push_back(L'\\');
        }
        result += segments[i];
    }
    return result;
}

} // namespace DragonOS::Explorer2
