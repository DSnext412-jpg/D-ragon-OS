/**
 * @file    AnimationState.hpp
 * @brief   Enumeration of states in an Animation's lifecycle.
 */

#pragma once

namespace DragonOS::Animation {

/**
 * @brief  Current state of an Animation.
 *
 * Transitions:
 *   Idle     --Play()--> Playing
 *   Playing  --Pause()--> Paused
 *   Playing  --Stop()-->  Idle
 *   Playing  (time up)--> Finished
 *   Paused   --Play()--> Playing
 *   Paused   --Stop()-->  Idle
 *   Finished --Reset()--> Idle
 *   Finished --Play()--> Playing
 */
enum class AnimationState {
    Idle,     ///< Not started / stopped.
    Playing,  ///< Actively advancing.
    Paused,   ///< Frozen at current elapsed time.
    Finished, ///< Reached the end of its duration.
};

} // namespace DragonOS::Animation
