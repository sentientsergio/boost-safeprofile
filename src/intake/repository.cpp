// Boost.SafeProfile - C++ Safety Profile conformance analysis tool
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "repository.hpp"
#include <algorithm>
#include <set>

namespace boost {
namespace safeprofile {
namespace intake {

repository::repository(const fs::path& root) : root_(root) {
    if (!fs::exists(root_)) {
        throw std::runtime_error("Path does not exist: " + root_.string());
    }
    if (!fs::is_directory(root_)) {
        throw std::runtime_error("Path is not a directory: " + root_.string());
    }
}

std::vector<source_file> repository::discover_sources() const {
    std::vector<source_file> sources;

    // Recursively walk the directory tree
    for (fs::recursive_directory_iterator it(root_), end; it != end; ++it) {
        if (fs::is_regular_file(it->status())) {
            const fs::path& file_path = it->path();
            if (is_cpp_source(file_path)) {
                source_file src;
                src.path = file_path;
                src.extension = file_path.extension().string();
                sources.push_back(src);
            }
        }
    }

    // Sort for deterministic output
    std::sort(sources.begin(), sources.end(),
              [](const source_file& a, const source_file& b) {
                  return a.path < b.path;
              });

    return sources;
}

bool repository::is_cpp_source(const fs::path& file) {
    static const std::set<std::string> cpp_extensions = {
        ".cpp", ".cxx", ".cc", ".c++",
        ".hpp", ".hxx", ".hh", ".h++", ".h"
    };

    std::string ext = file.extension().string();
    // Convert to lowercase for case-insensitive comparison
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    return cpp_extensions.find(ext) != cpp_extensions.end();
}

} // namespace intake
} // namespace safeprofile
} // namespace boost
