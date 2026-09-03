#include "slicer/Mesh.hpp"

#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace forgeslice::slicer {

Mesh Mesh::loadAsciiStl(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Could not open STL: " + path);
    Mesh mesh;
    std::string line;
    std::vector<Vec3> vertices;
    while (std::getline(in, line)) {
        std::istringstream ss(line);
        std::string word;
        ss >> word;
        if (word != "vertex") continue;
        Vec3 v;
        if (ss >> v.x >> v.y >> v.z) vertices.push_back(v);
        if (vertices.size() == 3) {
            mesh.triangles.push_back({vertices[0], vertices[1], vertices[2]});
            vertices.clear();
        }
    }
    mesh.recalculateBounds();
    if (mesh.empty()) throw std::runtime_error("No ASCII STL triangles found. Binary STL is not supported yet.");
    return mesh;
}

void Mesh::recalculateBounds() {
    const double inf = std::numeric_limits<double>::infinity();
    min = {inf, inf, inf}; max = {-inf, -inf, -inf};
    for (const auto& t : triangles) for (const auto& v : {t.a, t.b, t.c}) {
        min.x = std::min(min.x, v.x); min.y = std::min(min.y, v.y); min.z = std::min(min.z, v.z);
        max.x = std::max(max.x, v.x); max.y = std::max(max.y, v.y); max.z = std::max(max.z, v.z);
    }
}

} // namespace forgeslice::slicer
