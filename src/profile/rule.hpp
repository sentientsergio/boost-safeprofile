// Boost.SafeProfile - C++ Safety Profile conformance analysis tool
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_SAFEPROFILE_PROFILE_RULE_HPP
#define BOOST_SAFEPROFILE_PROFILE_RULE_HPP

#include <string>

namespace boost {
namespace safeprofile {
namespace profile {

/// Severity levels for rule violations
enum class severity {
    blocker,  // Must be fixed before release
    major,    // Should be fixed
    minor,    // Nice to fix
    info      // Informational only
};

/// A single profile rule definition
struct rule {
    std::string id;              // e.g., "SP-OWN-001"
    std::string title;           // e.g., "Naked new expression"
    std::string description;     // Detailed explanation
    severity level;              // Violation severity
    std::string pattern;         // Search pattern (simple keyword for Phase 0)
};

} // namespace profile
} // namespace safeprofile
} // namespace boost

#endif // BOOST_SAFEPROFILE_PROFILE_RULE_HPP
