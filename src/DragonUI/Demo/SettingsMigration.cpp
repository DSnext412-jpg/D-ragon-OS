#include <DragonUI/Demo/SettingsMigration.hpp>
#include <DragonUI/Controls/Label.hpp>
#include <DragonUI/Controls/ToggleSwitch.hpp>
#include <DragonUI/Controls/CheckBox.hpp>
#include <DragonUI/Controls/TextBox.hpp>
#include <DragonUI/Controls/Button.hpp>
#include <DragonUI/Controls/Panel.hpp>
#include <DragonUI/Controls/Separator.hpp>
#include <DragonUI/Core/Container.hpp>

#include <Graphics/Renderer.hpp>
#include <Theme/ThemeManager.hpp>
#include <Input/InputManager.hpp>

#include <memory>

namespace DragonOS::DragonUI::Demo {

SettingsMigrationSection::SettingsMigrationSection(
    Graphics::Renderer& renderer,
    const Theme::ThemeManager& theme,
    Input::InputManager& input) noexcept
{
    m_host = std::make_unique<WindowHost>(renderer, theme, input);

    // Build a DragonUI settings section for "Network"
    auto panel = std::make_unique<UIPanel>();
    panel->SetPadding(Thickness(12));

    auto heading = std::make_unique<UILabel>(L"Network & Internet (DragonUI)");
    heading->SetTextAlignment(Alignment::Start);
    heading->SetMinSize(0, 24);
    panel->AddChild(std::move(heading));

    auto sep = std::make_unique<UISeparator>();
    sep->SetMargin(Thickness(0, 4, 0, 4));
    panel->AddChild(std::move(sep));

    auto wifiToggle = std::make_unique<UIToggleSwitch>(L"Wi-Fi");
    wifiToggle->SetToggled(true);
    wifiToggle->SetMargin(Thickness(0, 4, 0, 0));
    wifiToggle->SetOnToggled([](UIToggleSwitch&, bool val) {
        (void)val;
    });
    panel->AddChild(std::move(wifiToggle));

    auto btToggle = std::make_unique<UIToggleSwitch>(L"Bluetooth");
    btToggle->SetToggled(true);
    btToggle->SetMargin(Thickness(0, 4, 0, 0));
    panel->AddChild(std::move(btToggle));

    auto airplaneToggle = std::make_unique<UIToggleSwitch>(L"Airplane mode");
    airplaneToggle->SetToggled(false);
    airplaneToggle->SetMargin(Thickness(0, 4, 0, 0));
    panel->AddChild(std::move(airplaneToggle));

    auto sep2 = std::make_unique<UISeparator>();
    sep2->SetMargin(Thickness(0, 8, 0, 4));
    panel->AddChild(std::move(sep2));

    auto proxyLabel = std::make_unique<UILabel>(L"Proxy server:");
    proxyLabel->SetMargin(Thickness(0, 4, 0, 0));
    panel->AddChild(std::move(proxyLabel));

    auto proxyInput = std::make_unique<UITextBox>(L"proxy.dragonos.local:8080");
    proxyInput->SetMargin(Thickness(0, 2, 0, 0));
    panel->AddChild(std::move(proxyInput));

    auto saveBtn = std::make_unique<UIButton>(L"Save");
    saveBtn->SetMargin(Thickness(0, 8, 0, 0));
    saveBtn->SetOnClick([](UIButton&) {
    });
    panel->AddChild(std::move(saveBtn));

    m_host->SetRoot(std::move(panel));
}

void SettingsMigrationSection::Resize(float width, float height) noexcept
{
    if (m_host)
        m_host->Resize(width, height);
}

void SettingsMigrationSection::Render() noexcept
{
    if (m_host)
        m_host->Render();
}

void SettingsMigrationSection::Update(float deltaTime) noexcept
{
    if (m_host)
        m_host->Update(deltaTime);
}

} // namespace
