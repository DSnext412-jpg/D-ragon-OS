
cmake_minimum_required(VERSION 3.20)

project(DragonOS
    VERSION     0.1.0
    DESCRIPTION "DragonOS — A modern Windows desktop environment"
    LANGUAGES   CXX
)

set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

file(GLOB_RECURSE DRAGONOS_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(DragonOS WIN32 ${DRAGONOS_SOURCES})

target_include_directories(DragonOS PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/sdk"
)


if(MSVC)
    target_compile_options(DragonOS PRIVATE
        /W4                         
        /permissive-                
        /utf-8                      
        /Zc:preprocessor           
        /Zc:__cplusplus             
        /Zc:throwingNew             
        /Zc:referenceBinding       
        /Zc:rvalueCast              
        /EHsc                       
    )
endif()

target_compile_definitions(DragonOS PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    _CRT_SECURE_NO_WARNINGS
)

option(DRAGONOS_BUILD_SAMPLE_PLUGIN "Build the sample plugin" ON)
if(DRAGONOS_BUILD_SAMPLE_PLUGIN AND EXISTS "${CMAKE_SOURCE_DIR}/plugins/sample/CMakeLists.txt")
    add_subdirectory(plugins/sample)
endif()

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
