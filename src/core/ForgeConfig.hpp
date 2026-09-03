#pragma once

namespace forgeslice::core {

struct ForgeConfig {
    bool dark_mode{true};
    bool show_advanced_settings{false};
    bool show_expert_settings{false};
    bool enable_background_slicing{true};
    bool enable_mesh_cache{true};
};

} // namespace forgeslice::core
