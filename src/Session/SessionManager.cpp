#include <Session/SessionManager.hpp>

#include <Desktop/DesktopManager.hpp>
#include <FileSystem/FileSystemService.hpp>
#include <Theme/ThemeManager.hpp>
#include <WindowManager/WindowManager.hpp>
#include <WindowManager/DragonWindow.hpp>
#include <WindowManager/WindowCollection.hpp>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <sstream>
#include <vector>

namespace DragonOS::Session {

bool SessionManager::Initialize(Engine::EngineContext& /*ctx*/) noexcept
{
    if (m_initialized) { return true; }
    m_initialized = true;
    return true;
}

void SessionManager::Shutdown() noexcept
{
    if (!m_initialized) { return; }

    if (m_autoSave)
    {
        SaveSession();
    }

    m_pWindowMgr = nullptr;
    m_pThemeMgr = nullptr;
    m_pDesktopMgr = nullptr;
    m_pFS = nullptr;
    m_initialized = false;
}

void SessionManager::Update(float deltaTime) noexcept
{
    if (!m_initialized || !m_autoSave) { return; }

    m_autoSaveTimer += deltaTime;
    if (m_autoSaveTimer >= AutoSaveInterval)
    {
        m_autoSaveTimer = 0.0f;
        SaveSession();
    }
}

void SessionManager::Render(Engine::EngineContext& /*ctx*/) noexcept
{
}

void SessionManager::Resize(float /*width*/, float /*height*/) noexcept
{
}

void SessionManager::SaveSession() noexcept
{
    if (!m_pWindowMgr) { return; }

    SessionData session;
    session.sessionName = L"Default";
    session.timestamp = static_cast<uint64_t>(std::time(nullptr));

    if (m_pThemeMgr)
    {
        {
            const auto name = m_pThemeMgr->GetCurrentTheme().GetName();
            session.themeName.assign(name.begin(), name.end());
        }
    }

    const auto& windows = m_pWindowMgr->GetCollection().GetAll();
    for (const auto& w : windows)
    {
        if (!w->IsVisible()) { continue; }

        SavedWindowState sw;
        sw.windowId = w->GetId();
        sw.title = w->GetTitle();
        sw.x = w->GetX();
        sw.y = w->GetY();
        sw.width = w->GetWidth();
        sw.height = w->GetHeight();
        sw.maximized = (w->GetState() == WindowManager::WindowState::Maximized);

        session.windows.push_back(std::move(sw));
    }

    m_lastSession = std::move(session);
}

bool SessionManager::RestoreSession() noexcept
{
    if (m_sessionRestored || m_lastSession.windows.empty()) { return false; }

    if (m_pThemeMgr && !m_lastSession.themeName.empty())
    {
    }

    m_sessionRestored = true;
    return true;
}

void SessionManager::SetWindowManager(
    WindowManager::WindowManager& wm) noexcept
{
    m_pWindowMgr = &wm;
}

void SessionManager::SetThemeManager(
    Theme::ThemeManager& tm) noexcept
{
    m_pThemeMgr = &tm;
}

void SessionManager::SetDesktopManager(
    Desktop::DesktopManager& dm) noexcept
{
    m_pDesktopMgr = &dm;
}

void SessionManager::SetFileSystemService(
    FileSystem::FileSystemService& fs) noexcept
{
    m_pFS = &fs;
}

SessionData SessionManager::GetCurrentSessionSnapshot() const noexcept
{
    return m_lastSession;
}

std::wstring SessionManager::GetSessionFilePath() const noexcept
{
    if (!m_pFS) { return L"session.dat"; }
    const auto home = m_pFS->GetKnownFolderPath(
        FileSystem::KnownFolder::Home);
    return FileSystem::FileSystemService::Combine(home, L".dragonos_session");
}

} // namespace DragonOS::Session
