#include "slicer/Slicer.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <stdexcept>

namespace forgeslice::slicer {
namespace {
constexpr double EPS = 1e-7;

std::vector<Segment2> intersect(const Triangle& t, double z) {
    std::vector<Point2> p;
    auto edge = [&](const Vec3& a, const Vec3& b) {
        if ((a.z < z && b.z > z) || (a.z > z && b.z < z)) {
            const double q = (z - a.z) / (b.z - a.z);
            p.push_back({a.x + q * (b.x - a.x), a.y + q * (b.y - a.y)});
        } else if (std::abs(a.z - z) < EPS && std::abs(b.z - z) > EPS) {
            p.push_back({a.x, a.y});
        }
    };
    edge(t.a,t.b); edge(t.b,t.c); edge(t.c,t.a);
    if (p.size() < 2) return {};
    auto same = [](const Point2& a, const Point2& b) { return std::hypot(a.x-b.x,a.y-b.y) < EPS; };
    if (same(p[0],p[1])) return {};
    return {{{p[0],p[1]}}};
}

bool near(const Point2& a, const Point2& b) { return std::hypot(a.x-b.x,a.y-b.y) < 1e-4; }

std::vector<std::vector<Point2>> chain(std::vector<Segment2> segs) {
    std::vector<std::vector<Point2>> loops;
    while (!segs.empty()) {
        Segment2 s = segs.back(); segs.pop_back();
        std::vector<Point2> loop{s.a,s.b};
        bool extended = true;
        while (extended) {
            extended = false;
            for (size_t i=0;i<segs.size();++i) {
                if (near(segs[i].a, loop.back())) { loop.push_back(segs[i].b); segs.erase(segs.begin()+i); extended=true; break; }
                if (near(segs[i].b, loop.back())) { loop.push_back(segs[i].a); segs.erase(segs.begin()+i); extended=true; break; }
            }
        }
        if (loop.size() >= 3 && near(loop.front(), loop.back())) loop.pop_back();
        if (loop.size() >= 3) loops.push_back(std::move(loop));
    }
    return loops;
}

void emitMove(std::ofstream& out, const Point2& p, double z, double feed, double& e, bool extrude, double width) {
    out << "G1 X" << std::fixed << std::setprecision(3) << p.x << " Y" << p.y << " Z" << z;
    if (extrude) {
        e += std::hypot(p.x, p.y) * 0.001 * width;
        out << " E" << std::setprecision(5) << e;
    }
    out << " F" << std::setprecision(0) << feed*60.0 << "\n";
}
}

std::vector<SliceLayer> Slicer::slice(const Mesh& mesh) const {
    if (mesh.empty()) throw std::runtime_error("Cannot slice an empty mesh");
    std::vector<SliceLayer> result;
    const double start = mesh.min.z + settings_.first_layer_height_mm * 0.5;
    const size_t count = static_cast<size_t>(std::ceil((mesh.max.z - start) / settings_.layer_height_mm)) + 1;
    result.reserve(count);
    for (size_t i=0;i<count;++i) {
        const double z = start + i * settings_.layer_height_mm;
        if (z > mesh.max.z + EPS) break;
        std::vector<Segment2> segs;
        for (const auto& t : mesh.triangles) {
            auto s = intersect(t,z);
            segs.insert(segs.end(),s.begin(),s.end());
        }
        auto loops = chain(std::move(segs));
        if (!loops.empty()) result.push_back({z,std::move(loops)});
    }
    return result;
}

void Slicer::writeGcode(const Mesh& mesh, const std::string& output) const {
    const auto layers = slice(mesh);
    std::ofstream out(output);
    if (!out) throw std::runtime_error("Could not create G-code: " + output);
    out << "; ForgeSlice 0.1 MVP\n; Printer: FlashForge Adventurer 5M Pro\n; Layers: " << layers.size() << "\nG90\nM82\nG28\nM104 S200\nM140 S60\nM109 S200\nM190 S60\n";
    double e=0;
    for (size_t li=0;li<layers.size();++li) {
        const auto& layer=layers[li];
        out << ";LAYER:" << li << "\n";
        for (const auto& loop : layer.loops) {
            emitMove(out,loop.front(),layer.z,settings_.travel_speed_mm_s,e,false,settings_.line_width_mm);
            for (size_t j=1;j<=loop.size();++j) emitMove(out,loop[j%loop.size()],layer.z,settings_.print_speed_mm_s,e,true,settings_.line_width_mm);
        }
    }
    out << "M104 S0\nM140 S0\nG28 X0\nM84\n";
}

} // namespace forgeslice::slicer
