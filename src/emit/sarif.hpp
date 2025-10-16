// Boost.SafeProfile - C++ Safety Profile conformance analysis tool
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_SAFEPROFILE_EMIT_SARIF_HPP
#define BOOST_SAFEPROFILE_EMIT_SARIF_HPP

#include "../analysis/detector.hpp"
#include "../profile/rule.hpp"
#include <boost/filesystem.hpp>
#include <boost/json.hpp>
#include <vector>
#include <string>

namespace boost {
namespace safeprofile {
namespace emit {

namespace fs = boost::filesystem;
namespace json = boost::json;

/// SARIF 2.1.0 emitter
class sarif_emitter {
public:
    sarif_emitter() = default;

    /// Generate SARIF JSON from analysis findings
    /// Returns a Boost.JSON object representing the complete SARIF document
    json::object generate(
        const std::vector<analysis::finding>& findings,
        const std::vector<profile::rule>& rules) const;

    /// Write SARIF JSON to a file
    void write_to_file(
        const json::object& sarif_doc,
        const fs::path& output_path) const;

private:
    /// Convert severity to SARIF level
    std::string severity_to_level(profile::severity sev) const;

    /// Create SARIF tool.driver section
    json::object create_tool_driver(const std::vector<profile::rule>& rules) const;

    /// Create SARIF result from a finding
    json::object create_result(const analysis::finding& f) const;
};

} // namespace emit
} // namespace safeprofile
} // namespace boost

#endif // BOOST_SAFEPROFILE_EMIT_SARIF_HPP
