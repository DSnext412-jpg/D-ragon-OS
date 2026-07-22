#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace dragonos::sdk {

class IResourceService {
public:
    virtual ~IResourceService() noexcept = default;

    virtual std::vector<uint8_t> LoadBinary(std::wstring_view path) noexcept = 0;
    virtual std::wstring LoadText(std::wstring_view path) noexcept = 0;
    virtual std::wstring ResolvePath(std::wstring_view relativePath) noexcept = 0;
    virtual bool Exists(std::wstring_view path) noexcept = 0;
};

} // namespace dragonos::sdk
