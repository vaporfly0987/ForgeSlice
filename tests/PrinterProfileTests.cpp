#include "core/PrinterProfile.hpp"
#include "slicer/GcodeValidator.hpp"

#include <cassert>
#include <cmath>
#include <string>

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

    const std::string good =
        "G90\nM82\nG28\nM104 S200\nM140 S60\n"
        ";LAYER:0\nG1 X10 Y10 Z0.2 F4800\nG1 X100 Y10 E1.2 F4800\n"
        ";LAYER:1\nG1 X100 Y100 Z0.4 F4800\nG1 X10 Y100 E2.4 F4800\nM84\n";
    assert(forgeslice::slicer::validateGcode(good, profile).ok);

    const std::string outOfBounds =
        "G90\nM82\nG28\n;LAYER:0\nG1 X221 Y10 Z0.2 F4800\nG1 X20 Y10 E1 F4800\nM84\n";
    assert(!forgeslice::slicer::validateGcode(outOfBounds, profile).ok);

    return 0;
}
