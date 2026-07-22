cmake_minimum_required(VERSION 3.20)
project(HelloDragonOS VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} SHARED
    SamplePlugin.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/sdk
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    # No additional link dependencies - SDK is header-only
)

# MSVC settings
target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive- /utf-8 /EHsc)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    PREFIX ""
    SUFFIX ".dll"
)
