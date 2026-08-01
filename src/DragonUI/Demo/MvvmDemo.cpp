#include <DragonUI/Demo/MvvmDemo.hpp>

#include <DragonUI/Controls/Label.hpp>
#include <DragonUI/Controls/TextBox.hpp>
#include <DragonUI/Controls/Button.hpp>
#include <DragonUI/Controls/ToggleSwitch.hpp>
#include <DragonUI/Controls/Panel.hpp>
#include <DragonUI/Controls/Separator.hpp>
#include <DragonUI/Controls/ListView.hpp>
#include <DragonUI/Controls/StackPanel.hpp>
#include <DragonUI/Controls/ScrollViewer.hpp>
#include <DragonUI/DataBinding/CollectionViewSource.hpp>

#include <Graphics/Renderer.hpp>
#include <Theme/ThemeManager.hpp>
#include <Input/InputManager.hpp>

#include <any>
#include <memory>
#include <string>

namespace DragonOS::DragonUI::Demo {

// ── DemoViewModel ────────────────────────────────────────────────────────

void DemoViewModel::Initialize() noexcept
{
    AddEvent = std::make_shared<RelayCommand>(
        [weak = this->weak_from_this()]()
        {
            if (auto vm = weak.lock())
                vm->AppendEvent(L"Event #" + std::to_wstring(vm->EventsAdded->Get() + 1));
        });

    ClearEvents = std::make_shared<RelayCommand>(
        [weak = this->weak_from_this()]()
        {
            if (auto vm = weak.lock())
                vm->EventLog->Clear();
        },
        [weak = this->weak_from_this()]() -> bool
        {
            auto vm = weak.lock();
            return vm && !vm->EventLog->IsEmpty();
        });

    m_logListener = EventLog->AddListener(
        [weak = this->weak_from_this()](const ObservableCollection<std::wstring>::Change&)
        {
            if (auto vm = weak.lock())
                vm->ClearEvents->NotifyCanExecuteChanged();
        });

    Validator.AddRule(MakeValidatorRule(L"Name", std::make_shared<RequiredValidator>()), DisplayName);
    Validator.AddRule(
        MakeValidationRule<std::wstring>(
            L"Name",
            [](const std::wstring& value) -> ValidationResult
            {
                if (value.size() < 3)
                    return {ValidationState::Invalid, L"At least 3 characters required."};
                return {ValidationState::Valid};
            }),
        DisplayName);
    Validator.ValidateAll();
}

void DemoViewModel::AppendEvent(std::wstring text) noexcept
{
    EventsAdded->Set(EventsAdded->Get() + 1);
    EventLog->Add(std::move(text));
}

// ── MvvmDemoSection ──────────────────────────────────────────────────────

namespace {

std::wstring AccentText(const Theme::ThemeColor& color)
{
    return L"Accent color: RGB(" +
           std::to_wstring(static_cast<int>(color.r * 255.0f)) + L", " +
           std::to_wstring(static_cast<int>(color.g * 255.0f)) + L", " +
           std::to_wstring(static_cast<int>(color.b * 255.0f)) + L")";
}

} // namespace

MvvmDemoSection::MvvmDemoSection(
    Graphics::Renderer& renderer,
    Theme::ThemeManager& theme,
    Input::InputManager& input) noexcept
{
    m_host = std::make_unique<WindowHost>(renderer, theme, input);
    m_themeManager = &theme;
    m_viewModel = std::make_shared<DemoViewModel>();
    m_viewModel->Initialize();
    BuildView();
}

void MvvmDemoSection::BuildView() noexcept
{
    auto& vm = *m_viewModel;
    m_bindings = std::make_unique<BindingManager>();
    m_themeBinding = std::make_unique<ThemeBinding>(*m_themeManager);

    auto scroll = std::make_unique<UIScrollViewer>();
    auto panel = std::make_unique<UIStackPanel>();
    panel->SetSpacing(6.0f);
    panel->SetPadding(Thickness(12));
    panel->SetMinSize(0, 0);

    // ── Header ──────────────────────────────────────────────────────────
    auto heading = std::make_unique<UILabel>(L"Data Binding & MVVM");
    heading->SetTextAlignment(Alignment::Start);
    heading->SetMinSize(0, 24);
    panel->AddChild(std::move(heading));

    auto sep = std::make_unique<UISeparator>();
    sep->SetMargin(Thickness(0, 2, 0, 2));
    panel->AddChild(std::move(sep));

    // ── Profile: two-way text binding + validation ──────────────────────
    auto sectionLabel = std::make_unique<UILabel>(L"Profile");
    sectionLabel->SetTextAlignment(Alignment::Start);
    panel->AddChild(std::move(sectionLabel));

    auto nameCaption = std::make_unique<UILabel>(L"Display name (two-way binding):");
    nameCaption->SetMargin(Thickness(0, 2, 0, 0));
    panel->AddChild(std::move(nameCaption));

    auto nameBox = std::make_unique<UITextBox>(L"Type a name...");
    nameBox->SetMinSize(220, 30);
    m_nameBox = nameBox.get();
    m_nameBox->SetValidator(std::make_shared<RequiredValidator>());
    panel->AddChild(std::move(nameBox));

    auto nameErrorLabel = std::make_unique<UILabel>();
    m_nameError.SetLabel(nameErrorLabel.get());
    m_nameError.SetErrorGetter(
        [this]() -> std::wstring
        {
            const auto* msg = m_viewModel->Validator.GetErrors().GetError(L"Name");
            return msg ? *msg : std::wstring{};
        });
    panel->AddChild(std::move(nameErrorLabel));

    auto greeting = std::make_unique<UILabel>();
    m_greetingLabel = greeting.get();
    panel->AddChild(std::move(greeting));

    // ── Profile: two-way toggle binding ─────────────────────────────────
    auto autoSaveToggle = std::make_unique<UIToggleSwitch>(L"Auto-save changes");
    autoSaveToggle->SetToggled(vm.AutoSave->Get());
    m_autoSaveToggle = autoSaveToggle.get();
    panel->AddChild(std::move(autoSaveToggle));

    auto autoSaveStatus = std::make_unique<UILabel>();
    m_autoSaveStatusLabel = autoSaveStatus.get();
    panel->AddChild(std::move(autoSaveStatus));

    auto sep2 = std::make_unique<UISeparator>();
    sep2->SetMargin(Thickness(0, 4, 0, 2));
    panel->AddChild(std::move(sep2));

    // ── Event log: ObservableCollection → ListView ──────────────────────
    auto logSection = std::make_unique<UILabel>(L"Event log (ObservableCollection)");
    logSection->SetTextAlignment(Alignment::Start);
    panel->AddChild(std::move(logSection));

    auto entryCount = std::make_unique<UILabel>();
    m_entryCountLabel = entryCount.get();
    panel->AddChild(std::move(entryCount));

    auto buttonRow = std::make_unique<UIStackPanel>(Orientation::Horizontal);
    buttonRow->SetSpacing(8.0f);

    auto addButton = std::make_unique<UIButton>(L"Add event");
    addButton->SetOnClick([cmd = vm.AddEvent](UIButton&)
    {
        cmd->Execute();
    });
    buttonRow->AddChild(std::move(addButton));

    auto clearButton = std::make_unique<UIButton>(L"Clear");
    clearButton->SetOnClick([cmd = vm.ClearEvents](UIButton&)
    {
        cmd->Execute();
    });
    m_clearButton = clearButton.get();
    m_clearButton->SetEnabled(vm.ClearEvents->CanExecute());
    vm.ClearEvents->SetOnCanExecuteChanged(
        [this]()
        {
            if (m_clearButton)
                m_clearButton->SetEnabled(m_viewModel->ClearEvents->CanExecute());
        });
    buttonRow->AddChild(std::move(clearButton));

    panel->AddChild(std::move(buttonRow));

    auto logList = std::make_unique<UIListView>();
    logList->SetMinSize(0, 150);
    auto logSource = CollectionViewSource<std::wstring>::Create(vm.EventLog);
    logList->SetItemSource(logSource);
    logSource->SetListView(logList.get());
    logList->SetPrimaryTextProvider(
        [](const std::any& item) -> std::wstring
        {
            if (!item.has_value())
                return {};
            try
            {
                return std::any_cast<std::wstring>(item);
            }
            catch (const std::bad_any_cast&)
            {
                return {};
            }
        });
    panel->AddChild(std::move(logList));

    auto sep3 = std::make_unique<UISeparator>();
    sep3->SetMargin(Thickness(0, 4, 0, 2));
    panel->AddChild(std::move(sep3));

    // ── Theme binding ───────────────────────────────────────────────────
    auto themeSection = std::make_unique<UILabel>(L"Theme binding");
    themeSection->SetTextAlignment(Alignment::Start);
    panel->AddChild(std::move(themeSection));

    auto accentLabel = std::make_unique<UILabel>();
    m_accentLabel = accentLabel.get();
    panel->AddChild(std::move(accentLabel));

    scroll->AddChild(std::move(panel));
    m_host->SetRoot(std::move(scroll));

    // ── Bindings ────────────────────────────────────────────────────────

    // Two-way: DisplayName ↔ TextBox.
    m_bindings->Bind<std::wstring>(
        vm.DisplayName,
        [this]() { return m_nameBox->GetText(); },
        [this](const std::wstring& value) { m_nameBox->SetText(value); },
        BindingMode::TwoWay,
        [this](Binding<std::wstring>::TargetChangedCallback onTargetChanged)
        {
            m_nameBox->SetOnTextChanged(
                [onTargetChanged](UITextBox& box)
                {
                    onTargetChanged(box.GetText());
                });
        });

    // One-way: DisplayName → greeting label.
    m_bindings->Bind<std::wstring>(
        vm.DisplayName,
        []() { return std::wstring{}; },
        [this](const std::wstring& value)
        {
            m_greetingLabel->SetText(L"Hello, " + value + L"!");
        },
        BindingMode::OneWay);

    // Two-way: AutoSave ↔ ToggleSwitch.
    m_bindings->Bind<bool>(
        vm.AutoSave,
        [this]() { return m_autoSaveToggle->IsToggled(); },
        [this](const bool& value)
        {
            m_autoSaveToggle->SetToggled(value);
        },
        BindingMode::TwoWay,
        [this](Binding<bool>::TargetChangedCallback onTargetChanged)
        {
            m_autoSaveToggle->SetOnToggled(
                [onTargetChanged](UIToggleSwitch&, bool value)
                {
                    onTargetChanged(value);
                });
        });

    // One-way: AutoSave → status label.
    m_bindings->Bind<bool>(
        vm.AutoSave,
        []() { return false; },
        [this](const bool& value)
        {
            m_autoSaveStatusLabel->SetText(value ? L"Auto-save: ON" : L"Auto-save: OFF");
        },
        BindingMode::OneWay);

    // One-way: EventsAdded → counter label.
    m_bindings->Bind<int>(
        vm.EventsAdded,
        []() { return 0; },
        [this](const int& value)
        {
            m_entryCountLabel->SetText(L"Events added: " + std::to_wstring(value));
        },
        BindingMode::OneWay);

    // ThemeBinding: accent color → label (refreshes when the theme changes).
    auto accentColor = m_themeBinding->BindColor(Theme::SemanticColor::Accent);
    accentColor->SetOnChanged(
        [this](const Theme::ThemeColor&, const Theme::ThemeColor& newValue)
        {
            m_accentLabel->SetText(AccentText(newValue));
        });
    m_accentLabel->SetText(AccentText(accentColor->Get()));

    // Initial error-template state.
    m_nameError.Refresh();
}

void MvvmDemoSection::Resize(float width, float height) noexcept
{
    if (m_host)
        m_host->Resize(width, height);
}

void MvvmDemoSection::Render() noexcept
{
    if (m_host)
        m_host->Render();
}

void MvvmDemoSection::Update(float deltaTime) noexcept
{
    if (m_host)
        m_host->Update(deltaTime);
}

} // namespace DragonOS::DragonUI::Demo
