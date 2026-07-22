#pragma once

#include <DragonOS/Resource.hpp>

#include <Resources/ResourceManager.hpp>

#include <string>
#include <vector>

namespace DragonOS::SDK {

class ResourceServiceAdapter final : public dragonos::sdk::IResourceService {
public:
    explicit ResourceServiceAdapter(
        Resources::ResourceManager& mgr) noexcept
        : m_resourceManager{ mgr }
    {
    }

    std::vector<uint8_t> LoadBinary(
        std::wstring_view path) noexcept override;
    std::wstring LoadText(
        std::wstring_view path) noexcept override;
    std::wstring ResolvePath(
        std::wstring_view relativePath) noexcept override;
    bool Exists(std::wstring_view path) noexcept override;

private:
    Resources::ResourceManager& m_resourceManager;
};

} // namespace DragonOS::SDK
