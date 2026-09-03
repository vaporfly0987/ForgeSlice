#pragma once

#include <string>
#include <vector>

namespace forgeslice::core {

struct NozzleOption {
    double diameter_mm{};
    std::string label;
};

struct BuildVolume {
    double width_mm{};
    double depth_mm{};
    double height_mm{};
};

struct PrinterProfile {
    std::string id;
    std::string name;
    std::string manufacturer;
    BuildVolume build_volume;
    std::vector<NozzleOption> nozzles;
    double max_nozzle_temperature_c{};
    double max_bed_temperature_c{};
    double max_travel_speed_mm_s{};
    double max_acceleration_mm_s2{};
    bool heated_bed{false};
    bool enclosed{false};
    bool supports_wifi{false};
    bool supports_ethernet{false};
    bool supports_camera{false};

    static PrinterProfile adventurer5mPro();
};

} // namespace forgeslice::core
