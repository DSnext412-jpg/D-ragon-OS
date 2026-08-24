// ============================================================================
//  IconLibrary.hpp — Glyph resolution for file system entries.
//
//  DragonUI renders icons as Unicode glyph code points (see CodepointToUtf16).
//  The library maps entries to glyphs by kind/extension and caches extension
//  lookups so repeated listings stay cheap.
// ============================================================================

#pragma once

#include <FileSystem/FileEntry.hpp>

#include <cstdint>
#include <string_view>
#include <unordered_map>

namespace DragonOS::Explorer2 {

class IconLibrary final {
public:
    // ── Well-known glyphs ────────────────────────────────────────────────
    static constexpr uint32_t Folder        = 0x1F4C1; // 📁
    static constexpr uint32_t FolderOpen    = 0x1F4C2; // 📂
    static constexpr uint32_t Drive         = 0x1F4BE; // 💾
    static constexpr uint32_t Desktop       = 0x1F5A8; // 🖨 (desktop pc fallback)
    static constexpr uint32_t Documents     = 0x1F4C4; // 📄
    static constexpr uint32_t Downloads     = 0x1F4E5; // 📥
    static constexpr uint32_t Pictures      = 0x1F5BC; // 🖼
    static constexpr uint32_t Music         = 0x1F3B5; // 🎵
    static constexpr uint32_t Videos        = 0x1F3AC; // 🎬
    static constexpr uint32_t Home          = 0x1F3E0; // 🏠
    static constexpr uint32_t Network       = 0x1F310; // 🌐
    static constexpr uint32_t ThisPC        = 0x1F4BB; // 💻
    static constexpr uint32_t Star          = 0x2B50;  // ⭐
    static constexpr uint32_t FileGeneric   = 0x1F5CB; // 🗋 blank page
    static constexpr uint32_t SearchGlyph   = 0x1F50D; // 🔍
    static constexpr uint32_t ChevronRight  = 0x25B8;  // ▸
    static constexpr uint32_t ChevronDown   = 0x25BE;  // ▾

    /// Glyph for a directory listing entry.
    [[nodiscard]] static uint32_t GlyphFor(const FileSystem::FileEntry& entry) noexcept;

    /// Glyph for a plain folder.
    [[nodiscard]] static uint32_t GlyphForFolder() noexcept { return Folder; }

    /// Glyph for the special "This PC" / drives view.
    [[nodiscard]] static uint32_t GlyphForDrivesView() noexcept { return ThisPC; }

    /// Glyph for an extension without the leading dot ("png").  Cached.
    [[nodiscard]] static uint32_t GlyphForExtension(std::wstring_view ext) noexcept;

private:
    IconLibrary() = delete;

    struct ExtensionCache {
        std::unordered_map<uint64_t, uint32_t> map;
    };

    /// FNV-1a hash of the lowercased extension, used as the cache key.
    [[nodiscard]] static uint64_t HashExtension(std::wstring_view ext) noexcept;
};

} // namespace DragonOS::Explorer2
