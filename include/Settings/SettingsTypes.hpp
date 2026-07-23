#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace DragonOS::Settings {

struct SettingsPageDef {
    std::wstring name;
    std::wstring displayName;
    std::wstring icon;
};

struct SettingEntry {
    std::wstring label;
    std::wstring description;
    enum class Type { Toggle, Slider, Combo, Button, Info } type;
    bool toggleValue{false};
    float sliderValue{0.5f};
    int comboIndex{0};
    std::vector<std::wstring> comboOptions;
    std::wstring infoText;
};

} // namespace
