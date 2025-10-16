// Boost.SafeProfile - C++ Safety Profile conformance analysis tool
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "cli/arguments.hpp"
#include "intake/repository.hpp"
#include "profile/loader.hpp"
#include <iostream>
#include <exception>

int main(int argc, char* argv[]) {
    try {
        auto args = boost::safeprofile::cli::parse_arguments(argc, argv);

        if (!args) {
            // Help or version was shown, or parsing failed
            return 0;
        }

        std::cout << "=== Boost.SafeProfile Analysis ===\n";
        std::cout << "Target: " << args->target_path << "\n";
        std::cout << "Profile: " << args->profile << "\n";
        std::cout << "Mode: " << (args->offline ? "offline" : "online") << "\n\n";

        // Step 1: Intake - discover source files
        std::cout << "Discovering C++ source files...\n";
        boost::safeprofile::intake::repository repo(args->target_path);
        auto sources = repo.discover_sources();

        std::cout << "Found " << sources.size() << " source file(s):\n";
        for (const auto& src : sources) {
            std::cout << "  " << src.path.string() << "\n";
        }
        std::cout << "\n";

        // Step 2: Load profile rules
        std::cout << "Loading profile: " << args->profile << "...\n";
        auto rules = boost::safeprofile::profile::loader::load_profile(args->profile);

        std::cout << "Loaded " << rules.size() << " rule(s):\n";
        for (const auto& rule : rules) {
            std::cout << "  [" << rule.id << "] " << rule.title << "\n";
        }
        std::cout << "\n";

        // Placeholder: analysis and output generation not yet implemented
        std::cout << "[Analysis and output generation not yet implemented]\n";

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
