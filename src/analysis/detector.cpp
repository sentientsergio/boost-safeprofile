// Boost.SafeProfile - C++ Safety Profile conformance analysis tool
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "detector.hpp"
#include <fstream>
#include <sstream>

namespace boost {
namespace safeprofile {
namespace analysis {

std::vector<finding> detector::analyze(
    const std::vector<intake::source_file>& sources,
    const std::vector<profile::rule>& rules) const {

    std::vector<finding> findings;

    // For each source file
    for (const auto& source : sources) {
        // Apply each rule
        for (const auto& rule : rules) {
            auto file_findings = search_file(source.path, rule);
            findings.insert(findings.end(), file_findings.begin(), file_findings.end());
        }
    }

    return findings;
}

std::vector<finding> detector::search_file(
    const fs::path& file_path,
    const profile::rule& rule) const {

    std::vector<finding> findings;

    std::ifstream file(file_path.string());
    if (!file.is_open()) {
        return findings;  // Skip files we can't open
    }

    std::string line;
    int line_number = 0;

    while (std::getline(file, line)) {
        ++line_number;

        // Phase 0: Simple substring search for the pattern
        size_t pos = line.find(rule.pattern);
        if (pos != std::string::npos) {
            finding f;
            f.rule_id = rule.id;
            f.file_path = file_path;
            f.line_number = line_number;
            f.column_number = static_cast<int>(pos) + 1;  // 1-based column
            f.snippet = line;
            f.severity = rule.level;

            findings.push_back(f);
        }
    }

    return findings;
}

} // namespace analysis
} // namespace safeprofile
} // namespace boost
