/**
 * @file    WindowCollection.hpp
 * @brief   Ordered container of all active DragonWindows.
 *
 * Windows are stored in back-to-front Z-order (index 0 = bottom-most).
 * Helper methods provide sorted access for rendering and hit-testing.
 */

#pragma once

#include <memory>
#include <string_view>
#include <vector>

namespace DragonOS::Graphics   { class Renderer; }
namespace DragonOS::WindowManager { class DragonWindow; }

namespace DragonOS::WindowManager {

/**
 * @brief  Manages the list of live DragonWindows.
 *
 * Handles insertion, removal, Z-order promotion, and batch
 * Render / Update for every window in the collection.
 */
class WindowCollection final {
public:
    WindowCollection() noexcept = default;
    ~WindowCollection() noexcept;

    WindowCollection(const WindowCollection&)            = delete;
    WindowCollection& operator=(const WindowCollection&) = delete;
    WindowCollection(WindowCollection&&)                 = delete;
    WindowCollection& operator=(WindowCollection&&)      = delete;

    // ── Window management ─────────────────────────────────────────────────

    /**
     * @brief  Add a window to the collection (top-most Z-order).
     * @return Raw pointer to the inserted window.
     */
    DragonWindow* Add(std::unique_ptr<DragonWindow> window) noexcept;

    /**
     * @brief  Remove a window from the collection.
     * @return true if the window was found and removed.
     */
    bool Remove(DragonWindow* window) noexcept;

    /// @brief  Move a window to the top of the Z-order.
    void BringToFront(DragonWindow* window) noexcept;

    /**
     * @brief  Find a window by title (first match).
     * @return Pointer, or nullptr if not found.
     */
    [[nodiscard]] DragonWindow* Find(std::wstring_view title) noexcept;

    // ── Batch operations ──────────────────────────────────────────────────

    /// @brief  Render every window in Z-order (back to front).
    void Render(Graphics::Renderer& renderer) noexcept;

    /// @brief  Update every window.
    void Update(float deltaTime) noexcept;

    // ── Accessors ─────────────────────────────────────────────────────────

    [[nodiscard]] const std::vector<std::unique_ptr<DragonWindow>>& GetAll() const noexcept
    {
        return m_windows;
    }

    [[nodiscard]] size_t Count() const noexcept { return m_windows.size(); }
    [[nodiscard]] bool   Empty() const noexcept { return m_windows.empty(); }

    /// @brief  Remove every window from the collection.
    void Clear() noexcept;

private:
    std::vector<std::unique_ptr<DragonWindow>> m_windows;
};

} // namespace DragonOS::WindowManager
