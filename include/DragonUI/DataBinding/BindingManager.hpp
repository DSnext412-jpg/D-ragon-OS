#pragma once

#include <DragonUI/DataBinding/Binding.hpp>

#include <memory>
#include <vector>

namespace DragonOS::DragonUI {

/**
 * @brief Owns a set of bindings and disposes them together.
 *
 * Bindings are reference-counted so a target's change event may hold a weak
 * reference that never dangles.  Destroying the manager (or calling Clear)
 * removes every binding's source listener, so nothing fires afterwards.
 *
 * A BindingManager is typically a member of the view/section that owns both
 * the observables and the controls, keeping binding lifetimes and control
 * lifetimes identical.
 */
class BindingManager {
public:
    BindingManager() = default;
    ~BindingManager() = default;

    BindingManager(const BindingManager&) = delete;
    BindingManager& operator=(const BindingManager&) = delete;

    /**
     * @brief Creates, stores and initializes a binding.
     *
     * The binding is always pushed source-to-target once at bind time.  For
     * OneTime that is the only transfer; OneWay continues on source changes;
     * TwoWay additionally writes target edits back to the source.
     *
     * @tparam T  The bound value type.
     * @param source        The observable that drives the binding.
     * @param targetGet     Reads the current target value (TwoWay/Pull).
     * @param targetSet     Writes a value to the target.
     * @param mode          The synchronization mode.
     * @param targetSubscribe  Optional TwoWay hook that routes control edits
     *                         back to the source.
     * @return The shared binding, kept alive by this manager; the caller may
     *         also keep a copy to wire manual target events.
     */
    template <typename T>
    std::shared_ptr<Binding<T>> Bind(
        ObservablePtr<T> source,
        typename Binding<T>::Getter targetGet,
        typename Binding<T>::Setter targetSet,
        BindingMode mode = BindingMode::OneWay,
        typename Binding<T>::TargetSubscribeHook targetSubscribe = {})
    {
        auto binding = std::make_shared<Binding<T>>(
            std::move(source), std::move(targetGet), std::move(targetSet), mode, std::move(targetSubscribe));
        m_bindings.push_back(binding);
        binding->Initialize();
        binding->Apply();
        return binding;
    }

    /// Unsubscribes and drops every owned binding.
    void Clear() noexcept
    {
        for (auto& binding : m_bindings)
            binding.reset();
        m_bindings.clear();
    }

    [[nodiscard]] size_t GetBindingCount() const noexcept { return m_bindings.size(); }

private:
    std::vector<std::shared_ptr<BindingBase>> m_bindings;
};

} // namespace DragonOS::DragonUI
