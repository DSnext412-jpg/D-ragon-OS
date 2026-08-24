#include <Explorer2/NavigationPane.hpp>
#include <Explorer2/DirectoryCache.hpp>
#include <Explorer2/IconLibrary.hpp>

#include <DragonUI/Core/Glyph.hpp>
#include <DragonUI/Controls/TreeView.hpp>
#include <DragonUI/Controls/TreeNode.hpp>
#include <DragonUI/Controls/Button.hpp>
#include <DragonUI/Controls/StackPanel.hpp>

#include <FileSystem/FileSystemService.hpp>

namespace DragonOS::Explorer2 {

using namespace DragonOS::DragonUI;

// ============================================================================
//  Construction
// ============================================================================

NavigationPane::NavigationPane(
    FileSystem::FileSystemService& fileSystem,
    DirectoryCache& cache,
    BookmarkStore& bookmarks) noexcept
    : m_fileSystem(fileSystem)
    , m_cache(cache)
    , m_bookmarks(bookmarks)
{
    m_root->SetAccessibleName(L"Navigation pane");
    m_root->SetAutomationId(L"Explorer2/Navigation");

    auto layout = std::make_unique<UIStackPanel>(Orientation::Vertical);
    m_layout = layout.get();
    m_layout->SetSpacing(4.0f);
    m_layout->SetAccessibleName(L"Navigation tree");
    m_root->AddChild(std::move(layout));

    // ── Tree ──────────────────────────────────────────────────────────────
    auto tree = std::make_unique<UITreeView>();
    m_tree = tree.get();
    m_tree->SetRowHeight(24.0f);
    m_tree->SetAccessibleName(L"Folder tree");
    m_tree->SetMinSize(200.0f, 200.0f);

    BuildRoots();

    m_tree->SetOnNodeActivated([this](UITreeView&, UITreeNode& node) {
        if (const std::wstring* path = PathOf(node); path && !path->empty())
        {
            if (m_onNavigate)
            {
                m_onNavigate(*path);
            }
        }
    });

    m_layout->AddChild(std::move(tree));

    // ── Bookmark toolbar ─────────────────────────────────────────────────
    auto pin = std::make_unique<UIButton>(L"Pin current folder");
    m_pinButton = pin.get();
    pin->SetAccessibleName(L"Pin current folder to Quick access");
    pin->SetOnClick([this](UIButton&) {
        if (m_onPinRequested)
        {
            m_onPinRequested();
        }
    });
    m_layout->AddChild(std::move(pin));
}

// ============================================================================
//  Root building
// ============================================================================

void NavigationPane::BuildRoots() noexcept
{
    m_tree->ClearNodes();
    m_nodePaths.clear();

    // ── Quick access ─────────────────────────────────────────────────────
    auto* quick = m_tree->AddRootNode(L"Quick access", IconLibrary::Star);
    quick->SetExpanded(true);
    m_quickAccessNode = quick;

    const struct {
        FileSystem::KnownFolder folder;
        uint32_t glyph;
    } known[] = {
        { FileSystem::KnownFolder::Home,     IconLibrary::Home },
        { FileSystem::KnownFolder::Desktop,  IconLibrary::Desktop },
        { FileSystem::KnownFolder::Documents, IconLibrary::Documents },
        { FileSystem::KnownFolder::Downloads, IconLibrary::Downloads },
        { FileSystem::KnownFolder::Pictures, IconLibrary::Pictures },
        { FileSystem::KnownFolder::Music,    IconLibrary::Music },
        { FileSystem::KnownFolder::Videos,   IconLibrary::Videos },
    };

    for (const auto& [folder, glyph] : known)
    {
        const std::wstring path = m_fileSystem.GetKnownFolderPath(folder);
        const std::wstring name = FileSystem::FileSystemService::GetKnownFolderDisplayName(folder);
        auto* node = AddPathNode(*quick, name, path, glyph);
        if (node)
        {
            node->SetLeaf(false);
            node->SetLazyLoader([this](UITreeNode& dirNode) {
                if (const std::wstring* path = PathOf(dirNode))
                {
                    LoadDirectoryChildren(dirNode, *path);
                }
            });
        }
    }

    // ── This PC ──────────────────────────────────────────────────────────
    auto* pc = m_tree->AddRootNode(L"This PC", IconLibrary::ThisPC);
    pc->SetLazyLoader([this](UITreeNode& node) {
        // First expansion lists the logical drives; each drive lazily loads
        // its own children on demand.
        for (const std::wstring& drive : m_fileSystem.GetLogicalDrives())
        {
            auto* driveNode = AddPathNode(node, drive, drive, IconLibrary::Drive);
            driveNode->SetLeaf(false);
            driveNode->SetLazyLoader([this](UITreeNode& dirNode) {
                if (const std::wstring* path = PathOf(dirNode))
                {
                    LoadDirectoryChildren(dirNode, *path);
                }
            });
        }
    });
    m_thisPCNode = pc;

    // ── Network (placeholder until a network provider exists) ────────────
    auto* network = m_tree->AddRootNode(L"Network", IconLibrary::Network);
    network->SetLeaf(true);

    m_tree->Refresh();
}

// ============================================================================
//  Node helpers
// ============================================================================

DragonUI::UITreeNode* NavigationPane::AddPathNode(
    UITreeNode& parent, const std::wstring& name,
    const std::wstring& path, uint32_t glyph) noexcept
{
    UITreeNode* node = parent.AddChild(name, glyph);
    if (node)
    {
        StorePath(*node, path);
    }
    return node;
}

const std::wstring* NavigationPane::PathOf(const UITreeNode& node) const noexcept
{
    const auto it = m_nodePaths.find(&node);
    return it != m_nodePaths.end() ? &it->second : nullptr;
}

void NavigationPane::StorePath(UITreeNode& node, const std::wstring& path)
{
    m_nodePaths.insert_or_assign(&node, path);
}

// ============================================================================
//  Lazy loading
// ============================================================================

void NavigationPane::LoadDirectoryChildren(UITreeNode& node, const std::wstring& path) noexcept
{
    const DirectoryResult* result = m_cache.Get(path);
    if (!result)
    {
        DirectoryResult listed = m_fileSystem.ListDirectory(path);
        if (!listed.success)
        {
            node.SetLeaf(true);
            return;
        }
        m_cache.Put(path, std::move(listed));
        result = m_cache.Get(path);
        if (!result) { node.SetLeaf(true); return; }
    }

    size_t addedFolders = 0;
    for (const auto& entry : result->entries)
    {
        if (!entry.IsDirectory() || entry.IsHidden())
        {
            continue;
        }
        auto* child = AddPathNode(node, entry.name, entry.fullPath, IconLibrary::GlyphFor(entry));
        if (child)
        {
            child->SetLeaf(false);
            child->SetLazyLoader([this](UITreeNode& grandchild) {
                if (const std::wstring* childPath = PathOf(grandchild))
                {
                    LoadDirectoryChildren(grandchild, *childPath);
                }
            });
            ++addedFolders;
        }
    }

    if (addedFolders == 0)
    {
        node.SetLeaf(true);
    }
}

// ============================================================================
//  Public updates
// ============================================================================

void NavigationPane::RefreshDrives() noexcept
{
    if (m_thisPCNode)
    {
        m_thisPCNode->ClearChildren();
        // Force lazy reload on next expansion.
        m_thisPCNode->SetLeaf(false);
    }
}

void NavigationPane::RefreshQuickAccess() noexcept
{
    if (!m_quickAccessNode)
    {
        return;
    }

    // Drop previous quick-access children and re-add bookmarks.
    m_quickAccessNode->ClearChildren();

    for (const Bookmark& bookmark : m_bookmarks.GetAll())
    {
        auto* node = AddPathNode(*m_quickAccessNode, bookmark.name, bookmark.path,
                                  bookmark.glyph ? bookmark.glyph : IconLibrary::Star);
        if (node)
        {
            node->SetLeaf(false);
            node->SetLazyLoader([this](UITreeNode& dirNode) {
                if (const std::wstring* path = PathOf(dirNode))
                {
                    LoadDirectoryChildren(dirNode, *path);
                }
            });
        }
    }

    m_tree->Refresh();
}

void NavigationPane::HighlightPath(const std::wstring& path) noexcept
{
    if (path.empty() || !m_thisPCNode || !m_quickAccessNode)
    {
        return;
    }

    // Walk the tree matching path segments, expanding as we go.
    const std::vector<std::wstring> segments = SplitPath(path);
    if (segments.empty())
    {
        return;
    }

    UITreeNode* current = nullptr;

    // Check Quick access first (bookmarks / known folders).
    for (auto& child : m_quickAccessNode->GetChildren())
    {
        const std::wstring* childPath = PathOf(*child);
        if (childPath && *childPath == path)
        {
            current = child.get();
            break;
        }
    }

    // Then walk "This PC" → drive → folders by prefix match.
    if (!current)
    {
        current = m_thisPCNode;
        for (size_t i = 0; i < segments.size(); ++i)
        {
            const std::wstring prefix = JoinPathSegments(segments, i + 1);
            bool matched = false;
            for (auto& child : current->GetChildren())
            {
                const std::wstring* childPath = PathOf(*child);
                if (childPath &&
                    (_wcsnicmp(childPath->c_str(), prefix.c_str(), prefix.size()) == 0))
                {
                    current = child.get();
                    matched = true;
                    break;
                }
            }
            if (!matched)
            {
                break;
            }
            current->LoadChildrenIfNeeded();
        }
    }

    if (current && current != m_thisPCNode)
    {
        m_tree->ExpandNode(*current);
        m_tree->SelectNode(*current);
        m_tree->ScrollToNode(*current);
    }
}

// ============================================================================
//  Shared helpers (anonymous namespace)
// ============================================================================

namespace {

[[nodiscard]] std::wstring SplitPath(const std::wstring& path) noexcept
{
    std::vector<std::wstring> result;
    size_t start = 0;
    size_t end = path.find(L'\\', start);
    while (end != std::wstring::npos)
    {
        result.push_back(path.substr(start, end - start));
        start = end + 1;
        end = path.find(L'\\', start);
    }
    result.push_back(path.substr(start));
    return {};
}

[[nodiscard]] std::wstring JoinPathSegments(const std::wstring& path, size_t count) noexcept
{
    if (path.empty()) return {};
    if (count == 0) return {};
    
    size_t end = 0;
    for (size_t i = 0; i < count && i < path.size(); ++i)
    {
        size_t next = path.find(L'\\', end);
        if (next == std::wstring::npos) next = path.size();
        end = next + 1;
    }
    return path.substr(0, end - 1);
}

} // namespace

} // namespace DragonOS::Explorer2