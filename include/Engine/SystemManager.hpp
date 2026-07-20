/**
 * @file    SystemManager.hpp
 * @brief   Ordered registry of all runtime systems.
 *
 * SystemManager owns every System via unique_ptr and provides
 * batch lifecycle calls (InitializeAll, UpdateAll, RenderAll, …).
 * Systems render in registration order (back-to-front).
 */

#pragma once

#include <memory>
#include <type_traits>
#include <vector>

namespace DragonOS::Engine {

class System;
class EngineContext;

/**
 * @brief  Manages the lifecycle and iteration of all Engine systems.
 *
 * Systems are stored in a std::vector and are rendered in the order
 * they were registered (background → desktop → windows → overlays).
 */
class SystemManager final {
public:
    SystemManager() noexcept = default;
    ~SystemManager() noexcept;

    SystemManager(const SystemManager&)            = delete;
    SystemManager& operator=(const SystemManager&) = delete;
    SystemManager(SystemManager&&)                 = delete;
    SystemManager& operator=(SystemManager&&)      = delete;

    // ── Registration ────────────────────────────────────────────────────

    /**
     * @brief  Construct and register a new system.
     *
     * @tparam T     Concrete System type.
     * @tparam Args  Constructor argument types for T.
     * @return Raw pointer to the registered system.
     */
    template<typename T, typename... Args,
             typename = std::enable_if_t<std::is_base_of_v<System, T>>>
    T* Register(Args&&... args)
    {
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        auto* raw   = system.get();
        m_systems.push_back(std::move(system));
        return raw;
    }

    /// @brief  Remove a previously registered system by pointer.
    bool Remove(System* system) noexcept;

    /**
     * @brief  Find a registered system by its concrete type.
     * @return Pointer or nullptr if not found.
     */
    template<typename T>
    [[nodiscard]] T* Find() const noexcept
    {
        for (auto& s : m_systems)
        {
            auto* casted = dynamic_cast<T*>(s.get());
            if (casted) { return casted; }
        }
        return nullptr;
    }

    // ── Batch lifecycle ─────────────────────────────────────────────────

    /// @brief  Call Initialize() on every registered system.
    void InitializeAll(EngineContext& ctx) noexcept;

    /// @brief  Call Shutdown() on every registered system (reverse order).
    void ShutdownAll() noexcept;

    /// @brief  Call Update() on every registered system.
    void UpdateAll(float deltaTime) noexcept;

    /// @brief  Call Render() on every registered system.
    void RenderAll(EngineContext& ctx) noexcept;

    /// @brief  Call Resize() on every registered system.
    void ResizeAll(float width, float height) noexcept;

    // ── Accessors ───────────────────────────────────────────────────────

    [[nodiscard]] size_t Count() const noexcept { return m_systems.size(); }
    [[nodiscard]] bool   Empty()  const noexcept { return m_systems.empty(); }

private:
    std::vector<std::unique_ptr<System>> m_systems;
};

} // namespace DragonOS::Engine
