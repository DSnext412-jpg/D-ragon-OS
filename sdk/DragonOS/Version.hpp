#pragma once

#define DRAGONOS_SDK_VERSION_MAJOR 1
#define DRAGONOS_SDK_VERSION_MINOR 0
#define DRAGONOS_SDK_VERSION_PATCH 0

#define DRAGONOS_SDK_VERSION \
    (DRAGONOS_SDK_VERSION_MAJOR * 10000 + \
     DRAGONOS_SDK_VERSION_MINOR * 100 + \
     DRAGONOS_SDK_VERSION_PATCH)

#define DRAGONOS_SDK_VERSION_STRING "1.0.0"

namespace dragonos::sdk {

struct SDKVersion {
    int major{ DRAGONOS_SDK_VERSION_MAJOR };
    int minor{ DRAGONOS_SDK_VERSION_MINOR };
    int patch{ DRAGONOS_SDK_VERSION_PATCH };

    constexpr int ToInt() const noexcept { return major * 10000 + minor * 100 + patch; }

    static constexpr bool IsCompatible(int requiredVersion) noexcept
    {
        const int current = DRAGONOS_SDK_VERSION;
        const int reqMajor = requiredVersion / 10000;
        const int curMajor = current / 10000;
        return reqMajor == curMajor && requiredVersion <= current;
    }
};

} // namespace dragonos::sdk
