#include "app/Application.hpp"
#include "slicer/Mesh.hpp"
#include "slicer/Slicer.hpp"

#include <iostream>
#include <string>

namespace {
void usage() {
    std::cout << "ForgeSlice 0.2\n"
              << "Usage: ForgeSlice model.stl [options]\n\n"
              << "Options:\n"
              << "  -o <file>              Output G-code path\n"
              << "  --layer-height <mm>    Layer height (default 0.20)\n"
              << "  --first-layer <mm>     First layer height\n"
              << "  --walls <n>            Wall count setting\n"
              << "  --infill <percent>     Infill density 0-100\n"
              << "  --speed <mm/s>         Print speed\n"
              << "  --travel <mm/s>        Travel speed\n"
              << "  --help                 Show this help\n";
}
}

int main(int argc, char** argv) {
    if (argc < 2) return forgeslice::app::Application{}.run();

    std::string input, output = "ForgeSlice.gcode";
    forgeslice::slicer::SliceSettings settings;
    try {
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "-o" && i + 1 < argc) output = argv[++i];
            else if (arg == "--layer-height" && i + 1 < argc) settings.layer_height_mm = std::stod(argv[++i]);
            else if (arg == "--first-layer" && i + 1 < argc) settings.first_layer_height_mm = std::stod(argv[++i]);
            else if (arg == "--walls" && i + 1 < argc) settings.walls = std::stoi(argv[++i]);
            else if (arg == "--infill" && i + 1 < argc) settings.infill_percent = std::stod(argv[++i]);
            else if (arg == "--speed" && i + 1 < argc) settings.print_speed_mm_s = std::stod(argv[++i]);
            else if (arg == "--travel" && i + 1 < argc) settings.travel_speed_mm_s = std::stod(argv[++i]);
            else if (arg == "--help") { usage(); return 0; }
            else if (arg.rfind("-", 0) != 0) input = arg;
            else { std::cerr << "Unknown option: " << arg << "\n"; usage(); return 2; }
        }
        if (input.empty()) { usage(); return 2; }
        const auto mesh = forgeslice::slicer::Mesh::loadAsciiStl(input);
        forgeslice::slicer::Slicer slicer(settings);
        const auto layers = slicer.slice(mesh);
        slicer.writeGcode(mesh, output);
        std::cout << "Sliced " << input << " -> " << output << " (" << layers.size() << " layers)\n";
    } catch (const std::exception& e) {
        std::cerr << "ForgeSlice error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
