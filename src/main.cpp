// Boost.SafeProfile - C++ Safety Profile conformance analysis tool
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "cli/arguments.hpp"
#include "intake/repository.hpp"
#include "profile/loader.hpp"
#include "analysis/detector.hpp"
#include "analysis/ast_detector.hpp"
#include "emit/sarif.hpp"
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

        // Step 3: Run analysis (using AST-based detector)
        std::cout << "Running AST-based analysis...\n";
        boost::safeprofile::analysis::ast_detector ast_det;

        // Extract file paths from sources
        std::vector<boost::filesystem::path> file_paths;
        for (const auto& src : sources) {
            file_paths.push_back(src.path);
        }

        auto ast_findings = ast_det.analyze_files(file_paths, rules);

        // Convert AST findings to regular findings for compatibility
        std::vector<boost::safeprofile::analysis::finding> findings;
        for (const auto& af : ast_findings) {
            findings.push_back({
                af.rule_id,
                af.file,
                static_cast<int>(af.line),
                static_cast<int>(af.column),
                af.snippet,
                af.severity
            });
        }

        std::cout << "Analysis complete. Found " << findings.size() << " violation(s).\n";
        std::cout << "(AST-based detection - no false positives in comments/strings)\n\n";

        // Display findings
        if (!findings.empty()) {
            std::cout << "Violations:\n";
            for (const auto& f : findings) {
                std::cout << "  " << f.file_path.string() << ":" << f.line_number
                          << ":" << f.column_number << " [" << f.rule_id << "]\n";
                std::cout << "    " << f.snippet << "\n";
            }
            std::cout << "\n";
        } else {
            std::cout << "No violations found! ✓\n\n";
        }

        // Step 4: Generate SARIF output (if requested)
        if (args->sarif_output) {
            std::cout << "Generating SARIF output...\n";
            boost::safeprofile::emit::sarif_emitter emitter;
            auto sarif_doc = emitter.generate(findings, rules);
            emitter.write_to_file(sarif_doc, *args->sarif_output);
            std::cout << "SARIF written to: " << *args->sarif_output << "\n\n";
        }

        return findings.empty() ? 0 : 1;  // Exit code reflects findings

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
