/**
 * @file    System.hpp
 * @brief   Abstract base class for every runtime system in DragonOS.
 *
 * All systems (Renderer, Desktop, WindowManager, and future
 * InputManager, AnimationManager, etc.) inherit from this interface
 * so the SystemManager can treat them uniformly.
 */

#pragma once

namespace DragonOS::Engine {

// Forward declarations.
class EngineContext;

/**
 * @brief  Interface for a pluggable runtime system.
 *
 * Every system participates in the engine's lifecycle:
 *   Initialize()  — called once at startup
 *   Shutdown()   — called once at teardown
 *   Update()     — called every frame for non-rendering logic
 *   Render()     — called every frame to issue draw commands
 *   Resize()     — called when the viewport changes
 */
class System {
public:
    virtual ~System() = default;

    /// @brief  One-time initialisation.  @p ctx  provides shared state.
    /// @return true on success.
    [[nodiscard]] virtual bool Initialize(EngineContext& ctx) = 0;

    /// @brief  Release all resources owned by the system.
    virtual void Shutdown() = 0;

    /**
     * @brief  Per-frame update for non-rendering logic (animations, input).
     * @param deltaTime  Seconds elapsed since the previous frame.
     */
    virtual void Update(float deltaTime) = 0;

    /**
     * @brief  Issue draw commands for this system.
     *
     * Called inside a Renderer BeginFrame / EndFrame pair.  Systems
     * draw in registration order (background first, UI on top).
     *
     * @param ctx  Engine context providing access to the renderer.
     */
    virtual void Render(EngineContext& ctx) = 0;

    /// @brief  Respond to a viewport resize.
    virtual void Resize(float width, float height) = 0;
};

} // namespace DragonOS::Engine
