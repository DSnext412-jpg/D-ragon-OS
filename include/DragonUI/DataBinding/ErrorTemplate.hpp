#pragma once

#include <DragonUI/Controls/Label.hpp>

#include <functional>
#include <string>

namespace DragonOS::DragonUI {

/**
 * @brief Renders a validation error message for a view-model property.
 *
 * A ValidationErrorTemplate drives a UILabel's text, visibility and error
 * color from an error-getter function.  The view supplies the getter
 * (typically reading a ValidationErrors instance) and refreshes the template
 * whenever errors change.
 *
 * The label is owned by the view's layout tree; the template only holds a raw
 * handle and must be destroyed before the label (same owner, so this is
 * automatic).
 *
 * \code
 * ValidationErrorTemplate nameError;
 * nameError.SetLabel(nameErrorLabel);       // already added to the tree
 * nameError.SetErrorGetter([&]() {
 *     auto* msg = validator.GetErrors().GetError(L"Name");
 *     return msg ? *msg : std::wstring{};
 * });
 * validator.SetOnErrorsChanged([&]() { nameError.Refresh(); });
 * \endcode
 */
class ValidationErrorTemplate {
public:
    using ErrorGetter = std::function<std::wstring()>;

    /// Assigns the label that displays the error.  The label must live as long
    /// as this template (typically both are members of the same view).
    void SetLabel(UILabel* label) noexcept { m_label = label; }

    [[nodiscard]] UILabel* GetLabel() const noexcept { return m_label; }

    /// Assigns the function that returns the current error message (empty when
    /// the property is valid).
    void SetErrorGetter(ErrorGetter getter) noexcept { m_errorGetter = std::move(getter); }

    /// Re-reads the error getter and updates the label.  Call this whenever the
    /// underlying errors change (e.g. from the ValidationErrors callback).
    void Refresh() noexcept
    {
        if (!m_label)
            return;
        std::wstring message;
        if (m_errorGetter)
            message = m_errorGetter();
        m_label->SetText(message);
        m_label->SetVisibility(message.empty() ? Visibility::Collapsed : Visibility::Visible);
        m_label->SetTextColor(Theme::SemanticColor::Error);
    }

private:
    UILabel* m_label{};
    ErrorGetter m_errorGetter;
};

} // namespace DragonOS::DragonUI
