#include <DragonUI/Dialogs/DialogManager.hpp>
#include <DragonUI/Core/FocusManager.hpp>

#include <algorithm>

namespace DragonOS::DragonUI {

bool DialogManager::HasModalOpen() const noexcept
{
    for (auto it = m_dialogs.rbegin(); it != m_dialogs.rend(); ++it)
    {
        if (it->dialog && it->dialog->IsOpen())
            return it->dialog->IsModal();
    }
    return false;
}

UIDialog* DialogManager::GetTopDialog() const noexcept
{
    for (auto it = m_dialogs.rbegin(); it != m_dialogs.rend(); ++it)
    {
        if (it->dialog && it->dialog->IsOpen())
            return it->dialog.get();
    }
    return nullptr;
}

void DialogManager::SweepClosed(FocusManager& hostFocus) noexcept
{
    while (!m_dialogs.empty() && m_dialogs.back().dialog && !m_dialogs.back().dialog->IsOpen())
    {
        auto& entry = m_dialogs.back();
        if (entry.dialog->IsModal() && entry.savedFocus)
            hostFocus.SetFocus(entry.savedFocus);
        if (m_active == entry.dialog.get())
            m_active = nullptr;
        m_dialogs.pop_back();
    }
}

void DialogManager::ShowDialog(std::unique_ptr<UIDialog> dialog, FocusManager& hostFocus) noexcept
{
    if (!dialog)
        return;

    Entry entry;
    if (dialog->IsModal())
        entry.savedFocus = hostFocus.GetFocused();

    dialog->SetViewport(m_viewportW, m_viewportH);
    dialog->CenterIn(m_viewportW, m_viewportH);
    dialog->Show();

    m_dialogs.push_back(Entry{std::move(dialog), entry.savedFocus});
    m_active = m_dialogs.back().dialog.get();
}

void DialogManager::Update(float deltaTime, FocusManager& hostFocus) noexcept
{
    (void)deltaTime;
    SweepClosed(hostFocus);

    LayoutSlot viewport{0, 0, m_viewportW, m_viewportH};
    for (auto& entry : m_dialogs)
    {
        if (entry.dialog && entry.dialog->IsOpen())
        {
            entry.dialog->Measure(viewport);
            entry.dialog->Arrange(viewport);
        }
    }
}

void DialogManager::Render(RenderContext& ctx, float viewportWidth, float viewportHeight) noexcept
{
    if (m_dialogs.empty())
        return;

    if (HasModalOpen())
    {
        D2D1_RECT_F viewport{0.0f, 0.0f, viewportWidth, viewportHeight};
        ctx.FillRectangle(viewport, Theme::SemanticColor::WindowTitleBar, 0.45f);
    }

    for (auto& entry : m_dialogs)
    {
        if (entry.dialog && entry.dialog->IsOpen())
            entry.dialog->Render(ctx);
    }
}

// ── Input routing ───────────────────────────────────────────────────────

bool DialogManager::HandleMouseMove(float x, float y) noexcept
{
    if (m_dialogs.empty())
        return false;

    if (HasModalOpen())
    {
        if (auto* top = GetTopDialog())
            top->HandleMouseMove(x, y);
        return true;
    }

    if (m_active && m_active->IsOpen())
    {
        m_active->HandleMouseMove(x, y);
        return true;
    }

    for (auto it = m_dialogs.rbegin(); it != m_dialogs.rend(); ++it)
    {
        auto* dlg = it->dialog.get();
        if (!dlg || !dlg->IsOpen())
            continue;
        if (dlg->ContainsPoint(x, y))
        {
            dlg->HandleMouseMove(x, y);
            return true;
        }
    }
    return false;
}

bool DialogManager::HandleMouseDown(float x, float y, Input::MouseButton button) noexcept
{
    if (m_dialogs.empty())
        return false;

    if (HasModalOpen())
    {
        if (auto* top = GetTopDialog())
        {
            top->HandleMouseDown(x, y, button);
            m_active = top;
        }
        return true;
    }

    for (auto it = m_dialogs.rbegin(); it != m_dialogs.rend(); ++it)
    {
        auto* dlg = it->dialog.get();
        if (!dlg || !dlg->IsOpen())
            continue;
        if (dlg->ContainsPoint(x, y))
        {
            dlg->HandleMouseDown(x, y, button);
            m_active = dlg;
            return true;
        }
    }

    m_active = nullptr;
    return false;
}

bool DialogManager::HandleMouseUp(float x, float y, Input::MouseButton button) noexcept
{
    if (m_dialogs.empty())
        return false;

    if (HasModalOpen())
    {
        if (auto* top = GetTopDialog())
            top->HandleMouseUp(x, y, button);
        return true;
    }

    if (m_active && m_active->IsOpen())
    {
        m_active->HandleMouseUp(x, y, button);
        m_active = nullptr;
        return true;
    }

    return false;
}

bool DialogManager::HandleMouseWheel(float delta, float x, float y) noexcept
{
    if (m_dialogs.empty())
        return false;

    if (HasModalOpen())
    {
        if (auto* top = GetTopDialog())
            top->HandleMouseWheel(delta, x, y);
        return true;
    }

    for (auto it = m_dialogs.rbegin(); it != m_dialogs.rend(); ++it)
    {
        auto* dlg = it->dialog.get();
        if (!dlg || !dlg->IsOpen())
            continue;
        if (dlg->ContainsPoint(x, y))
        {
            dlg->HandleMouseWheel(delta, x, y);
            return true;
        }
    }
    return false;
}

bool DialogManager::HandleKey(Input::KeyCode key, bool ctrl, bool shift, bool alt) noexcept
{
    if (!HasModalOpen())
        return false;

    if (auto* top = GetTopDialog())
    {
        top->HandleKey(key, ctrl, shift, alt);
        return true;
    }
    return false;
}

bool DialogManager::HandleText(wchar_t ch) noexcept
{
    if (!HasModalOpen())
        return false;

    if (auto* top = GetTopDialog())
    {
        top->HandleText(ch);
        return true;
    }
    return false;
}

} // namespace DragonOS::DragonUI
