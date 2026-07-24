#include <DragonUI/DragonUISystem.hpp>
#include <DragonUI/Demo/DemoElements.hpp>

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

    auto panel = std::make_unique<Demo::DemoPanel>(Thickness(60, 80, 60, 60));
    panel->SetPadding(Thickness(16));

    auto title = std::make_unique<Demo::DemoText>(L"DragonUI Framework Demo");
    title->SetMinSize(0, 28);

    auto desc = std::make_unique<Demo::DemoText>(
        L"This panel validates layout, rendering, theme, and input.");
    desc->SetMinSize(0, 20);

    auto btn = std::make_unique<Demo::DemoButton>(L"Click Me");
    btn->SetMargin(Thickness(0, 12, 0, 0));

    auto btnRight = std::make_unique<Demo::DemoButton>(L"Focus Me (Tab)");
    btnRight->SetFocusable(true);
    btnRight->SetMargin(Thickness(8, 12, 0, 0));

    panel->AddChild(std::move(title));
    panel->AddChild(std::move(desc));
    panel->AddChild(std::move(btn));
    panel->AddChild(std::move(btnRight));

    m_host->SetRoot(std::move(panel));

    m_host->Resize(ctx.GetViewportWidth(), ctx.GetViewportHeight());

    return true;
}

void DragonUISystem::Shutdown() noexcept
{
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
            m_host->OnMouseDown(
                evt.data.mouseButton.x,
                evt.data.mouseButton.y,
                evt.data.mouseButton.button);
            break;

        case Input::EventType::MouseUp:
            m_host->OnMouseUp(
                evt.data.mouseButton.x,
                evt.data.mouseButton.y,
                evt.data.mouseButton.button);
            break;

        case Input::EventType::MouseDoubleClick:
            break;

        case Input::EventType::MouseWheel:
            break;

        case Input::EventType::KeyDown:
            m_host->OnKeyDown(evt.data.key.key, ctrl, shift, alt);
            break;

        case Input::EventType::KeyUp:
            break;

        case Input::EventType::CharacterInput:
            m_host->OnTextInput(evt.data.character.character);
            break;
        }
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
