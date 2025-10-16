// Boost.SafeProfile - C++ Safety Profile conformance analysis tool
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_SAFEPROFILE_INTAKE_REPOSITORY_HPP
#define BOOST_SAFEPROFILE_INTAKE_REPOSITORY_HPP

#include <boost/filesystem.hpp>
#include <vector>
#include <string>

namespace boost {
namespace safeprofile {
namespace intake {

namespace fs = boost::filesystem;

/// Represents a discovered source file
struct source_file {
    fs::path path;
    std::string extension;
};

/// Repository ingestion - discovers C++ source files
class repository {
public:
    explicit repository(const fs::path& root);

    /// Scan the repository and discover C++ source files
    std::vector<source_file> discover_sources() const;

    /// Get the repository root path
    const fs::path& root() const { return root_; }

private:
    fs::path root_;

    /// Check if a file has a C++ extension
    static bool is_cpp_source(const fs::path& file);
};

} // namespace intake
} // namespace safeprofile
} // namespace boost

#endif // BOOST_SAFEPROFILE_INTAKE_REPOSITORY_HPP
