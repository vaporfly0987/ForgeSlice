#pragma once

#include <string>
#include <vector>

namespace forgeslice::slicer {

struct Vec3 { double x{}, y{}, z{}; };
struct Triangle { Vec3 a, b, c; };

struct Mesh {
    std::vector<Triangle> triangles;
    Vec3 min{}, max{};
    static Mesh loadAsciiStl(const std::string& path);
    bool empty() const { return triangles.empty(); }
    void recalculateBounds();
};

} // namespace forgeslice::slicer
