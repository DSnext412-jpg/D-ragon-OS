#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace DragonOS::DragonUI {

/**
 * @brief Observable dynamic list of items with change notification.
 *
 * An ObservableCollection is the MVVM counterpart of a vector: it owns its
 * items, supports mutation, and raises a change event describing each
 * mutation.  Virtualized controls (via CollectionViewSource) can refresh from
 * the collection without copying the underlying buffer.
 *
 * Listeners receive a Change describing the action and affected indices.
 * Reset covers bulk changes (Clear, bulk assignment) where individual indices
 * are not meaningful.
 *
 * @tparam T The item type.
 */
template <typename T>
class ObservableCollection : public std::enable_shared_from_this<ObservableCollection<T>> {
public:
    enum class Action : uint8_t {
        Add,     ///< A single item was inserted at index.
        Remove,  ///< A single item was removed at index.
        Replace, ///< The item at index was replaced.
        Move,    ///< The item moved from oldIndex to index.
        Reset,   ///< The collection was cleared or bulk-assigned.
    };

    struct Change {
        Action action{Action::Reset};
        int64_t index{-1};    ///< Primary index (Add/Remove/Replace/Move target).
        int64_t oldIndex{-1}; ///< Move source.
        int64_t count{0};
    };

    using CollectionChangedCallback = std::function<void(const Change&)>;

