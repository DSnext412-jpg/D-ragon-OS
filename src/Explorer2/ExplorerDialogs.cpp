#include <Explorer2/ExplorerDialogs.hpp>
#include <Explorer2/ExplorerTypes.hpp>
#include <FileSystem/FileSystemService.hpp>

#include <DragonUI/Controls/Label.hpp>
#include <DragonUI/Controls/StackPanel.hpp>
#include <DragonUI/Controls/TextBox.hpp>

namespace DragonOS::Explorer2 {

using namespace DragonOS::DragonUI;

namespace {

constexpr float kDialogWidth = 420.0f;
constexpr float kDialogHeight = 180.0f;

/// Shared builder: label + text box + OK/Cancel wired to a commit callback.
std::unique_ptr<UIDialog> CreateTextDialog(
    const std::wstring& title, const std::wstring& prompt,
    const std::wstring& initialText, const ExplorerDialogs::TextCallback& onCommit) noexcept
{
    auto dialog = std::make_unique<UIDialog>(title, /*modal=*/true);
    dialog->SetSize(kDialogWidth, kDialogHeight);
    dialog->SetResizable(false);

    auto promptLabel = std::make_unique<UILabel>(prompt);
    promptLabel->SetAccessibleName(prompt);
    promptLabel->SetAutomationId(L"Explorer2/Dialog/Prompt");
    dialog->AddContent(std::move(promptLabel));

    auto box = std::make_unique<UITextBox>();
    UITextBox* textBox = box.get();
    textBox->SetText(initialText);
    textBox->SelectAll();
    textBox->SetAccessibleName(title);
    textBox->SetAutomationId(L"Explorer2/Dialog/Input");
    dialog->AddContent(std::move(box));

    UIButton* okButton = dialog->AddButton(L"OK", DialogResult::OK);
    UIButton* cancelButton = dialog->AddButton(L"Cancel", DialogResult::Cancel);
    dialog->SetDefaultButton(okButton);
    dialog->SetCancelButton(cancelButton);

    dialog->SetOnClosed([textBox, onCommit](UIDialog&, DialogResult result) {
        if (result == DialogResult::OK && onCommit)
        {
            const std::wstring value = textBox->GetText();
            if (!value.empty())
            {
                onCommit(value);
            }
        }
    });

    return dialog;
}

} // namespace

// ============================================================================
//  Factories
// ============================================================================

std::unique_ptr<UIDialog> ExplorerDialogs::CreateNewFolder(const TextCallback& onCommit) noexcept
{
    return CreateTextDialog(
        L"New folder", L"Choose a folder name:", L"New Folder", onCommit);
}

std::unique_ptr<UIDialog> ExplorerDialogs::CreateRename(
    const std::wstring& currentName, const TextCallback& onCommit) noexcept
{
    return CreateTextDialog(L"Rename", L"Enter the new name:", currentName, onCommit);
}

std::unique_ptr<UIDialog> ExplorerDialogs::CreateProperties(
    const FileSystem::FileEntry& entry) noexcept
{
    auto dialog = std::make_unique<UIDialog>(L"Properties — " + entry.name, true);
    dialog->SetSize(460.0f, 340.0f);

    auto addRow = [&dialog](const std::wstring& label, const std::wstring& value) {
        auto row = std::make_unique<UIStackPanel>(Orientation::Horizontal);
        row->SetSpacing(12.0f);

        auto caption = std::make_unique<UILabel>(label);
        caption->SetTextColor(Theme::SemanticColor::TextSecondary);
        caption->SetAutomationId(L"Explorer2/Properties/" + label + L"/Caption");
        row->AddChild(std::move(caption));

        auto field = std::make_unique<UILabel>(value);
        field->SetWordWrap(true);
        field->SetAutoSize(false);
        field->SetAutomationId(L"Explorer2/Properties/" + label + L"/Value");
        row->AddChild(std::move(field));

        dialog->AddContent(std::move(row));
    };

    addRow(L"Name:", entry.name);
    addRow(L"Type:", GetTypeDisplayName(entry));
    addRow(L"Location:", FileSystem::FileSystemService::GetParentPath(entry.fullPath));
    addRow(L"Size:",
           entry.IsDirectory() ? std::wstring(L"\x2014") : FileSystem::FileSystemService::FormatFileSize(entry.size));
    addRow(L"Modified:", FileSystem::FileSystemService::FormatDateTime(entry.lastModified));

    std::wstring attributes;
    if (entry.IsReadOnly()) { attributes += L"Read-only "; }
    if (entry.IsHidden())   { attributes += L"Hidden "; }
    if (entry.IsDirectory()){ attributes += L"Directory "; }
    addRow(L"Attributes:", attributes.empty() ? std::wstring(L"(none)") : attributes);

    dialog->AddButton(L"Close", DialogResult::Close);
    return dialog;
}

// ============================================================================
//  Positioning
// ============================================================================

void ExplorerDialogs::CenterOverClient(
    UIDialog& dialog,
    float clientX, float clientY, float clientW, float clientH,
    float viewportW, float viewportH) noexcept
{
    // Dialog bounds are absolute; the viewport is the full virtual desktop so
    // the modal backdrop covers everything while the window itself centers
    // over its client area.
    dialog.SetViewport(viewportW, viewportH);

    const float w = dialog.GetWidth();
    const float h = dialog.GetHeight();
    const float x = clientX + ((clientW - w) * 0.5f);
    const float y = clientY + ((clientH - h) * 0.5f);
    dialog.SetPosition(x, y);
}

} // namespace DragonOS::Explorer2
