#pragma once

#include <DragonUI/Core/WindowHost.hpp>
#include <DragonUI/DataBinding/Observable.hpp>
#include <DragonUI/DataBinding/ICommand.hpp>
#include <DragonUI/DataBinding/ViewModelBase.hpp>
#include <DragonUI/DataBinding/ObservableCollection.hpp>
#include <DragonUI/DataBinding/Validation.hpp>
#include <DragonUI/DataBinding/BindingManager.hpp>
#include <DragonUI/DataBinding/ErrorTemplate.hpp>
#include <DragonUI/DataBinding/ThemeBinding.hpp>

#include <memory>

namespace DragonOS::Graphics { class Renderer; }
namespace DragonOS::Theme { class ThemeManager; }
namespace DragonOS::Input { class InputManager; }

namespace DragonOS::DragonUI {
class UITextBox;
class UILabel;
class UIButton;
class UIToggleSwitch;

namespace Demo {

/**
 * @brief View model backing the data-binding demo.
 *
 * Exposes Observable properties, an ObservableCollection and commands, plus a
 * ViewModelValidator.  It never references controls; the MvvmDemoSection binds
 * them to this model with a BindingManager.
 */
class DemoViewModel final : public ViewModelBase,
                            public std::enable_shared_from_this<DemoViewModel> {
public:
    ObservablePtr<std::wstring> DisplayName = MakeObservable<std::wstring>(L"Dragon User");
    ObservablePtr<bool> AutoSave = MakeObservable<bool>(true);
    ObservablePtr<int> EventsAdded = MakeObservable<int>(0);

    ObservableCollectionPtr<std::wstring> EventLog = MakeObservableCollection<std::wstring>();

    CommandPtr AddEvent;
    CommandPtr ClearEvents;

    ViewModelValidator Validator;

    /// Wires commands and validation.  Call once after make_shared.
    void Initialize() noexcept;

private:
    void AppendEvent(std::wstring text) noexcept;

    ObservableCollection<std::wstring>::Listener m_logListener;
};

/**
 * @brief A complete MVVM data-binding demo.
 *
 * Demonstrates one-way and two-way bindings, commands, validation with an
 * error template, an ObservableCollection feeding a UIListView, and theme
 * binding.  Shown as the "MVVM" page of the Settings app.
 */
class MvvmDemoSection final {
public:
    MvvmDemoSection(
        Graphics::Renderer& renderer,
        Theme::ThemeManager& theme,
        Input::InputManager& input) noexcept;

    void Resize(float width, float height) noexcept;
    void Render() noexcept;
    void Update(float deltaTime) noexcept;

    [[nodiscard]] WindowHost& GetHost() noexcept { return *m_host; }

private:
    void BuildView() noexcept;

    std::unique_ptr<WindowHost> m_host;
    std::shared_ptr<DemoViewModel> m_viewModel;
    std::unique_ptr<BindingManager> m_bindings;
    std::unique_ptr<ThemeBinding> m_themeBinding;
    ValidationErrorTemplate m_nameError;

    // Raw handles to controls owned by the host's tree (kept for bindings).
    UITextBox* m_nameBox{};
    UIToggleSwitch* m_autoSaveToggle{};
    UILabel* m_greetingLabel{};
    UILabel* m_autoSaveStatusLabel{};
    UILabel* m_entryCountLabel{};
    UILabel* m_accentLabel{};
    UIButton* m_clearButton{};

    Theme::ThemeManager* m_themeManager{};
};

} // namespace DragonOS::DragonUI::Demo
} // namespace DragonOS::DragonUI
