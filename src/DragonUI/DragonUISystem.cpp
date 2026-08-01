#include <DragonUI/DragonUISystem.hpp>
#include <DragonUI/Demo/DataControlsDemo.hpp>
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

    // ── Data Controls Demo ──────────────────────────────────────────
    // Demonstrates UIListView, UITreeView, UIGridView, SelectionManager,
    // item templates, virtualization (50k-row list) and the dialog
    // framework (MessageBox, file / color / font / progress dialogs).
    m_host->SetRoot(Demo::DataControlsDemo::Create(*m_host));
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
            m_host->OnMouseDown(evt.data.mouseButton.x, evt.data.mouseButton.y, evt.data.mouseButton.button);
            break;
        case Input::EventType::MouseUp:
            m_host->OnMouseUp(evt.data.mouseButton.x, evt.data.mouseButton.y, evt.data.mouseButton.button);
            break;
        case Input::EventType::MouseWheel:
            m_host->OnMouseWheel(evt.data.mouseWheel.delta, evt.data.mouseWheel.x, evt.data.mouseWheel.y);
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
