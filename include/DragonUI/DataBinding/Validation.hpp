#pragma once

#include <DragonUI/DataBinding/Observable.hpp>
#include <DragonUI/Validation/Validation.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace DragonOS::DragonUI {

/**
 * @brief Aggregated per-property validation error messages.
 *
 * Owns the error text for each named property and raises a callback whenever
 * the set of errors changes, so the view can show or hide error templates.
 */
class ValidationErrors {
public:
    using ErrorsChangedCallback = std::function<void()>;

    /// Sets (or replaces) the error message for @p property and notifies.
    void SetError(std::wstring_view property, std::wstring_view message) noexcept
    {
        auto key = std::wstring(property);
        auto it = m_errors.find(key);
        if (it != m_errors.end() && it->second == message)
            return;
        m_errors[key] = std::wstring(message);
        Notify();
    }

    /// Removes the error for @p property (if any) and notifies.
    void ClearError(std::wstring_view property) noexcept
    {
        if (m_errors.erase(std::wstring(property)) > 0)
            Notify();
    }

    /// Removes every error and notifies.
    void ClearAll() noexcept
    {
        if (m_errors.empty())
            return;
        m_errors.clear();
        Notify();
    }

    /// Returns the error message for @p property, or null when it is valid.
    [[nodiscard]] const std::wstring* GetError(std::wstring_view property) const noexcept
    {
        auto it = m_errors.find(std::wstring(property));
        return it != m_errors.end() ? &it->second : nullptr;
    }

    [[nodiscard]] bool HasErrors() const noexcept { return !m_errors.empty(); }
    [[nodiscard]] bool HasError(std::wstring_view property) const noexcept { return GetError(property) != nullptr; }
    [[nodiscard]] const std::map<std::wstring, std::wstring>& GetAll() const noexcept { return m_errors; }

    /// Assigns the callback raised whenever the set of errors changes.
    void SetOnErrorsChanged(ErrorsChangedCallback cb) noexcept { m_onErrorsChanged = std::move(cb); }

private:
    void Notify() noexcept
    {
        if (m_onErrorsChanged)
            m_onErrorsChanged();
    }

    std::map<std::wstring, std::wstring> m_errors;
    ErrorsChangedCallback m_onErrorsChanged;
};

/**
 * @brief A validation rule for a single named view-model property.
 *
 * Rules evaluate a value of type T and produce a ValidationResult.  The
 * existing Validator family from Validation/Validation.hpp is reused for text
 * properties through ValidatorValidationRule.
 *
 * @tparam T The property value type.
 */
template <typename T>
class PropertyValidationRule {
public:
    using RuleFunction = std::function<ValidationResult(const T& value)>;

    /// @param propertyName  The view-model property this rule guards.
    /// @param ruleFunction  Evaluates the property value.
    PropertyValidationRule(std::wstring propertyName, RuleFunction ruleFunction) noexcept
        : m_propertyName(std::move(propertyName))
        , m_ruleFunction(std::move(ruleFunction))
    {
    }

    [[nodiscard]] const std::wstring& GetPropertyName() const noexcept { return m_propertyName; }
    [[nodiscard]] ValidationResult Validate(const T& value) const noexcept { return m_ruleFunction(value); }

private:
    std::wstring m_propertyName;
    RuleFunction m_ruleFunction;
};

/**
 * @brief Creates a generic validation rule for a named property.
 *
 * @tparam T The property value type.
 * @param propertyName  The view-model property this rule guards.
 * @param ruleFunction  Evaluates the property value and returns a result.
 */
template <typename T>
inline std::shared_ptr<PropertyValidationRule<T>> MakeValidationRule(
    std::wstring propertyName,
    typename PropertyValidationRule<T>::RuleFunction ruleFunction) noexcept
{
    return std::make_shared<PropertyValidationRule<T>>(std::move(propertyName), std::move(ruleFunction));
}

/**
 * @brief Creates a rule for a text property backed by the string Validator
 *        interface (RequiredValidator, MinLengthValidator, RegexValidator, ...).
 */
inline std::shared_ptr<PropertyValidationRule<std::wstring>> MakeValidatorRule(
    std::wstring propertyName,
    std::shared_ptr<Validator> validator) noexcept
{
    return std::make_shared<PropertyValidationRule<std::wstring>>(
        std::move(propertyName),
        [validator = std::move(validator)](const std::wstring& value) noexcept
        {
            if (validator)
                return validator->Validate(value);
            return ValidationResult{ValidationState::Valid};
        });
}

/**
 * @brief Validates a set of observable view-model properties.
 *
 * Rules are registered for named properties and evaluated on demand or
 * automatically whenever the bound observable changes.  Results are written
 * into a shared ValidationErrors instance, which the view consumes to render
 * error templates.
 */
class ViewModelValidator {
public:
    using ErrorsChangedCallback = std::function<void()>;

    /// Registers a rule for its property.  When @p property is supplied the
    /// property is re-validated automatically on every change.
    template <typename T>
    void AddRule(std::shared_ptr<PropertyValidationRule<T>> rule, const ObservablePtr<T>& property = {}) noexcept
    {
        const std::wstring name = rule->GetPropertyName();
        auto sharedRule = std::move(rule);

        auto validateFn = [this, sharedRule, property, name]()
        {
            ValidationResult result{ValidationState::Valid};
            if (property)
                result = sharedRule->Validate(property->Get());
            if (result.state == ValidationState::Valid)
                m_errors.ClearError(name);
            else
                m_errors.SetError(name, result.errorMessage);
        };

        if (property)
        {
            m_listeners.push_back(std::make_shared<ListenerHolderImpl<T>>(
                property->AddListener(
                    [this, name](const T&, const T&)
                    {
                        Validate(name);
                    })));
        }

        m_rules.push_back(std::make_shared<ValidationEntry>(name, std::move(validateFn)));
    }

    /// Re-evaluates the rule for @p propertyName and updates the errors.
    void Validate(std::wstring_view propertyName) noexcept
    {
        for (auto& rule : m_rules)
        {
            if (rule->propertyName == propertyName)
                rule->validate();
        }
    }

    /// Re-evaluates every registered rule.
    void ValidateAll() noexcept
    {
        for (auto& rule : m_rules)
            rule->validate();
    }

    [[nodiscard]] bool IsValid() const noexcept { return !m_errors.HasErrors(); }

    [[nodiscard]] ValidationErrors& GetErrors() noexcept { return m_errors; }
    [[nodiscard]] const ValidationErrors& GetErrors() const noexcept { return m_errors; }

    /// Forwards a callback to the underlying ValidationErrors.
    void SetOnErrorsChanged(ErrorsChangedCallback cb) noexcept { m_errors.SetOnErrorsChanged(std::move(cb)); }

private:
    /// Type-erased holder that keeps an Observable<T>::Listener alive.
    class ListenerHolder {
    public:
        virtual ~ListenerHolder() = default;
    };

    template <typename T>
    class ListenerHolderImpl final : public ListenerHolder {
    public:
        explicit ListenerHolderImpl(typename Observable<T>::Listener listener) noexcept
            : m_listener(std::move(listener))
        {
        }

    private:
        typename Observable<T>::Listener m_listener;
    };

    struct ValidationEntry {
        std::wstring propertyName;
        std::function<void()> validate;
    };

    std::vector<std::shared_ptr<ValidationEntry>> m_rules;
    std::vector<std::shared_ptr<ListenerHolder>> m_listeners;
    ValidationErrors m_errors;
};

} // namespace DragonOS::DragonUI
