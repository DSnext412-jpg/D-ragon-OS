#pragma once

#include <functional>
#include <memory>
#include <vector>

namespace DragonOS::DragonUI {

/**
 * @brief Observable value holder with change notification.
 *
 * The foundation of the DragonUI data-binding stack.  A ViewModel exposes its
 * mutable state through Observable members; bindings and controls subscribe to
 * the change callback and re-read the value whenever it fires.
 *
 * \code
 * auto name = std::make_shared<Observable<std::wstring>>(L"Dragon");
 * name->SetOnChanged([](const std::wstring& oldValue, const std::wstring& newValue) {
 *     std::wprintf(L"%ls -> %ls\n", oldValue.c_str(), newValue.c_str());
 * });
 * name->Set(L"Phoenix");
 * \endcode
 *
 * Multiple listeners are supported.  AddListener returns a RAII Listener token
 * that removes the subscription automatically when it is destroyed (or moved),
 * so bindings never leak callbacks.
 *
 * @tparam T The observed value type.
 */
template <typename T>
class Observable : public std::enable_shared_from_this<Observable<T>> {
public:
    using ChangeCallback = std::function<void(const T& oldValue, const T& newValue)>;

    /**
     * @brief RAII subscription handle returned by AddListener.
     *
     * Holds a weak reference to its Observable so it is safe regardless of
     * destruction order.  Destroying (or move-assigning over) the token
     * unsubscribes it.
     */
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener&) = delete;
        Listener& operator=(const Listener&) = delete;

        Listener(Listener&& other) noexcept { *this = std::move(other); }

        Listener& operator=(Listener&& other) noexcept
        {
            if (this != &other)
            {
                Release();
                m_observable = std::move(other.m_observable);
                m_id = other.m_id;
                other.m_id = 0;
            }
            return *this;
        }

        ~Listener() { Release(); }

        [[nodiscard]] explicit operator bool() const noexcept { return m_id != 0; }

        /// Unsubscribes immediately without waiting for destruction.
        void Detach() noexcept { Release(); }

    private:
        friend class Observable;

        Listener(std::weak_ptr<Observable<T>> observable, uint64_t id) noexcept
            : m_observable(std::move(observable))
            , m_id(id)
        {
        }

        void Release() noexcept
        {
            if (m_id == 0)
                return;
            if (auto observable = m_observable.lock())
                observable->RemoveListener(m_id);
            m_id = 0;
            m_observable.reset();
        }

        std::weak_ptr<Observable<T>> m_observable;
        uint64_t m_id{};
    };

    Observable() = default;
    explicit Observable(const T& value) noexcept
        : m_value(value)
    {
    }
    explicit Observable(T&& value) noexcept
        : m_value(std::move(value))
    {
    }

    Observable(const Observable&) = delete;
    Observable& operator=(const Observable&) = delete;

    /// Registers a multicast listener and returns its RAII token.
    Listener AddListener(ChangeCallback cb) noexcept
    {
        auto id = m_nextId++;
        m_listeners.push_back(ListenerEntry{id, std::move(cb)});
        return Listener{this->weak_from_this(), id};
    }

    /// Assigns the convenience single-listener callback.  Calling this again
    /// replaces the previous callback; it is invoked in addition to any
    /// listeners registered through AddListener.
    void SetOnChanged(ChangeCallback cb) noexcept { m_onChanged = std::move(cb); }

    /// Stores @p value and raises all change callbacks unconditionally.
    void Set(const T& value) noexcept
    {
        T old = m_value;
        m_value = value;
        Notify(old, m_value);
    }

    /// Stores @p value and raises all change callbacks unconditionally.
    void Set(T&& value) noexcept
    {
        T old = m_value;
        m_value = std::move(value);
        Notify(old, m_value);
    }

    /// Stores @p value without raising any change callbacks.
    void SetSilently(const T& value) noexcept { m_value = value; }

    /// Stores @p value without raising any change callbacks.
    void SetSilently(T&& value) noexcept { m_value = std::move(value); }

    /// Stores @p value but only raises the callbacks when it differs from the
    /// current value.  Two-way bindings use this so a target writing back an
    /// unchanged value never causes a notification cycle.
    void SetIfChanged(const T& value) noexcept
    {
        if (!(m_value == value))
            Set(value);
    }

    /// Stores @p value but only raises the callbacks when it differs from the
    /// current value.  Two-way bindings use this so a target writing back an
    /// unchanged value never causes a notification cycle.
    void SetIfChanged(T&& value) noexcept
    {
        if (!(m_value == value))
            Set(std::move(value));
    }

    [[nodiscard]] const T& Get() const noexcept { return m_value; }
    [[nodiscard]] const T& GetValue() const noexcept { return m_value; }

    [[nodiscard]] const T& operator*() const noexcept { return m_value; }
    [[nodiscard]] const T* operator->() const noexcept { return &m_value; }
    explicit operator T() const noexcept { return m_value; }

    Observable& operator=(const T& value) noexcept
    {
        Set(value);
        return *this;
    }

    Observable& operator=(T&& value) noexcept
    {
        Set(std::move(value));
        return *this;
    }

private:
    friend class Listener;

    void RemoveListener(uint64_t id) noexcept
    {
        for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it)
        {
            if (it->id == id)
            {
                m_listeners.erase(it);
                return;
            }
        }
    }

    void Notify(const T& oldValue, const T& newValue) noexcept
    {
        if (m_onChanged)
            m_onChanged(oldValue, newValue);
        if (m_listeners.empty())
            return;
        std::vector<ChangeCallback> callbacks;
        callbacks.reserve(m_listeners.size());
        for (const auto& entry : m_listeners)
            callbacks.push_back(entry.cb);
        for (const auto& cb : callbacks)
        {
            if (cb)
                cb(oldValue, newValue);
        }
    }

    struct ListenerEntry {
        uint64_t id{};
        ChangeCallback cb;
    };

    T m_value{};
    ChangeCallback m_onChanged;
    std::vector<ListenerEntry> m_listeners;
    uint64_t m_nextId{1};
};

/// Convenience alias for the shared-ownership form used by bindings.
template <typename T>
using ObservablePtr = std::shared_ptr<Observable<T>>;

/// Creates a shared observable with a default-constructed value.
template <typename T>
[[nodiscard]] ObservablePtr<T> MakeObservable()
{
    return std::make_shared<Observable<T>>();
}

/// Creates a shared observable initialized with @p value.
template <typename T>
[[nodiscard]] ObservablePtr<T> MakeObservable(const T& value)
{
    return std::make_shared<Observable<T>>(value);
}

/// Creates a shared observable initialized with @p value.
template <typename T>
[[nodiscard]] ObservablePtr<T> MakeObservable(T&& value)
{
    return std::make_shared<Observable<T>>(std::move(value));
}

} // namespace DragonOS::DragonUI
