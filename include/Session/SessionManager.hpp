#pragma once

#include <Engine/System.hpp>

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace DragonOS::WindowManager { class WindowManager; }
namespace DragonOS::Theme          { class ThemeManager; }
namespace DragonOS::Desktop        { class DesktopManager; }
namespace DragonOS::FileSystem     { class FileSystemService; }

namespace DragonOS::Session {

struct SavedWindowState final {
    uint64_t windowId{ 0 };
    std::wstring title;
    std::wstring appName;
    std::wstring currentPath;       ///< For Explorer: current directory
    float x{ 100.0f }, y{ 100.0f }, width{ 800.0f }, height{ 600.0f };
    bool maximized{ false };
};

struct SessionData final {
    std::wstring themeName;
    std::vector<SavedWindowState> windows;
    std::vector<std::wstring> explorerPaths;
    std::wstring desktopWallpaper;
    std::wstring sessionName;
    uint64_t timestamp{ 0 };
};

class SessionManager final : public Engine::System {
public:
    SessionManager() noexcept = default;
    ~SessionManager() noexcept override { Shutdown(); }

    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;
    SessionManager(SessionManager&&) = delete;
    SessionManager& operator=(SessionManager&&) = delete;

    bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float width, float height) noexcept override;

    void SaveSession() noexcept;
    bool RestoreSession() noexcept;

    void SetWindowManager(WindowManager::WindowManager& wm) noexcept;
    void SetThemeManager(Theme::ThemeManager& tm) noexcept;
    void SetDesktopManager(Desktop::DesktopManager& dm) noexcept;
    void SetFileSystemService(FileSystem::FileSystemService& fs) noexcept;

    SessionData GetCurrentSessionSnapshot() const noexcept;
    void SetAutoSave(bool enabled) noexcept { m_autoSave = enabled; }
    bool IsAutoSaveEnabled() const noexcept { return m_autoSave; }

    static constexpr float AutoSaveInterval = 30.0f;
    static constexpr size_t MaxSessionHistory = 10;

private:
    std::wstring GetSessionFilePath() const noexcept;

    SessionData m_lastSession;
    float m_autoSaveTimer{ 0.0f };
    bool m_autoSave{ true };
    bool m_sessionRestored{ false };

    WindowManager::WindowManager*  m_pWindowMgr{ nullptr };
    Theme::ThemeManager*           m_pThemeMgr{ nullptr };
    Desktop::DesktopManager*       m_pDesktopMgr{ nullptr };
    FileSystem::FileSystemService* m_pFS{ nullptr };

    bool m_initialized{ false };
};

} // namespace DragonOS::Session
