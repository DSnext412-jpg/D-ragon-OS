#pragma once

#include <DragonUI/DataBinding/Observable.hpp>

#include <functional>
#include <memory>
#include <utility>

namespace DragonOS::DragonUI {

/// Determines how a binding synchronizes its source and target.
enum class BindingMode : uint8_t {
    /// Pushes the source value to the target exactly once at bind time.
    OneTime,
    /// Pushes the source value to the target whenever it changes.
    OneWay,
    /// Pushes source changes to the target and target changes back to source.
    TwoWay,
};

/// Type-erased base for all bindings, regardless of the bound value type.
class BindingBase : public std::enable_shared_from_this<BindingBase> {
public:
    virtual ~BindingBase() = default;

    /// Pushes the current source value to the target.
    virtual void Apply() noexcept = 0;

    /// Pushes the current target value back to the source.
    virtual void Pull() noexcept = 0;
};

/**
 * @brief Synchronizes an Observable source with a control property.
 *
 * The target is described by a getter and setter pair; the getter is only used
 * in TwoWay mode and for Pull().  OneTime/OneWay push source-to-target; TwoWay
 * additionally subscribes to the target through an optional subscribe hook and
 * writes target changes back to the source via SetIfChanged (so an unchanged
 * write never loops).
 *
 * Bindings are created through BindingManager::Bind, which owns them via
 * shared_ptr so the target's subscribe hook can hold a weak reference that is
 * safe regardless of destruction order.
 *
 * @tparam T The bound value type.
 */
template <typename T>
class Binding final : public BindingBase {
public:
    using Getter = std::function<T()>;
    using Setter = std::function<void(const T& value)>;
    using TargetChangedCallback = std::function<void(T value)>;

    /// Wires a control's own change event to @p onTargetChanged.  The control
    /// invokes the callback with its current value whenever the user edits it.
    using TargetSubscribeHook = std::function<void(TargetChangedCallback onTargetChanged)>;

    /// @param source  The observable that drives the binding.
    /// @param targetGet  Reads the current target value (TwoWay/Pull only).
    /// @param targetSet  Writes a value to the target.
    /// @param mode   The synchronization mode.
    /// @param targetSubscribe  Optional TwoWay hook that routes control edits
    ///                         back to the source.
    Binding(ObservablePtr<T> source,
            Getter targetGet,
            Setter targetSet,
            BindingMode mode = BindingMode::OneWay,
            TargetSubscribeHook targetSubscribe = {})
        : m_source(std::move(source))
        , m_targetGet(std::move(targetGet))
        , m_targetSet(std::move(targetSet))
        , m_mode(mode)
        , m_targetSubscribe(std::move(targetSubscribe))
    {
    }

    void Apply() noexcept override
    {
        if (m_source && m_targetSet)
            m_targetSet(m_source->Get());
    }

    void Pull() noexcept override
    {
        if (m_source && m_targetGet && m_mode == BindingMode::TwoWay)
            m_source->SetIfChanged(m_targetGet());
    }

    /// Manual entry point for TwoWay targets that cannot use the subscribe
    /// hook.  Wire the control's change event to this method.
    void OnTargetChanged(T value) noexcept { PullValue(std::move(value)); }

    /// Wires the source listener and the target subscribe hook.  Called by
    /// BindingManager once the shared_ptr exists, so weak references are valid.
    void Initialize() noexcept
    {
        std::weak_ptr<BindingBase> weak = std::enable_shared_from_this<BindingBase>::shared_from_this();
        m_sourceListener = m_source->AddListener(
            [weak](const T&, const T&)
            {
                if (auto self = weak.lock())
                {
                    if (auto binding = std::static_pointer_cast<Binding<T>>(self))
                        binding->Apply();
                }
            });

        if (m_targetSubscribe)
        {
            m_targetSubscribe(
                [weak](T value)
                {
                    if (auto self = weak.lock())
                    {
                        if (auto binding = std::static_pointer_cast<Binding<T>>(self))
                            binding->PullValue(std::move(value));
                    }
                });
        }
    }

    [[nodiscard]] BindingMode GetMode() const noexcept { return m_mode; }

private:
    void PullValue(T value) noexcept
    {
        if (!m_source)
            return;
        if (m_mode == BindingMode::TwoWay)
            m_source->SetIfChanged(std::move(value));
    }

    ObservablePtr<T> m_source;
    Getter m_targetGet;
    Setter m_targetSet;
    BindingMode m_mode{BindingMode::OneWay};
    TargetSubscribeHook m_targetSubscribe;
    typename Observable<T>::Listener m_sourceListener;
};

} // namespace DragonOS::DragonUI
