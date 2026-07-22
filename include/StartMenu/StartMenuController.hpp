#pragma once

#include <Apps/AppInfo.hpp>
#include <StartMenu/SearchTypes.hpp>
#include <Animation/AnimationCurve.hpp>
#include <Graphics/Color.hpp>
#include <Input/HitTest.hpp>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace DragonOS::Graphics    { class Renderer; }
namespace DragonOS::Theme       { class ThemeManager; }
namespace DragonOS::Input       { class MouseManager; }
namespace DragonOS::Animation   { class AnimationManager; class Animation; }
namespace DragonOS::Apps        { class ApplicationRegistry; }
namespace DragonOS::Notifications { class NotificationManager; }

namespace DragonOS::StartMenu {

enum class StartMenuHitRegion : uint8_t {
    None,
    SearchBox,
    PinnedApp,
    AllAppItem,
    PowerShutdown,
    PowerRestart,
    PowerSleep,
    PowerLock,
    PowerSignOut,
    SettingsShortcut,
    Header,
    Background,
};

struct PinnedAppTile final {
    const Apps::AppInfo* pApp{ nullptr };
    Input::Bounds       bounds{};
    bool                isHovered{ false };
    bool                isPressed{ false };
    float               hoverAnim{ 0.0f };
};

struct AllAppEntry final {
    const Apps::AppInfo* pApp{ nullptr };
    Input::Bounds       bounds{};
    bool                isHovered{ false };
    bool                isPressed{ false };
    float               hoverAnim{ 0.0f };
    std::wstring        sectionLabel;
};

struct PowerButton final {
    Input::Bounds bounds{};
    bool          isHovered{ false };
    bool          isPressed{ false };
};

class StartMenuController final {
public:
    StartMenuController() noexcept;
    ~StartMenuController() noexcept;

    StartMenuController(const StartMenuController&)            = delete;
    StartMenuController& operator=(const StartMenuController&) = delete;
    StartMenuController(StartMenuController&&)                 = delete;
    StartMenuController& operator=(StartMenuController&&)      = delete;

    // ── Lifecycle ─────────────────────────────────────────────────────────

    bool Initialize(
        Theme::ThemeManager&         themeManager,
        Input::MouseManager&         mouseManager,
        Animation::AnimationManager& animManager,
        Apps::ApplicationRegistry&   appRegistry) noexcept;

    void Shutdown() noexcept;

    // ─── Open / Close / Toggle ────────────────────────────────────────────

    void Open() noexcept;
    void Close() noexcept;
    void Toggle() noexcept;
    [[nodiscard]] bool IsOpen() const noexcept { return m_isOpen; }

    // ── Per-frame ─────────────────────────────────────────────────────────

    void Render(Graphics::Renderer& renderer) noexcept;
    void Update(float deltaTime) noexcept;
    void Resize(float viewportWidth, float viewportHeight) noexcept;

    // ── Input ─────────────────────────────────────────────────────────────

    void ProcessInput() noexcept;

    // ── App launching ─────────────────────────────────────────────────────

    /// @brief  Callback invoked when a Start Menu app item is clicked.
    using LaunchAppCallback = std::function<void(const Apps::AppInfo*)>;

    /// @brief  Set the callback for launching applications.
    void SetLaunchAppCallback(LaunchAppCallback callback) noexcept
    {
        m_launchCallback = std::move(callback);
    }

    void SetNotificationManager(Notifications::NotificationManager& mgr) noexcept
    {
        m_pNotifMgr = &mgr;
    }

    // ── Accessors ─────────────────────────────────────────────────────────

    [[nodiscard]] Input::Bounds GetBounds() const noexcept { return m_bounds; }
    [[nodiscard]] bool WantsInput() const noexcept { return m_isOpen || m_animProgress > 0.001f; }

private:
    // ── Layout ────────────────────────────────────────────────────────────

    struct Layout final {
        Input::Bounds menu{};

        // Sections
        Input::Bounds headerArea{};
        Input::Bounds searchArea{};
        Input::Bounds pinnedArea{};
        Input::Bounds allAppsArea{};
        Input::Bounds powerArea{};
        Input::Bounds settingsArea{};

