/**
 * @file    AnimationManager.hpp
 * @brief   Central coordinator for all animations in DragonOS.
 *
 * AnimationManager owns an AnimationTimeline and provides the
 * full lifecycle (Initialize, Shutdown, Update) required by the
 * Engine::System interface.
 *
 * Registration with the Engine happens in a future milestone.
 */

#pragma once

#include <Animation/Animation.hpp>
#include <Animation/AnimationTimeline.hpp>

#include <Engine/System.hpp>

#include <memory>

namespace DragonOS::Animation {

/**
 * @brief  Owns and drives every active Animation in the system.
 *
 * ## Lifecycle (Engine::System)
 *   - Initialize() — no-op, returns true.
 *   - Shutdown()   — clears the timeline.
 *   - Update()     — advances the timeline by deltaTime.
 *   - Render()     — no-op (animation logic is not a visual layer).
 *   - Resize()     — no-op.
 *
 * ## Public API
 *   Play()   — convenience: create and add an animation.
 *   Pause()  — pause a specific animation.
 *   Stop()   — stop and remove a specific animation.
 *   Remove() — remove an animation from the timeline.
 *   Clear()  — remove all animations.
 */
class AnimationManager final : public Engine::System {
public:
    AnimationManager() noexcept = default;
    ~AnimationManager() noexcept { Shutdown(); }

    AnimationManager(const AnimationManager&)            = delete;
    AnimationManager& operator=(const AnimationManager&) = delete;
    AnimationManager(AnimationManager&&)                 = delete;
    AnimationManager& operator=(AnimationManager&&)      = delete;

    // ── Engine::System ───────────────────────────────────────────────────

    /// @brief  Prepare the manager for operation.
    [[nodiscard]] bool Initialize(Engine::EngineContext& /*ctx*/) noexcept override
    {
        return true;
    }

    /// @brief  Stop and remove every active animation.
    void Shutdown() noexcept override { m_timeline.Clear(); }

    /**
     * @brief  Advance every active animation.
     *
     * @param deltaTime  Seconds elapsed since the previous frame.
     */
    void Update(float deltaTime) noexcept override { m_timeline.Update(deltaTime); }

    /// @brief  No-op — animation logic is not a renderable layer.
    void Render(Engine::EngineContext& /*ctx*/) noexcept override {}

    /// @brief  No-op — animations respond to viewport changes through
    ///         their target properties, not through the manager.
    void Resize(float /*width*/, float /*height*/) noexcept override {}

    // ── Animation management ─────────────────────────────────────────────

    /**
     * @brief  Add an animation and start it immediately.
     *
     * TODO:  Return a handle (weak_ptr / ID) for future lookup.
     *
     * @param animation  The animation to play (manager takes ownership).
     */
    void Play(std::unique_ptr<Animation> animation) noexcept;

    /**
     * @brief  Pause a running animation.
     *
     * @param animation  Raw pointer to the animation to pause.
     */
    void Pause(Animation* animation) noexcept;

    /**
     * @brief  Stop and remove a running animation.
     *
     * @param animation  Raw pointer to the animation to stop.
     */
    void Stop(Animation* animation) noexcept;

    /// @brief  Remove a specific animation from the timeline.
    void Remove(Animation* animation) noexcept;

    /// @brief  Remove every animation.
    void Clear() noexcept;

private:
    AnimationTimeline m_timeline;
};

} // namespace DragonOS::Animation
