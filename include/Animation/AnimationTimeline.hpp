/**
 * @file    AnimationTimeline.hpp
 * @brief   Ordered container for running multiple Animations in parallel.
 *
 * A Timeline owns a set of Animations and advances them together
 * on each Update() call.  Finished animations are automatically
 * removed (unless looping).
 */

#pragma once

#include <Animation/Animation.hpp>

#include <cstddef>
#include <memory>
#include <vector>

namespace DragonOS::Animation {

/**
 * @brief  Manages a collection of concurrently running Animations.
 *
 * Animations are added via AddAnimation() and are updated together
 * when Update() is called.  The timeline owns the animations and
 * destroys them when they finish (unless looping) or when removed.
 */
class AnimationTimeline final {
public:
    AnimationTimeline() noexcept = default;
    ~AnimationTimeline() noexcept = default;

    AnimationTimeline(const AnimationTimeline&)            = delete;
    AnimationTimeline& operator=(const AnimationTimeline&) = delete;
    AnimationTimeline(AnimationTimeline&&)                 = delete;
    AnimationTimeline& operator=(AnimationTimeline&&)      = delete;

    // ── Management ───────────────────────────────────────────────────────

    /**
     * @brief  Add an animation to the timeline.
     *
     * The timeline takes ownership via unique_ptr.
     *
     * @param animation  The animation to manage.
     */
    void AddAnimation(std::unique_ptr<Animation> animation) noexcept;

    /**
     * @brief  Remove a specific animation by raw pointer.
     *
     * @param animation  Pointer to the animation to remove.
     *
     * @return true if the animation was found and removed.
     */
    bool RemoveAnimation(Animation* animation) noexcept;

    /// @brief  Remove every animation from the timeline.
    void Clear() noexcept;

    // ── Update ───────────────────────────────────────────────────────────

    /**
     * @brief  Advance every active animation.
     *
     * Finished non-looping animations are automatically removed.
     *
     * @param deltaTime  Seconds elapsed since the last frame.
     */
    void Update(float deltaTime) noexcept;

    // ── Accessors ────────────────────────────────────────────────────────

    /// @brief  Number of animations currently in the timeline.
    [[nodiscard]] std::size_t Count() const noexcept { return m_animations.size(); }

    /// @brief  True when no animations are registered.
    [[nodiscard]] bool Empty() const noexcept { return m_animations.empty(); }

private:
    std::vector<std::unique_ptr<Animation>> m_animations;
};

} // namespace DragonOS::Animation
