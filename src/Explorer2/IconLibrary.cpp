#include <Explorer2/IconLibrary.hpp>

#include <DragonUI/Core/Glyph.hpp>

#include <cwctype>

namespace DragonOS::Explorer2 {

// ============================================================================
//  Glyph resolution
// ============================================================================

namespace {

using FileSystem::FileEntry;

[[nodiscard]] bool IsImageExtension(std::wstring_view ext) noexcept
{
    return ext == L"png" || ext == L"jpg" || ext == L"jpeg" || ext == L"bmp" ||
           ext == L"gif" || ext == L"webp" || ext == L"ico" || ext == L"tif";
}

[[nodiscard]] bool IsTextExtension(std::wstring_view ext) noexcept
{
    return ext == L"txt" || ext == L"log" || ext == L"md" || ext == L"ini" ||
           ext == L"cfg" || ext == L"json" || ext == L"xml";
}

[[nodiscard]] bool IsCodeExtension(std::wstring_view ext) noexcept
{
    return ext == L"h" || ext == L"hpp" || ext == L"c" || ext == L"cpp" ||
           ext == L"cc" || ext == L"mm" || ext == L"cs" || ext == L"py" ||
           ext == L"js" || ext == L"rs";
}

[[nodiscard]] bool IsAudioExtension(std::wstring_view ext) noexcept
{
    return ext == L"wav" || ext == L"mp3" || ext == L"ogg" || ext == L"flac";
}

[[nodiscard]] bool IsVideoExtension(std::wstring_view ext) noexcept
{
    return ext == L"mp4" || ext == L"avi" || ext == L"mkv" || ext == L"mov";
}

[[nodiscard]] bool IsArchiveExtension(std::wstring_view ext) noexcept
{
    return ext == L"zip" || ext == L"7z" || ext == L"rar" || ext == L"tar" || ext == L"gz";
}

} // namespace

uint32_t IconLibrary::GlyphFor(const FileEntry& entry) noexcept
{
    if (entry.IsDirectory())
    {
        // Known folders get distinct glyphs so the navigation pane and the
        // file list read consistently.
        if (entry.name == L"Desktop")  { return Desktop; }
        if (entry.name == L"Documents") { return Documents; }
        if (entry.name == L"Downloads") { return Downloads; }
        if (entry.name == L"Pictures") { return Pictures; }
        if (entry.name == L"Music")    { return Music; }
        if (entry.name == L"Videos")   { return Videos; }
        return Folder;
    }

    const size_t dot = entry.name.rfind(L'.');
    if (dot == std::wstring::npos || dot + 1 >= entry.name.size())
    {
        return FileGeneric;
    }
    return GlyphForExtension(std::wstring_view{ entry.name }.substr(dot + 1));
}

uint32_t IconLibrary::GlyphForExtension(std::wstring_view ext) noexcept
{
    static ExtensionCache cache;

    const uint64_t key = HashExtension(ext);
    if (const auto it = cache.map.find(key); it != cache.map.end())
    {
        return it->second;
    }

    std::wstring lowered;
    lowered.reserve(ext.size());
    for (const wchar_t c : ext)
    {
        lowered.push_back(static_cast<wchar_t>(std::towlower(c)));
    }

    uint32_t glyph = FileGeneric;
    if (lowered == L"pdf")                       { glyph = 0x1F4D5; } // 📕
    else if (IsImageExtension(lowered))          { glyph = Pictures; }
    else if (IsAudioExtension(lowered))          { glyph = Music; }
    else if (IsVideoExtension(lowered))          { glyph = Videos; }
    else if (IsArchiveExtension(lowered))        { glyph = 0x1F4E6; } // 📦
    else if (lowered == L"exe" || lowered == L"dll" || lowered == L"sys")
                                                 { glyph = 0x2699; }  // ⚙
    else if (IsCodeExtension(lowered))           { glyph = 0x1F4DD; } // 📝
    else if (IsTextExtension(lowered))           { glyph = Documents; }

    // The glyph is stored pre-converted to its UTF-16 hash slot; callers use
    // CodepointToUtf16 themselves, we only cache the code point.
    cache.map.emplace(key, glyph);
    return glyph;
}

uint64_t IconLibrary::HashExtension(std::wstring_view ext) noexcept
{
    uint64_t hash = 1469598103934665603ull;
    for (const wchar_t c : ext)
    {
        const uint64_t lowered = static_cast<uint64_t>(std::towlower(c)) & 0xFFull;
        hash ^= lowered;
        hash *= 1099511628211ull;
    }
    return hash;
}

} // namespace DragonOS::Explorer2