        // Power buttons (bottom-right cluster)
        Input::Bounds powerShutdown{};
        Input::Bounds powerRestart{};
        Input::Bounds powerSleep{};
        Input::Bounds powerLock{};
        Input::Bounds powerSignOut{};

        // Scroll
        float allAppsScrollOffset{ 0.0f };
        float allAppsContentHeight{ 0.0f };
    };

    Layout CalculateLayout(float vpW, float vpH) const noexcept;
    void   BuildAppEntries() noexcept;

    // ── Rendering helpers ─────────────────────────────────────────────────

    void RenderBackground(Graphics::Renderer& renderer) noexcept;
    void RenderHeader(Graphics::Renderer& renderer) noexcept;
    void RenderSearchBox(Graphics::Renderer& renderer) noexcept;
    void RenderPinnedGrid(Graphics::Renderer& renderer) noexcept;
    void RenderAllAppsList(Graphics::Renderer& renderer) noexcept;
    void RenderPowerSection(Graphics::Renderer& renderer) noexcept;
    void RenderSettingsShortcut(Graphics::Renderer& renderer) noexcept;

    void RenderPinnedTile(Graphics::Renderer& renderer, const PinnedAppTile& tile) noexcept;
    void RenderAllAppEntry(Graphics::Renderer& renderer, const AllAppEntry& entry) noexcept;

    // ── Animation ─────────────────────────────────────────────────────────

    void StartOpenAnimation() noexcept;
    void StartCloseAnimation() noexcept;
    void UpdateAnimations(float deltaTime) noexcept;

    // ── Hit testing ───────────────────────────────────────────────────────

    [[nodiscard]] StartMenuHitRegion HitTestMenu(float px, float py) const noexcept;
    [[nodiscard]] int HitTestPinnedIndex(float px, float py) const noexcept;
    [[nodiscard]] int HitTestAllAppsIndex(float px, float py) const noexcept;

    // ── Data ──────────────────────────────────────────────────────────────

    Layout                m_layout{};
    Input::Bounds         m_bounds{};
    float                 m_viewportWidth{ 0.0f };
    float                 m_viewportHeight{ 0.0f };

    // ── State ─────────────────────────────────────────────────────────────

    bool                  m_isOpen{ false };
    float                 m_animProgress{ 0.0f };
    float                 m_animTarget{ 0.0f };
    float                 m_animVelocity{ 0.0f };

    // ── Hit regions ───────────────────────────────────────────────────────

    StartMenuHitRegion    m_hoveredRegion{ StartMenuHitRegion::None };
    StartMenuHitRegion    m_pressedRegion{ StartMenuHitRegion::None };
    int                   m_hoveredPinnedIdx{ -1 };
    int                   m_hoveredAllAppsIdx{ -1 };
    int                   m_focusedItemIdx{ -1 };

    // ── Search ────────────────────────────────────────────────────────────

    SearchBox             m_searchBox;

    // ── Pinned apps ───────────────────────────────────────────────────────

    std::vector<PinnedAppTile> m_pinnedTiles;

    // ── All apps ──────────────────────────────────────────────────────────

    std::vector<AllAppEntry> m_allAppsEntries;
    bool                  m_allAppsBuilt{ false };

    // ── Power section ─────────────────────────────────────────────────────

    PowerButton           m_powerShutdown;
    PowerButton           m_powerRestart;
    PowerButton           m_powerSleep;
    PowerButton           m_powerLock;
    PowerButton           m_powerSignOut;
    PowerButton           m_settingsButton;

    // ── Launch callback ───────────────────────────────────────────────────

    LaunchAppCallback     m_launchCallback;

    // ── Non-owning references ─────────────────────────────────────────────

    Theme::ThemeManager*              m_pThemeManager{ nullptr };
    Input::MouseManager*              m_pMouse{ nullptr };
    Animation::AnimationManager*      m_pAnimManager{ nullptr };
    Apps::ApplicationRegistry*        m_pAppRegistry{ nullptr };
    Notifications::NotificationManager* m_pNotifMgr{ nullptr };

    bool                  m_initialized{ false };
};

} // namespace DragonOS::StartMenu
