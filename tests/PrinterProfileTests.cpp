#include "core/PrinterProfile.hpp"

#include <cassert>
#include <cmath>

int main() {
    const auto profile = forgeslice::core::PrinterProfile::adventurer5mPro();

    assert(profile.name == "Adventurer 5M Pro");
    assert(profile.manufacturer == "FlashForge");
    assert(std::abs(profile.build_volume.width_mm - 220.0) < 1e-9);
    assert(std::abs(profile.build_volume.depth_mm - 220.0) < 1e-9);
    assert(std::abs(profile.build_volume.height_mm - 220.0) < 1e-9);
    assert(profile.nozzles.size() == 4);
    assert(profile.supports_camera);
    assert(profile.supports_wifi);
    assert(profile.supports_ethernet);

    return 0;
}
