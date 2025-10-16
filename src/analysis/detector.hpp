// Boost.SafeProfile - C++ Safety Profile conformance analysis tool
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_SAFEPROFILE_ANALYSIS_DETECTOR_HPP
#define BOOST_SAFEPROFILE_ANALYSIS_DETECTOR_HPP

#include "../profile/rule.hpp"
#include "../intake/repository.hpp"
#include <boost/filesystem.hpp>
#include <vector>
#include <string>

namespace boost {
namespace safeprofile {
namespace analysis {

namespace fs = boost::filesystem;

/// A single violation finding
struct finding {
    std::string rule_id;           // Rule that was violated
    fs::path file_path;            // File containing violation
    int line_number;               // Line number (1-based)
    int column_number;             // Column number (1-based)
    std::string snippet;           // Code snippet showing violation
    profile::severity severity;    // Severity level from rule
};

/// Simple keyword-based detector (Phase 0 stub)
class detector {
public:
    detector() = default;

    /// Analyze source files against rules
    /// Phase 0: Simple keyword search in file contents
    std::vector<finding> analyze(
        const std::vector<intake::source_file>& sources,
        const std::vector<profile::rule>& rules) const;

private:
    /// Search a single file for a pattern
    std::vector<finding> search_file(
        const fs::path& file_path,
        const profile::rule& rule) const;
};

} // namespace analysis
} // namespace safeprofile
} // namespace boost

#endif // BOOST_SAFEPROFILE_ANALYSIS_DETECTOR_HPP
