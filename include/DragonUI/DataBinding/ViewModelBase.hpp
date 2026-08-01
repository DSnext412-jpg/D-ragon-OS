#pragma once

#include <DragonUI/DataBinding/Observable.hpp>
#include <DragonUI/DataBinding/ICommand.hpp>

#include <utility>

namespace DragonOS::DragonUI {

/**
 * @brief Base class for MVVM view models.
 *
 * A view model exposes its state through Observable members and its actions
 * through shared ICommand members.  It never references controls; a view (or
 * section) creates the controls and binds them to the view model with a
 * BindingManager.
 *
 * \code
 * class GreeterViewModel : public ViewModelBase {
 * public:
 *     ObservablePtr<std::wstring> Name = MakeObservable<std::wstring>(L"Dragon");
 *     CommandPtr SayHello = std::make_shared<RelayCommand>(...);
 * };
 * \endcode
 */
class ViewModelBase {
public:
    virtual ~ViewModelBase() = default;

    /// Stores @p value into @p property only when it changed and reports
    /// whether a change occurred.  Safe for two-way bindings (no cycles).
    template <typename T, typename U>
    static bool SetProperty(const ObservablePtr<T>& property, U&& value) noexcept
    {
        if (!property)
            return false;
        if (property->Get() == value)
            return false;
        property->Set(std::forward<U>(value));
        return true;
    }

protected:
    ViewModelBase() = default;
};

} // namespace DragonOS::DragonUI
