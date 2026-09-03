#include "slicer/GcodeValidator.hpp"
#include <algorithm>
#include <cmath>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <string>

namespace forgeslice::slicer {
namespace {
constexpr double EPS = 1e-6;

void issue(GcodeValidationResult& r, int line, const std::string& msg) {
    r.ok = false;
    r.issues.push_back({line, msg});
}

bool valueAfter(const std::string& s, char key, double& out) {
    const auto p = s.find(key);
    if (p == std::string::npos) return false;
    const char* begin = s.c_str() + p + 1;
    char* end = nullptr;
    out = std::strtod(begin, &end);
    return end != begin && std::isfinite(out);
}

std::string commandOnly(std::string s) {
    const auto p = s.find(';');
    if (p != std::string::npos) s.resize(p);
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
    return s;
}
} // namespace

GcodeValidationResult validateGcode(const std::string& gcode,
                                    const core::PrinterProfile& profile) {
    GcodeValidationResult result;
    if (gcode.empty()) {
        issue(result, 0, "G-code is empty");
        return result;
    }

    double x = 0.0, y = 0.0, z = 0.0, e = 0.0;
    double lastZ = -EPS;
    bool absoluteXYZ = true;
    bool absoluteE = true;
    bool sawMotion = false;
    bool sawLayer = false;
    bool sawHome = false;
    bool sawExtrusion = false;
    bool sawEnd = false;
    int layerCount = 0;

    std::istringstream input(gcode);
    std::string raw;
    int lineNo = 0;
    while (std::getline(input, raw)) {
        ++lineNo;
        const std::string s = commandOnly(raw);
        if (s.empty()) continue;

        std::istringstream ss(s);
        std::string cmd;
        ss >> cmd;
        if (cmd.empty()) continue;
        std::string rest;
        std::getline(ss, rest);

        if (cmd == "G90") { absoluteXYZ = true; continue; }
        if (cmd == "G91") { absoluteXYZ = false; continue; }
        if (cmd == "M82") { absoluteE = true; continue; }
        if (cmd == "M83") { absoluteE = false; continue; }
        if (cmd == "G28") { sawHome = true; continue; }
        if (cmd == "G92") {
            double v = 0.0;
            if (valueAfter(rest, 'E', v)) e = v;
            if (valueAfter(rest, 'X', v)) x = v;
            if (valueAfter(rest, 'Y', v)) y = v;
            if (valueAfter(rest, 'Z', v)) z = v;
            continue;
        }
        if (cmd == "M104" || cmd == "M109") {
            double t = 0.0;
            if (valueAfter(rest, 'S', t) && (t < 0.0 || t > profile.max_nozzle_temperature_c + EPS))
                issue(result, lineNo, "Nozzle temperature exceeds printer profile limits");
            continue;
        }
        if (cmd == "M140" || cmd == "M190") {
            double t = 0.0;
            if (valueAfter(rest, 'S', t) && (t < 0.0 || t > profile.max_bed_temperature_c + EPS))
                issue(result, lineNo, "Bed temperature exceeds printer profile limits");
            continue;
        }
        if (cmd == ";LAYER:" || cmd.rfind(";LAYER:", 0) == 0) {
            sawLayer = true;
            ++layerCount;
            continue;
        }
        if (cmd == "M84" || cmd == "M104" && rest.find('S') != std::string::npos) {
            if (cmd == "M84") sawEnd = true;
            continue;
        }

        if (cmd == "G0" || cmd == "G1") {
            sawMotion = true;
            double nx = x, ny = y, nz = z, ne = e, f = 0.0;
            const bool hasX = valueAfter(rest, 'X', nx);
            const bool hasY = valueAfter(rest, 'Y', ny);
            const bool hasZ = valueAfter(rest, 'Z', nz);
            const bool hasE = valueAfter(rest, 'E', ne);
            const bool hasF = valueAfter(rest, 'F', f);
            if (!std::isfinite(nx) || !std::isfinite(ny) || !std::isfinite(nz) || !std::isfinite(ne)) {
                issue(result, lineNo, "Non-finite motion coordinate or extrusion");
                continue;
            }
            if (!absoluteXYZ) {
                if (hasX) nx += x;
                if (hasY) ny += y;
                if (hasZ) nz += z;
            }
            if (!absoluteE && hasE) ne += e;
            if (nx < -EPS || ny < -EPS || nx > profile.build_volume.width_mm + EPS ||
                ny > profile.build_volume.depth_mm + EPS || nz < -EPS ||
                nz > profile.build_volume.height_mm + EPS) {
                issue(result, lineNo, "Motion leaves the printer build volume");
            }
            if (hasF && f < 0.0) issue(result, lineNo, "Negative feed rate");
            if (hasE && std::abs(ne - e) > 1e-7) sawExtrusion = true;
            if (hasZ && nz + EPS < lastZ) issue(result, lineNo, "Layer Z moved backwards");
            x = nx; y = ny; z = nz; e = ne;
            if (z > lastZ) lastZ = z;
            continue;
        }

        if (cmd == "M999") issue(result, lineNo, "G-code contains a firmware reset/recovery command");
    }

    if (!sawHome) issue(result, 0, "Missing homing command (G28)");
    if (!sawMotion) issue(result, 0, "No motion commands found");
    if (!sawLayer) issue(result, 0, "No layer markers found");
    if (!sawExtrusion) issue(result, 0, "No extrusion moves found");
    if (layerCount <= 0) issue(result, 0, "No printable layers found");
    if (!sawEnd) issue(result, 0, "Missing end-of-print command (M84)");
    return result;
}

} // namespace forgeslice::slicer
