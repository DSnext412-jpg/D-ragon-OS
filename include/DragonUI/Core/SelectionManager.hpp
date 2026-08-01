#pragma once

#include <cstdint>
#include <functional>
#include <vector>

namespace DragonOS::DragonUI {

enum class SelectionMode : uint8_t {
    None,   //< No selection allowed.
    Single, //< At most one item selected.
    Multi,  //< Multiple items; Ctrl toggles, Shift extends a range.
};

/**
 * @brief  Reusable selection engine shared by UIListView, UITreeView,
 *         and UIGridView.
 *
 * The engine owns the set of selected indices plus the anchor used to
 * build Shift-ranges.  It is deliberately independent of any concrete
 * control so the exact same semantics apply everywhere:
 *
 *   - plain click     → select only the clicked item
 *   - Ctrl+click      → toggle the clicked item (keep the rest)
 *   - Shift+click     → select the range from the anchor to the item
 *   - Ctrl+Shift+click→ extend the current selection by the range
 *   - Ctrl+A          → select all
 */
class SelectionManager final {
public:
    using ChangeCallback = std::function<void(const SelectionManager&)>;

    SelectionManager() noexcept = default;

    void SetMode(SelectionMode mode) noexcept;
    [[nodiscard]] SelectionMode GetMode() const noexcept { return m_mode; }

    // ── Item count ───────────────────────────────────────────────────

    void SetItemCount(int64_t count) noexcept;
    [[nodiscard]] int64_t GetItemCount() const noexcept { return m_itemCount; }

    // ── Interaction ──────────────────────────────────────────────────

    void OnPointerClick(int64_t index, bool ctrl, bool shift) noexcept;
    void OnKeyMove(int64_t index, bool ctrl, bool shift) noexcept;
    void SelectSingle(int64_t index) noexcept;
    void Toggle(int64_t index) noexcept;
    void SelectRange(int64_t from, int64_t to, bool additive) noexcept;
    void SelectAll() noexcept;
    void Clear() noexcept;

    // ── Queries ──────────────────────────────────────────────────────

    [[nodiscard]] bool IsSelected(int64_t index) const noexcept;
    [[nodiscard]] int64_t GetSelectedCount() const noexcept { return static_cast<int64_t>(m_selected.size()); }
    [[nodiscard]] const std::vector<int64_t>& GetSelectedIndices() const noexcept { return m_selected; }
    [[nodiscard]] int64_t GetFirstSelected() const noexcept;
    [[nodiscard]] int64_t GetLastSelected() const noexcept;

    // ── Navigation anchor / current ──────────────────────────────────

    void SetAnchor(int64_t index) noexcept;
    void SetCurrent(int64_t index) noexcept;
    [[nodiscard]] int64_t GetAnchor() const noexcept { return m_anchor; }
    [[nodiscard]] int64_t GetCurrent() const noexcept { return m_current; }

    void SetOnChanged(ChangeCallback cb) noexcept { m_onChanged = std::move(cb); }

private:
    [[nodiscard]] bool Contains(int64_t index) const noexcept;
    void InsertSorted(int64_t index) noexcept;
    void NotifyChanged() noexcept;

    SelectionMode m_mode{SelectionMode::Single};
    std::vector<int64_t> m_selected;
    int64_t m_itemCount{};
    int64_t m_anchor{-1};
    int64_t m_current{-1};
    ChangeCallback m_onChanged;
    bool m_suppressNotify{};
};

} // namespace DragonOS::DragonUI
