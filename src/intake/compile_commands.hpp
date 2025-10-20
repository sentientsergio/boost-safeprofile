// Boost.SafeProfile - C++ Safety Profile conformance analysis tool
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_SAFEPROFILE_INTAKE_COMPILE_COMMANDS_HPP
#define BOOST_SAFEPROFILE_INTAKE_COMPILE_COMMANDS_HPP

#include <boost/filesystem.hpp>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

namespace boost {
namespace safeprofile {
namespace intake {

namespace fs = boost::filesystem;

/// Compilation flags for a single source file
struct compilation_flags {
    std::vector<std::string> include_paths;   // -I flags
    std::vector<std::string> defines;          // -D flags
    std::string std_version;                   // -std=c++XX
    std::string working_directory;             // Directory context for relative paths
};

/// Reads and parses compile_commands.json
/// This enables analyzing real-world projects with correct include paths and flags
class compile_commands_reader {
public:
    /// Try to load compile_commands.json from a directory
    /// Returns true if found and loaded successfully
    bool load_from_directory(const fs::path& directory);

    /// Get compilation flags for a specific source file
    /// Returns nullopt if no entry found for this file
    std::optional<compilation_flags> get_flags_for_file(const fs::path& source_file) const;

    /// Check if compile_commands.json was successfully loaded
    bool is_loaded() const { return loaded_; }

    /// Get number of entries in compilation database
    size_t entry_count() const { return commands_.size(); }

private:
    bool loaded_ = false;
    std::unordered_map<std::string, compilation_flags> commands_;  // key: absolute file path

    /// Parse compiler command string and extract flags
    compilation_flags parse_command(
        const std::string& command,
        const std::string& working_directory
    ) const;

    /// Normalize file path to absolute canonical form for lookup
    std::string normalize_path(const fs::path& path) const;
};

} // namespace intake
} // namespace safeprofile
} // namespace boost

#endif // BOOST_SAFEPROFILE_INTAKE_COMPILE_COMMANDS_HPP
