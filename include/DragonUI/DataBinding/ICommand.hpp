#pragma once

#include <functional>
#include <memory>
#include <utility>

namespace DragonOS::DragonUI {

/**
 * @brief Command abstraction for MVVM.
 *
 * A command decouples the action of a control (e.g. a button click) from the
 * view model that performs it.  The control wires its event to Execute() and
 * may query CanExecute() to enable/disable itself.
 */
class ICommand {
public:
    using CanExecuteChangedCallback = std::function<void()>;

    virtual ~ICommand() = default;

    /// Returns whether the command may currently execute.
    [[nodiscard]] virtual bool CanExecute() const noexcept = 0;

    /// Performs the command action.
    virtual void Execute() noexcept = 0;

    /// Assigns the callback raised whenever CanExecute() may have changed.
    void SetOnCanExecuteChanged(CanExecuteChangedCallback cb) noexcept { m_canExecuteChanged = std::move(cb); }

    /// Public entry point to raise CanExecuteChanged after state that feeds
    /// CanExecute() changes (e.g. an observable the command observes).
    void NotifyCanExecuteChanged() noexcept { RaiseCanExecuteChanged(); }

protected:
    /// Raises the CanExecuteChanged callback.  Call after internal state that
    /// affects CanExecute() changes.
    void RaiseCanExecuteChanged() noexcept
    {
        if (m_canExecuteChanged)
            m_canExecuteChanged();
    }

private:
    CanExecuteChangedCallback m_canExecuteChanged;
};

/// Convenience alias for shared command ownership.
using CommandPtr = std::shared_ptr<ICommand>;

/**
 * @brief Concrete command backed by free callbacks.
 *
 * \code
 * auto cmd = std::make_shared<RelayCommand>(
 *     [vm = vmPtr]() { vm->Save(); },
 *     [vm = vmPtr]() { return vm->IsDirty(); });
 * button->SetOnClick([cmd](UIControl&) { cmd->Execute(); });
 * \endcode
 */
class RelayCommand final : public ICommand {
public:
    using ExecuteCallback = std::function<void()>;
    using CanExecuteCallback = std::function<bool()>;

    /// @param execute    The action invoked by Execute().
    /// @param canExecute Optional guard; when null CanExecute() is always true.
    RelayCommand(ExecuteCallback execute, CanExecuteCallback canExecute = {}) noexcept
        : m_execute(std::move(execute))
        , m_canExecute(std::move(canExecute))
    {
    }

    [[nodiscard]] bool CanExecute() const noexcept override
    {
        if (m_canExecute)
            return m_canExecute();
        return true;
    }

    void Execute() noexcept override
    {
        if (CanExecute() && m_execute)
            m_execute();
    }

private:
    ExecuteCallback m_execute;
    CanExecuteCallback m_canExecute;
};

} // namespace DragonOS::DragonUI
