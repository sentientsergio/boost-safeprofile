// Boost.SafeProfile - C++ Safety Profile conformance analysis tool
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "cli/arguments.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    auto args = boost::safeprofile::cli::parse_arguments(argc, argv);

    if (!args) {
        // Help or version was shown, or parsing failed
        return 0;
    }

    // Placeholder: actual analysis will be implemented in later steps
    std::cout << "Analyzing: " << args->target_path << "\n";
    std::cout << "Profile: " << args->profile << "\n";
    std::cout << "Mode: " << (args->offline ? "offline" : "online") << "\n";

    if (args->sarif_output) {
        std::cout << "SARIF output: " << *args->sarif_output << "\n";
    }
    if (args->html_output) {
        std::cout << "HTML report: " << *args->html_output << "\n";
    }
    if (args->evidence_dir) {
        std::cout << "Evidence directory: " << *args->evidence_dir << "\n";
    }

    std::cout << "\n[Analysis pipeline not yet implemented - Phase 0 stub]\n";

    return 0;
}
