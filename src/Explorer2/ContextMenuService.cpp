#include <Explorer2/ContextMenuService.hpp>
#include <Explorer2/ExplorerTypes.hpp>
#include <Explorer2/IconLibrary.hpp>

#include <DragonUI/Controls/ContextMenu.hpp>
#include <DragonUI/Controls/Button.hpp>
#include <DragonUI/Controls/Separator.hpp>

namespace DragonOS::Explorer2 {

using namespace DragonOS::DragonUI;

// ============================================================================
//  Extension management
// ============================================================================

void ContextMenuService::AddExtension(IExplorerContextMenuExtension* extension) noexcept
{
    m_extensions.push_back(extension);
}

void ContextMenuService::RemoveExtension(IExplorerContextMenuExtension* extension) noexcept
{
    auto it = std::find(m_extensions.begin(), m_extensions.end(), extension);
    if (it != m_extensions.end())
    {
        m_extensions.erase(it);
    }
}

// ============================================================================
//  Menu building
// ============================================================================

void ContextMenuService::ShowBackgroundMenu(float x, float y, const State& state) const noexcept
{
    auto menu = std::make_unique<UIContextMenu>();

    // Always-appended items
    if (state.canGoBack && state.canGoBack())
    {
        menu->AddItem(L"Back", [&state]() { state.target->NavigateBack(); });
    }
    if (state.canGoForward && state.canGoForward())
    {
        menu->AddItem(L"Forward", [&state]() { state.target->NavigateForward(); });
    }

    // Plugin extensions
    AppendPluginItems(*menu, state);

    // Open with (from plugin host)
    AppendOpenWithItems(*menu, state);

    // Separator
    menu->AddSeparator();

    // Properties
    menu->AddItem(L"Properties", [&state]() { state.target->ShowProperties(&state.selection.empty() ? nullptr : &state.selection[0]); });

    // If background (no selection), show additional items
    if (state.selection.empty())
    {
        menu->AddItem(L"New folder", [&state]() { state.target->NewFolder(); });
        menu->AddSeparator();
        menu->AddItem(L"Paste", [&state]() { state.target->PasteIntoCurrentFolder(); });
    }

    // Route to host overlay system
    if (m_sink)
    {
        m_sink(std::move(menu), x, y);
    }
}

void ContextMenuService::ShowItemMenu(float x, float y, const State& state) const noexcept
{
    auto menu = std::make_unique<UIContextMenu>();

    const bool hasSingle = state.selection.size() == 1;
    const auto& entry = hasSingle ? state.selection[0] : FileSystem::FileEntry{};

    // Open
    menu->AddItem(L"Open", [&state]() {
        if (!state.selection.empty()) state.target->OpenEntry(state.selection[0]);
    });

    // Properties
    menu->AddItem(L"Properties", [&state]() { state.target->ShowProperties(hasSingle ? &entry : nullptr); });

    // Edit / Rename
    if (hasSingle)
    {
        menu->AddItem(L"Rename", [&state]() { state.target->RenameSelection(); });
    }

    // Delete
    menu->AddItem(L"Delete", [&state]() { state.target->DeleteSelection(); });

    // Copy / Cut / Paste
    menu->AddItem(L"Copy", [&state]() { state.target->CopySelection(); });
    menu->AddItem(L"Cut", [&state]() { state.target->CutSelection(); });
    if (!state.selection.empty())
    {
        menu->AddItem(L"Paste", [&state]() { state.target->PasteIntoCurrentFolder(); });
    }

    // View options (always available)
    menu->AddSeparator();
    menu->AddItem(L"Properties", [&state]() { state.target->ShowProperties(hasSingle ? &entry : nullptr); });

    // Plugin extensions
    AppendPluginItems(*menu, state);

    // Open with
    AppendOpenWithItems(*menu, state);

    // Route to host overlay system
    if (m_sink)
    {
        m_sink(std::move(menu), x, y);
    }
}

void ContextMenuService::AppendPluginItems(
    UIContextMenu& menu, const State& state) const noexcept
{
    for (auto* ext : m_extensions)
    {
        if (ext->AppliesTo(state.selection, state.currentPath))
        {
            ext->AppendItems(menu, state.selection, state.currentPath);
        }
    }
}

void ContextMenuService::AppendOpenWithItems(
    UIContextMenu& menu, const State& state) const noexcept
{
    if (!m_openWith) { return; }

    // Get open-with actions for the selected file (or background)
    std::vector<OpenWithAction> actions;
    if (state.selection.size() == 1)
    {
        actions = m_openWith(state.selection[0]);
    }

    if (!actions.empty())
    {
        menu->AddSeparator();
        for (const auto& action : actions)
        {
            menu->AddItem(action.label,
                [action = std::move(action)]() { action.action(action); });
        }
    }
}

// ============================================================================
//  (IContextMenuTarget methods are implemented by ExplorerWindow)
// ============================================================================

} // namespace DragonOS::Explorer2