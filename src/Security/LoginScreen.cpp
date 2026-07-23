#include <Security/LoginScreen.hpp>
#include <Security/UserManager.hpp>

#include <Engine/EngineContext.hpp>
#include <Graphics/Renderer.hpp>
#include <Input/InputManager.hpp>
#include <Input/Keyboard.hpp>
#include <Input/KeyCodes.hpp>
#include <Input/MouseManager.hpp>
#include <Theme/ThemeManager.hpp>
#include <Theme/ThemePalette.hpp>

#include <algorithm>
#include <d2d1.h>

#include <Windows.h>

namespace DragonOS::Security {

void LoginScreen::SetDependencies(
    Theme::ThemeManager& themeManager,
    Input::MouseManager& mouseManager,
    Input::InputManager& inputManager) noexcept
{
    m_pTheme = &themeManager;
    m_pMouse = &mouseManager;
    m_pInput = &inputManager;
    m_initialized = true;
}

void LoginScreen::SetVisible(bool visible) noexcept
{
    m_visible = visible;
    if (visible)
    {
        m_mode = Mode::Login;
        m_password.clear();
        m_cursorPos = 0;
        m_error = false;
        m_errorMessage.clear();

        if (m_pUserMgr)
        {
            const auto* currentUser = m_pUserMgr->GetCurrentUser();
            if (currentUser && m_pUserMgr->IsSessionLocked())
            {
                m_mode = Mode::Lock;
                m_username = currentUser->username;
            }
            else
            {
                // Show user list
                m_mode = Mode::UserSelect;
                m_userList.clear();
                for (const auto* user : m_pUserMgr->GetAllUsers())
                {
                    m_userList.push_back(*user);
                }
            }
        }
    }
}

void LoginScreen::ProcessInput() noexcept
{
    if (!m_visible || !m_pMouse || !m_pInput) return;

    auto& keyboard = m_pInput->GetKeyboard();

    for (wchar_t ch : m_pInput->GetCharBuffer())
    {
        if (ch == L'\r')
        {
            if (m_mode == Mode::Login || m_mode == Mode::Lock)
            {
                if (m_pUserMgr)
                {
                    UserId result = InvalidUserId;
                    if (m_mode == Mode::Lock)
                        result = m_pUserMgr->UnlockSession(m_password) ? m_pUserMgr->GetCurrentUserId() : InvalidUserId;
                    else
                        result = m_pUserMgr->Authenticate(m_username, m_password);

                    if (result != InvalidUserId)
                    {
                        m_error = false;
                        m_errorMessage.clear();
                        if (m_onLogin) m_onLogin(result);
                    }
                    else
                    {
                        m_error = true;
                        m_errorMessage = L"Invalid username or password";
                    }
                    m_password.clear();
                    m_cursorPos = 0;
                }
            }
        }
        else if (ch == L'\b')
        {
            if (m_cursorPos > 0 && !m_password.empty())
            {
                m_password.erase(m_cursorPos - 1, 1);
                --m_cursorPos;
            }
        }
        else if (ch >= 32)
        {
            if (m_password.size() < 128)
            {
                m_password.insert(m_cursorPos, 1, ch);
                ++m_cursorPos;
            }
        }
    }

    if (m_mode == Mode::UserSelect && m_pMouse->WasLeftClicked())
    {
        const auto pos = m_pMouse->GetPosition();

        const float boxW = 400.0f;
        const float boxH = 60.0f + m_userList.size() * 70.0f;
        const float boxX = (m_viewportWidth - boxW) / 2.0f;
        const float boxY = (m_viewportHeight - boxH) / 2.0f;

        for (size_t i = 0; i < m_userList.size(); ++i)
        {
            float ux = boxX + 20.0f;
            float uy = boxY + 60.0f + static_cast<float>(i) * 70.0f;
            float uw = boxW - 40.0f;
            float uh = 60.0f;

            if (pos.x >= ux && pos.x <= ux + uw && pos.y >= uy && pos.y <= uy + uh)
            {
                m_username = m_userList[i].username;
                if (m_userList[i].role == UserRole::Guest)
                {
                    if (m_pUserMgr)
                    {
                        m_pUserMgr->CreateGuestSession();
                        if (m_onLogin) m_onLogin(m_userList[i].uid);
                    }
                }
                else
                {
                    m_mode = Mode::Login;
                    m_password.clear();
                    m_cursorPos = 0;
                    m_error = false;
                }
            }
        }
    }

    if (keyboard.IsKeyReleased(Input::KeyCode::Escape))
    {
        if (m_mode == Mode::Login)
        {
            m_mode = Mode::UserSelect;
            m_password.clear();
            m_cursorPos = 0;
            m_error = false;
        }
    }
}

void LoginScreen::Update() noexcept
{
    if (!m_initialized || !m_visible) return;
    ProcessInput();
}

void LoginScreen::Render(Graphics::Renderer& renderer) noexcept
{
    if (!m_initialized || !m_visible || !m_pTheme) return;

    RenderBackground(renderer);

    if (m_mode == Mode::Lock)
        RenderLockBox(renderer);
    else if (m_mode == Mode::UserSelect)
        RenderUserList(renderer);
    else
        RenderLoginBox(renderer);
}

void LoginScreen::RenderBackground(Graphics::Renderer& renderer) noexcept
{
    D2D1_RECT_F fullRect = D2D1::RectF(0, 0, m_viewportWidth, m_viewportHeight);
    const Graphics::Color bgColor{ 0.05f, 0.05f, 0.08f, 1.0f };
    renderer.FillRectangle(fullRect, bgColor);

    // Subtle gradient overlay
    const Graphics::Color gradColor{ 0.1f, 0.1f, 0.2f, 0.3f };
    D2D1_RECT_F gradRect = D2D1::RectF(0, 0, m_viewportWidth, m_viewportHeight * 0.4f);
    renderer.FillRectangle(gradRect, gradColor);
}

void LoginScreen::RenderLoginBox(Graphics::Renderer& renderer) noexcept
{
    const float boxW = 360.0f;
    const float boxH = 280.0f;
    const float boxX = (m_viewportWidth - boxW) / 2.0f;
    const float boxY = (m_viewportHeight - boxH) / 2.0f;

    const D2D1_RECT_F boxRect = D2D1::RectF(boxX, boxY, boxX + boxW, boxY + boxH);
    const Graphics::Color boxBg{ 0.12f, 0.12f, 0.15f, 0.95f };
    renderer.FillRectangle(boxRect, boxBg);

    const Graphics::Color borderCol{ 0.3f, 0.3f, 0.4f, 1.0f };
    renderer.DrawRectangle(boxRect, borderCol, 1.0f);

    // Title
    const D2D1_RECT_F titleRect = D2D1::RectF(boxX + 20, boxY + 20, boxX + boxW - 20, boxY + 60);
    renderer.DrawText(L"Sign In", titleRect, Graphics::Color{ 1, 1, 1, 1 });

    // Username label
    const D2D1_RECT_F userLabelRect = D2D1::RectF(boxX + 20, boxY + 70, boxX + boxW - 20, boxY + 90);
    renderer.DrawText(L"Username:", userLabelRect, Graphics::Color{ 0.7f, 0.7f, 0.8f, 1 });

    // Username field (disabled, showing selected user)
    const D2D1_RECT_F userFieldRect = D2D1::RectF(boxX + 20, boxY + 92, boxX + boxW - 20, boxY + 120);
    const Graphics::Color fieldBg{ 0.18f, 0.18f, 0.22f, 1.0f };
    renderer.FillRectangle(userFieldRect, fieldBg);
    renderer.DrawText(m_username, D2D1::RectF(boxX + 24, boxY + 94, boxX + boxW - 24, boxY + 118), Graphics::Color{ 1, 1, 1, 1 });

    // Password label
    const D2D1_RECT_F passLabelRect = D2D1::RectF(boxX + 20, boxY + 130, boxX + boxW - 20, boxY + 150);
    renderer.DrawText(L"Password:", passLabelRect, Graphics::Color{ 0.7f, 0.7f, 0.8f, 1 });

    // Password field
    const D2D1_RECT_F passFieldRect = D2D1::RectF(boxX + 20, boxY + 152, boxX + boxW - 20, boxY + 180);
    renderer.FillRectangle(passFieldRect, fieldBg);
    renderer.DrawRectangle(passFieldRect, Graphics::Color{ 0.4f, 0.4f, 0.5f, 1 }, 1.0f);

    std::wstring masked;
    masked.append(m_password.size(), L'\u25CF');
    renderer.DrawText(masked, D2D1::RectF(boxX + 24, boxY + 154, boxX + boxW - 24, boxY + 178), Graphics::Color{ 1, 1, 1, 1 });

    // Error message
    if (m_error)
    {
        const D2D1_RECT_F errRect = D2D1::RectF(boxX + 20, boxY + 190, boxX + boxW - 20, boxY + 215);
        renderer.DrawText(m_errorMessage, errRect, Graphics::Color{ 1, 0.3f, 0.3f, 1 });
    }

    // Hint text
    const D2D1_RECT_F hintRect = D2D1::RectF(boxX + 20, boxY + boxH - 30, boxX + boxW - 20, boxY + boxH - 10);
    renderer.DrawText(L"Press Enter to sign in, Esc to go back", hintRect, Graphics::Color{ 0.5f, 0.5f, 0.6f, 1 });
}

void LoginScreen::RenderLockBox(Graphics::Renderer& renderer) noexcept
{
    const float boxW = 360.0f;
    const float boxH = 240.0f;
    const float boxX = (m_viewportWidth - boxW) / 2.0f;
    const float boxY = (m_viewportHeight - boxH) / 2.0f;

    const D2D1_RECT_F boxRect = D2D1::RectF(boxX, boxY, boxX + boxW, boxY + boxH);
    const Graphics::Color boxBg{ 0.12f, 0.12f, 0.15f, 0.95f };
    renderer.FillRectangle(boxRect, boxBg);

    const Graphics::Color borderCol{ 0.3f, 0.3f, 0.4f, 1.0f };
    renderer.DrawRectangle(boxRect, borderCol, 1.0f);

    // Lock icon and title
    const D2D1_RECT_F titleRect = D2D1::RectF(boxX + 20, boxY + 20, boxX + boxW - 20, boxY + 60);
    renderer.DrawText(L"Locked", titleRect, Graphics::Color{ 1, 1, 1, 1 });

    // Username display
    const D2D1_RECT_F userRect = D2D1::RectF(boxX + 20, boxY + 65, boxX + boxW - 20, boxY + 90);
    std::wstring userText = L"Signed in as: " + m_username;
    renderer.DrawText(userText, userRect, Graphics::Color{ 0.7f, 0.7f, 0.8f, 1 });

    // Password field
    const D2D1_RECT_F passFieldRect = D2D1::RectF(boxX + 20, boxY + 105, boxX + boxW - 20, boxY + 133);
    const Graphics::Color fieldBg{ 0.18f, 0.18f, 0.22f, 1.0f };
    renderer.FillRectangle(passFieldRect, fieldBg);
    renderer.DrawRectangle(passFieldRect, Graphics::Color{ 0.4f, 0.4f, 0.5f, 1 }, 1.0f);

    std::wstring masked;
    masked.append(m_password.size(), L'\u25CF');
    renderer.DrawText(masked, D2D1::RectF(boxX + 24, boxY + 107, boxX + boxW - 24, boxY + 131), Graphics::Color{ 1, 1, 1, 1 });

    if (m_error)
    {
        const D2D1_RECT_F errRect = D2D1::RectF(boxX + 20, boxY + 140, boxX + boxW - 20, boxY + 165);
        renderer.DrawText(m_errorMessage, errRect, Graphics::Color{ 1, 0.3f, 0.3f, 1 });
    }
}

void LoginScreen::RenderUserList(Graphics::Renderer& renderer) noexcept
{
    const float boxW = 400.0f;
    const float boxH = 60.0f + m_userList.size() * 70.0f;
    const float boxX = (m_viewportWidth - boxW) / 2.0f;
    const float boxY = (m_viewportHeight - boxH) / 2.0f;

    const D2D1_RECT_F boxRect = D2D1::RectF(boxX, boxY, boxX + boxW, boxY + boxH);
    const Graphics::Color boxBg{ 0.12f, 0.12f, 0.15f, 0.95f };
    renderer.FillRectangle(boxRect, boxBg);

    const Graphics::Color borderCol{ 0.3f, 0.3f, 0.4f, 1.0f };
    renderer.DrawRectangle(boxRect, borderCol, 1.0f);

    // Title
    const D2D1_RECT_F titleRect = D2D1::RectF(boxX + 20, boxY + 15, boxX + boxW - 20, boxY + 50);
    renderer.DrawText(L"Select User", titleRect, Graphics::Color{ 1, 1, 1, 1 });

    // User buttons
    for (size_t i = 0; i < m_userList.size(); ++i)
    {
        float ux = boxX + 20.0f;
        float uy = boxY + 60.0f + static_cast<float>(i) * 70.0f;
        float uw = boxW - 40.0f;
        float uh = 60.0f;

        const auto pos = m_pMouse ? m_pMouse->GetPosition() : Input::Point{};
        bool hovered = (pos.x >= ux && pos.x <= ux + uw && pos.y >= uy && pos.y <= uy + uh);

        RenderUserButton(renderer, m_userList[i], ux, uy, uw, uh, hovered);
    }
}

void LoginScreen::RenderUserButton(Graphics::Renderer& renderer, const UserProfile& user, float x, float y, float w, float h, bool hovered) noexcept
{
    const D2D1_RECT_F btnRect = D2D1::RectF(x, y, x + w, y + h);

    if (hovered)
    {
        const Graphics::Color hoverBg{ 0.2f, 0.2f, 0.3f, 1.0f };
        renderer.FillRectangle(btnRect, hoverBg);
    }

    const Graphics::Color borderCol{ 0.3f, 0.3f, 0.4f, 1.0f };
    renderer.DrawRectangle(btnRect, borderCol, 1.0f);

    // Avatar placeholder (rounded rect)
    const float avatarSize = 40.0f;
    const float avatarX = x + 10.0f;
    const float avatarY = y + (h - avatarSize) / 2.0f;
    const D2D1_RECT_F avatarRect = D2D1::RectF(avatarX, avatarY, avatarX + avatarSize, avatarY + avatarSize);

    const Graphics::Color avatarColor{ 0.3f, 0.5f, 0.8f, 1.0f };
    renderer.FillRectangle(avatarRect, avatarColor);

    // Display name
    const D2D1_RECT_F nameRect = D2D1::RectF(avatarX + avatarSize + 15, y + 8, x + w - 10, y + 30);
    renderer.DrawText(user.displayName, nameRect, Graphics::Color{ 1, 1, 1, 1 });

    // Role label
    std::wstring roleStr = (user.role == UserRole::Administrator) ? L"Administrator"
        : (user.role == UserRole::Guest) ? L"Guest" : L"User";

    const D2D1_RECT_F roleRect = D2D1::RectF(avatarX + avatarSize + 15, y + 32, x + w - 10, y + h - 8);
    renderer.DrawText(roleStr, roleRect, Graphics::Color{ 0.6f, 0.6f, 0.7f, 1 });
}

} // namespace DragonOS::Security
