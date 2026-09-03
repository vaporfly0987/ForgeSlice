#pragma once

#include "core/PrinterProfile.hpp"
#include <string>
#include <vector>

namespace forgeslice::slicer {

struct GcodeValidationIssue {
    int line{};
    std::string message;
};

struct GcodeValidationResult {
    bool ok{true};
    std::vector<GcodeValidationIssue> issues;
};

GcodeValidationResult validateGcode(const std::string& gcode,
                                    const core::PrinterProfile& profile);

} // namespace forgeslice::slicer
