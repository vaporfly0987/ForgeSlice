#include "app/Application.hpp"
#include "slicer/Mesh.hpp"
#include "slicer/Slicer.hpp"

#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc < 2) return forgeslice::app::Application{}.run();

    std::string input;
    std::string output = "ForgeSlice.gcode";
    forgeslice::slicer::SliceSettings settings;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc) output = argv[++i];
        else if (arg == "--layer-height" && i + 1 < argc) settings.layer_height_mm = std::stod(argv[++i]);
        else if (arg == "--walls" && i + 1 < argc) settings.walls = std::stoi(argv[++i]);
        else if (arg == "--infill" && i + 1 < argc) settings.infill_percent = std::stod(argv[++i]);
        else if (arg == "--help") {
            std::cout << "ForgeSlice 0.1 MVP\nUsage: ForgeSlice model.stl [-o output.gcode] [--layer-height mm] [--walls n] [--infill percent]\n";
            return 0;
        } else if (arg.rfind("-", 0) != 0) input = arg;
    }
    if (input.empty()) { std::cerr << "No STL file specified. Use --help.\n"; return 2; }

    try {
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
