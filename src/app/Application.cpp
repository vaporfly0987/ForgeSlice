#include "app/Application.hpp"

#include "core/BuildInfo.hpp"
#include "core/PrinterProfile.hpp"
#include "core/SettingsMode.hpp"

#include <iostream>

namespace forgeslice::app {

int Application::run() const {
    const auto printer = core::PrinterProfile::adventurer5mPro();

    std::cout << core::kProductName << " " << core::kVersion << "\n";
    std::cout << "Printer: " << printer.manufacturer << " " << printer.name << "\n";
    std::cout << "Build volume: " << printer.build_volume.width_mm << " x "
              << printer.build_volume.depth_mm << " x " << printer.build_volume.height_mm
              << " mm\n";
    std::cout << "Settings: " << core::toString(core::SettingsMode::Basic) << " -> "
              << core::toString(core::SettingsMode::Advanced) << " -> "
              << core::toString(core::SettingsMode::Expert) << "\n";
    return 0;
}

} // namespace forgeslice::app
