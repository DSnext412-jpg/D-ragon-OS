// ============================================================================
//  PreviewPane.hpp — Preview pane + provider framework (Part 6).
//
//  Shows a rich preview of the single selected entry:
//    - images  → large glyph preview,
//    - text    → first bytes rendered as wrapped text in a scroll viewer,
//    - pdf     → IPreviewProvider plugins; placeholder until one registers,
//    - default → icon + metadata block.
//
//  Third parties extend coverage by registering an IPreviewProvider through
//  the Explorer plugin host (see ExplorerPluginAPI.hpp).
//
//  Accessibility is handled by the host's AccessibilityManager (Part 11).
// ============================================================================

#pragma once

#include <Explorer2/ExplorerTypes.hpp>
#include <DragonUI/Controls/DockPanel.hpp>
#include <DragonUI/Controls/Label.hpp>
#include <DragonUI/Controls/StackPanel.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace DragonOS::Explorer2 {

class IPreviewProvider {
public:
    virtual ~IPreviewProvider() = default;
    /// @return true when this provider can render @p entry.
    [[nodiscard]] virtual bool CanPreview(const FileSystem::FileEntry& entry) noexcept = 0;
    /// Builds the preview content element.  Ownership transfers to the pane.
    [[nodiscard]] virtual std::unique_ptr<DragonUI::Element> CreatePreview(
        const FileSystem::FileEntry& entry) noexcept = 0;
};

class PreviewPane final {
public:
    PreviewPane() noexcept;

    [[nodiscard]] DragonUI::Element* GetControl() noexcept { return m_root.get(); }
    /// Transfers the root panel into the owning UI tree (assembly-time).
    [[nodiscard]] std::unique_ptr<DragonUI::Element> TakeRoot() noexcept
    {
        return std::move(m_root);
    }

    void ShowEntry(const FileSystem::FileEntry& entry) noexcept;
    void ShowMultiSelection(size_t count) noexcept;
    void Clear() noexcept;

    void RegisterProvider(IPreviewProvider* provider) noexcept;

private:
    void BuildHeader(const std::wstring& title, uint32_t glyph) noexcept;
    void AddMetadataBlock(const FileSystem::FileEntry& entry) noexcept;
    bool TryPluginPreview(const FileSystem::FileEntry& entry) noexcept;
    void TryTextPreview(const FileSystem::FileEntry& entry) noexcept;
    void ShowPlaceholder(const std::wstring& message) noexcept;

    std::unique_ptr<DragonUI::Element> m_root{std::make_unique<DragonUI::UIDockPanel>()};
    Element* m_releasedRoot{};  ///< Alias kept after TakeRoot.
    Element* m_content{};       ///< Stack panel for content
    Element* m_titleLabel{};    ///< Title label

    std::vector<IPreviewProvider*> m_providers;

    // Accessibility peers are created by the host's AccessibilityManager
};
} // namespace DragonOS::Explorer2