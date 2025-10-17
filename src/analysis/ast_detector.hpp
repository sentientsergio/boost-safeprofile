// Boost.SafeProfile - C++ Safety Profile conformance analysis tool
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_SAFEPROFILE_ANALYSIS_AST_DETECTOR_HPP
#define BOOST_SAFEPROFILE_ANALYSIS_AST_DETECTOR_HPP

#include "profile/rule.hpp"
#include <boost/filesystem.hpp>
#include <vector>
#include <string>

namespace boost {
namespace safeprofile {
namespace analysis {

namespace fs = boost::filesystem;

/// Finding from AST analysis
struct ast_finding {
    fs::path file;
    unsigned int line;
    unsigned int column;
    std::string message;
    std::string rule_id;
    profile::severity severity;
    std::string snippet;  // Code snippet showing the violation
};

/// AST-based detector using Clang LibTooling
/// This replaces the keyword-based detector with proper semantic analysis
class ast_detector {
public:
    /// Analyze a single source file using AST
    /// Returns findings for the given rule
    std::vector<ast_finding> analyze_file(
        const fs::path& source_file,
        const profile::rule& rule
    ) const;

    /// Analyze multiple source files
    std::vector<ast_finding> analyze_files(
        const std::vector<fs::path>& source_files,
        const std::vector<profile::rule>& rules
    ) const;
};

} // namespace analysis
} // namespace safeprofile
} // namespace boost

#endif // BOOST_SAFEPROFILE_ANALYSIS_AST_DETECTOR_HPP
