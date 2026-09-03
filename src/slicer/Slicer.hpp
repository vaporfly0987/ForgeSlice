#pragma once

#include "slicer/Mesh.hpp"
#include <string>
#include <vector>

namespace forgeslice::slicer {

struct Point2 { double x{}, y{}; };
struct Segment2 { Point2 a, b; };
struct SliceLayer { double z{}; std::vector<std::vector<Point2>> loops; };

enum class InfillPattern {
    Rectilinear, Grid, Lines, ZigZag, Cross, Cross3D, Triangles, TriHexagon,
    Honeycomb, Hexagon, Cubic, CubicSubdivision, Octet, Gyroid, Concentric,
    Hilbert, ArchimedeanChords, Stars, Scaffolding, Lightning, AdaptiveLines,
    AdaptiveCubic, Voronoi, Random, Hilbert3D, Sierpinski,
    Diamond, QuarterCubic, SupportCubic, 3DHoneycomb, Weave, Waves,
    CrossHatch, Bricks, Tetrahedral, Rhombic, Rectilinear3D, Spiral,
    Smooth, Lattice, DenseLattice, Fractal, Fermat, Lightning2D,
    Honeycomb3D, Gyroid3D, SchwarzP, SchwarzD, IWP, Neovius,
    FischerKoch, Hilbert2D, Cubic3D, Triangles3D, Hexagons, Concentric3D
};

const char* infillPatternName(InfillPattern pattern);
InfillPattern infillPatternFromString(const std::string& name);
std::vector<std::string> availableInfillPatterns();

struct SliceSettings {
    double layer_height_mm = 0.20;
    double first_layer_height_mm = 0.20;
    double nozzle_mm = 0.40;
    double filament_diameter_mm = 1.75;
    double line_width_mm = 0.42;
    int walls = 3;
    double infill_percent = 20.0;
    InfillPattern infill_pattern = InfillPattern::Gyroid;
    double print_speed_mm_s = 80.0;
    double travel_speed_mm_s = 180.0;
    double retraction_mm = 0.8;
    double retraction_speed_mm_s = 35.0;
    int bottom_layers = 5;
    int top_layers = 5;
    double nozzle_temperature_c = 200.0;
    double bed_temperature_c = 60.0;
    bool enable_retraction = true;
    bool avoid_crossing_perimeters = false;
    bool relative_extrusion = false;
    unsigned int infill_seed = 1337;
};

class Slicer {
public:
    explicit Slicer(SliceSettings settings = {}): settings_(settings) {}
    std::vector<SliceLayer> slice(const Mesh& mesh) const;
    void writeGcode(const Mesh& mesh, const std::string& output) const;
private:
    SliceSettings settings_;
};

} // namespace forgeslice::slicer
