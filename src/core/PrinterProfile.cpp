#include "core/PrinterProfile.hpp"

namespace forgeslice::core {

PrinterProfile PrinterProfile::adventurer5mPro() {
    PrinterProfile profile;
    profile.id = "flashforge_adventurer_5m_pro";
    profile.name = "Adventurer 5M Pro";
    profile.manufacturer = "FlashForge";
    profile.build_volume = {220.0, 220.0, 220.0};
    profile.nozzles = {
        {0.25, "0.25 mm"},
        {0.40, "0.40 mm"},
        {0.60, "0.60 mm"},
        {0.80, "0.80 mm"},
    };
    profile.max_nozzle_temperature_c = 280.0;
    profile.max_bed_temperature_c = 100.0;
    profile.max_travel_speed_mm_s = 600.0;
    profile.max_acceleration_mm_s2 = 20000.0;
    profile.heated_bed = true;
    profile.enclosed = true;
    profile.supports_wifi = true;
    profile.supports_ethernet = true;
    profile.supports_camera = true;
    return profile;
}

} // namespace forgeslice::core
