# ============================================================================
#  DragonOS — Modern Windows desktop environment
#  CMake build configuration
# ============================================================================

cmake_minimum_required(VERSION 3.20)

project(DragonOS
    VERSION     0.1.0
    DESCRIPTION "DragonOS — A modern Windows desktop environment"
    LANGUAGES   CXX
)

# ---------------------------------------------------------------------------
#  C++ Standard
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# ---------------------------------------------------------------------------
#  Executable target
# ---------------------------------------------------------------------------
# NOTE: GLOB_RECURSE is used for the skeleton; replace with explicit
# source-file lists in production builds.
file(GLOB_RECURSE DRAGONOS_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(DragonOS WIN32 ${DRAGONOS_SOURCES})

# ---------------------------------------------------------------------------
#  Resources module — Phase 8A
#  include/Resources/  (headers)
#  src/Resources/      (sources, auto-discovered by GLOB_RECURSE below)
#  No additional libraries required at this phase.
# ---------------------------------------------------------------------------

# ---------------------------------------------------------------------------
#  Theme module — Phase 1 (Theme Engine Foundation)
#  include/Theme/  (headers)
#  src/Theme/      (sources, auto-discovered by GLOB_RECURSE below)
#  No additional libraries required; pure data structures.
# ---------------------------------------------------------------------------

# ---------------------------------------------------------------------------
#  Animation module — Animation Engine Foundation
#  include/Animation/  (headers)
#  src/Animation/      (sources, auto-discovered by GLOB_RECURSE below)
#  No additional libraries required; pure math and state management.
# ---------------------------------------------------------------------------

# ---------------------------------------------------------------------------
#  Include directories
# ---------------------------------------------------------------------------
target_include_directories(DragonOS PRIVATE "${CMAKE_SOURCE_DIR}/include")

# ---------------------------------------------------------------------------
#  Compiler options — MSVC
# ---------------------------------------------------------------------------
if(MSVC)
    target_compile_options(DragonOS PRIVATE
        /W4                         # Warning level 4
        /permissive-                # Strict standards conformance
        /utf-8                      # Source and execution charsets: UTF-8
        /Zc:preprocessor            # Conforming preprocessor
        /Zc:__cplusplus             # Correct __cplusplus value
        /Zc:throwingNew             # operator new throws on failure
        /Zc:referenceBinding        # Enforce reference-binding rules
        /Zc:rvalueCast              # Enforce rvalue-cast rules
        /EHsc                       # C++ exceptions only (no SEH)
    )
endif()

# ---------------------------------------------------------------------------
#  Preprocessor definitions
# ---------------------------------------------------------------------------
target_compile_definitions(DragonOS PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    _CRT_SECURE_NO_WARNINGS
)

# ---------------------------------------------------------------------------
#  Linker dependencies — Windows SDK libraries
# ---------------------------------------------------------------------------
target_link_libraries(DragonOS PRIVATE
    comctl32
    comdlg32
    d2d1
    dwrite
    dwmapi
    gdi32
    ole32
    shell32
    user32
    uxtheme
    windowscodecs
)
