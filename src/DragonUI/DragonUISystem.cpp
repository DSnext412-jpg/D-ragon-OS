#include <DragonUI/DragonUISystem.hpp>
#include <DragonUI/Demo/DemoElements.hpp>
#include <DragonUI/Controls/Label.hpp>
#include <DragonUI/Controls/Button.hpp>
#include <DragonUI/Controls/Image.hpp>
#include <DragonUI/Controls/Panel.hpp>
#include <DragonUI/Controls/ProgressBar.hpp>
#include <DragonUI/Controls/Separator.hpp>

#include <Engine/EngineContext.hpp>
#include <Graphics/Renderer.hpp>
#include <Theme/ThemeManager.hpp>
#include <Input/InputManager.hpp>
#include <Input/InputEvent.hpp>

namespace DragonOS::DragonUI {

bool DragonUISystem::Initialize(Engine::EngineContext& ctx) noexcept
{
    auto* renderer = ctx.GetRenderer();
    auto* theme = ctx.GetThemeManager();
    auto* input = ctx.GetInputManager();

    if (!renderer || !theme || !input)
        return false;

    m_input = input;

    m_host = std::make_unique<WindowHost>(*renderer, *theme, *input);

    // ── Demo scene ──────────────────────────────────────────────────
    auto panel = std::make_unique<Demo::DemoPanel>(Thickness(60, 80, 60, 60));
    panel->SetPadding(Thickness(20));

    // Title
    auto title = std::make_unique<UILabel>(L"DragonUI Controls Demo");
    title->SetTextAlignment(Alignment::Center);
    title->SetMinSize(0, 30);

    panel->AddChild(std::move(title));

    // Separator
    auto sep1 = std::make_unique<UISeparator>();
    sep1->SetThickness(1.0f);
    sep1->SetMargin(Thickness(0, 4, 0, 4));
    panel->AddChild(std::move(sep1));

    // Description
    auto desc = std::make_unique<UILabel>(
        L"This window validates all basic DragonUI controls: labels, "
        L"buttons, images, panels, progress bars, and separators. "
        L"All controls respond to hover and focus.");
    desc->SetWordWrap(true);
    desc->SetMinSize(0, 36);
    panel->AddChild(std::move(desc));

    // Buttons row: three buttons side by side
    auto btn1 = std::make_unique<UIButton>(L"Click Me");
    btn1->SetMargin(Thickness(0, 12, 0, 0));
    btn1->SetOnClick([](UIButton& b) {
        b.SetText(L"Clicked!");
    });

    auto btn2 = std::make_unique<UIButton>(L"Disabled");
    btn2->SetMargin(Thickness(8, 12, 0, 0));
    btn2->SetEnabled(false);

    auto btn3 = std::make_unique<UIButton>(L"Tab Target");
    btn3->SetMargin(Thickness(8, 12, 0, 0));
    btn3->SetFocusable(true);

    panel->AddChild(std::move(btn1));
    panel->AddChild(std::move(btn2));
    panel->AddChild(std::move(btn3));

    // Icon row
    auto iconLabel = std::make_unique<UILabel>(L"Icon preview:");
    iconLabel->SetMinSize(0, 20);
    iconLabel->SetMargin(Thickness(0, 12, 0, 0));
    panel->AddChild(std::move(iconLabel));

    auto icon = std::make_unique<UIImage>();
    icon->SetGlyph(0xE768, 48.0f); // Settings gear icon
    icon->SetColor(Theme::SemanticColor::Accent);
    icon->SetMinSize(48, 48);
    panel->AddChild(std::move(icon));

    // Progress bar
    auto progLabel = std::make_unique<UILabel>(L"Progress:");
    progLabel->SetMinSize(0, 20);
    progLabel->SetMargin(Thickness(0, 8, 0, 0));
    panel->AddChild(std::move(progLabel));

    auto progress = std::make_unique<UIProgressBar>();
    progress->SetRange(0, 100);
    progress->SetValue(65);
    progress->SetMargin(Thickness(0, 4, 0, 0));
    m_progressBar = progress.get();
    panel->AddChild(std::move(progress));

    // Bottom separator
    auto sep2 = std::make_unique<UISeparator>();
    sep2->SetMargin(Thickness(0, 8, 0, 4));
    panel->AddChild(std::move(sep2));

    // Status text
    auto status = std::make_unique<UILabel>(L"Status: All systems nominal");
    status->SetTextAlignment(Alignment::End);
    status->SetTextColor(Theme::SemanticColor::TextSecondary);
    status->SetMinSize(0, 18);
    panel->AddChild(std::move(status));

    m_host->SetRoot(std::move(panel));
    m_host->Resize(ctx.GetViewportWidth(), ctx.GetViewportHeight());

    return true;
}

void DragonUISystem::Shutdown() noexcept
{
    m_progressBar = nullptr;
    m_host.reset();
    m_input = nullptr;
}

void DragonUISystem::Update(float deltaTime) noexcept
{
    if (!m_host || !m_input)
        return;

    auto& events = m_input->GetEvents();

    bool ctrl = m_input->IsKeyHeld(Input::KeyCode::LControl) ||
                m_input->IsKeyHeld(Input::KeyCode::RControl);
    bool shift = m_input->IsKeyHeld(Input::KeyCode::LShift) ||
                 m_input->IsKeyHeld(Input::KeyCode::RShift);
    bool alt = m_input->IsKeyHeld(Input::KeyCode::LAlt) ||
               m_input->IsKeyHeld(Input::KeyCode::RAlt);

    for (const auto& evt : events)
    {
        switch (evt.type)
        {
        case Input::EventType::MouseMove:
            m_host->OnMouseMove(evt.data.mouseMove.x, evt.data.mouseMove.y);
            break;
        case Input::EventType::MouseDown:
            m_host->OnMouseDown(evt.data.mouseButton.x, evt.data.mouseButton.y, evt.data.mouseButton.button);
            break;
        case Input::EventType::MouseUp:
            m_host->OnMouseUp(evt.data.mouseButton.x, evt.data.mouseButton.y, evt.data.mouseButton.button);
            break;
        case Input::EventType::KeyDown:
            m_host->OnKeyDown(evt.data.key.key, ctrl, shift, alt);
            break;
        case Input::EventType::CharacterInput:
            m_host->OnTextInput(evt.data.character.character);
            break;
        default:
            break;
        }
    }

    // Animate progress bar
    if (m_progressBar)
    {
        static float t = 0;
        t += deltaTime * 20.0f;
        float v = std::fmod(t, 200.0f);
        m_progressBar->SetValue(v > 100.0f ? 200.0f - v : v);
    }

    m_host->Update(deltaTime);
}

void DragonUISystem::Render(Engine::EngineContext& ctx) noexcept
{
    (void)ctx;
    if (m_host)
        m_host->Render();
}

void DragonUISystem::Resize(float width, float height) noexcept
{
    if (m_host)
        m_host->Resize(width, height);
}

} // namespace
