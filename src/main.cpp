// Boost.SafeProfile - C++ Safety Profile conformance analysis tool
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "cli/arguments.hpp"
#include "intake/repository.hpp"
#include "intake/compile_commands.hpp"
#include "profile/loader.hpp"
#include "analysis/detector.hpp"
#include "analysis/ast_detector.hpp"
#include "emit/sarif.hpp"
#include <iostream>
#include <exception>
#include <memory>

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

        // Step 2.5: Try to load compile_commands.json (optional)
        auto compile_db = std::make_shared<boost::safeprofile::intake::compile_commands_reader>();
        if (compile_db->load_from_directory(args->target_path)) {
            std::cout << "Loaded compile_commands.json (" << compile_db->entry_count() << " entries)\n";
            std::cout << "Using compilation database for include paths and flags.\n\n";
        } else {
            std::cout << "No compile_commands.json found - using default C++20 flags.\n";
            std::cout << "(Tip: Generate with 'cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON' for better results)\n\n";
        }

        // Step 3: Run analysis (using AST-based detector)
        std::cout << "Running AST-based analysis...\n";
        boost::safeprofile::analysis::ast_detector ast_det;

        // Set compilation database if loaded
        if (compile_db->is_loaded()) {
            ast_det.set_compilation_database(compile_db);
        } else {
            // No compilation database - infer include paths from analyzed directory
            // This helps analyze projects without needing a full build
            std::vector<std::string> inferred_includes;

            // Add the target directory itself (for relative includes)
            boost::filesystem::path target_abs = boost::filesystem::absolute(args->target_path);
            inferred_includes.push_back(target_abs.string());

            // If analyzing within an 'include/' directory, also add parent
            // (supports common pattern: boost-json/include/boost/json/...)
            if (target_abs.filename() == "include") {
                inferred_includes.push_back(target_abs.parent_path().string());
            }
            // If inside a subdirectory of 'include/', add the include root
            else {
                auto parent = target_abs.parent_path();
                while (!parent.empty() && parent != parent.parent_path()) {
                    if (parent.filename() == "include") {
                        inferred_includes.push_back(parent.string());
                        break;
                    }
                    parent = parent.parent_path();
                }
            }

            // Auto-detect Boost headers (for analyzing Boost libraries)
            // Check common system install locations
            std::vector<boost::filesystem::path> boost_search_paths = {
                "/opt/homebrew/include",               // macOS Homebrew
                "/usr/local/include",                  // Standard Unix/Linux
                "/usr/include",                        // Debian/Ubuntu
                boost::filesystem::path(getenv("HOME") ? getenv("HOME") : "") / ".local/include"  // User install
            };

            boost::filesystem::path boost_root;
            for (const auto& search_path : boost_search_paths) {
                if (boost::filesystem::exists(search_path / "boost" / "config.hpp")) {
                    // System install (boost/ directory at root)
                    boost_root = search_path;
                    inferred_includes.push_back(boost_root.string());
                    break;
                } else if (boost::filesystem::exists(search_path / "libs" / "config" / "include" / "boost" / "config.hpp")) {
                    // Boost super-project layout (libs/*/include/)
                    boost_root = search_path;
                    // Add all libs/*/include directories
                    if (boost::filesystem::exists(boost_root / "libs")) {
                        boost::filesystem::directory_iterator end_iter;
                        for (boost::filesystem::directory_iterator lib_iter(boost_root / "libs"); lib_iter != end_iter; ++lib_iter) {
                            if (boost::filesystem::is_directory(lib_iter->status())) {
                                auto include_dir = lib_iter->path() / "include";
                                if (boost::filesystem::exists(include_dir)) {
                                    inferred_includes.push_back(include_dir.string());
                                }
                            }
                        }
                    }
                    break;
                }
            }

            if (!inferred_includes.empty()) {
                ast_det.set_additional_include_paths(inferred_includes);
                std::cout << "Inferred include path(s):\n";
                if (!boost_root.empty()) {
                    std::cout << "  Found Boost headers at: " << boost_root.string() << "\n";
                }
                std::cout << "  Total: " << inferred_includes.size() << " include path(s)\n\n";
            }
        }

        // Extract file paths from sources
        std::vector<boost::filesystem::path> file_paths;
        for (const auto& src : sources) {
            file_paths.push_back(src.path);
        }

        std::vector<boost::safeprofile::analysis::file_analysis_result> failed_files;
        auto ast_findings = ast_det.analyze_files(file_paths, rules, failed_files);

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

        // Report compilation failures
        if (!failed_files.empty()) {
            std::cerr << "⚠️  WARNING: " << failed_files.size() << " file(s) failed to compile and were not analyzed:\n";
            for (const auto& failed : failed_files) {
                std::cerr << "  " << failed.file.string() << ": " << failed.error_message << "\n";
            }
            std::cerr << "\nNote: Compilation errors prevent AST analysis. ";
            std::cerr << "Ensure files compile with C++20 or provide compile_commands.json.\n\n";
        }

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
            if (failed_files.empty()) {
                std::cout << "No violations found! ✓\n\n";
            } else {
                std::cout << "No violations found in successfully analyzed files.\n";
                std::cout << "(However, some files failed to compile - see warnings above)\n\n";
            }
        }

        // Step 4: Generate SARIF output (if requested)
        if (args->sarif_output) {
            std::cout << "Generating SARIF output...\n";
            boost::safeprofile::emit::sarif_emitter emitter;
            auto sarif_doc = emitter.generate(findings, rules);
            emitter.write_to_file(sarif_doc, *args->sarif_output);
            std::cout << "SARIF written to: " << *args->sarif_output << "\n\n";
        }

        // Exit codes:
        // 0 = no violations, all files analyzed successfully
        // 1 = violations found (but all files analyzed successfully)
        // 2 = some files failed to compile (partial analysis)
        if (!failed_files.empty()) {
            return 2;  // Partial failure - some files couldn't be analyzed
        }
        return findings.empty() ? 0 : 1;  // Success or violations

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 3;  // Fatal error
    }
}
