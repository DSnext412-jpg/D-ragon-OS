#pragma once

#include <Apps/AppInfo.hpp>
#include <Engine/System.hpp>

#include <cstdint>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace DragonOS::Apps {

class ApplicationRegistry final : public Engine::System {
public:
    ApplicationRegistry() noexcept = default;
    ~ApplicationRegistry() noexcept = default;

    ApplicationRegistry(const ApplicationRegistry&)            = delete;
    ApplicationRegistry& operator=(const ApplicationRegistry&) = delete;
    ApplicationRegistry(ApplicationRegistry&&)                 = delete;
    ApplicationRegistry& operator=(ApplicationRegistry&&)      = delete;

    // ── Engine::System ────────────────────────────────────────────────────

    [[nodiscard]] bool Initialize(Engine::EngineContext& /*ctx*/) noexcept override
    {
        return InitializeInternal();
    }

    void Shutdown() noexcept override { ShutdownInternal(); }

    void Update(float /*deltaTime*/) noexcept override {}
    void Render(Engine::EngineContext& /*ctx*/) noexcept override {}
    void Resize(float /*width*/, float /*height*/) noexcept override {}

    // ── Application Registry ──────────────────────────────────────────────

    // ── Registration ──────────────────────────────────────────────────────

    uint64_t Register(AppInfo info) noexcept;
    bool     Unregister(uint64_t appId) noexcept;

    // ── Queries ───────────────────────────────────────────────────────────

    [[nodiscard]] AppInfo*       Find(uint64_t appId) noexcept;
    [[nodiscard]] const AppInfo* Find(uint64_t appId) const noexcept;

    [[nodiscard]] AppInfo*       FindByName(std::wstring_view name) noexcept;
    [[nodiscard]] const AppInfo* FindByName(std::wstring_view name) const noexcept;

    [[nodiscard]] std::vector<const AppInfo*> GetPinned() const noexcept;
    [[nodiscard]] std::vector<const AppInfo*> GetAll() const noexcept;
    [[nodiscard]] std::vector<const AppInfo*> GetByCategory(AppCategory cat) const noexcept;

    [[nodiscard]] size_t Count() const noexcept { return m_apps.size(); }

    // ── Mutations ─────────────────────────────────────────────────────────

    void SetPinned(uint64_t appId, bool pinned) noexcept;

private:
    bool InitializeInternal() noexcept;
    void ShutdownInternal() noexcept;

    uint64_t NextId() noexcept
    {
        static uint64_t s_nextId = 1;
        return s_nextId++;
    }

    std::unordered_map<uint64_t, AppInfo> m_apps;
    bool m_initialized{ false };
};

} // namespace DragonOS::Apps
