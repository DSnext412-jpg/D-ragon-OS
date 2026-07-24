#include <Security/SecuritySystem.hpp>
#include <Security/UserManager.hpp>
#include <Security/PermissionManager.hpp>
#include <Security/CredentialManager.hpp>
#include <Security/SecurityConfig.hpp>
#include <Security/LoginScreen.hpp>
#include <Security/SecurityCommands.hpp>

#include <Engine/EngineContext.hpp>
#include <Theme/ThemeManager.hpp>
#include <Input/InputManager.hpp>
#include <Input/MouseManager.hpp>
#include <Command/CommandRegistry.hpp>

namespace DragonOS::Security {

bool SecuritySystem::Initialize(Engine::EngineContext& ctx) noexcept
{
    if (m_initialized) return true;

    m_viewportWidth = ctx.GetViewportWidth();
    m_viewportHeight = ctx.GetViewportHeight();

    m_pCredMgr = std::make_unique<CredentialManager>();
    if (!m_pCredMgr->Initialize(ctx)) return false;

    m_pSecConfig = std::make_unique<SecurityConfig>();
    if (!m_pSecConfig->Initialize(ctx)) return false;

    m_pPermMgr = std::make_unique<PermissionManager>();
    if (!m_pPermMgr->Initialize(ctx)) return false;

    m_pUserMgr = std::make_unique<UserManager>();
    m_pUserMgr->SetCredentialManager(*m_pCredMgr);
    if (!m_pUserMgr->Initialize(ctx)) return false;

    m_pPermMgr->SetUserManager(*m_pUserMgr);
    m_pSecConfig->SetUserManager(*m_pUserMgr);

    m_pLoginScreen = std::make_unique<LoginScreen>();
    if (m_pThemeMgr && m_pMouseMgr && m_pInputMgr)
    {
        m_pLoginScreen->SetDependencies(*m_pThemeMgr, *m_pMouseMgr, *m_pInputMgr);
    }
    m_pLoginScreen->SetUserManager(*m_pUserMgr);
    m_pLoginScreen->SetViewportSize(m_viewportWidth, m_viewportHeight);

    m_pLoginScreen->SetOnLoginCallback(
        [this](UserId uid)
        {
            (void)uid;
            m_pUserMgr->SetLoginScreenVisible(false);
        });

    m_pLoginScreen->SetOnLockCallback(
        [this]()
        {
            m_pUserMgr->LockSession();
        });

    // Set security command pointers for terminal commands
    SetSecurityCommandPointers(m_pUserMgr.get(), m_pPermMgr.get(), m_pCredMgr.get());

    // Auto-login or show login screen
    if (m_pUserMgr->IsAutoLoginEnabled())
    {
        const auto* currentUser = m_pUserMgr->GetCurrentUser();
        if (currentUser)
        {
            m_pUserMgr->SwitchUser(currentUser->uid);
        }
    }

    m_initialized = true;
    return true;
}

void SecuritySystem::Shutdown() noexcept
{
    if (!m_initialized) return;

    m_pLoginScreen.reset();
    if (m_pUserMgr) m_pUserMgr->Shutdown();
    if (m_pPermMgr) m_pPermMgr->Shutdown();
    if (m_pCredMgr) m_pCredMgr->Shutdown();
    if (m_pSecConfig) m_pSecConfig->Shutdown();

    m_pUserMgr.reset();
    m_pPermMgr.reset();
    m_pCredMgr.reset();
    m_pSecConfig.reset();

    m_initialized = false;
}

void SecuritySystem::Update(float deltaTime) noexcept
{
    if (!m_initialized) return;

    if (m_pLoginScreen && m_pLoginScreen->IsVisible())
    {
        m_pLoginScreen->Update();
    }

    if (m_pUserMgr) m_pUserMgr->Update(deltaTime);
    if (m_pPermMgr) m_pPermMgr->Update(deltaTime);
    if (m_pCredMgr) m_pCredMgr->Update(deltaTime);
    if (m_pSecConfig) m_pSecConfig->Update(deltaTime);
}

void SecuritySystem::Render(Engine::EngineContext& ctx) noexcept
{
    if (!m_initialized) return;

    if (m_pLoginScreen && m_pLoginScreen->IsVisible())
    {
        m_pLoginScreen->Render(*ctx.GetRenderer());
    }

    if (m_pUserMgr) m_pUserMgr->Render(ctx);
    if (m_pPermMgr) m_pPermMgr->Render(ctx);
    if (m_pCredMgr) m_pCredMgr->Render(ctx);
    if (m_pSecConfig) m_pSecConfig->Render(ctx);
}

void SecuritySystem::Resize(float width, float height) noexcept
{
    m_viewportWidth = width;
    m_viewportHeight = height;
    if (m_pLoginScreen)
        m_pLoginScreen->SetViewportSize(width, height);
}

} // namespace DragonOS::Security
