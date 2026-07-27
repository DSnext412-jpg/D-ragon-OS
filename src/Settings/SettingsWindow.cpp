#include <Settings/SettingsWindow.hpp>
#include <Theme/ThemeManager.hpp>
#include <Theme/ThemePalette.hpp>
#include <Theme/ThemeMetrics.hpp>
#include <Graphics/Renderer.hpp>
#include <Input/MouseManager.hpp>
#include <Input/InputManager.hpp>
#include <Input/InputEvent.hpp>
#include <WindowManager/DragonWindow.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <d2d1.h>
#include <algorithm>

namespace DragonOS::Settings {

SettingsWindow::SettingsWindow() noexcept = default;

void SettingsWindow::SetDependencies(
    WindowManager::DragonWindow& window,
    Theme::ThemeManager& themeManager,
    Input::MouseManager& mouseManager,
    Input::InputManager& inputManager) noexcept
{
    m_pWindow = &window;
    m_pTheme  = &themeManager;
    m_pMouse  = &mouseManager;
    m_pInput  = &inputManager;

    BuildUI();

    m_initialized = true;
}

UI::UIRenderer SettingsWindow::MakeUIRenderer(Graphics::Renderer& renderer) const noexcept
{
    return UI::UIRenderer(renderer, *m_pTheme);
}

void SettingsWindow::BuildUI() noexcept
{
    m_tabControl = std::make_unique<UI::TabControl>();
    m_tabControl->SetStyle(UI::UIStyle::DefaultTabControl());

    m_tabControl->SetOnSelectionChanged([this](int index) {
        OnCategoryChanged(index);
    });

    auto buildSystemPage = [this]() -> std::unique_ptr<UI::StackPanel> {
        auto panel = std::make_unique<UI::StackPanel>(UI::Orientation::Vertical);
        panel->SetSpacing(12.0f);
        panel->SetPadding(12.0f, 8.0f, 12.0f, 8.0f);

        auto heading1 = std::make_unique<UI::Label>(L"Display");
        heading1->SetStyle(UI::UIStyle::DefaultLabel());
        heading1->SetTextAlignment(UI::Alignment::Start);

        auto nightLight = std::make_unique<UI::ToggleSwitch>(L"Night light");
        nightLight->SetStyle(UI::UIStyle::DefaultToggleSwitch());
        nightLight->SetToggled(false);
        nightLight->SetOnToggled([](bool val) {
            (void)val;
        });

        auto brightnessSlider = std::make_unique<UI::Slider>();
        brightnessSlider->SetStyle(UI::UIStyle::DefaultSlider());
        brightnessSlider->SetRange(0.0f, 100.0f);
        brightnessSlider->SetValue(70.0f);
        brightnessSlider->SetStep(1.0f);
        brightnessSlider->SetOnValueChanged([](float val) {
            (void)val;
        });

        auto brightnessLabel = std::make_unique<UI::Label>(L"Brightness");
        brightnessLabel->SetStyle(UI::UIStyle::DefaultLabel());

        auto heading2 = std::make_unique<UI::Label>(L"Notifications");
        heading2->SetStyle(UI::UIStyle::DefaultLabel());
        heading2->SetTextAlignment(UI::Alignment::Start);

        auto dndToggle = std::make_unique<UI::ToggleSwitch>(L"Do not disturb");
        dndToggle->SetStyle(UI::UIStyle::DefaultToggleSwitch());
        dndToggle->SetToggled(false);

        panel->AddChild(std::move(heading1));
        panel->AddChild(std::move(nightLight));
        panel->AddChild(std::move(brightnessLabel));
        panel->AddChild(std::move(brightnessSlider));
        panel->AddChild(std::move(heading2));
        panel->AddChild(std::move(dndToggle));

        return panel;
    };

    auto buildPersonalizationPage = [this]() -> std::unique_ptr<UI::StackPanel> {
        auto panel = std::make_unique<UI::StackPanel>(UI::Orientation::Vertical);
        panel->SetSpacing(12.0f);
        panel->SetPadding(12.0f, 8.0f, 12.0f, 8.0f);

        auto darkMode = std::make_unique<UI::ToggleSwitch>(L"Dark mode");
        darkMode->SetStyle(UI::UIStyle::DefaultToggleSwitch());
        darkMode->SetToggled(false);
        darkMode->SetOnToggled([](bool val) {
            (void)val;
        });

        auto accentLabel = std::make_unique<UI::Label>(L"Accent color");
        accentLabel->SetStyle(UI::UIStyle::DefaultLabel());

        auto accentCombo = std::make_unique<UI::ComboBox>();
        accentCombo->SetStyle(UI::UIStyle::DefaultComboBox());
        accentCombo->SetItems({L"Blue", L"Green", L"Purple", L"Orange", L"Red"});
        accentCombo->SetSelectedIndex(0);
        accentCombo->SetOnSelectionChanged([](int index) {
            (void)index;
        });

        auto wallpaperBtn = std::make_unique<UI::Button>(L"Browse wallpaper");
        wallpaperBtn->SetStyle(UI::UIStyle::DefaultButton());
        wallpaperBtn->SetOnClick([]() {
        });

        panel->AddChild(std::move(darkMode));
        panel->AddChild(std::move(accentLabel));
        panel->AddChild(std::move(accentCombo));
        panel->AddChild(std::move(wallpaperBtn));

        return panel;
    };

    auto buildAccountsPage = [this]() -> std::unique_ptr<UI::StackPanel> {
        auto panel = std::make_unique<UI::StackPanel>(UI::Orientation::Vertical);
        panel->SetSpacing(12.0f);
        panel->SetPadding(12.0f, 8.0f, 12.0f, 8.0f);

        auto infoLabel = std::make_unique<UI::Label>(L"Signed in as: User");
        infoLabel->SetStyle(UI::UIStyle::DefaultLabel());

        auto signInToggle = std::make_unique<UI::ToggleSwitch>(L"Require sign-in");
        signInToggle->SetStyle(UI::UIStyle::DefaultToggleSwitch());
        signInToggle->SetToggled(true);

        auto manageBtn = std::make_unique<UI::Button>(L"Manage accounts");
        manageBtn->SetStyle(UI::UIStyle::DefaultButton());
        manageBtn->SetOnClick([]() {
        });

        panel->AddChild(std::move(infoLabel));
        panel->AddChild(std::move(signInToggle));
        panel->AddChild(std::move(manageBtn));

        return panel;
    };

    // Network page replaced by DragonUI section (rendered separately)
    auto buildNetworkPage = [this]() -> std::unique_ptr<UI::StackPanel> {
        auto panel = std::make_unique<UI::StackPanel>(UI::Orientation::Vertical);
        panel->SetSpacing(12.0f);
        panel->SetPadding(12.0f, 8.0f, 12.0f, 8.0f);

        // This page is rendered by DragonUI framework instead
        auto label = std::make_unique<UI::Label>(L"(DragonUI controls below)");
        label->SetStyle(UI::UIStyle::DefaultLabel());
        panel->AddChild(std::move(label));

        return panel;
    };

    auto buildAppsPage = [this]() -> std::unique_ptr<UI::StackPanel> {
        auto panel = std::make_unique<UI::StackPanel>(UI::Orientation::Vertical);
        panel->SetSpacing(12.0f);
        panel->SetPadding(12.0f, 8.0f, 12.0f, 8.0f);

        auto browserLabel = std::make_unique<UI::Label>(L"Default browser: DragonOS Browser");
        browserLabel->SetStyle(UI::UIStyle::DefaultLabel());

        auto manageAppsBtn = std::make_unique<UI::Button>(L"Manage default apps");
        manageAppsBtn->SetStyle(UI::UIStyle::DefaultButton());
        manageAppsBtn->SetOnClick([]() {
        });

        panel->AddChild(std::move(browserLabel));
        panel->AddChild(std::move(manageAppsBtn));

        return panel;
    };

    auto pageSystem = buildSystemPage();
    m_tabControl->AddPage(L"System", std::move(pageSystem));

    auto pagePersonalization = buildPersonalizationPage();
    m_tabControl->AddPage(L"Personalization", std::move(pagePersonalization));

    auto pageAccounts = buildAccountsPage();
    m_tabControl->AddPage(L"Accounts", std::move(pageAccounts));

    auto pageNetwork = buildNetworkPage();
    m_tabControl->AddPage(L"Network", std::move(pageNetwork));

    auto pageApps = buildAppsPage();
    m_tabControl->AddPage(L"Apps", std::move(pageApps));

    m_statusBar = std::make_unique<UI::StatusBar>();
    m_statusBar->SetStyle(UI::UIStyle::DefaultStatusBar());
    m_statusBar->SetText(L"Settings — Network section uses DragonUI");
}

void SettingsWindow::OnCategoryChanged(int index) noexcept
{
    m_selectedCategory = index;
}

void SettingsWindow::Update() noexcept
{
    if (!m_initialized || !m_pWindow) { return; }
    if (!m_pWindow->IsVisible()) { return; }

    ProcessInput();

    if (m_dragonUISection && m_selectedCategory == 3)
    {
        m_dragonUISection->Update(0.016f);
    }
}

void SettingsWindow::ProcessInput() noexcept
{
    if (!m_initialized || !m_pMouse || !m_pWindow) { return; }
    if (!m_pWindow->IsVisible()) { return; }

    const auto pos = m_pMouse->GetPosition();

    if (m_tabControl)
    {
        UI::UIEvent ev;
        ev.x = pos.x;
        ev.y = pos.y;
        ev.type = UI::UIEventType::MouseMove;
        m_tabControl->OnEvent(ev);

        if (m_pMouse->WasLeftClicked())
        {
            ev.type = UI::UIEventType::Click;
            ev.button = Input::MouseButton::Left;
            m_tabControl->OnEvent(ev);
        }
    }

    // DragonUI input is handled via InputManager events in Update
}

void SettingsWindow::Render(Graphics::Renderer& renderer) noexcept
{
    if (!m_initialized || !m_pWindow || !m_pWindow->IsVisible()) return;
    if (!m_pTheme) return;

    m_pRenderer = &renderer;

    // Create DragonUI section lazily on first render (needs renderer)
    if (!m_dragonUISection && m_pInput)
    {
        m_dragonUISection = std::make_unique<DragonUI::Demo::SettingsMigrationSection>(
            renderer, *m_pTheme, *m_pInput);
    }

    const float w = m_pWindow->GetWidth();
    const float h = m_pWindow->GetHeight();
    m_viewportWidth = w;
    m_viewportHeight = h;

    auto uiRenderer = MakeUIRenderer(renderer);

    const auto& bgCol = m_pTheme->GetColor(Theme::SemanticColor::WindowBackground);
    renderer.FillRectangle(D2D1::RectF(0, 0, w, h),
        Graphics::Color{ bgCol.r, bgCol.g, bgCol.b, bgCol.a });

    if (m_tabControl)
    {
        D2D1_RECT_F tcBounds = { 8, 8, w - 8, h - 36 };
        m_tabControl->Measure(D2D1::RectF(0, 0, w - 16, h - 44));
        m_tabControl->Arrange(tcBounds);
        m_tabControl->Render(uiRenderer);
    }

    // Network tab (index 3) — render DragonUI section on top
    if (m_dragonUISection && m_selectedCategory == 3)
    {
        D2D1_RECT_F sectionBounds = { 8, 8, w - 8, h - 36 };
        m_dragonUISection->Resize(
            sectionBounds.right - sectionBounds.left,
            sectionBounds.bottom - sectionBounds.top);
        m_dragonUISection->Render();
    }

    if (m_statusBar)
    {
        D2D1_RECT_F sbBounds = { 8, h - 28, w - 8, h - 4 };
        m_statusBar->Measure(D2D1::RectF(0, 0, w - 16, 24));
        m_statusBar->Arrange(sbBounds);
        m_statusBar->Render(uiRenderer);
    }
}

uint64_t SettingsWindow::GetWindowId() const noexcept
{
    return m_pWindow ? m_pWindow->GetId() : 0;
}

} // namespace
