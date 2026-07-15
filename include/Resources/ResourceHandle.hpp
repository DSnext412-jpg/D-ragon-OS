/**
 * @file    ResourceHandle.hpp
 * @brief   Lightweight non-owning handle to a managed Resource.
 *
 * ResourceHandle wraps a std::weak_ptr<Resource> and provides
 * safe access with validity checks.  It is the primary mechanism
 * through which subsystems reference resources without extending
 * their lifetime.
 */

#pragma once

#include <Resources/Resource.hpp>

#include <concepts>
#include <memory>
#include <type_traits>

namespace DragonOS::Resources {

template <typename T>
concept DerivedFromResource = std::derived_from<T, Resource>;

/**
 * @brief  Non-owning, nullable handle to a Resource managed by
 *         the ResourceManager.
 *
 * ResourceHandle is the safe alternative to raw pointers.  It stores
 * a std::weak_ptr internally and validates liveness on every access.
 *
 * @tparam T  Concrete resource type (must derive from Resource).
 */
template <DerivedFromResource T>
class ResourceHandle final {
public:
    /// @brief  Construct a null handle.
    ResourceHandle() noexcept = default;

    /// @brief  Construct from a shared_ptr.
    explicit ResourceHandle(std::shared_ptr<T> ptr) noexcept
        : m_weak(ptr)
    {
    }

    /// @brief  Check whether the underlying resource is still alive.
    [[nodiscard]] bool IsValid() const noexcept
    {
        return !m_weak.expired();
    }

    /// @brief  Lock and retrieve the shared pointer.
    /// @return A shared_ptr<T> if the resource is alive, nullptr otherwise.
    [[nodiscard]] std::shared_ptr<T> Get() const noexcept
    {
        return m_weak.lock();
    }

    /// @brief  Access the resource (checked).
    /// @return A shared_ptr<T> to the live resource.
    /// @warning  Asserts if the resource has been destroyed.
    [[nodiscard]] std::shared_ptr<T> operator->() const noexcept
    {
        return Get();
    }

    /// @brief  Dereference the handle (checked).
    /// @return Reference to the underlying resource.
    /// @warning  Asserts if the resource has been destroyed.
    [[nodiscard]] T& operator*() const noexcept
    {
        return *Get();
    }

    /// @brief  Bool conversion -- true if the resource is alive.
    explicit operator bool() const noexcept { return IsValid(); }

private:
    std::weak_ptr<T> m_weak;
};

} // namespace DragonOS::Resources