    /// RAII subscription handle returned by AddListener (see Observable<T>).
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
                m_collection = std::move(other.m_collection);
                m_id = other.m_id;
                other.m_id = 0;
            }
            return *this;
        }

        ~Listener() { Release(); }

        [[nodiscard]] explicit operator bool() const noexcept { return m_id != 0; }
        void Detach() noexcept { Release(); }

    private:
        friend class ObservableCollection;

        Listener(std::weak_ptr<ObservableCollection<T>> collection, uint64_t id) noexcept
            : m_collection(std::move(collection))
            , m_id(id)
        {
        }

        void Release() noexcept
        {
            if (m_id == 0)
                return;
            if (auto collection = m_collection.lock())
                collection->RemoveListener(m_id);
            m_id = 0;
            m_collection.reset();
        }

        std::weak_ptr<ObservableCollection<T>> m_collection;
        uint64_t m_id{};
    };

    using value_type = T;
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    ObservableCollection() = default;
    explicit ObservableCollection(std::initializer_list<T> items)
        : m_items(items)
    {
    }

    ObservableCollection(const ObservableCollection&) = delete;
    ObservableCollection& operator=(const ObservableCollection&) = delete;

    /// Registers a multicast listener and returns its RAII token.
    Listener AddListener(CollectionChangedCallback cb) noexcept
    {
        auto id = m_nextId++;
        m_listeners.push_back(ListenerEntry{id, std::move(cb)});
        return Listener{this->weak_from_this(), id};
    }

    /// Assigns the convenience single-listener callback.
    void SetOnCollectionChanged(CollectionChangedCallback cb) noexcept { m_onChanged = std::move(cb); }

    // ── Queries ────────────────────────────────────────────────────────

    [[nodiscard]] int64_t GetCount() const noexcept { return static_cast<int64_t>(m_items.size()); }
    [[nodiscard]] size_t Size() const noexcept { return m_items.size(); }
    [[nodiscard]] bool IsEmpty() const noexcept { return m_items.empty(); }

    // Vector-compatible aliases so existing code can migrate without churn.
    [[nodiscard]] size_t size() const noexcept { return m_items.size(); }
    [[nodiscard]] bool empty() const noexcept { return m_items.empty(); }
    void clear() noexcept { Clear(); }

    [[nodiscard]] const T& Get(int64_t index) const noexcept { return m_items[static_cast<size_t>(index)]; }
    [[nodiscard]] const T& operator[](int64_t index) const noexcept { return m_items[static_cast<size_t>(index)]; }
    [[nodiscard]] const T& At(int64_t index) const noexcept { return m_items.at(static_cast<size_t>(index)); }

    [[nodiscard]] const_iterator begin() const noexcept { return m_items.begin(); }
    [[nodiscard]] const_iterator end() const noexcept { return m_items.end(); }
    [[nodiscard]] iterator begin() noexcept { return m_items.begin(); }
    [[nodiscard]] iterator end() noexcept { return m_items.end(); }

    // ── Mutation ───────────────────────────────────────────────────────

    /// Appends @p item and raises Add.
    void Add(const T& item) noexcept
    {
        const int64_t index = GetCount();
        m_items.push_back(item);
        Notify({Action::Add, index, -1, 1});
    }

    /// Appends @p item and raises Add.
    void Add(T&& item) noexcept
    {
        const int64_t index = GetCount();
        m_items.push_back(std::move(item));
        Notify({Action::Add, index, -1, 1});
    }

    /// Inserts @p item at @p index and raises Add.
    void Insert(int64_t index, const T& item) noexcept
    {
        m_items.insert(m_items.begin() + index, item);
        Notify({Action::Add, index, -1, 1});
    }

    /// Removes the item at @p index and raises Remove.
    void RemoveAt(int64_t index) noexcept
    {
        if (index < 0 || index >= GetCount())
            return;
        m_items.erase(m_items.begin() + index);
        Notify({Action::Remove, index, -1, 1});
    }

    /// Removes the first occurrence of @p item and returns whether it was found.
    bool Remove(const T& item) noexcept
    {
        auto it = std::find(m_items.begin(), m_items.end(), item);
        if (it == m_items.end())
            return false;
        const int64_t index = static_cast<int64_t>(std::distance(m_items.begin(), it));
        m_items.erase(it);
        Notify({Action::Remove, index, -1, 1});
        return true;
    }

    /// Moves the item from @p oldIndex to @p index and raises Move.
    void Move(int64_t oldIndex, int64_t index) noexcept
    {
        if (oldIndex < 0 || oldIndex >= GetCount() || index < 0 || index > GetCount())
            return;
        if (oldIndex == index)
            return;
        T item = std::move(m_items[static_cast<size_t>(oldIndex)]);
        m_items.erase(m_items.begin() + oldIndex);
        m_items.insert(m_items.begin() + index, std::move(item));
        Notify({Action::Move, index, oldIndex, 1});
    }

    /// Replaces the item at @p index and raises Replace.
    void Replace(int64_t index, const T& item) noexcept
    {
        if (index < 0 || index >= GetCount())
            return;
        m_items[static_cast<size_t>(index)] = item;
        Notify({Action::Replace, index, -1, 1});
    }

    /// Clears every item and raises Reset.
    void Clear() noexcept
    {
        if (m_items.empty())
            return;
        m_items.clear();
        Notify({Action::Reset, -1, -1, 0});
    }

    /// Replaces the entire contents in one Reset change.  Efficient for bulk
    /// loads (e.g. a directory listing) because it avoids per-item changes.
    void Assign(std::vector<T> items) noexcept
    {
        m_items = std::move(items);
        Notify({Action::Reset, -1, -1, static_cast<int64_t>(m_items.size())});
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

    void Notify(const Change& change) noexcept
    {
        if (m_onChanged)
            m_onChanged(change);
        if (m_listeners.empty())
            return;
        std::vector<CollectionChangedCallback> callbacks;
        callbacks.reserve(m_listeners.size());
        for (const auto& entry : m_listeners)
            callbacks.push_back(entry.cb);
        for (const auto& cb : callbacks)
        {
            if (cb)
                cb(change);
        }
    }

    struct ListenerEntry {
        uint64_t id{};
        CollectionChangedCallback cb;
    };

    std::vector<T> m_items;
    CollectionChangedCallback m_onChanged;
    std::vector<ListenerEntry> m_listeners;
    uint64_t m_nextId{1};
};

/// Convenience alias for shared collection ownership.
template <typename T>
using ObservableCollectionPtr = std::shared_ptr<ObservableCollection<T>>;

/// Creates an empty shared observable collection.
template <typename T>
[[nodiscard]] ObservableCollectionPtr<T> MakeObservableCollection()
{
    return std::make_shared<ObservableCollection<T>>();
}

/// Creates a shared observable collection initialized with @p items.
template <typename T>
[[nodiscard]] ObservableCollectionPtr<T> MakeObservableCollection(std::vector<T> items)
{
    return std::make_shared<ObservableCollection<T>>(std::move(items));
}

} // namespace DragonOS::DragonUI
