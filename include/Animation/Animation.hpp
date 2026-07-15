/**
 * @file    Animation.hpp
 * @brief   Abstract base class for all animatable properties in DragonOS.
 *
 * Concrete animations (e.g. FloatAnimation, ColorAnimation) derive
 * from this class and implement OnUpdate() to apply the eased progress
 * to a target value.
 */

#pragma once

#include <Animation/AnimationCurve.hpp>
#include <Animation/AnimationState.hpp>
#include <Animation/Easing.hpp>

#include <functional>

namespace DragonOS::Animation {

/**
 * @brief  Base class for a single, reusable animation.
 *
 * Manages the core state machine (Idle -> Playing -> Paused -> Finished),
 * time tracking, looping, reversing, and easing.  Derived types
 * override OnUpdate() to drive actual property changes.
 *
 * ## Lifecycle
 *   Create  -> Play() -> [Update() x N] -> Finished -> Reset() -> Play() ...
 *                     -> Pause() -> Play() -> ...
 *                     -> Stop() -> Play() ...
 */
class Animation {
public:
    /// @brief  Signature for the on-finished callback.
    using Callback = std::function<void()>;

    /**
     * @brief  Construct an animation.
     *
     * @param duration  Duration in seconds.
     * @param curve     Easing curve (default Linear).
     */
    explicit Animation(
        float           duration,
        AnimationCurve  curve = AnimationCurve::Linear) noexcept;

    virtual ~Animation() noexcept = default;

    Animation(const Animation&)            = delete;
    Animation& operator=(const Animation&) = delete;
    Animation(Animation&&)                 = delete;
    Animation& operator=(Animation&&)      = delete;

    // ── State control ────────────────────────────────────────────────────

    /// @brief  Start or resume the animation.
    void Play() noexcept;

    /// @brief  Pause at the current position.
    void Pause() noexcept;

    /// @brief  Stop and reset to the beginning.
    void Stop() noexcept;

    /// @brief  Reset elapsed time to zero without changing state.
    void Reset() noexcept;

    // ── Per-frame ────────────────────────────────────────────────────────

    /**
     * @brief  Advance the animation by the given time delta.
     *
     * Calls the virtual OnUpdate() with the eased progress whenever
     * the animation is Playing.
     *
     * @param deltaTime  Seconds elapsed since the last frame.
     */
    void Update(float deltaTime) noexcept;

    // ── Accessors ────────────────────────────────────────────────────────

    [[nodiscard]] float               GetDuration()    const noexcept { return m_duration; }
    [[nodiscard]] float               GetElapsedTime() const noexcept { return m_elapsed; }
    [[nodiscard]] float               GetProgress()    const noexcept;
    [[nodiscard]] float               GetEasedProgress() const noexcept;
    [[nodiscard]] AnimationState      GetState()       const noexcept { return m_state; }
    [[nodiscard]] bool                IsPlaying()     const noexcept { return m_state == AnimationState::Playing; }
    [[nodiscard]] bool                IsPaused()      const noexcept { return m_state == AnimationState::Paused; }
    [[nodiscard]] bool                IsFinished()    const noexcept { return m_state == AnimationState::Finished; }
    [[nodiscard]] bool                GetLoop()       const noexcept { return m_loop; }
    [[nodiscard]] bool                GetReverse()    const noexcept { return m_reverse; }

    // ── Configuration ────────────────────────────────────────────────────

    /// @brief  Set the total duration in seconds.
    void SetDuration(float duration) noexcept { m_duration = duration; }

    /// @brief  Enable or disable looping.
    void SetLoop(bool loop) noexcept { m_loop = loop; }

    /// @brief  When true, the animation plays in reverse after finish.
    void SetReverse(bool reverse) noexcept { m_reverse = reverse; }

    /// @brief  Change the easing curve.
    void SetCurve(AnimationCurve curve) noexcept { m_curve = curve; }

    /// @brief  Register a callback invoked when the animation finishes.
    void SetOnFinished(Callback callback) noexcept { m_onFinished = std::move(callback); }

protected:
    /**
     * @brief  Called every frame with the eased progress value.
     *
     * Derived classes override this to drive actual property changes.
     *
     * @param progress  Eased value in [0, 1] (or [1, 0] if reversed).
     */
    virtual void OnUpdate(float progress) noexcept = 0;

    /// @brief  Called once when the animation reaches Finished state.
    virtual void OnFinished() noexcept;

private:
    float           m_duration;
    float           m_elapsed{ 0.0f };
    AnimationState  m_state{ AnimationState::Idle };
    AnimationCurve  m_curve;
    bool            m_loop{ false };
    bool            m_reverse{ false };
    Callback        m_onFinished;
};

} // namespace DragonOS::Animation
