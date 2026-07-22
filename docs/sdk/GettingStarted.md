# DragonOS SDK — Getting Started

This guide walks you through setting up the DragonOS SDK and building
your first plugin in C++.

## Prerequisites

- Windows 10/11
- Visual Studio 2022 or later with "Desktop development with C++"
- CMake 3.20+
- DragonOS installed and running on the target machine

## Project Structure

```
MyPlugin/
├── CMakeLists.txt
├── plugin.json
├── src/
│   └── MyPlugin.cpp
└── README.md
```

## 1. Create CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyPlugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} SHARED src/MyPlugin.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
    path/to/DragonOS/sdk
)

target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive- /utf-8 /EHsc)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN
)
```

## 2. Create plugin.json

```json
{
    "name": "MyPlugin",
    "displayName": "My Plugin",
    "description": "Does something amazing",
    "version": "1.0.0",
    "author": "Your Name",
    "vendor": "Your Company",
    "sdkVersion": 10000,
    "permissions": ["Notifications"],
    "dependencies": []
}
```

## 3. Write the Plugin

```cpp
#include <DragonOS/DragonOS.hpp>

class MyApp final : public dragonos::sdk::IApplication {
public:
    bool Initialize(dragonos::sdk::PluginContext& ctx) noexcept override
    {
        auto& log = dragonos::sdk::Logger::Get();
        log.Info(L"MyPlugin: Initialized!");
        return true;
    }

    void Shutdown() noexcept override {}

    const dragonos::sdk::AppMetadata& GetMetadata() const noexcept override
    {
        static const dragonos::sdk::AppMetadata m{
            .name = L"MyPlugin",
            .displayName = L"My Plugin",
            .description = L"Does something amazing",
            .version = L"1.0.0",
            .author = L"Your Name",
            .vendor = L"Your Company",
            .sdkVersion = DRAGONOS_SDK_VERSION,
        };
        return m;
    }
};

DRAGONOS_DECLARE_PLUGIN(MyApp)
```

## 4. Build

```cmd
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

## 5. Install

Copy the output DLL and `plugin.json` to:

```
<DragonOS>/plugins/
```

DragonOS scans this directory at startup and loads compatible plugins.

## Next Steps

- Read the [Plugin Guide](PluginGuide.md) for detailed API usage
- Browse the [API Reference](APIReference.md) for all available services
