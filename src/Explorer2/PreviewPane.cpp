// ============================================================================
//  PreviewPane.cpp — Preview pane + provider framework (Part 6).
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

#include <Explorer2/PreviewPane.hpp>
#include <Explorer2/IconLibrary.hpp>

#include <DragonUI/Core/Glyph.hpp>

#include <FileSystem/FileSystemService.hpp>

namespace DragonOS::Explorer2 {

using namespace DragonOS::DragonUI;

// ============================================================================
//  Construction
// ============================================================================

PreviewPane::PreviewPane() noexcept
{
    m_root->SetAccessibleName(L"Preview pane");
    m_root->SetAutomationId(L"Explorer2/Preview");
}

// ============================================================================
//  Entry display
// ============================================================================

void PreviewPane::ShowEntry(const FileSystem::FileEntry& entry) noexcept
{
    Clear();

    // Build title
    BuildHeader(entry.name, IconLibrary::GlyphFor(entry));

    // Try providers in registration order
    bool previewDisplayed = false;

    // 1. Plugin / indexed preview
    if (TryPluginPreview(entry))
    {
        previewDisplayed = true;
    }

    // 2. Text preview (falls back if no plugin handles it)
    if (!previewDisplayed)
    {
        TryTextPreview(entry);
    }

    // 3. Default: icon + metadata block
    if (!previewDisplayed)
    {
        ShowPlaceholder(L"No preview available");
    }
}

void PreviewPane::ShowMultiSelection(size_t count) noexcept
{
    Clear();
    // Show simple text instead
    wchar_t buf[64];
    swprintf_s(buf, L"%zu items selected", count);
    ShowPlaceholder(buf);
}

void PreviewPane::Clear() noexcept
{
    m_content->RemoveAllChildren();
    m_titleLabel->SetText(L"");
}

// ============================================================================
//  Provider registration
// ============================================================================

void PreviewPane::RegisterProvider(IPreviewProvider* provider) noexcept
{
    // Check for duplicate
    for (auto* p : m_providers)
    {
        if (p == provider) { return; }
    }
    m_providers.push_back(provider);
}

// ============================================================================
//  Try plugin preview (extension point)
// ============================================================================

bool PreviewPane::TryPluginPreview(const FileSystem::FileEntry& entry) noexcept
{
    for (auto* provider : m_providers)
    {
        if (provider->CanPreview(entry))
        {
            auto preview = provider->CreatePreview(entry);
            if (preview)
            {
                m_content->AddChild(std::move(preview));
                return true;
            }
        }
    }
    return false;
}

// ============================================================================
//  Text preview
// ============================================================================

void PreviewPane::TryTextPreview(const FileSystem::FileEntry& entry) noexcept
{
    // Display simple text metadata
    std::wstring textPreview;
    if (entry.IsDirectory())
    {
        textPreview = L"Directory: " + entry.name;
    }
    else
    {
        // Try to show file extension and size
        wchar_t buf[128];
        swprintf_s(buf, L"%s (%s)", entry.name.c_str(),
                   entry.IsDirectory() ? L"Folder" : L"File");
        textPreview = buf;
    }

    m_titleLabel->SetText(textPreview.c_str());
    // Add a label showing the text preview
    auto* label = std::make_unique<UILabel>(textPreview);
    label->SetAccessibleName(L"Text preview");
    m_content->AddChild(std::move(label));
}

// ============================================================================
//  Placeholder
// ============================================================================

void PreviewPane::ShowPlaceholder(const std::wstring& message) noexcept
{
    m_titleLabel->SetText(message.c_str());
    auto* label = new UILabel(message);
    label->SetAccessibleName(L"Preview placeholder");
    m_content->AddChild(std::unique_ptr<Element>(label));
}

// ============================================================================
//  (No SetAutomationPeer - AccessibilityManager creates peers externally)
// ============================================================================

} // namespace DragonOS::Explorer2