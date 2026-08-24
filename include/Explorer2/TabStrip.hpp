// ============================================================================
//  TabStrip.hpp — Document tabs with drag-reorder (Part 8).
//
//  Data-driven: the window owns std::vector<TabState>; the strip renders one
//  button cell per tab (title + close).  Interactions:
//    - click            → activate
//    - middle click / ✕ → close
//    - drag horizontally→ live reorder (model order mutates, strip rebuilds)
//    - "+"              → new tab request
//
//  Restore support lives in the window (closed-tab stack + Ctrl+Shift+T).
//
//  Because Container::RemoveChild destroys children, reordering rebuilds the
//  strip from the model — cheap for tab counts (< 32).
//
//  All DragonUI controls are used exclusively.  Accessibility is handled by
//  the host's AccessibilityManager (Part 11).
// ============================================================================

#pragma once

#include <Explorer2/ExplorerTypes.hpp>
#include <DragonUI/Controls/Button.hpp>
#include <DragonUI/Core/Container.hpp>
#include <DragonUI/Controls/DockPanel.hpp>

#include <functional>
#include <vector>

namespace DragonOS::Explorer2 {

class TabStrip final : public DragonUI::Container {
public:
    struct Host final {
        std::function<void(uint64_t)> onActivate;
        std::function<void(uint64_t)> onClose;
        std::function<void(size_t, size_t)> onReorder;   ///< from → to.
        std::function<void()> onNewTabRequest;
        std::function<const std::vector<TabState>&()> getTabs;
        std::function<uint64_t()> getActiveTabId;
    };

    TabStrip() noexcept;

    void SetHost(Host host) noexcept;

    /// Rebuilds cells to match the current model.  Call after any change.
    void Rebuild() noexcept;

    // ── Input entry points (routed by ExplorerWindow's pump) ─────────────

    /// @return true when a left press landed on a cell and started tracking.
    bool HandleMousePress(float x, float y, bool middleButton) noexcept;

    /// Continues/ends drag tracking.  @p isRelease ends the gesture.
    bool HandleMouseMove(float x, float y, bool leftHeld) noexcept;
    bool HandleMouseRelease(float x, float y) noexcept;

    [[nodiscard]] bool IsDragging() const noexcept { return m_dragIndex >= 0; }

private:
    struct Cell final {
        uint64_t tabId{};
        float x{};
        float width{};
        Element* panel{};
        Button* titleButton{};
        Button* closeButton{};
    };

    void BuildCells() noexcept;
    [[nodiscard]] int HitTestCell(float x, float y) const noexcept;
    [[nodiscard]] size_t InsertionIndexForX(float x) const noexcept;

    [[nodiscard]] static float EstimateCellWidth(const std::wstring& title) noexcept;

    static constexpr float kStripHeight = 34.0f;
    static constexpr float kCellPadding = 8.0f;
    static constexpr float kCloseButtonWidth = 22.0f;
    static constexpr float kNewTabWidth = 30.0f;
    static constexpr float kDragThreshold = 6.0f;

    Host m_host;
    std::vector<Cell> m_cells;

    int  m_pressedIndex{ -1 };
    int  m_dragIndex{ -1 };
    float m_pressX{};
    bool  m_dragMoved{};

    // Accessibility peer is created by host's AccessibilityManager
};
} // namespace DragonOS::Explorer2