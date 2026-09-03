#pragma once

#include <string_view>

namespace forgeslice::core {

enum class SettingsMode {
    Basic,
    Advanced,
    Expert,
};

constexpr std::string_view toString(SettingsMode mode) noexcept {
    switch (mode) {
        case SettingsMode::Basic: return "Basic";
        case SettingsMode::Advanced: return "Advanced";
        case SettingsMode::Expert: return "Expert";
    }
    return "Basic";
}

} // namespace forgeslice::core
