#pragma once

#include <Input/HitTest.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace DragonOS::FileSystem { struct FileEntry; }

namespace DragonOS::Explorer {

enum class ViewMode : uint8_t {
    Grid,
    List,
    Details,
};

enum class ExplorerHitRegion : uint8_t {
    None,
    ToolbarBack,
    ToolbarForward,
    ToolbarUp,
    ToolbarRefresh,
    ToolbarNewFolder,
    ToolbarCopy,
    ToolbarPaste,
    ToolbarDelete,
    ToolbarRename,
    ToolbarProperties,
    AddressBar,
    AddressBarItem,
    NavPaneItem,
    FileViewItem,
    FileViewBackground,
    StatusBar,
    ContextMenuItem,
    ScrollBar,
};

struct ContextMenuItem final {
    std::wstring label;
    bool         isEnabled{ true };
    bool         isSeparator{ false };
};

struct NavPaneEntry final {
    std::wstring displayName;
    std::wstring path;
    bool         isExpanded{ false };
    bool         isHovered{ false };
    uint32_t     iconGlyph{ 0 };
};

struct FileViewItem final {
    Input::Bounds bounds{};
    bool          isHovered{ false };
    bool          isSelected{ false };
    size_t        entryIndex{ 0 };
};

struct ExplorerLayout final {
    Input::Bounds clientArea{};

    // Toolbar (button strip at top)
    Input::Bounds toolbarArea{};
    Input::Bounds btnBack{};
    Input::Bounds btnForward{};
    Input::Bounds btnUp{};
    Input::Bounds btnRefresh{};
    Input::Bounds btnNewFolder{};
    Input::Bounds btnCopy{};
    Input::Bounds btnPaste{};
    Input::Bounds btnDelete{};
    Input::Bounds btnRename{};
    Input::Bounds btnProperties{};

    // Address bar
    Input::Bounds addressBarArea{};

    // Navigation pane (left sidebar)
    float     navPaneWidth{ 200.0f };
    Input::Bounds navPaneArea{};

    // File view (right panel)
    Input::Bounds fileViewArea{};

    // Status bar (bottom)
    Input::Bounds statusBarArea{};

    // Scroll
    float scrollOffset{ 0.0f };
    float scrollContentHeight{ 0.0f };
    float scrollMaxOffset{ 0.0f };

    // File view item height constants
    static constexpr float GridItemSize   = 80.0f;
    static constexpr float GridGap        = 8.0f;
    static constexpr int   GridCols       = 6;
    static constexpr float ListItemHeight = 28.0f;
    static constexpr float DetailsItemHeight = 22.0f;
    static constexpr float SectionPadding = 12.0f;
};

} // namespace DragonOS::Explorer
