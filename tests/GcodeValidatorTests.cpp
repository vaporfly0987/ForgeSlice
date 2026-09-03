#include "slicer/GcodeValidator.hpp"
#include <cassert>
#include <string>

using namespace forgeslice;

int main() {
    const auto profile = core::PrinterProfile::adventurer5mPro();

    const std::string good =
        "; ForgeSlice test\n"
        "G90\nM82\nG28\n"
        "M104 S200\nM140 S60\n"
        ";LAYER:0\n"
        "G1 X10 Y10 Z0.2 F10800\n"
        "G1 X100 Y10 E1.2 F4800\n"
        ";LAYER:1\n"
        "G1 X100 Y100 Z0.4 F10800\n"
        "G1 X10 Y100 E2.4 F4800\n"
        "M84\n";
    const auto ok = slicer::validateGcode(good, profile);
    assert(ok.ok);

    const std::string bad =
        "G90\nM82\nG28\n;LAYER:0\n"
        "G1 X221 Y10 Z0.2 F4800\n"
        "G1 X10 Y10 E1 F4800\nM84\n";
    const auto rejected = slicer::validateGcode(bad, profile);
    assert(!rejected.ok);
    assert(!rejected.issues.empty());

    const std::string badTemp =
        "G90\nM82\nG28\nM104 S400\n;LAYER:0\n"
        "G1 X10 Y10 Z0.2 F4800\nG1 X20 Y10 E1 F4800\nM84\n";
    const auto rejectedTemp = slicer::validateGcode(badTemp, profile);
    assert(!rejectedTemp.ok);

    return 0;
}
