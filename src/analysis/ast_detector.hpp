// Boost.SafeProfile - C++ Safety Profile conformance analysis tool
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_SAFEPROFILE_ANALYSIS_AST_DETECTOR_HPP
#define BOOST_SAFEPROFILE_ANALYSIS_AST_DETECTOR_HPP

#include "profile/rule.hpp"
#include "intake/compile_commands.hpp"
#include <boost/filesystem.hpp>
#include <vector>
#include <string>
#include <memory>

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

/// Analysis result for a single file
struct file_analysis_result {
    fs::path file;
    bool success;  // true if analysis succeeded, false if compilation failed
    std::string error_message;  // populated if success == false
    std::vector<ast_finding> findings;  // populated if success == true
};

/// AST-based detector using Clang LibTooling
/// This replaces the keyword-based detector with proper semantic analysis
class ast_detector {
public:
    /// Set compilation database for resolving include paths and flags
    /// Should be called before analyze_files() for best results
    void set_compilation_database(std::shared_ptr<intake::compile_commands_reader> db) {
        compile_db_ = db;
    }

    /// Set additional include paths to use when no compilation database is available
    /// Typically used to infer include paths from the analyzed directory
    void set_additional_include_paths(const std::vector<std::string>& paths) {
        additional_include_paths_ = paths;
    }

    /// Analyze a single source file using AST
    /// Returns result with success status and findings (or error message)
    file_analysis_result analyze_file(
        const fs::path& source_file,
        const profile::rule& rule
    ) const;

    /// Analyze a single source file with explicit compiler flags
    /// Used internally when compilation database is available
    file_analysis_result analyze_file_with_flags(
        const fs::path& source_file,
        const profile::rule& rule,
        const std::vector<std::string>& compiler_args
    ) const;

    /// Analyze multiple source files
    /// Returns findings from all successfully analyzed files
    /// Files that fail to compile are tracked separately
    std::vector<ast_finding> analyze_files(
        const std::vector<fs::path>& source_files,
        const std::vector<profile::rule>& rules,
        std::vector<file_analysis_result>& failed_files  // OUT: files that failed analysis
    ) const;

private:
    std::shared_ptr<intake::compile_commands_reader> compile_db_;
    std::vector<std::string> additional_include_paths_;  // Inferred include paths

    /// Build default compiler arguments if no compilation database available
    std::vector<std::string> get_default_compiler_args() const;

    /// Build compiler arguments from compilation flags
    std::vector<std::string> build_compiler_args(
        const intake::compilation_flags& flags
    ) const;
};

} // namespace analysis
} // namespace safeprofile
} // namespace boost

#endif // BOOST_SAFEPROFILE_ANALYSIS_AST_DETECTOR_HPP
