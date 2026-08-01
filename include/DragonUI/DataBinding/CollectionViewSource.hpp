#pragma once

#include <DragonUI/Core/VirtualItemSource.hpp>
#include <DragonUI/Controls/ListView.hpp>
#include <DragonUI/DataBinding/ObservableCollection.hpp>

#include <any>
#include <functional>
#include <memory>

namespace DragonOS::DragonUI {

/**
 * @brief Adapter that exposes an ObservableCollection through the
 *        VirtualItemSource interface used by UIListView.
 *
 * Whenever the backing collection changes, the bound list view (if any) is
 * refreshed automatically, and the optional OnCollectionChanged callback is
 * raised.
 *
 * \code
 * auto items = MakeObservableCollection<std::wstring>();
 * auto source = CollectionViewSource<std::wstring>::Create(items);
 * listView->SetItemSource(source);
 * source->SetListView(listView);           // auto-refresh on every change
 * items->Add(L"New item");                  // list view updates itself
 * \endcode
 *
 * The bound list view owns this source (UIListView keeps it alive through its
 * SetItemSource shared_ptr), so the raw list-view handle never dangles.
 *
 * @tparam T The item type stored in the collection.
 */
template <typename T>
class CollectionViewSource final : public VirtualItemSource,
                                   public std::enable_shared_from_this<CollectionViewSource<T>> {
public:
    /// Creates a source bound to @p collection.  Use this instead of
    /// make_shared so the change listener can capture a weak reference safely.
    static std::shared_ptr<CollectionViewSource<T>> Create(ObservableCollectionPtr<T> collection) noexcept
    {
        auto source = std::shared_ptr<CollectionViewSource<T>>(new CollectionViewSource<T>(std::move(collection)));
        source->Initialize();
        return source;
    }

    /// Binds a list view that is refreshed automatically on every change.
    /// The list view must outlive this source; it does, because UIListView
    /// owns the source it was given through SetItemSource.
    void SetListView(UIListView* view) noexcept { m_listView = view; }

    /// Assigns a callback raised whenever the collection changes.
    void SetOnCollectionChanged(std::function<void()> cb) noexcept { m_onCollectionChanged = std::move(cb); }

    [[nodiscard]] std::shared_ptr<ObservableCollection<T>> GetCollection() const noexcept { return m_collection; }

    [[nodiscard]] int64_t GetCount() const noexcept override
    {
        return m_collection ? m_collection->GetCount() : 0;
    }

    [[nodiscard]] std::any GetItem(int64_t index) const override
    {
        if (m_collection && index >= 0 && index < m_collection->GetCount())
            return m_collection->Get(index);
        return std::any{};
    }

private:
    CollectionViewSource(ObservableCollectionPtr<T> collection) noexcept
        : m_collection(std::move(collection))
    {
    }

    void Initialize() noexcept
    {
        std::weak_ptr<CollectionViewSource<T>> weak = this->shared_from_this();
        m_listener = m_collection->AddListener(
            [weak](const typename ObservableCollection<T>::Change&)
            {
                if (auto self = weak.lock())
                    self->OnCollectionChanged();
            });
    }

    void OnCollectionChanged() noexcept
    {
        if (m_onCollectionChanged)
            m_onCollectionChanged();
        if (m_listView)
            m_listView->Refresh();
    }

    ObservableCollectionPtr<T> m_collection;
    typename ObservableCollection<T>::Listener m_listener;
    UIListView* m_listView{};
    std::function<void()> m_onCollectionChanged;
};

} // namespace DragonOS::DragonUI
