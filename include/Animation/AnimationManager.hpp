#pragma once

#include <Animation/Animation.hpp>
#include <Animation/AnimationTimeline.hpp>

#include <Engine/System.hpp>

#include <memory>

namespace DragonOS::Animation {

class AnimationManager final : public Engine::System {
public:
    AnimationManager() noexcept = default;
    ~AnimationManager() noexcept { Shutdown(); }

    AnimationManager(const AnimationManager&)            = delete;
    AnimationManager& operator=(const AnimationManager&) = delete;
    AnimationManager(AnimationManager&&)                 = delete;
    AnimationManager& operator=(AnimationManager&&)      = delete;

    [[nodiscard]] bool Initialize(Engine::EngineContext& /*ctx*/) noexcept override
    {
        return true;
    }

    void Shutdown() noexcept override { m_timeline.Clear(); }

    void Update(float deltaTime) noexcept override { m_timeline.Update(deltaTime); }
    void Render(Engine::EngineContext& /*ctx*/) noexcept override {}

    void Resize(float , float ) noexcept override {}

    void Play(std::unique_ptr<Animation> animation) noexcept;
    void Pause(Animation* animation) noexcept;

    void Stop(Animation* animation) noexcept;

    void Remove(Animation* animation) noexcept;

    void Clear() noexcept;

private:
    AnimationTimeline m_timeline;
};

} 
