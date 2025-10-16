// Boost.SafeProfile - C++ Safety Profile conformance analysis tool
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_SAFEPROFILE_CLI_ARGUMENTS_HPP
#define BOOST_SAFEPROFILE_CLI_ARGUMENTS_HPP

#include <string>
#include <optional>

namespace boost {
namespace safeprofile {
namespace cli {

/// Parsed command-line arguments for the analyze command
struct analyze_args {
    std::string target_path;                    // Repository or directory path
    std::string profile{"core-safety"};         // Profile to use
    std::optional<std::string> config_file;     // Optional config file path
    std::optional<std::string> sarif_output;    // SARIF output path
    std::optional<std::string> html_output;     // HTML report output path
    std::optional<std::string> evidence_dir;    // Evidence pack directory
    bool offline{true};                         // Offline mode (default)
    bool help{false};                           // Show help
    bool version{false};                        // Show version
};

/// Parse command-line arguments
/// Returns std::nullopt if help/version requested or parsing failed
std::optional<analyze_args> parse_arguments(int argc, char* argv[]);

} // namespace cli
} // namespace safeprofile
} // namespace boost

#endif // BOOST_SAFEPROFILE_CLI_ARGUMENTS_HPP
